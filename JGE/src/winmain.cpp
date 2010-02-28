//========================================================================
//    NeHe OpenGL Wizard : NeHeSimple.cpp
//    Wizard Created by: Vic Hollis
//========================================================================
/*
 *		This Code Was Created By Jeff Molofee 2000
 *		A HUGE Thanks To Fredric Echols For Cleaning Up
 *		And Optimizing This Code, Making It More Flexible!
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 */

int actualWidth;
int actualHeight;

#ifdef WIN32
#include <windows.h>		// Header File For Windows
#else
typedef struct _HDC {} *HDC;
typedef struct _HGLRC {} *HGLRC;
typedef struct _HWND {} *HWND;
typedef struct _HINSTANCE {} *HINSTANCE;
typedef struct _WNDCLASS
{
  int wc;
  int style;
  int* lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  int* hIcon;
  int* hCursor;
  int hbrBackground;
  char* lpszMenuName;
  const char* lpszClassName;
} WNDCLASS;
typedef struct
{
  unsigned short dmSize;
  unsigned int dmFields;
  unsigned int dmBitsPerPel;
  unsigned int dmPelsWidth;
  unsigned int dmPelsHeight;
} DEVMODE;

typedef HINSTANCE HMODULE;
typedef struct _RECT { int left; int right; int top; int bottom; } RECT;
typedef struct _MSG { int wParam; int message; } MSG;
typedef unsigned int LRESULT;
typedef unsigned int UINT;
typedef signed int WPARAM;
typedef unsigned int LPARAM;
typedef int* WNDPROC;
typedef int* HCURSOR;
typedef HCURSOR HICON;
typedef HCURSOR HMENU;
typedef struct _PIXELFORMATDESCRIPTOR {
  int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;int m;int n;int o;int p;int q;int r;int s;int t;int u;int v;int w;int x;int y;int z; } PIXELFORMATDESCRIPTOR;
typedef char* LPSTR;

enum { VK_CONTROL, VK_RETURN, VK_ESCAPE, VK_SPACE, VK_F1, VK_F2, VK_F3 };
enum { MB_OK, MB_ICONINFORMATION, MB_ICONSTOP, MB_YESNO, MB_ICONEXCLAMATION, IDYES };
enum { CS_HREDRAW, CS_VREDRAW, CS_OWNDC };
enum { WS_EX_APPWINDOW, WS_POPUP, WS_EX_WINDOWEDGE, WS_OVERLAPPED, WS_CAPTION, WS_MINIMIZEBOX, WS_MAXIMIZEBOX, WS_SIZEBOX, WS_SYSMENU, WS_CLIPSIBLINGS, WS_CLIPCHILDREN };
enum { WM_ACTIVATE, WM_SYSCOMMAND, WM_CLOSE, WM_KEYDOWN, WM_KEYUP, WM_LBUTTONDOWN, WM_LBUTTONUP, WM_RBUTTONDOWN, WM_RBUTTONUP, WM_MOUSEMOVE, WM_SIZE, WM_QUIT };
enum { DM_BITSPERPEL, DM_PELSWIDTH, DM_PELSHEIGHT };
enum { CDS_FULLSCREEN };
enum { DISP_CHANGE_SUCCESSFUL };
enum { SC_SCREENSAVE, SC_MONITORPOWER };
enum { PFD_DRAW_TO_WINDOW, PFD_SUPPORT_OPENGL, PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, PFD_MAIN_PLANE };
enum { SW_SHOW };
enum { IDC_ARROW };
enum { PM_REMOVE };

