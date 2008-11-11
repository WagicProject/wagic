#include <GL/gl.h>			// Header File For The OpenGL32 Library
#include <GL/glu.h>			// Header File For The GLu32 Library
#include <GL/glut.h>
#include <sys/time.h>
#include <queue>

#include "../../JGE/include/JGE.h"
#include "../../JGE/include/JTypes.h"
#include "../../JGE/include/JApp.h"
#include "../../JGE/include/JFileSystem.h"
#include "../../JGE/include/JRenderer.h"

#include "../../JGE/include/JGameLauncher.h"

struct window_state_t
{
  bool	fullscreen;
  unsigned int width;
  unsigned int height;
  unsigned int x;
  unsigned int y;
} window_state = { false, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0 };


uint64_t	lastTickCount;


//------------------------------------------------------------------------

JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;

//------------------------------------------------------------------------

int glWindowID = 0;


static u32 gButtons = 0;
static u32 gOldButtons = 0;
static u32 gKeyPresses = 0;

static u32 gPSPKeyMasks[] =
{
  PSP_CTRL_SELECT,
  PSP_CTRL_START,
  PSP_CTRL_UP,
  PSP_CTRL_RIGHT,
  PSP_CTRL_DOWN,
  PSP_CTRL_LEFT,
  PSP_CTRL_LTRIGGER,
  PSP_CTRL_RTRIGGER,
  PSP_CTRL_TRIANGLE,
  PSP_CTRL_CIRCLE,
  PSP_CTRL_CROSS,
  PSP_CTRL_SQUARE,
  PSP_CTRL_HOME,
  PSP_CTRL_HOLD,
  PSP_CTRL_NOTE,
  PSP_CTRL_CIRCLE,
  PSP_CTRL_START,
};


#define KEY_BACKSPACE 8
#define KEY_RETURN 10
#define KEY_DELETE 4
#define KEY_SPACE ' '
#define KEY_ESCAPE 27

static const int gGlutKeyCodes[] =
  {
    0,
    0,
    GLUT_KEY_UP,
    GLUT_KEY_RIGHT,
    GLUT_KEY_DOWN,
    GLUT_KEY_LEFT,
    GLUT_KEY_INSERT,
    GLUT_KEY_PAGE_UP,
    GLUT_KEY_HOME,
    GLUT_KEY_PAGE_DOWN,
    GLUT_KEY_END,
    0,
    GLUT_KEY_F1,
    GLUT_KEY_F2,
    GLUT_KEY_F3,
    0,
    0
  };

static const unsigned char gNonGlutKeyCodes[] =
  {
    KEY_BACKSPACE,
    KEY_RETURN,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    KEY_DELETE,
    0,
    0,
    0,
    KEY_SPACE,
    KEY_ESCAPE
  };

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)	// Resize And Initialize The GL Window
{
  if (0 == height)					// Prevent A Divide By Zero By
    height=1;						// Making Height Equal One

  glViewport(0, 0, width, height);			// Reset The Current Viewport

  glMatrixMode(GL_PROJECTION);				// Select The Projection Matrix
  glLoadIdentity();					// Reset The Projection Matrix

  // Calculate The Aspect Ratio Of The Window
  gluPerspective(75.0f, (GLfloat)width / (GLfloat)height, 0.5f, 1000.0f);

  glMatrixMode(GL_MODELVIEW);				// Select The Modelview Matrix
  glLoadIdentity();					// Reset The Modelview Matrix
}

int InitGL(void)					// All Setup For OpenGL Goes Here
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);		// Black Background (yes that's the way fuckers)
  glClearDepth (1.0f);					// Depth Buffer Setup
  glDepthFunc (GL_LEQUAL);				// The Type Of Depth Testing (Less Or Equal)
  glEnable (GL_DEPTH_TEST);				// Enable Depth Testing
  glShadeModel (GL_SMOOTH);				// Select Smooth Shading
  glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Set Perspective Calculations To Most Accurate

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// Set Line Antialiasing
  glEnable(GL_LINE_SMOOTH);				// Enable it!

  glEnable(GL_CULL_FACE);				// do not calculate inside of poly's
  glFrontFace(GL_CCW);					// counter clock-wise polygons are out

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SCISSOR_TEST);				// Enable Clipping
  //glScissor(20, 20, 320, 240);

  return true;						// Initialization Went OK
}


int InitGame(void)
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
  DestroyGame();
  if (glWindowID)
    glutDestroyWindow(glWindowID);
  glWindowID = 0;
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (true) Or Windowed Mode (false)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits __attribute__((unused)), bool fullscreenflag __attribute__((unused)))
{
  glWindowID = glutCreateWindow(title);
  ReSizeGLScene(width, height);
  if (!InitGL())
    {
      KillGLWindow();
      printf("Initializing GL failed.");
      return false;
    }
  if (!InitGame())
    {
      KillGLWindow();
      printf("Initializing game failed.");
      return false;
    }
  return true;
}









