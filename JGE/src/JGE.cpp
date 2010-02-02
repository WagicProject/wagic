//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

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

int JGE::GetTime(void)
{
  return (int)GetTickCount();
}

u8 JGE::GetAnalogX()
{
  if (JGEGetKeyState(VK_LEFT)) return 0;
  if (JGEGetKeyState(VK_RIGHT)) return 0xff;

  return 0x80;
}


u8 JGE::GetAnalogY()
{
  if (JGEGetKeyState(VK_UP)) return 0;
  if (JGEGetKeyState(VK_DOWN)) return 0xff;

  return 0x80;
}

#elif defined (LINUX)    // Unix specific code
#include <sys/time.h>
#include "png.h"
#include "../Dependencies/include/fmod.h"

int JGE::GetTime(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

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



#if defined (WIN32) || defined (LINUX) // Non-PSP code

JGE::JGE()
{
  mApp = NULL;

  strcpy(mDebuggingMsg, "");
  mCurrentMusic = NULL;
  Init();

}


JGE::~JGE()
{
  JRenderer::Destroy();
  JFileSystem::Destroy();
  JSoundSystem::Destroy();
}



void JGE::Init()
{
  mDone = false;
  mPaused = false;
  mCriticalAssert = false;

  JRenderer::GetInstance();
  JFileSystem::GetInstance();
  JSoundSystem::GetInstance();
}

void JGE::Run()
{

}

void JGE::SetDelta(int delta)
{
  mDeltaTime = (float)delta / 1000.0f;		// change to second
}

float JGE::GetDelta()
{
  return mDeltaTime;
  //return hge->Timer_GetDelta()*1000;
}


float JGE::GetFPS()
{
  //return (float)hge->Timer_GetFPS();
  return 0.0f;
}


bool JGE::GetButtonState(u32 button)
{
  //return (gButtons&button)==button;
  return JGEGetButtonState(button);
}


bool JGE::GetButtonClick(u32 button)
{
  //return (gButtons&button)==button && (gOldButtons&button)!=button;
  return JGEGetButtonClick(button);
}

u32 JGE::ReadButton()
{
  return JGEReadKey();
}

void JGE::ResetInput()
{
  JGEResetInput();
}

//////////////////////////////////////////////////////////////////////////
#else		///// PSP specified code



#include <queue>
static queue<u32> gKeyBuffer;
static const int gKeyCodeList[] = {
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
  /* Do not test keys we cannot get anyway, that's just wasted proc time
  PSP_CTRL_HOME, // Home button.
  PSP_CTRL_NOTE, // Music Note button.
  PSP_CTRL_SCREEN, // Screen button.
  PSP_CTRL_VOLUP, // Volume up button.
  PSP_CTRL_VOLDOWN, // Volume down button.
  PSP_CTRL_WLAN, // _UP    Wlan switch up.
  PSP_CTRL_REMOTE, // Remote hold position.
  PSP_CTRL_DISC, // Disc present.
  PSP_CTRL_MS // Memory stick present.
  */
};
static u32 gHolds = 0;


JGE::JGE()
{
  mApp = NULL;

  Init();


}

JGE::~JGE()
{
  JRenderer::Destroy();
  JSoundSystem::Destroy();
  JFileSystem::Destroy();

}


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
  mOldButtons = mVeryOldButtons = 0;

  mTickFrequency = sceRtcGetTickResolution();
  sceRtcGetCurrentTick(&mLastTime);
}


// returns number of milliseconds since game started
int JGE::GetTime(void)
{

  u64 curr;
  sceRtcGetCurrentTick(&curr);
  return (int)((curr * 1000) / mTickFrequency);
}


float JGE::GetDelta()
{
  return mDelta;
}


float JGE::GetFPS()
{
  return 1.0f / mDelta;
}


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

u8 JGE::GetAnalogX()
{
  return mCtrlPad.Lx;
}


u8 JGE::GetAnalogY()
{
  return mCtrlPad.Ly;
}

#define REPEAT_DELAY 0.5
#define REPEAT_FREQUENCY 7
void JGE::Run()
{
  u64 curr;
  long long int nextInput = 0;

  const u32 ticksPerSecond = sceRtcGetTickResolution();
  const u64 repeatDelay = REPEAT_DELAY * ticksPerSecond;
  const u64 repeatPeriod = ticksPerSecond / REPEAT_FREQUENCY;

  while (!mDone)
    {
      if (!mPaused)
	{
	  sceRtcGetCurrentTick(&curr);

	  mDelta = (curr-mLastTime) / (float)mTickFrequency;// * 1000.0f;

	  sceCtrlPeekBufferPositive(&mCtrlPad, 1);
	  for (signed int i = sizeof(gKeyCodeList)/sizeof(gKeyCodeList[0]) - 1; i >= 0; --i)
	    {
	      if (gKeyCodeList[i] & mCtrlPad.Buttons)
		{
		  if (!(gKeyCodeList[i] & mOldButtons))
		    {
		      if (!(gHolds & gKeyCodeList[i])) gKeyBuffer.push(gKeyCodeList[i]);
		      nextInput = repeatDelay;
		      gHolds |= gKeyCodeList[i];
		    }
		  else if (nextInput < 0)
		    {
		      if (!(gHolds & gKeyCodeList[i])) gKeyBuffer.push(gKeyCodeList[i]);
		      nextInput = repeatPeriod;
		      gHolds |= gKeyCodeList[i];
		    }
		}
	      if (!(gKeyCodeList[i] & mCtrlPad.Buttons))
		if (gKeyCodeList[i] & mOldButtons)
		  gHolds &= ~gKeyCodeList[i];
	    }
	  mOldButtons = mCtrlPad.Buttons;

	  nextInput -= (curr - mLastTime);
	  mLastTime = curr;

	  Update();
	  Render();

	  if (mDebug)
	    {
	      if (strlen(mDebuggingMsg)>0)
		{
		  pspDebugScreenSetXY(0, 0);
		  pspDebugScreenPrintf(mDebuggingMsg);
		}
	  }
     mVeryOldButtons = mCtrlPad.Buttons;
	  }
      else
	  {
     sceKernelDelayThread(1);
	  }
  }
}



#endif		///// PSP specified code


//////////////////////////////////////////////////////////////////////////
JGE* JGE::mInstance = NULL;
//static int gCount = 0;

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


void JGE::Update()
{
  if (mApp != NULL)
    mApp->Update();
}

void JGE::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();

  renderer->BeginScene();

  if (mApp != NULL)
    mApp->Render();

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