static const char* IDI_WINLOGO = "";
#define CALLBACK
#define WINAPI
#define ZeroMemory bzero
int GetTickCount() { return 0; }
long ChangeDisplaySettings(DEVMODE*, unsigned int);
int ShowCursor(bool);
HGLRC wglCreateContext(HDC);
bool wglMakeCurrent(HDC, HGLRC);
bool wglDeleteContext(HGLRC);
int ReleaseDC(HWND, HDC);
int MessageBox(HWND, const char*, const char*, unsigned int);
bool DestroyWindow(HWND);
bool UnregisterClass(const char*, HINSTANCE);
HMODULE GetModuleHandle(char*);
HCURSOR LoadCursor(HINSTANCE, int);
HICON LoadIcon(HINSTANCE, const char*);
int* RegisterClass(const WNDCLASS*);
bool AdjustWindowRectEx(RECT*, unsigned int, bool, unsigned int);
HWND CreateWindowEx(unsigned int, const char*, const char*, unsigned int, int, int, int, int, HWND, HMENU, HINSTANCE, void*);
HDC GetDC(HWND);
int ChoosePixelFormat(HDC, const PIXELFORMATDESCRIPTOR*);
bool SetPixelFormat(HDC, int, PIXELFORMATDESCRIPTOR*);
bool ShowWindow(HWND, int);
bool SetForegroundWindow(HWND);
HWND SetFocus(HWND);
unsigned short HIWORD(unsigned int);
unsigned short LOWORD(unsigned int);
LRESULT DefWindowProc(HWND, unsigned int, WPARAM, LPARAM);
void PostQuitMessage(int);
bool PeekMessage(MSG*, HWND, unsigned int, unsigned int, unsigned int);
bool TranslateMessage(const MSG*);
LRESULT DispatchMessage(const MSG*);
bool SwapBuffers(HDC);
#endif



#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <queue>

#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JTypes.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JFileSystem.h"
#include "../../JGE/include/JRenderer.h"

#include "../../JGE/include/JGameLauncher.h"

//#include "..\src\GameApp.h"

#ifdef WIN32
#pragma comment( lib, "opengl32.lib" )	// Search For OpenGL32.lib While Linking
#pragma comment( lib, "glu32.lib" )		// Search For GLu32.lib While Linking
#pragma comment( lib, "User32.lib" )
#pragma comment( lib, "Gdi32.lib" )
#pragma comment( lib, "Comdlg32.lib" )
#endif

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	active=TRUE;			// Window Active Flag Set To TRUE By Default
bool	fullscreen=FALSE;		// Windowed Mode By Default


DWORD	lastTickCount;

BOOL	g_keys[256];
BOOL	g_holds[256];


//------------------------------------------------------------------------

JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;

//------------------------------------------------------------------------


static u32 gButtons = 0;
static u32 gOldButtons = 0;

static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
  {
    { VK_CONTROL,	JGE_BTN_CTRL },
    { VK_RETURN,	JGE_BTN_MENU },
    { VK_ESCAPE,	JGE_BTN_MENU },
    { VK_UP,		JGE_BTN_UP },
    { VK_RIGHT,		JGE_BTN_RIGHT },
    { VK_DOWN,		JGE_BTN_DOWN },
    { VK_LEFT,		JGE_BTN_LEFT },
    { 'Z',		JGE_BTN_UP },
    { 'D',		JGE_BTN_RIGHT },
    { 'S',		JGE_BTN_DOWN },
    { 'Q',		JGE_BTN_LEFT },
    { 'A',		JGE_BTN_PREV },
    { 'E',		JGE_BTN_NEXT },
    { 'I',		JGE_BTN_CANCEL },
    { 'L',		JGE_BTN_OK },
    { VK_SPACE,		JGE_BTN_OK },
    { 'K',		JGE_BTN_SEC },
    { 'J',		JGE_BTN_PRI },
  };

void JGECreateDefaultBindings()
{
  for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
    g_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);
}

int JGEGetTime()
{
  return (int)GetTickCount();
}

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
  if (height==0)										// Prevent A Divide By Zero By
      height=1;										// Making Height Equal One

  actualWidth = width;
  actualHeight = height;

  glScissor(0, 0, width, height);
  glViewport (0, 0, width, height);	// Reset The Current Viewport
  glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
  glLoadIdentity ();													// Reset The Projection Matrix

  gluOrtho2D(0.0f, (float) width-1.0f, 0.0f, (float) height -1.0f);

  glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
  glLoadIdentity ();													// Reset The Modelview Matrix

  glDisable (GL_DEPTH_TEST);
}

