//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#include <iostream>
#include <map>
#include <limits>

#include "../include/JGE.h"
#include "../include/JApp.h"
#include "../include/JRenderer.h"
#include "../include/JSoundSystem.h"
#include "../include/Vector2D.h"
#include "../include/JResourceManager.h"
#include "../include/JFileSystem.h"
//#include "../include/JParticleSystem.h"


//////////////////////////////////////////////////////////////////////////
#if defined (WIN32)    // WIN32 specific code
#include "../../Dependencies/include/fmod.h"

u8 JGE::GetAnalogX()
{
  if (GetButtonState(JGE_BTN_LEFT)) return 0;
  if (GetButtonState(JGE_BTN_RIGHT)) return 0xff;
  return 0x80;
}


u8 JGE::GetAnalogY()
{
  if (GetButtonState(JGE_BTN_UP)) return 0;
  if (GetButtonState(JGE_BTN_DOWN)) return 0xff;
  return 0x80;
}

#elif defined (LINUX)    // Unix specific code
#include <sys/time.h>
#include "../Dependencies/include/fmod.h"


u8 JGE::GetAnalogX()
{
  if (GetButtonState(JGE_BTN_LEFT)) return 0;
  if (GetButtonState(JGE_BTN_RIGHT)) return 0xff;
  return 0x80;
}


u8 JGE::GetAnalogY()
{
  if (GetButtonState(JGE_BTN_UP)) return 0;
  if (GetButtonState(JGE_BTN_DOWN)) return 0xff;
  return 0x80;
}

#endif

static map<JButton, float> holds;
static map<JButton, float> oldHolds;
#define REPEAT_DELAY 0.5
#define REPEAT_PERIOD 0.07

void JGE::PressKey(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  if (rng.first == rng.second)
    keyBuffer.push(make_pair(sym, JGE_BTN_NONE));
  else for (keycodes_it it = rng.first; it != rng.second; ++it)
         keyBuffer.push(*it);
}
void JGE::PressKey(const JButton sym)
{
  keyBuffer.push(make_pair(LOCAL_KEY_NONE, sym));
}
void JGE::HoldKey(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  if (rng.first == rng.second)
    keyBuffer.push(make_pair(sym, JGE_BTN_NONE));
  else for (keycodes_it it = rng.first; it != rng.second; ++it)
    {
      keyBuffer.push(*it);
      if (holds.end() == holds.find(it->second))
        holds[it->second] = REPEAT_DELAY;
    }
}
void JGE::HoldKey_NoRepeat(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  if (rng.first == rng.second)
    keyBuffer.push(make_pair(sym, JGE_BTN_NONE));
  else for (keycodes_it it = rng.first; it != rng.second; ++it)
    {
      keyBuffer.push(*it);
      if (holds.end() == holds.find(it->second))
        holds[it->second] = std::numeric_limits<float>::quiet_NaN();
    }
}
void JGE::HoldKey(const JButton sym)
{
  keyBuffer.push(make_pair(LOCAL_KEY_NONE, sym));
  if (holds.end() == holds.find(sym)) holds[sym] = REPEAT_DELAY;
}
void JGE::HoldKey_NoRepeat(const JButton sym)
{
  keyBuffer.push(make_pair(LOCAL_KEY_NONE, sym));
  if (holds.end() == holds.find(sym)) holds[sym] = std::numeric_limits<float>::quiet_NaN();
}
void JGE::ReleaseKey(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  for (keycodes_it it = rng.first; it != rng.second; ++it)
    holds.erase(it->second);
}
void JGE::ReleaseKey(const JButton sym)
{
  holds.erase(sym);
}
void JGE::Update(float dt)
{
  for (map<JButton, float>::iterator it = holds.begin(); it != holds.end(); ++it)
    {
      if (it->second < 0) { keyBuffer.push(make_pair(LOCAL_KEY_NONE, it->first)); it->second = REPEAT_PERIOD; }
      it->second -= dt;
    }
  if (mApp != NULL) mApp->Update();
  oldHolds = holds;
}


bool JGE::GetButtonState(JButton button)
{
  return (holds.end() != holds.find(button));
}


bool JGE::GetButtonClick(JButton button)
{
  return ((holds.end() != holds.find(button)) && (oldHolds.end() == oldHolds.find(button)));
}


JButton JGE::ReadButton()
{
  if (keyBuffer.empty()) return JGE_BTN_NONE;
  JButton val = keyBuffer.front().second;
  keyBuffer.pop();
  return val;
}

LocalKeySym JGE::ReadLocalKey()
{
  if (keyBuffer.empty()) return LOCAL_KEY_NONE;
  LocalKeySym val = keyBuffer.front().first;
  keyBuffer.pop();
  return val;
}

u32 JGE::BindKey(LocalKeySym sym, JButton button)
{
  keyBinds.insert(make_pair(sym, button));
  return keyBinds.size();
}

u32 JGE::UnbindKey(LocalKeySym sym, JButton button)
{
  for (keycodes_it it = keyBinds.begin(); it != keyBinds.end(); )
    if (sym == it->first && button == it->second)
      {
        keycodes_it er = it;
        ++it;
        keyBinds.erase(er);
      }
    else ++it;
  return keyBinds.size();
}

u32 JGE::UnbindKey(LocalKeySym sym)
{
  pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  keyBinds.erase(rng.first, rng.second);
  return keyBinds.size();
}

void JGE::ClearBindings()
{
  keyBinds.clear();
}

void JGE::ResetBindings()
{
  keyBinds.clear();
  JGECreateDefaultBindings();
}


JGE::keybindings_it JGE::KeyBindings_begin() { return keyBinds.begin(); }
JGE::keybindings_it JGE::KeyBindings_end()   { return keyBinds.end(); }

