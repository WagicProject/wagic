#include <SDL.h>
#if (defined ANDROID)
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#ifdef WIN32
#undef GL_VERSION_2_0
#endif
#endif

#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"
#include "DebugRoutines.h"
#include <stdexcept>
#include <iostream>


#if (defined FORCE_GLES)
#undef GL_ES_VERSION_2_0
#undef GL_VERSION_2_0
#define GL_VERSION_ES_CM_1_1 1
#define glOrthof glOrtho
#define glClearDepthf glClearDepth
#endif

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((GLfloat)ACTUAL_SCREEN_WIDTH / (GLfloat)ACTUAL_SCREEN_HEIGHT)

enum eDisplayMode
{
	DisplayMode_lowRes = 0,
	DisplayMode_hiRes,
	DisplayMode_fullscreen
};

unsigned int gDisplayMode = DisplayMode_lowRes;

const int kHitzonePliancy = 50;

// tick value equates to ms
const int kTapEventTimeout = 250;


uint64_t	lastTickCount;
JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;
#ifdef ANDROID
JNIEnv * mJNIEnv = NULL;
jclass * mJNIClass = NULL;
#endif

class SdlApp;

SdlApp *g_SdlApp = NULL;


#ifdef ANDROID
// Pause
extern "C" void Java_org_libsdl_app_SDLActivity_nativePause(
                                    JNIEnv* env, jclass cls)
{    
	DebugTrace("Attempt pause");
	if (!g_engine)
		return;
   g_engine->Pause();
   DebugTrace("Pause done");
}

// Resume
extern "C" void Java_org_libsdl_app_SDLActivity_nativeResume(
                                    JNIEnv* env, jclass cls)
{    
	if (!g_engine)
		return;
	g_engine->Resume();
}
#endif


class SdlApp
{
public: /* For easy interfacing with JGE static functions */
	bool            Running;
	SDL_Window*     window;
	SDL_Surface*    Surf_Display;
	SDL_Rect        viewPort;
	Uint32          lastMouseUpTime;
    Uint32          lastFingerDownTime;
	int             windowed_w;
	int             windowed_h;
	int             windowed_pos_x;
	int             windowed_pos_y;

    int mMouseDownX;
    int mMouseDownY;

public:
	SdlApp() : Surf_Display(NULL), window(NULL), lastMouseUpTime(0), lastFingerDownTime(0), Running(true), mMouseDownX(0), mMouseDownY(0)
	{
	}

	int OnExecute()
	{
		if (OnInit() == false)
		{
			return -1;
		}

		SDL_Event Event;

		while(Running)
		{
			if (g_engine && !g_engine->IsPaused())
			{
				while(SDL_WaitEventTimeout(&Event, 0))
				{
					OnEvent(&Event);
				}
				OnUpdate();
			}
		}

		OnCleanup();

		return 0;
	};

public:
	bool OnInit();

	void OnResize(int width, int height)
	{
		DebugTrace("OnResize Width " << width << " height " << height);

		if ((GLfloat)width / (GLfloat)height <= ACTUAL_RATIO)
		{
			viewPort.x = 0;
			viewPort.y = -(static_cast<int>(width / ACTUAL_RATIO) - height) / 2;
			viewPort.w = width;
			viewPort.h = static_cast<int>(width / ACTUAL_RATIO);
		}
		else
		{
			viewPort.x = -(static_cast<int>(height * ACTUAL_RATIO) - width) / 2;
			viewPort.y = 0;
			viewPort.w = static_cast<int>(height * ACTUAL_RATIO);
			viewPort.h = height;
		}

		glViewport(viewPort.x, viewPort.y, viewPort.w, viewPort.h);

		JRenderer::GetInstance()->SetActualWidth(static_cast<float>(viewPort.w));
		JRenderer::GetInstance()->SetActualHeight(static_cast<float>(viewPort.h));
		glScissor(0, 0, width, height);

#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

		glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
		glLoadIdentity ();													// Reset The Projection Matrix

#if (defined GL_VERSION_ES_CM_1_1)
		glOrthof(0.0f, (float) (viewPort.w)-1.0f, 0.0f, (float) (viewPort.h)-1.0f, -1.0f, 1.0f);
#else
		gluOrtho2D(0.0f, (float) (viewPort.w)-1.0f, 0.0f, (float) (viewPort.h)-1.0f);
#endif
		glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
		glLoadIdentity ();													// Reset The Modelview Matrix

		glDisable (GL_DEPTH_TEST);

#endif
	}