int InitGL(void)												// All Setup For OpenGL Goes Here
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);						// Black Background
  glClearDepth (1.0f);										// Depth Buffer Setup
  glDepthFunc (GL_LEQUAL);									// The Type Of Depth Testing (Less Or Equal)
  glEnable (GL_DEPTH_TEST);									// Enable Depth Testing
  glShadeModel (GL_SMOOTH);									// Select Smooth Shading
  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);						// Set Line Antialiasing
  //glEnable(GL_LINE_SMOOTH);									// Enable it!

  glEnable(GL_CULL_FACE);										// do not calculate inside of poly's
  glFrontFace(GL_CCW);										// counter clock-wise polygons are out

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SCISSOR_TEST);									// Enable Clipping
  //glScissor(20, 20, 320, 240);

  return TRUE;												// Initialization Went OK
}


int InitGame(void)
{
  g_engine = JGE::GetInstance();

  //JGameLauncher *launcher = new JGameLauncher();
  g_app = g_launcher->GetGameApp();
  g_app->Create();
  g_engine->SetApp(g_app);

  JRenderer::GetInstance()->Enable2D();

  lastTickCount = GetTickCount();

  ZeroMemory(g_keys, 256);
  ZeroMemory(g_holds, 256);

  //delete launcher;

  return TRUE;
}


void DestroyGame(void)
{
  //	JParticleSystem::Destroy();
  //	JMotionSystem::Destroy();

  g_engine->SetApp(NULL);
  if (g_app)
    {
      g_app->Destroy();
      delete g_app;
      g_app = NULL;
    }

  JGE::Destroy();

  g_engine = NULL;

}


int DrawGLScene(void)									// Here's Where We Do All The Drawing
{

  // 	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
  // 	glLoadIdentity ();											// Reset The Modelview Matrix

  //if (g_app)
  //	g_app->Render();
  g_engine->Render();

  //	glFlush ();

  return TRUE;										// Everything Went OK
}

void Update(float dt)
{
  g_engine->SetDelta(dt);
  g_engine->Update(dt);
}