void JGE::ResetInput()
{
  while (!keyBuffer.empty()) keyBuffer.pop();
  holds.clear();
}


JGE::JGE()
{
  mApp = NULL;
#if defined (WIN32) || defined (LINUX)
  strcpy(mDebuggingMsg, "");
  mCurrentMusic = NULL;
#endif
  Init();
}


JGE::~JGE()
{
  JRenderer::Destroy();
  JFileSystem::Destroy();
  JSoundSystem::Destroy();
}



#if defined (WIN32) || defined (LINUX) // Non-PSP code

void JGE::Init()
{
  mDone = false;
  mPaused = false;
  mCriticalAssert = false;
  JRenderer::GetInstance();
  JFileSystem::GetInstance();
  JSoundSystem::GetInstance();
}

void JGE::SetDelta(float delta)
{
  mDeltaTime = delta;
}

float JGE::GetDelta()
{
  return mDeltaTime;
}

float JGE::GetFPS()
{
  return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
#else		///// PSP specific code


void JGE::Init()
{
#ifdef DEBUG_PRINT
  mDebug = true;
#else
  mDebug = false;
#endif

  if (mDebug)
    pspDebugScreenInit();	// do this so that we can use pspDebugScreenPrintf

  strcpy(mDebuggingMsg, "");

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  JRenderer::GetInstance();
  JFileSystem::GetInstance();
  JSoundSystem::GetInstance();

  mDone = false;
  mPaused = false;
  mCriticalAssert = false;
}

float JGE::GetDelta()
{
  return mDelta;
}


float JGE::GetFPS()
{
  return 1.0f / mDelta;
}

u8 JGE::GetAnalogX()
{
  return JGEGetAnalogX();
}

u8 JGE::GetAnalogY()
{
  return JGEGetAnalogY();
}

static SceCtrlData gCtrlPad;

void JGE::Run()
{
  static const int keyCodeList[] = {
    PSP_CTRL_SELECT, // Select button.
    PSP_CTRL_START, // Start button.
    PSP_CTRL_UP, // Up D-Pad button.
    PSP_CTRL_RIGHT, // Right D-Pad button.
    PSP_CTRL_DOWN, // Down D-Pad button.
    PSP_CTRL_LEFT, // Left D-Pad button.
    PSP_CTRL_LTRIGGER, // Left trigger.
    PSP_CTRL_RTRIGGER, // Right trigger.
    PSP_CTRL_TRIANGLE, // Triangle button.
    PSP_CTRL_CIRCLE, // Circle button.
    PSP_CTRL_CROSS, // Cross button.
    PSP_CTRL_SQUARE, // Square button.
    PSP_CTRL_HOLD, // Hold button.
  };
  u64 curr;
  long long int nextInput = 0;
  u64 lastTime;
  u32 oldButtons;
  u32 veryOldButtons;
  u32 gTickFrequency = sceRtcGetTickResolution();

  sceRtcGetCurrentTick(&lastTime);
  oldButtons = veryOldButtons = 0;
  JGECreateDefaultBindings();
  while (!mDone)
    {
      if (!mPaused)
       {
         sceRtcGetCurrentTick(&curr);
          float dt = (curr - lastTime) / (float)gTickFrequency;
         mDelta = dt;
         sceCtrlPeekBufferPositive(&gCtrlPad, 1);
         for (signed int i = sizeof(keyCodeList)/sizeof(keyCodeList[0]) - 1; i >= 0; --i)
           {
             if (keyCodeList[i] & gCtrlPad.Buttons)
                {
                  if (!(keyCodeList[i] & oldButtons))
                    HoldKey(keyCodeList[i]);
                }
              else
                if (keyCodeList[i] & oldButtons)
                  ReleaseKey(keyCodeList[i]);
           }
         oldButtons = gCtrlPad.Buttons;
         Update(dt);
         Render();
         if (mDebug)
           {
             if (strlen(mDebuggingMsg)>0)
               {
                 pspDebugScreenSetXY(0, 0);
                 pspDebugScreenPrintf(mDebuggingMsg);
               }
            }
          veryOldButtons = gCtrlPad.Buttons;
        }
      else
        sceKernelDelayThread(1);
      lastTime = curr;
    }
}

#endif		///// PSP specific code


//////////////////////////////////////////////////////////////////////////
JGE* JGE::mInstance = NULL;

// returns number of milliseconds since game started
int JGE::GetTime()
{
  return JGEGetTime();
}


JGE* JGE::GetInstance()
{
  if (mInstance == NULL) mInstance = new JGE();
  return mInstance;
}


void JGE::Destroy()
{
  if (mInstance)
    {
      delete mInstance;
      mInstance = NULL;
    }
}


void JGE::SetApp(JApp *app)
{
  mApp = app;
}

void JGE::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();

  renderer->BeginScene();
  if (mApp != NULL) mApp->Render();
  renderer->EndScene();
}

void JGE::End()
{
  mDone = true;
}

void JGE::printf(const char *format, ...)
{
  va_list list;

  va_start(list, format);
  vsprintf(mDebuggingMsg, format, list);
  va_end(list);

}

void JGE::Pause()
{
  if (mPaused) return;

  mPaused = true;
  if (mApp != NULL) mApp->Pause();
}


void JGE::Resume()
{
  if (mPaused)
    {
      mPaused = false;
      if (mApp != NULL)
	mApp->Resume();
    }
}

void JGE::Assert(const char *filename, long lineNumber)
{
  mAssertFile = filename;
  mAssertLine = lineNumber;
  mCriticalAssert = true;
}

std::queue< pair<LocalKeySym, JButton> > JGE::keyBuffer;
std::multimap<LocalKeySym, JButton> JGE::keyBinds;