void JGEControl()
{
  gOldButtons = gButtons;
}



void Update(int dt)
{
  JGEControl();
  g_engine->SetDelta(dt);
  g_engine->Update();
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
  return gButtons & button;
}

bool JGEGetButtonClick(u32 button)
{
  return (gButtons & button) && (!(gOldButtons & button));
}

bool JGEGetKeyState(int key __attribute__((unused)))
{
  return false; // (g_keys[key]);
}

void displayCallBack(void)
{
  static uint64_t tickCount;

  if (g_engine->IsDone())
    {
      if (g_launcher)
	delete g_launcher;

      // Shutdown
      KillGLWindow();							// Kill The Window
      exit(0);
    }
  else								// Not Time To Quit, Update Screen
    {
      struct timeval tv;
      uint dt;
      gettimeofday(&tv, NULL);
      tickCount = tv.tv_sec * 1000 + tv.tv_usec / 1000;
      dt = (tickCount - lastTickCount);
      lastTickCount = tickCount;
      Update(dt);						// Update frame

      DrawGLScene();						// Draw The Scene
      glutSwapBuffers();
    }
}

void idleCallBack(void)
{
  glutPostRedisplay();
}

void initGlut(int* argc, char* argv[])
{
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void specialKey(int key, int x __attribute__((unused)), int y __attribute((unused)))
{
  for (signed int i = sizeof(gGlutKeyCodes)/sizeof(gGlutKeyCodes[0]); i > 0; --i)
    if (gGlutKeyCodes[i] == key) { gButtons |= gPSPKeyMasks[i]; gKeyPresses |= gPSPKeyMasks[i]; return; }
}
void specialUp(int key, int x __attribute__((unused)), int y __attribute((unused)))
{
  for (signed int i = sizeof(gGlutKeyCodes)/sizeof(gGlutKeyCodes[0]); i > 0; --i)
    if (gGlutKeyCodes[i] == key) { gButtons &= ~gPSPKeyMasks[i]; return; }
}
void normalKey(unsigned char key, int x __attribute__((unused)), int y __attribute((unused)))
{
  if ('f' == key)
    {
      if (window_state.fullscreen)
	{
	  glutReshapeWindow(window_state.width, window_state.height);
	  glutPositionWindow(window_state.x, window_state.y);
	  window_state.fullscreen = false;
	  ReSizeGLScene(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
      else
	{
	  window_state.fullscreen = true;
	  window_state.x = glutGet(GLUT_WINDOW_X);
	  window_state.y = glutGet(GLUT_WINDOW_Y);
	  window_state.width = glutGet(GLUT_WINDOW_WIDTH);
	  window_state.height = glutGet(GLUT_WINDOW_HEIGHT);
	  glutFullScreen();
	  ReSizeGLScene(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
    }
  for (signed int i = sizeof(gNonGlutKeyCodes)/sizeof(gNonGlutKeyCodes[0]); i > 0; --i)
    if (gNonGlutKeyCodes[i] == key) { gButtons |= gPSPKeyMasks[i]; gKeyPresses |= gPSPKeyMasks[i]; return; }
}
void normalUp(unsigned char key, int x __attribute__((unused)), int y __attribute((unused)))
{
  for (signed int i = sizeof(gNonGlutKeyCodes)/sizeof(gNonGlutKeyCodes[0]); i > 0; --i)
    if (gNonGlutKeyCodes[i] == key) { gButtons &= ~gPSPKeyMasks[i]; return; }
}

void reshapeFunc(int width, int height)
{
  ReSizeGLScene(width, height);
}

int main(int argc, char* argv[])
{
  g_launcher = new JGameLauncher();

  u32 flags = g_launcher->GetInitFlags();

  if ((flags&JINIT_FLAG_ENABLE3D)!=0)
    JRenderer::Set3DFlag(true);

  initGlut(&argc, argv);

  // Create Our OpenGL Window
  if (!CreateGLWindow(g_launcher->GetName(), SCREEN_WIDTH, SCREEN_HEIGHT, 32, window_state.fullscreen))
    return 0;								// Quit If Window Was Not Created

  glutIdleFunc(&idleCallBack);
  glutDisplayFunc(&displayCallBack);
  glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
  glutSpecialFunc(&specialKey);
  glutKeyboardFunc(&normalKey);
  glutSpecialUpFunc(&specialUp);
  glutKeyboardUpFunc(&normalUp);
  //  glutReshapeFunc(&reshapeFunc);
  glutMainLoop();

  return 0;
}
