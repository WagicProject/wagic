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
#include "../../Dependencies/include/png.h"
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
#include "png.h"
#include "../Dependencies/include/fmod.h"


u8 JGE::GetAnalogX()
{
  /* FIXME
     if (JGEGetKeyState(VK_LEFT)) return 0;
     if (JGEGetKeyState(VK_RIGHT)) return 0xff;
  */
  return 0x80;
}


u8 JGE::GetAnalogY()
{
  /* FIXME
     if (JGEGetKeyState(VK_UP)) return 0;
     if (JGEGetKeyState(VK_DOWN)) return 0xff;
  */

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
  for (keycodes_it it = rng.first; it != rng.second; ++it)
    keyBuffer.push(it->second);
}
void JGE::PressKey(const JButton sym)
{
  keyBuffer.push(sym);
}
void JGE::HoldKey(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  for (keycodes_it it = rng.first; it != rng.second; ++it)
    {
      keyBuffer.push(it->second);
      if (holds.end() == holds.find(it->second))
        holds[it->second] = REPEAT_DELAY;
    }
}
void JGE::HoldKey_NoRepeat(const LocalKeySym sym)
{
  const pair<keycodes_it, keycodes_it> rng = keyBinds.equal_range(sym);
  for (keycodes_it it = rng.first; it != rng.second; ++it)
    {
      keyBuffer.push(it->second);
      if (holds.end() == holds.find(it->second))
        holds[it->second] = std::numeric_limits<float>::quiet_NaN();
    }
}
void JGE::HoldKey(const JButton sym)
{
  keyBuffer.push(sym);
  if (holds.end() == holds.find(sym)) holds[sym] = REPEAT_DELAY;
}
void JGE::HoldKey_NoRepeat(const JButton sym)
{
  keyBuffer.push(sym);
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
      if (it->second < 0) { keyBuffer.push(it->first); it->second = REPEAT_PERIOD; }
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
  cout << "HOLDS : ";
  for (map<JButton, float>::iterator it = holds.begin(); it != holds.end(); ++it)
    cout << it->first << " ";
  cout << endl << "OLDHOLDS : ";
  for (map<JButton, float>::iterator it = oldHolds.begin(); it != oldHolds.end(); ++it)
    cout << it->first << " ";
  cout << endl;
  return ((holds.end() != holds.find(button)) && (oldHolds.end() == oldHolds.find(button)));
}


JButton JGE::ReadButton()
{
  if (keyBuffer.empty()) return JGE_BTN_NONE;
  JButton val = keyBuffer.front();
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
  mDeltaTime = (float)delta;
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

#if defined (WIN32) || defined (LINUX)
  tickFrequency = 120;
#else
  tickFrequency = sceRtcGetTickResolution();
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



/*
bool JGE::GetButtonState(u32 button)
{
  return (mCtrlPad.Buttons&button)==button;

}


bool JGE::GetButtonClick(u32 button)
{
  return (mCtrlPad.Buttons&button)==button && (mVeryOldButtons&button)!=button;
}

u32 JGE::ReadButton()
{
  if (gKeyBuffer.empty()) return 0;
  u32 val = gKeyBuffer.front();
  gHolds &= ~val;
  gKeyBuffer.pop();
  return val;
}

void JGE::ResetInput()
{
  while (!gKeyBuffer.empty()) gKeyBuffer.pop();
}
*/


#endif		///// PSP specific code


//////////////////////////////////////////////////////////////////////////
JGE* JGE::mInstance = NULL;
//static int gCount = 0;

// returns number of milliseconds since game started
int JGE::GetTime()
{
  return JGEGetTime();
}


JGE* JGE::GetInstance()
{
  if (mInstance == NULL)
    {
      mInstance = new JGE();
    }

  //gCount++;
  return mInstance;
}


void JGE::Destroy()
{
  //gCount--;
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
  if (mApp != NULL)
    mApp->Pause();
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

std::queue<JButton> JGE::keyBuffer;
std::multimap<LocalKeySym, JButton> JGE::keyBinds;