	void OnKeyPressed(const SDL_KeyboardEvent& event);
	void OnMouseDoubleClicked(const SDL_MouseButtonEvent& event);
	void OnMouseClicked(const SDL_MouseButtonEvent& event);
	void OnMouseMoved(const SDL_MouseMotionEvent& event);
  void OnMouseWheel(int x, int y);

  void OnTouchEvent(const SDL_TouchFingerEvent& event);

	void OnEvent(SDL_Event* Event)
	{
		/* if(Event->type < SDL_USEREVENT) 
		DebugTrace("Event received" << Event->type);*/
		switch(Event->type)
		{
		case SDL_QUIT:
			Running = false;
			break;

		/* On Android, this is triggered when the device orientation changed */
		case SDL_WINDOWEVENT: 
			window = SDL_GetWindowFromID(Event->window.windowID);
			int h,w;
			SDL_GetWindowSize(window, &w, &h);
			OnResize(w, h);
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			OnKeyPressed(Event->key);
			break;

		case SDL_MOUSEMOTION:
			OnMouseMoved(Event->motion);
			break;

		case SDL_MOUSEBUTTONDOWN:
			OnMouseClicked(Event->button);
			break;

		case SDL_MOUSEBUTTONUP:
			{
				Uint32 eventTime = SDL_GetTicks();
				if (eventTime - lastMouseUpTime <= 500)
				{
					OnMouseDoubleClicked(Event->button);
				}
				else
				{
					OnMouseClicked(Event->button);
				}
				lastMouseUpTime = eventTime;
			}

			break;

    case SDL_MOUSEWHEEL:
      OnMouseWheel(Event->wheel.x, Event->wheel.y);
      break;

		case SDL_FINGERMOTION:
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
			//DebugTrace("Touch Event triggered");
            //DebugTrace("touchId " << Event->tfinger.touchId);
			//DebugTrace("fingerId " << Event->tfinger.fingerId);
			//DebugTrace("state " << Event->tfinger.state);
			//DebugTrace("x " << Event->tfinger.x	<< ", y " << Event->tfinger.y);
			//DebugTrace("dx " << Event->tfinger.dx << ", dy " << Event->tfinger.dy);
			//DebugTrace("pressure " << Event->tfinger.pressure);

			OnTouchEvent(Event->tfinger);
			break;

		case SDL_MULTIGESTURE:
			DebugTrace("Multigesture : touchId " << Event->mgesture.touchId
				<< ", x " << Event->mgesture.x
				<< ", y " << Event->mgesture.y
				<< ", dTheta " << Event->mgesture.dTheta
				<< ", dDist " << Event->mgesture.dDist
				<< ", numFingers " << Event->mgesture.numFingers);
			break;

        case SDL_JOYBALLMOTION:
            DebugTrace("Flick gesture detected, x: " << Event->jball.xrel << ", y: " << Event->jball.yrel);
            g_engine->Scroll(Event->jball.xrel, Event->jball.yrel);
            break;
        }
	}

	void OnUpdate();

	void OnCleanup()
	{
		SDL_FreeSurface(Surf_Display);
		SDL_Quit();
	}
};

