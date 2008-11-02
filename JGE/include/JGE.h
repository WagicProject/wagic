//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------

#ifndef _JGE_H_
#define _JGE_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "JTypes.h"

#define DEBUG_PRINT

//#define _MP3_ENABLED_

#ifdef WIN32

	#include <windows.h>

	void JGEControl();
	BOOL JGEGetKeyState(int key);
	bool JGEGetButtonState(u32 button);
	bool JGEGetButtonClick(u32 button);

#else

	#include <pspgu.h>
	#include <pspkernel.h>
	#include <pspdisplay.h>
	#include <pspdebug.h> 
	#include <pspctrl.h>
	#include <time.h>
	#include <string.h>
	#include <pspaudiolib.h>
	#include <psprtc.h>

#endif

//#include "JGEInit.h"

//#include "JTypes.h"

#include "Vector2D.h"

class JApp;
class JResourceManager;
class JFileSystem;
class JParticleSystem;
class JMotionSystem;
class JMusic;

//////////////////////////////////////////////////////////////////////////
/// Game engine main interface.
//////////////////////////////////////////////////////////////////////////
class JGE
{
private:
	JApp *mApp;
//	JResourceManager *mResourceManager;
//	JFileSystem* mFileSystem;

//	JParticleSystem* mParticleSystem;
//	JMotionSystem* mMotionSystem;


#ifdef WIN32
	float mDeltaTime;

	JMusic *mCurrentMusic;
	
#else
	SceCtrlData mCtrlPad;
	u32 mOldButtons;

	u64 mLastTime;
	u32 mTickFrequency;

#endif

	bool mDone;

	float mDelta;

	bool mDebug;
	
	bool mPaused;

	char mDebuggingMsg[256];

	bool mCriticalAssert;
	const char *mAssertFile;
	int mAssertLine;


	static JGE* mInstance;
	

public:

	//////////////////////////////////////////////////////////////////////////
	/// Get JGE instance.
	///
	/// @return JGE instance.
	//////////////////////////////////////////////////////////////////////////
	static JGE* GetInstance();
	static void Destroy();

	void Init();
	void Run();
	void End();

	void Update();
	void Render();

	void Pause();
	void Resume();

	//////////////////////////////////////////////////////////////////////////
	/// Return system timer in milliseconds.
	///
	/// @return System time in milliseconds.
	//////////////////////////////////////////////////////////////////////////
	int GetTime(void);

	//////////////////////////////////////////////////////////////////////////
	/// Return elapsed time since last frame update.
	///
	/// @return Elapsed time in seconds.
	//////////////////////////////////////////////////////////////////////////
	float GetDelta();

	//////////////////////////////////////////////////////////////////////////
	/// Return frame rate.
	///
	/// @note This is just 1.0f/GetDelat().
	///
	/// @return Number of frames per second.
	//////////////////////////////////////////////////////////////////////////
	float GetFPS();

	//////////////////////////////////////////////////////////////////////////
	/// Check the current state of a button.
	///
	/// @param button - Button id.
	///
	/// @return Button state.
	//////////////////////////////////////////////////////////////////////////
	bool GetButtonState(u32 button);

	//////////////////////////////////////////////////////////////////////////
	/// Check if a button is down the first time.
	///
	/// @param button - Button id.
	///
	/// @return Button state.
	//////////////////////////////////////////////////////////////////////////
	bool GetButtonClick(u32 button);

	//////////////////////////////////////////////////////////////////////////
	/// Get x value of the analog pad.
	///
	/// @return X value (0 to 255).
	//////////////////////////////////////////////////////////////////////////
	u8 GetAnalogX();

	//////////////////////////////////////////////////////////////////////////
	/// Get y value of the analog pad.
	///
	/// @return Y value (0 to 255).
	//////////////////////////////////////////////////////////////////////////
	u8 GetAnalogY();


	//////////////////////////////////////////////////////////////////////////
	/// Get if the system is ended or not.
	///
	/// @return Status of the system.
	//////////////////////////////////////////////////////////////////////////
	bool IsDone() { return mDone; }
	

	//////////////////////////////////////////////////////////////////////////
	/// Set the user's core application class.
	///
	/// @param app - User defined application class.
	//////////////////////////////////////////////////////////////////////////
	void SetApp(JApp *app);
	

	//////////////////////////////////////////////////////////////////////////
	/// Print debug message.
	///
	//////////////////////////////////////////////////////////////////////////
	void printf(const char *format, ...);


	void Assert(const char *filename, long lineNumber);

#ifdef WIN32
	void SetDelta(int delta);
#endif

protected:
	JGE();
	~JGE();


};


#endif
