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
#include <queue>
#include <map>
#include <vector>
#include <string>

#include "JTypes.h"

#define DEBUG_PRINT

#if defined(QT_CONFIG)
#include <Qt>
typedef u32 LocalKeySym;
#define LOCAL_KEY_NONE Qt::Key_unknown

#elif defined(SDL_CONFIG)
#include <SDL.h>
typedef SDLKey LocalKeySym;
#define LOCAL_KEY_NONE SDLK_UNKNOWN

#elif defined(WIN32)
#include <windows.h>
typedef WPARAM LocalKeySym;
#define LOCAL_KEY_NONE ((WPARAM)-1)

#elif defined(LINUX)
#include <X11/XKBlib.h>
#include <X11/keysym.h>
typedef KeySym LocalKeySym;
#define LOCAL_KEY_NONE XK_VoidSymbol

#elif defined(ANDROID) // This is temporary until we understand how to send real key events from Java
typedef long unsigned int LocalKeySym;
#define LOCAL_KEY_NONE 0

#else
typedef u32 LocalKeySym;
#define LOCAL_KEY_NONE ((u32)-1)

#endif

#if defined(ANDROID)
#include <jni.h>
#endif

bool JGEGetButtonState(const JButton button);
bool JGEGetButtonClick(const JButton button);
void JGECreateDefaultBindings();
int JGEGetTime();
u8 JGEGetAnalogX();
u8 JGEGetAnalogY();
bool JGEToggleFullscreen();

#if defined (PSP)

// hack to fix a typedef definition of u32 inside of newlib's stdint.h
// this used to be defined as an unsigned long, but as of minpspw 11.1, it's 
// now an unsigned int:

// from stdint.h 
// /* Check if "long" is 64bit or 32bit wide */
// #if __STDINT_EXP(LONG_MAX) > 0x7fffffff
// #define __have_long64 1
// #elif __STDINT_EXP(LONG_MAX) == 0x7fffffff && !defined(__SPU__) && !defined(__psp__)
// #define __have_long32 1
// #endif

// Note the dependency on the definition of __psp__, which isn't defined in JGE.  This probably
// usually comes in via a makefile, but I couldn't unravel what setting it comes from...
#if !defined(__psp__)
#define __psp__ 1
#endif

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

#if defined (WIN32) || defined (LINUX)
  JMusic *mCurrentMusic;
#else
 public:
  void Run();
 private:
#endif

#if defined (ANDROID)
    JNIEnv * mJNIEnv;
    jclass mJNIClass;
    jmethodID midSendCommand;