static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
{  
	/* windows controls */
	{ SDLK_LCTRL,         JGE_BTN_CTRL },
	{ SDLK_RCTRL,         JGE_BTN_CTRL },
	{ SDLK_RETURN,        JGE_BTN_MENU },
	{ SDLK_KP_ENTER,      JGE_BTN_MENU },
	{ SDLK_ESCAPE,        JGE_BTN_MENU },
	{ SDLK_UP,            JGE_BTN_UP },
	{ SDLK_DOWN,          JGE_BTN_DOWN },
	{ SDLK_LEFT,          JGE_BTN_LEFT },
	{ SDLK_RIGHT,         JGE_BTN_RIGHT },
	{ SDLK_z,             JGE_BTN_UP },
	{ SDLK_d,             JGE_BTN_RIGHT },
	{ SDLK_s,             JGE_BTN_DOWN },
	{ SDLK_q,             JGE_BTN_LEFT },
	{ SDLK_a,             JGE_BTN_PREV },
	{ SDLK_e,             JGE_BTN_NEXT },
	{ SDLK_i,             JGE_BTN_CANCEL },
	{ SDLK_l,             JGE_BTN_OK },
	{ SDLK_SPACE,         JGE_BTN_OK },
	{ SDLK_k,             JGE_BTN_SEC },
	{ SDLK_j,             JGE_BTN_PRI },
	{ SDLK_f,             JGE_BTN_FULLSCREEN },

	/* old Qt ones, basically modified to comply with the N900 keyboard
	{ SDLK_a,             JGE_BTN_NEXT },
	{ SDLK_TAB,           JGE_BTN_CANCEL },
	{ SDLK_q,             JGE_BTN_PREV },
	{ SDLK_BACKSPACE,     JGE_BTN_CTRL },
	*/

	/* Android customs */
	{ SDLK_AC_BACK,       JGE_BTN_MENU },
	/* Android/maemo volume button mapping */
	{ SDLK_VOLUMEUP,      JGE_BTN_PREV },
	{ SDLK_VOLUMEDOWN,    JGE_BTN_SEC},
};

void JGECreateDefaultBindings()
{
	for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
		g_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);
}

int JGEGetTime()
{
	return (int)SDL_GetTicks();
}

bool JGEToggleFullscreen()
{
	//cycle between the display modes
	++gDisplayMode;
	if (gDisplayMode > DisplayMode_fullscreen)
		gDisplayMode = DisplayMode_lowRes;

	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(0, &mode);

	int width = 0;
	int height = 0;

	switch (gDisplayMode)
	{
	case DisplayMode_fullscreen:
		SDL_SetWindowSize(g_SdlApp->window, mode.w, mode.h);
		return (SDL_SetWindowFullscreen(g_SdlApp->window, SDL_TRUE) == 0);

		break;

	case DisplayMode_hiRes:
		width = SCREEN_WIDTH * 2;
		height = SCREEN_HEIGHT * 2;
		break;

	case DisplayMode_lowRes:
	default:
		width = SCREEN_WIDTH;
		height = SCREEN_HEIGHT;
		break;
	}

	if (SDL_SetWindowFullscreen(g_SdlApp->window, SDL_FALSE) < 0)
	{
		return false;
	}

	int x = (mode.w - width) / 2;
	int y = (mode.h - height) / 2;

	SDL_SetWindowPosition(g_SdlApp->window, x, y);
	SDL_SetWindowSize(g_SdlApp->window, width, height);

	return true;
}

bool InitGame(void)
{
	g_engine = JGE::GetInstance();
	g_app = g_launcher->GetGameApp();
	g_app->Create();
	g_engine->SetApp(g_app);
#ifdef ANDROID
    DebugTrace("Can I Set JNI Params ?");
     if (mJNIEnv)
        DebugTrace("mJNIEnv is ok");
    if (mJNIEnv && mJNIClass)
    {
        DebugTrace("Setting JNI Params");
        g_engine->SetJNIEnv(mJNIEnv, *mJNIClass);
    }
#endif	

	JRenderer::GetInstance()->Enable2D();
	lastTickCount = JGEGetTime();

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

void SdlApp::OnUpdate()
{
	static int tickCount = 0;
	tickCount = JGEGetTime();
	int64_t dt = (tickCount - lastTickCount);
	lastTickCount = tickCount;

	if(g_engine->IsDone())
	{
		SDL_Event event;
		event.user.type = SDL_QUIT;
		SDL_PushEvent(&event);
	}

	try
	{
		g_engine->SetDelta((float)dt / 1000.0f);
		g_engine->Update((float)dt / 1000.0f);
	}
	catch(out_of_range& oor)
	{
		cerr << oor.what();
	}

	if(g_engine)
		g_engine->Render();

	SDL_GL_SwapBuffers();
}

void SdlApp::OnKeyPressed(const SDL_KeyboardEvent& event)
{
	if (event.type == SDL_KEYDOWN)
	{
		g_engine->HoldKey_NoRepeat((LocalKeySym)event.keysym.sym);
	}
	else if(event.type == SDL_KEYUP)
	{
		g_engine->ReleaseKey((LocalKeySym)event.keysym.sym);
	}
}

void SdlApp::OnMouseMoved(const SDL_MouseMotionEvent& event)
{
	int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
	int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

	if (event.y >= viewPort.y &&
		event.y <= viewPort.y + viewPort.h &&
		event.x >= viewPort.x &&
		event.x <= viewPort.x + viewPort.w)
	{
		g_engine->LeftClicked(
			((event.x-viewPort.x)*SCREEN_WIDTH)/actualWidth,
			((event.y-viewPort.y)*SCREEN_HEIGHT)/actualHeight);
	}
}

void SdlApp::OnMouseDoubleClicked(const SDL_MouseButtonEvent& event)
{
#if (defined ANDROID) || (defined IOS)
	if(event.button == SDL_BUTTON_LEFT) /* Left button */
	{
		g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
	}
#endif
}

void SdlApp::OnMouseWheel(int x, int y)
{
  if(!x && y)
  { // Vertical wheel
    if(y > 0)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_UP);
    }
    else
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_DOWN);
    }
  }
  else if(x && !y)
  { // Horizontal wheel
    g_engine->HoldKey_NoRepeat(JGE_BTN_LEFT);
  }
  else
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_RIGHT);
  }
}

