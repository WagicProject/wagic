#include <unistd.h>
#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <GL/glx.h>
#include <X11/XKBlib.h>
#include <sys/time.h>
#include <queue>
#include <iostream>

#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JTypes.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JFileSystem.h"
#include "../../JGE/include/JRenderer.h"

#include "../../JGE/include/JGameLauncher.h"

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((GLfloat)ACTUAL_SCREEN_WIDTH / (GLfloat)ACTUAL_SCREEN_HEIGHT)
struct window_state_t
{
  bool	fullscreen;
  int width;
  int height;
  int x;
  int y;
} window_state = { false, ACTUAL_SCREEN_WIDTH, ACTUAL_SCREEN_HEIGHT, 0, 0 };

enum
 {
 _NET_WM_STATE_REMOVE =0,
 _NET_WM_STATE_ADD = 1,
 _NET_WM_STATE_TOGGLE =2
 };


uint64_t	lastTickCount;


//------------------------------------------------------------------------

JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;

//------------------------------------------------------------------------

Display* gXDisplay = NULL;
Window gXWindow = NULL;
GLXWindow glxWin = NULL;

static queue< pair<KeySym, u32> > gKeyBuffer;
static u32 gControllerState = 0;
static u32 gPrevControllerState = 0;
static u32 gHolds = 0;

static const struct { KeySym keysym; u32 pspCode; } gDefaultBindings[] =
  {
    { XK_Escape,	PSP_CTRL_START },
    { XK_Return,	PSP_CTRL_START },
    { XK_BackSpace,	PSP_CTRL_SELECT },
    { XK_Up,		PSP_CTRL_UP },
    { XK_KP_Up,		PSP_CTRL_UP },
    { XK_Down,		PSP_CTRL_DOWN },
    { XK_KP_Down,	PSP_CTRL_DOWN },
    { XK_Left,		PSP_CTRL_LEFT },
    { XK_KP_Left,	PSP_CTRL_LEFT },
    { XK_Right,		PSP_CTRL_RIGHT },
    { XK_KP_Right,	PSP_CTRL_RIGHT },
    { XK_space,		PSP_CTRL_CIRCLE },
    { XK_Control_L,	PSP_CTRL_CIRCLE },
    { XK_Control_R,	PSP_CTRL_CIRCLE },
    { XK_Tab,		PSP_CTRL_TRIANGLE },
    { XK_Alt_L,		PSP_CTRL_SQUARE },
    { XK_Caps_Lock,	PSP_CTRL_CROSS },
    { XK_Shift_L,	PSP_CTRL_LTRIGGER },
    { XK_Shift_R,	PSP_CTRL_RTRIGGER },
    { XK_F1,		PSP_CTRL_HOME },
    { XK_F2,		PSP_CTRL_HOLD },
    { XK_F3,		PSP_CTRL_NOTE }
  };

static vector< pair<KeySym, u32> > gKeyCodes;

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)	// Resize The GL Window
{

  if ((GLfloat)width / (GLfloat)height < ACTUAL_RATIO)
    glViewport(0, -((width/ACTUAL_RATIO)-height)/2, width, width / ACTUAL_RATIO);			// Reset The Current Viewport
  else
    glViewport(-(height*ACTUAL_RATIO-width)/2, 0, height * ACTUAL_RATIO, height);
  glScissor(0, 0, width, height);
}

GLvoid SizeGLScene(GLsizei width, GLsizei height)	// Initialize The GL Window
{
  if (0 == height)					// Prevent A Divide By Zero By
    height=1;						// Making Height Equal One

  glViewport(0, 0, width, height);			// Reset The Current Viewport

  glMatrixMode(GL_PROJECTION);				// Select The Projection Matrix
  glLoadIdentity();					// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective(75.0f, ACTUAL_RATIO, 0.5f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);				// Select The Modelview Matrix
  glLoadIdentity();					// Reset The Modelview Matrix

  //  glutReshapeWindow(width, height);
  ReSizeGLScene(width, height);
}

int InitGL(void)					// All Setup For OpenGL Goes Here
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// Black Background (yes that's the way fuckers)
  glClearDepth(1.0f);					// Depth Buffer Setup
  glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing (Less Or Equal)
  glEnable(GL_DEPTH_TEST);				// Enable Depth Testing
  glShadeModel(GL_SMOOTH);				// Select Smooth Shading
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Set Perspective Calculations To Most Accurate

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// Set Line Antialiasing
  glEnable(GL_LINE_SMOOTH);				// Enable it!

  glEnable(GL_CULL_FACE);				// do not calculate inside of poly's
  glFrontFace(GL_CCW);					// counter clock-wise polygons are out

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SCISSOR_TEST);				// Enable Clipping

  return true;						// Initialization Went OK
}