#endif

  bool mDone;
  float mDeltaTime;
  bool mDebug;
  bool mPaused;
  char mDebuggingMsg[256];
  bool mCriticalAssert;
  const char *mAssertFile;
  int mAssertLine;
  std::vector<std::string> mArgv;

  static JGE* mInstance;


  static std::queue< std::pair<std::pair<LocalKeySym, JButton>, bool> > keyBuffer;
  static std::multimap<LocalKeySym, JButton> keyBinds;
  typedef std::multimap<LocalKeySym, JButton>::iterator keycodes_it;

  // Mouse attributes
  int mLastLeftClickX;
  int mlastLeftClickY;

  friend void Run();

 public:

  //////////////////////////////////////////////////////////////////////////
  /// Get JGE instance.
  ///
  /// @return JGE instance.
  //////////////////////////////////////////////////////////////////////////
  static JGE* GetInstance();
  static void Destroy();

  void Init();
  void End();

  void Update(float);
  void Render();

  void Pause();
  void Resume();

  //////////////////////////////////////////////////////////////////////////
  /// Return argv.
  ///
  /// @return argv vector.
  //////////////////////////////////////////////////////////////////////////
  std::vector<std::string> GetARGV();
  
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

  // override the current delta time.
  void SetDelta(float delta);

  //////////////////////////////////////////////////////////////////////////
  /// Return frame rate.
  ///
  /// @note This is just 1.0f/GetDelta().
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
  bool GetButtonState(JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Check if a button is down the first time.
  /// THIS DOES NOT WORK RELIABLY. DO NOT USE THIS.
  /// USE ReadButton() INSTEAD.
  ///
  /// @param button - Button id.
  ///
  /// @return Button state.
  //////////////////////////////////////////////////////////////////////////
  bool GetButtonClick(JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Get the next keypress.
  ///
  /// @return Next pressed button, or 0 if none.
  //////////////////////////////////////////////////////////////////////////
  JButton ReadButton();
  LocalKeySym ReadLocalKey();

  //////////////////////////////////////////////////////////////////////////
  /// Bind an actual key to a symbolic button. A key can be bound to
  /// several buttons and even several times to the same button (for
  /// double clicks on one button, for example.
  ///
  /// @param keycode - The local code of the key
  /// @param button - The button to bind it to
  ///
  /// @return The number of bound keys so far
  //////////////////////////////////////////////////////////////////////////
  u32 BindKey(LocalKeySym keycode, JButton button);

  //////////////////////////////////////////////////////////////////////////
  /// Undo a binding.
  /// If the second parameter is omitted, remove all bindings to this key ;
  /// else, remove exactly once the binding from this key to this button.
  ///
  /// @param keycode - The local code of the key
  /// @param button - The button
  ///
  /// @return The number of still bound keys
  //////////////////////////////////////////////////////////////////////////
  u32 UnbindKey(LocalKeySym keycode, JButton button);
  u32 UnbindKey(LocalKeySym keycode);

  //////////////////////////////////////////////////////////////////////////
  /// Clear bindings.
  /// This removes ALL bindings. Take care to re-bind keys after doing it.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ClearBindings();

  //////////////////////////////////////////////////////////////////////////
  /// Reset bindings.
  /// This resets ALL bindings to their default value.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ResetBindings();

  //////////////////////////////////////////////////////////////////////////
  /// Iterators for bindings.
  //////////////////////////////////////////////////////////////////////////
  typedef std::multimap<LocalKeySym, JButton>::const_iterator keybindings_it;
  keybindings_it KeyBindings_begin();
  keybindings_it KeyBindings_end();


  //////////////////////////////////////////////////////////////////////////
  /// Reset the input buffer.
  /// This is necessary because there might be phases when only
  /// GetButtonState is used, thereby accumulating keypresses
  /// in the key buffer.
  ///
  //////////////////////////////////////////////////////////////////////////
  void ResetInput();

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
  /// Simulate a keypress, or a keyhold/release.
  ///
  //////////////////////////////////////////////////////////////////////////
  void PressKey(const LocalKeySym);
  void PressKey(const JButton);
  void HoldKey(const LocalKeySym);
  void HoldKey(const JButton);
  void HoldKey_NoRepeat(const LocalKeySym);
  void HoldKey_NoRepeat(const JButton);
  void ReleaseKey(const LocalKeySym);
  void ReleaseKey(const JButton);

  //////////////////////////////////////////////////////////////////////////
  /// Mouse events
  ///  x and y are int coordinates relative to SCREEN_WIDTH and SCREEN_HEIGHT
  //////////////////////////////////////////////////////////////////////////
  void LeftClicked(int x, int y);

  void LeftClickedProcessed();

  // Getter, may have to move that in the JGuiListener
  // Returns false if nothing has been clicked, true otherwise
  bool GetLeftClickCoordinates(int& x, int& y);


  // Scroll events - currently triggered by SDL JOYBALL events
  void Scroll(int inXVelocity, int inYVelocity);

  //////////////////////////////////////////////////////////////////////////
  /// Get if the system is ended/paused or not.
  ///
  /// @return Status of the system.
  //////////////////////////////////////////////////////////////////////////
  bool IsDone() { return mDone; }
  bool IsPaused() { return mPaused; }


  //////////////////////////////////////////////////////////////////////////
  /// Set the user's core application class.
  ///
  /// @param app - User defined application class.
  //////////////////////////////////////////////////////////////////////////
  void SetApp(JApp *app);

  
  //////////////////////////////////////////////////////////////////////////
  /// Setsn argv.
  ///
  /// 
  //////////////////////////////////////////////////////////////////////////
  void SetARGV(int argc, char * argv[]);  

  //////////////////////////////////////////////////////////////////////////
  /// Print debug message.
  ///
  //////////////////////////////////////////////////////////////////////////
  void printf(const char *format, ...);


  void Assert(const char *filename, long lineNumber);

  /// Sends a message through JGE
  /// Currently used only to communicate with the JNI Layer in Android
  void SendCommand(std::string command);
  void SendCommand(std::string command, std::string parameter);
  
 #if defined (ANDROID)
   /// Access to JNI Environment
   void SetJNIEnv(JNIEnv * env, jclass cls);
   void sendJNICommand(std::string command);
#endif 
  
 protected:
  JGE();
  ~JGE();


};


#endif