void KillGLWindow(void)								// Properly Kill The Window
{
  DestroyGame();

  if (fullscreen)										// Are We In Fullscreen Mode?
    {
      ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
      ShowCursor(TRUE);								// Show Mouse Pointer
    }

  if (hRC)											// Do We Have A Rendering Context?
    {
      if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
        MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
        MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      hRC=NULL;										// Set RC To NULL
    }

  if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
    {
      MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      hDC=NULL;										// Set DC To NULL
    }

  if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
    {
      MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      hWnd=NULL;										// Set hWnd To NULL
    }

  if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
    {
      MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
      hInstance=NULL;									// Set hInstance To NULL
    }
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{

  actualWidth = width;
  actualHeight = height;


  GLuint		PixelFormat;			// Holds The Results After Searching For A Match
  WNDCLASS	wc;						// Windows Class Structure
  DWORD		dwExStyle;				// Window Extended Style
  DWORD		dwStyle;				// Window Style
  RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
  WindowRect.left=(long)0;			// Set Left Value To 0
  WindowRect.right=(long)width;		// Set Right Value To Requested Width
  WindowRect.top=(long)0;				// Set Top Value To 0
  WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

  fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

  hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
  wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
  wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
  wc.cbClsExtra		= 0;									// No Extra Window Data
  wc.cbWndExtra		= 0;									// No Extra Window Data
  wc.hInstance		= hInstance;							// Set The Instance
  wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
  wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
  wc.hbrBackground	= NULL;									// No Background Required For GL
  wc.lpszMenuName		= NULL;									// We Don't Want A Menu
  wc.lpszClassName	= "OpenGL";								// Set The Class Name

  if (!RegisterClass(&wc))									// Attempt To Register The Window Class
    {
      MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;											// Return FALSE
    }

  if (fullscreen)												// Attempt Fullscreen Mode?
    {
      DEVMODE dmScreenSettings;								// Device Mode
      memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
      dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
      dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
      dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
      dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
      dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

      // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
      if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
	{
	  // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
	  if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
	    {
	      fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
	    }
	  else
	    {
	      // Pop Up A Message Box Letting User Know The Program Is Closing.
	      MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
	      return FALSE;									// Return FALSE
	    }
	}
    }

  if (fullscreen)												// Are We Still In Fullscreen Mode?
    {
      dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
      dwStyle=WS_POPUP;										// Windows Style
      ShowCursor(FALSE);										// Hide Mouse Pointer
    }
  else
    {
      dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
      //dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
      dwStyle= WS_OVERLAPPED     | \
	WS_CAPTION        | \
	WS_MINIMIZEBOX	   |
	WS_SIZEBOX	   |
	WS_MAXIMIZEBOX	   |
	//WS_MINIMIZE		|
	WS_SYSMENU;//        |
      //WS_THICKFRAME ;
    }

  AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

  // Create The Window
  if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
				"OpenGL",							// Class Name
				title,								// Window Title
				dwStyle |							// Defined Window Style
				WS_CLIPSIBLINGS |					// Required Window Style
				WS_CLIPCHILDREN,					// Required Window Style
				0, 0,								// Window Position
				WindowRect.right-WindowRect.left,	// Calculate Window Width
				WindowRect.bottom-WindowRect.top,	// Calculate Window Height
				NULL,								// No Parent Window
				NULL,								// No Menu
				hInstance,							// Instance
				NULL)))								// Dont Pass Anything To WM_CREATE
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
    {
      sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
      1,											// Version Number
      PFD_DRAW_TO_WINDOW |						// Format Must Support Window
      PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
      PFD_DOUBLEBUFFER,							// Must Support Double Buffering
      PFD_TYPE_RGBA,								// Request An RGBA Format
      bits,										// Select Our Color Depth
      0, 0, 0, 0, 0, 0,							// Color Bits Ignored
      0,											// No Alpha Buffer
      0,											// Shift Bit Ignored
      0,											// No Accumulation Buffer
      0, 0, 0, 0,									// Accumulation Bits Ignored
      16,											// 16Bit Z-Buffer (Depth Buffer)
      0,											// No Stencil Buffer
      0,											// No Auxiliary Buffer
      PFD_MAIN_PLANE,								// Main Drawing Layer
      0,											// Reserved
      0, 0, 0										// Layer Masks Ignored
    };

  if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  ShowWindow(hWnd,SW_SHOW);						// Show The Window
  SetForegroundWindow(hWnd);						// Slightly Higher Priority
  SetFocus(hWnd);									// Sets Keyboard Focus To The Window
  ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

  if (!InitGL())									// Initialize Our Newly Created GL Window
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  if (!InitGame())								// Initialize Our Game
    {
      KillGLWindow();								// Reset The Display
      MessageBox(NULL,"Game Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      return FALSE;								// Return FALSE
    }

  return TRUE;									// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
				UINT	uMsg,			// Message For This Window
				WPARAM	wParam,			// Additional Message Information
				LPARAM	lParam)			// Additional Message Information
{
  switch (uMsg)									// Check For Windows Messages
    {
    case WM_ACTIVATE:							// Watch For Window Activate Message
      {
	if (!HIWORD(wParam))					// Check Minimization State
	  {
	    active=TRUE;						// Program Is Active
	    if (g_engine != NULL)
	      g_engine->Resume();
	  }
	else
	  {
	    active=FALSE;						// Program Is No Longer Active
	    if (g_engine != NULL)
	      g_engine->Pause();
	  }

	return 0;								// Return To The Message Loop
      }

    case WM_SYSCOMMAND:							// Intercept System Commands
      {
	switch (wParam)							// Check System Calls
	  {
	  case SC_SCREENSAVE:					// Screensaver Trying To Start?
	  case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
	    return 0;							// Prevent From Happening
	  }
	break;									// Exit
      }

    case WM_CLOSE:								// Did We Receive A Close Message?
      {
	PostQuitMessage(0);						// Send A Quit Message
	return 0;								// Jump Back
      }

    case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
      if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
	{
          g_engine->HoldKey_NoRepeat(wParam);
	  return 0;
	}
      break;															// Break

    case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
      if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
	{
          g_engine->ReleaseKey(wParam);
	  return 0;												// Return
	}
      break;

    case WM_LBUTTONDOWN:
      {
	return 0;
      }

    case WM_LBUTTONUP:
      {
	return 0;
      }

    case WM_RBUTTONDOWN:
      {
	return 0;
      }

    case WM_RBUTTONUP:
      {
	return 0;
      }

    case WM_MOUSEMOVE:
      {
	//			Mint2D::SetMousePosition(LOWORD(lParam), HIWORD(lParam));
	return 0;
      }

    case WM_SIZE:								// Resize The OpenGL Window
      {

	ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
	return 0;								// Jump Back
      }
    }

  // Pass All Unhandled Messages To DefWindowProc
  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

bool JGEToggleFullscreen()
{
  return false; // Not implemented under windows
}

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
			HINSTANCE	hPrevInstance,		// Previous Instance
			LPSTR		lpCmdLine,			// Command Line Parameters
			int			nCmdShow)			// Window Show State
{
  MSG		msg;									// Windows Message Structure
  BOOL	done=FALSE;								// Bool Variable To Exit Loop

  DWORD	tickCount;
  int		dt;

  // Ask The User Which Screen Mode They Prefer
  //	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
  //	{
  //		fullscreen=FALSE;							// Windowed Mode
  //	}

  g_launcher = new JGameLauncher();

  u32 flags = g_launcher->GetInitFlags();

  JGECreateDefaultBindings();

  if ((flags&JINIT_FLAG_ENABLE3D)!=0)
    JRenderer::Set3DFlag(true);

  // Create Our OpenGL Window
  if (!CreateGLWindow(g_launcher->GetName(),SCREEN_WIDTH,SCREEN_HEIGHT,32,fullscreen))
    {
      return 0;									// Quit If Window Was Not Created
    }

  while(!done)									// Loop That Runs While done=FALSE
    {
      if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
	{
	  if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
	    {
	      done=TRUE;							// If So done=TRUE
	    }
	  else									// If Not, Deal With Window Messages
	    {
	      TranslateMessage(&msg);				// Translate The Message
	      DispatchMessage(&msg);				// Dispatch The Message
	    }
	}
      else										// If There Are No Messages
	{
	  // Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
	  if (active)								// Program Active?
	    {
	      if (g_engine->IsDone())
                done=TRUE;						// ESC Signalled A Quit
	      else								// Not Time To Quit, Update Screen
		{
		  tickCount = GetTickCount();					// Get The Tick Count
		  dt = (tickCount - lastTickCount);
		  lastTickCount = tickCount;
		  Update((float)dt/1000.0f);									// Update frame

		  //Mint2D::BackupKeys();

		  DrawGLScene();					// Draw The Scene
		  SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
		}
	    }

	  //			if (keys[VK_F1])						// Is F1 Being Pressed?
	  //			{
	  //				keys[VK_F1]=FALSE;					// If So Make Key FALSE
	  //				KillGLWindow();						// Kill Our Current Window
	  //				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
	  //				// Recreate Our OpenGL Window
	  //				if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen))
	  //				{
	  //					return 0;						// Quit If Window Was Not Created
	  //				}
	  //			}
	}
    }

  if (g_launcher)
    delete g_launcher;

  // Shutdown
  KillGLWindow();									// Kill The Window
  return (msg.wParam);							// Exit The Program
}