bool InitGame(void)
{
  g_engine = JGE::GetInstance();
  g_app = g_launcher->GetGameApp();
  g_app->Create();
  g_engine->SetApp(g_app);

  JRenderer::GetInstance()->Enable2D();
  struct timeval tv;
  gettimeofday(&tv, NULL);
  lastTickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  return true;
}



void DestroyGame(void)
{
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

void KillGLWindow(void) // Properly Kill The Window
{
  if (gXWindow && gXDisplay)
    XDestroyWindow(gXDisplay, gXWindow);
  gXWindow = NULL;
}


// Some parameters we wanna give to XGl
static const int doubleBufferAttributes[] = {
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE,   GLX_RGBA_BIT,
    GLX_DOUBLEBUFFER,  True,  /* Request a double-buffered color buffer with */
    GLX_RED_SIZE,      1,     /* the maximum number of bits per component    */
    GLX_GREEN_SIZE,    1,
    GLX_BLUE_SIZE,     1,
    None
};
static Bool WaitForNotify(Display *dpy, XEvent *event, XPointer arg)
{
  return (event->type == MapNotify) && (event->xmap.window == (Window) arg);
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (true) Or Windowed Mode (false)	*/
BOOL CreateGLWindow(char* title, int width, int height, int bits __attribute__((unused)), bool fullscreenflag __attribute__((unused)))
{
  // Open a connection to the X server
  gXDisplay = XOpenDisplay(NULL);
  if (NULL == gXDisplay)
    {
      printf("Unable to open a connection to the X server\n");
      return false;
    }

  for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
    gKeyCodes.push_back(make_pair(gDefaultBindings[i].keysym, gDefaultBindings[i].pspCode));

  // Get a suitable framebuffer config
  int numReturned;
  GLXFBConfig *fbConfigs = glXChooseFBConfig(gXDisplay, DefaultScreen(gXDisplay), doubleBufferAttributes, &numReturned);
  if (NULL == fbConfigs)
    {
      printf("Unable to get a double buffered configuration\n");
      return false;
    }

  // Create an X colormap and window with a visual matching the framebuffer config
  XVisualInfo *vInfo = glXGetVisualFromFBConfig(gXDisplay, fbConfigs[0]);

  // We must create a color map - I didn't understand why very well since we require TrueColor
  XSetWindowAttributes  swa;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  swa.colormap = XCreateColormap(gXDisplay, RootWindow(gXDisplay, vInfo->screen), vInfo->visual, AllocNone);

  // Create the window itself
  gXWindow = XCreateWindow(gXDisplay, RootWindow(gXDisplay, vInfo->screen), 0, 0, width, height,
			   0, vInfo->depth, InputOutput, vInfo->visual,
			   CWBorderPixel | CWColormap | CWEventMask, &swa);

  // Create a GLX context for OpenGL rendering
  GLXContext context = glXCreateNewContext(gXDisplay, fbConfigs[0], GLX_RGBA_TYPE, NULL, True);

  // Associate the frame buffer configuration with the created X window
  glxWin = glXCreateWindow(gXDisplay, fbConfigs[0], gXWindow, NULL);

  // Map the window to the screen
  XMapWindow(gXDisplay, gXWindow);

  // Wait for the window to appear
  XEvent event;
  XIfEvent(gXDisplay, &event, WaitForNotify, (XPointer) gXWindow);

  // Bind the GLX context to the Window
  glXMakeContextCurrent(gXDisplay, glxWin, glxWin, context);

  free(vInfo);
  free(fbConfigs);

  SizeGLScene(width, height);
  if (!InitGL())
    {
      KillGLWindow();
      printf("Initializing GL failed.");
      return false;
    }
  return true;
}









void JGEControl()
{
  gPrevControllerState = gControllerState;
}

void Update(int dt)
{
  g_engine->SetDelta(dt);
  g_engine->Update();
  JGEControl();
}


int DrawGLScene(void)									// Here's Where We Do All The Drawing
{

// 	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
// 	glLoadIdentity ();											// Reset The Modelview Matrix

	//if (g_app)
	//	g_app->Render();
	g_engine->Render();

//	glFlush ();

	return true;										// Everything Went OK
}



bool JGEGetButtonState(u32 button)
{
  return gControllerState & button;
}

bool JGEGetButtonClick(u32 button)
{
  return ((gControllerState & button) && (!(gPrevControllerState & button)));
}

bool JGEGetKeyState(int key __attribute__((unused)))
{
  return false;
}

u32 JGEReadKey()
{
  if (gKeyBuffer.empty()) return 0;
  u32 val = gKeyBuffer.front().second;
  gHolds &= ~val;
  gKeyBuffer.pop();
  return val;
}

u32 JGEReadLocalKey()
{
  if (gKeyBuffer.empty()) return 0;
  pair <KeyCode, u32> val = gKeyBuffer.front();
  gHolds &= ~val.second;
  gKeyBuffer.pop();
  return val.first;
}

void JGEBindLocalKey(u32 localKey, u32 pspSymbol)
{
}

void JGEResetInput()
{
  while (!gKeyBuffer.empty()) gKeyBuffer.pop();
}

void reshapeFunc(int width, int height)
{
  ReSizeGLScene(width, height);
}

void fullscreen()
{
  Atom wmState = XInternAtom(gXDisplay, "_NET_WM_STATE", False);
  Atom fullScreen = XInternAtom(gXDisplay, "_NET_WM_STATE_FULLSCREEN", False);

  XEvent xev;
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.window = gXWindow;
  xev.xclient.message_type = wmState;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = _NET_WM_STATE_TOGGLE;
  xev.xclient.data.l[1] = fullScreen;
  xev.xclient.data.l[2] = 0;

  if (window_state.fullscreen)
    {
      XSendEvent(gXDisplay, DefaultRootWindow(gXDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
      XMoveResizeWindow(gXDisplay, gXWindow, window_state.x, window_state.y, window_state.width, window_state.height);
      window_state.fullscreen = false;
    }
  else
    {
      XWindowAttributes xwa;
      Window child_return;
      int x, y;
      XGetWindowAttributes(gXDisplay, gXWindow, &xwa);
      XTranslateCoordinates(gXDisplay, gXWindow, DefaultRootWindow(gXDisplay), xwa.x, xwa.y, &window_state.x, &window_state.y, &child_return);
      window_state.width = xwa.width; window_state.height = xwa.height;
      window_state.fullscreen = true;

      XSendEvent(gXDisplay, DefaultRootWindow(gXDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

      XGetWindowAttributes(gXDisplay, DefaultRootWindow(gXDisplay), &xwa);
      XMoveResizeWindow(gXDisplay, gXWindow, 0, 0, xwa.width, xwa.height);
    }
}

int main(int argc, char* argv[])
{
  char* path = argv[0];
  while (*path) ++path;
  while ((*path != '/') && (path > argv[0])) --path;
  if ('/' == *path) *path = 0;
  if (strlen(argv[0]) != 0) chdir(argv[0]);

  g_launcher = new JGameLauncher();

  u32 flags = g_launcher->GetInitFlags();

  if ((flags&JINIT_FLAG_ENABLE3D)!=0)
    JRenderer::Set3DFlag(true);

  // Create Our OpenGL Window
  if (!CreateGLWindow(g_launcher->GetName(), ACTUAL_SCREEN_WIDTH, ACTUAL_SCREEN_HEIGHT, 32, window_state.fullscreen))
    {
      printf("Could not create the window\n");
      return 1;								// Quit If Window Was Not Created
    }

  if (!InitGame())
    {
      printf("Could not init the game\n");
      return 1;
    }

  XSelectInput(gXDisplay, gXWindow, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
  XkbSetDetectableAutoRepeat(gXDisplay, true, NULL);

  static uint64_t tickCount;
  while (!g_engine->IsDone())
    {
      struct timeval tv;
      uint dt;
      XEvent event;
      gettimeofday(&tv, NULL);
      tickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      dt = (tickCount - lastTickCount);
      lastTickCount = tickCount;
      Update(dt);						// Update frame

      DrawGLScene();						// Draw The Scene
      glXSwapBuffers(gXDisplay, glxWin);
      if (XCheckWindowEvent(gXDisplay, gXWindow, KeyPressMask | KeyReleaseMask | StructureNotifyMask, &event))
	switch (event.type)
	  {
	  case KeyPress:
	    {
	      KeySym sym = XKeycodeToKeysym(gXDisplay, event.xkey.keycode, 1);
	      if (XK_F == sym)
		fullscreen();
	      for (vector< pair<KeySym, u32> >::iterator it = gKeyCodes.begin(); it != gKeyCodes.end(); ++it)
		if (sym == it->first)
		  {
		    if (!(gHolds & it->second))
		      gKeyBuffer.push(*it);
		    gControllerState |= it->second;
		    gHolds |= it->second;
		    break;
		  }
	    }
	    break;
	  case KeyRelease:
	    {
	      KeySym sym = XKeycodeToKeysym(gXDisplay, event.xkey.keycode, 1);
	      for (vector< pair<KeySym, u32> >::iterator it = gKeyCodes.begin(); it != gKeyCodes.end(); ++it)
		if (sym == it->first)
		  {
		    gControllerState &= ~it->second;
		    gHolds &= ~it->second;
		    break;
		  }
	    }
	    break;
	  case ConfigureNotify:
	    ReSizeGLScene(event.xconfigure.width, event.xconfigure.height);
	    break;
	  default:
	    break;
	  }
    }

  if (g_launcher)
    delete g_launcher;

  // Shutdown
  DestroyGame();
  KillGLWindow();							// Kill The Window
  return 0;
}