void SdlApp::OnMouseClicked(const SDL_MouseButtonEvent& event)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		if (event.button == SDL_BUTTON_LEFT) /* Left button */
		{
			// this is intended to convert window coordinate into game coordinate.
			// this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
			int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
			int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

			if (event.y >= viewPort.y &&
				event.y <= viewPort.y + viewPort.h &&
				event.x >= viewPort.x &&
				event.x <= viewPort.x + viewPort.w)
			{
				g_engine->LeftClicked(
					((event.x-viewPort.x)*SCREEN_WIDTH)/actualWidth,
					((event.y-viewPort.y)*SCREEN_HEIGHT)/actualHeight);
#if (!defined ANDROID) && (!defined IOS)
				g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
#endif
			}
			else if(event.y < viewPort.y)
			{
				g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
			}
			else if(event.y > viewPort.y + viewPort.h)
			{
				g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
			}
		}
		else if(event.button == SDL_BUTTON_RIGHT) /* Right button */
		{ /* next phase please */
			g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
		}
		else if(event.button == SDL_BUTTON_MIDDLE) /* Middle button */
		{ /* interrupt please */
			g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		if(event.button == SDL_BUTTON_LEFT)
		{
			if (event.y >= viewPort.y &&
				event.y <= viewPort.y + viewPort.h &&
				event.x >= viewPort.x &&
				event.x <= viewPort.x + viewPort.w)
			{
#if (!defined ANDROID) && (!defined IOS)
				g_engine->ReleaseKey(JGE_BTN_OK);
#endif
			}
			else if(event.y < viewPort.y)
			{
				g_engine->ReleaseKey(JGE_BTN_MENU);
			}
			else if(event.y > viewPort.y + viewPort.h)
			{
				g_engine->ReleaseKey(JGE_BTN_NEXT);
			}
		}
		else if(event.button == SDL_BUTTON_RIGHT)
		{ /* next phase please */
			g_engine->ReleaseKey(JGE_BTN_PREV);
		}
		else if(event.button == SDL_BUTTON_MIDDLE)
		{ /* interrupt please */
			g_engine->ReleaseKey(JGE_BTN_SEC);
		}
	}
}

void SdlApp::OnTouchEvent(const SDL_TouchFingerEvent& event)
{
    // only respond to the first finger for mouse type movements - any additional finger
    // should be ignored, and will come through instead as a multigesture event
    if (event.fingerId == 0)
    {
        if (event.y >= viewPort.y &&
            event.y <= viewPort.y + viewPort.h &&
            event.x >= viewPort.x &&
            event.x <= viewPort.x + viewPort.w)
        {
            int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
            int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

            g_engine->LeftClicked(
                ((event.x - viewPort.x) * SCREEN_WIDTH) / actualWidth,
                ((event.y - viewPort.y) * SCREEN_HEIGHT) / actualHeight);


            Uint32 eventTime = SDL_GetTicks();
            if (event.type == SDL_FINGERDOWN)
            {
                mMouseDownX = event.x;
                mMouseDownY = event.y;

                lastFingerDownTime = eventTime;
            }

#if (defined ANDROID) || (defined IOS)
            if (event.type == SDL_FINGERUP)
            {
                if (eventTime - lastFingerDownTime <= kTapEventTimeout)
                {
                    // treat an up finger within 50 pixels of the down finger coords as a double click event
                    if (abs(mMouseDownX - event.x) < kHitzonePliancy && abs(mMouseDownY - event.y) < kHitzonePliancy)
                    {
                        g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
                    }
                }
            }
#endif
        }
    }
}

