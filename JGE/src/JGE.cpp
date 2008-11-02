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
#ifdef WIN32

#include "../../Dependencies/include/png.h"
#include "../../Dependencies/include/fmod.h"


JGE::JGE()
{
	mApp = NULL;
	
	strcpy(mDebuggingMsg, "");
	mCurrentMusic = NULL;
	Init();

//	mResourceManager = new JResourceManager();
//	mFileSystem = new JFileSystem();

//	mParticleSystem = NULL;//new JParticleSystem(500);
//	mMotionSystem = NULL;//new JMotionSystem();

}


JGE::~JGE()
{
	JRenderer::Destroy();
	JFileSystem::Destroy();
	JSoundSystem::Destroy();
	//JParticleSystem::Destroy();

	//DestroyGfx();
	//DestroySfx();

//	if (mResourceManager != NULL)
//		delete mResourceManager;
// 
// 	if (mFileSystem != NULL)
// 		delete mFileSystem;
// 
// 	if (mParticleSystem != NULL)
// 		delete mParticleSystem;
// 
// 	if (mMotionSystem != NULL)
// 		delete mMotionSystem;
}


	
void JGE::Init()
{
	mDone = false;
	mPaused = false;
	mCriticalAssert = false;
	
	JRenderer::GetInstance();
	JFileSystem::GetInstance();
	JSoundSystem::GetInstance();
	//JParticleSystem::GetInstance();

	//InitSfx();
}

void JGE::Run()
{
	
}

void JGE::SetDelta(int delta)
{
	mDeltaTime = (float)delta / 1000.0f;		// change to second
}

int JGE::GetTime(void)
{
	return (int)GetTickCount();
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


//////////////////////////////////////////////////////////////////////////
#else		///// PSP specified code



// include all the following so we only have one .o file
//#include "../src/JGfx.cpp"	
//#include "../src/JSfx.cpp"


JGE::JGE()
{
	mApp = NULL;

	Init();

//	mResourceManager = new JResourceManager();
//	mFileSystem = new JFileSystem();
//
//	mParticleSystem = new JParticleSystem(500);
//	mMotionSystem = new JMotionSystem();

}

JGE::~JGE()
{
	JRenderer::Destroy();
	JSoundSystem::Destroy();
	JFileSystem::Destroy();

	//DestroyGfx();
	//DestroySfx();

//	delete mResourceManager;
//	delete mFileSystem;
//	delete mParticleSystem;
//	delete mMotionSystem;

// 	if (mApp != NULL)
// 	{
// 		mApp->Destroy();
// 		delete mApp;
// 		mApp = NULL;
// 	}
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

	//InitSfx();

	//Create();
	
//	mCurrMS = 1.0f;
//	mFPSSlice = 0;

	//struct timeval tp;
	//gettimeofday(&tp, NULL);
	//mTimeBase = tp.tv_sec;

	//mLastTime = GetTime();

	
	mTickFrequency = sceRtcGetTickResolution();
	sceRtcGetCurrentTick(&mLastTime); 
}


// returns number of milliseconds since game started
int JGE::GetTime(void)
{

	u64 curr;
	sceRtcGetCurrentTick(&curr);
 
	return (int)(curr / mTickFrequency);
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
	return (mCtrlPad.Buttons&button)==button && (mOldButtons&button)!=button; 
}


u8 JGE::GetAnalogX() 
{ 
	return mCtrlPad.Lx; 
}


u8 JGE::GetAnalogY() 
{ 
	return mCtrlPad.Ly; 
}


void JGE::Run()
{

	u64 curr;

	while (!mDone)
	{
		if (!mPaused)
		{
			sceRtcGetCurrentTick(&curr);
			
			
			mDelta = (curr-mLastTime) / (float)mTickFrequency;// * 1000.0f; 
			mLastTime = curr;
			
			sceCtrlPeekBufferPositive(&mCtrlPad, 1);	// using sceCtrlPeekBufferPositive is faster than sceCtrlReadBufferPositive
														// because sceCtrlReadBufferPositive waits for vsync internally
		
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

			mOldButtons = mCtrlPad.Buttons;
		


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

//	FILE *f = fopen("jge.log", "a+");
//	fprintf(f, "%s\n", mDebuggingMsg);
//	fclose(f);

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