bool SdlApp::OnInit()
{
	int window_w, window_h;

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) 
	{
		return false;
	}

	const SDL_VideoInfo *pVideoInfo = SDL_GetVideoInfo();
	DebugTrace("Video Display : h " << pVideoInfo->current_h << ", w " << pVideoInfo->current_w);

#if (defined ANDROID) || (defined IOS)
	window_w = pVideoInfo->current_w;
	window_h = pVideoInfo->current_h;
#else
	window_w = ACTUAL_SCREEN_WIDTH;
	window_h = ACTUAL_SCREEN_HEIGHT;
#endif

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    	    8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  	    8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   	    8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  	    8);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  	    16);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,		    32);

	SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,	    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,	8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,	    8);
	SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,	8);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	if((Surf_Display = SDL_SetVideoMode(window_w, window_h, 32,
#ifdef ANDROID
		SDL_OPENGL | SDL_FULLSCREEN | SDL_WINDOW_BORDERLESS)) == NULL)
	{
#else
		SDL_OPENGL | SDL_RESIZABLE )) == NULL)
	{
#endif
		return false;
	}
	SDL_WM_SetCaption(g_launcher->GetName(), "");

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		// Black Background (yes that's the way fuckers)
#if (defined GL_ES_VERSION_2_0) || (defined GL_VERSION_2_0)
#if (defined GL_ES_VERSION_2_0)
	glClearDepthf(1.0f);					// Depth Buffer Setup
#else
	glClearDepth(1.0f);					// Depth Buffer Setup
#endif// (defined GL_ES_VERSION_2_0)

	glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing (Less Or Equal)
	glEnable(GL_DEPTH_TEST);				// Enable Depth Testing

#else
#if (defined GL_VERSION_ES_CM_1_1)
	glClearDepthf(1.0f);					// Depth Buffer Setup
#else
	glClearDepth(1.0f);					// Depth Buffer Setup
#endif
	glDepthFunc(GL_LEQUAL);				// The Type Of Depth Testing (Less Or Equal)
	glEnable(GL_DEPTH_TEST);				// Enable Depth Testing
	glShadeModel(GL_SMOOTH);				// Select Smooth Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Set Perspective Calculations To Most Accurate

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// Set Line Antialiasing
	glEnable(GL_LINE_SMOOTH);				// Enable it!
	glEnable(GL_TEXTURE_2D);

#endif

	glEnable(GL_CULL_FACE);				// do not calculate inside of poly's
	glFrontFace(GL_CCW);					// counter clock-wise polygons are out

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_SCISSOR_TEST);				// Enable Clipping

	JGECreateDefaultBindings();

	if (!InitGame())
	{
		cerr << "Could not init the game\n";
		return false;
	}

	OnResize(window_w, window_h);

	return true;
};

#if (defined ANDROID) || (defined WIN32)
int SDL_main(int argc, char * argv[])
#else
int main(int argc, char* argv[])
#endif //ANDROID
{

#if (defined ANDROID)
	if (argc > 2)
    {
		mJNIEnv = (JNIEnv * )argv[1];
        mJNIClass = (jclass * )argv[2];
    }
#endif		

	DebugTrace("I R in da native");

	g_launcher = new JGameLauncher();

	u32 flags = g_launcher->GetInitFlags();

	if ((flags&JINIT_FLAG_ENABLE3D)!=0)
	{
		JRenderer::Set3DFlag(true);
	}

	g_SdlApp = new SdlApp();

	int result = g_SdlApp->OnExecute();

	if (g_launcher)
		delete g_launcher;

	if(g_SdlApp)
		delete g_SdlApp;

	// Shutdown
	DestroyGame();

	return result;
}
