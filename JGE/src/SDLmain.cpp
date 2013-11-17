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
#include "corewrapper.h"
#include <stdexcept>
#include <iostream>
#include <math.h>

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
	WagicCore::getInstance()->setActive(false);
	DebugTrace("Pause done");
}

// Resume
extern "C" void Java_org_libsdl_app_SDLActivity_nativeResume(
                                    JNIEnv* env, jclass cls)
{    
	WagicCore::getInstance()->setActive(true);
}
#endif


class SdlApp
{
public: /* For easy interfacing with JGE static functions */
    bool            Running;
    SDL_Window*     window;
    SDL_Surface*    Surf_Display;
    Uint32          lastMouseUpTime;
    Uint32          lastFingerDownTime;
    int             windowed_w;
    int             windowed_h;
    int             windowed_pos_x;
    int             windowed_pos_y;

    int mMouseDownX;
    int mMouseDownY;
	WagicCore		m_Wagic;

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
            for (int x = 0; x < 5 && SDL_WaitEventTimeout(&Event, 10); ++x)
            {
                if(m_Wagic.getActive())
                    OnEvent(&Event);
            }
            if(m_Wagic.getActive())
                OnUpdate();
        }

        OnCleanup();

        return 0;
    };

public:
    bool OnInit();

    void OnResize(int width, int height)
    {
		m_Wagic.onWindowResize((void*)0, (float)width, (float)height);
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
        	DebugTrace("Finger Up/Down/Motion detected:" );
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
			m_Wagic.onWheelChanged(Event->jball.xrel, Event->jball.yrel);
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

void SdlApp::OnUpdate()
{
	Running = m_Wagic.onUpdate();
	if(Running)
		m_Wagic.onRender();

	SDL_GL_SwapBuffers();
}

void SdlApp::OnKeyPressed(const SDL_KeyboardEvent& event)
{
	if (event.type == SDL_KEYDOWN)
	{
		m_Wagic.onKeyDown((LocalKeySym)event.keysym.sym);
	}
	else if(event.type == SDL_KEYUP)
	{
		m_Wagic.onKeyUp((LocalKeySym)event.keysym.sym);
	}
}

void SdlApp::OnMouseMoved(const SDL_MouseMotionEvent& event)
{
	m_Wagic.onPointerMoved(WagicCore::LEFT, event.x, event.y);
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
	m_Wagic.onWheelChanged(x, y);

	/*
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
  }*/
}

void SdlApp::OnMouseClicked(const SDL_MouseButtonEvent& event)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		if (event.button == SDL_BUTTON_LEFT) /* Left button */
		{
			m_Wagic.onPointerPressed(WagicCore::LEFT, event.x, event.y);
		}
		else if(event.button == SDL_BUTTON_RIGHT) /* Right button */
		{
			m_Wagic.onPointerPressed(WagicCore::RIGHT, event.x, event.y);
		}
		else if(event.button == SDL_BUTTON_MIDDLE) /* Middle button */
		{
			m_Wagic.onPointerPressed(WagicCore::MIDLE, event.x, event.y);
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP)
	{
		if(event.button == SDL_BUTTON_LEFT)
		{
			m_Wagic.onPointerReleased(WagicCore::LEFT, event.x, event.y);
		}
		else if(event.button == SDL_BUTTON_RIGHT)
		{
			m_Wagic.onPointerReleased(WagicCore::RIGHT, event.x, event.y);
		}
		else if(event.button == SDL_BUTTON_MIDDLE)
		{
			m_Wagic.onPointerReleased(WagicCore::MIDLE, event.x, event.y);
		}
	}
}

void SdlApp::OnTouchEvent(const SDL_TouchFingerEvent& event)
{
    // only respond to the first finger for mouse type movements - any additional finger
    // should be ignored, and will come through instead as a multigesture event
    if (event.fingerId == 0)
    {
        if (event.type == SDL_FINGERDOWN)
        {
			m_Wagic.onPointerPressed(WagicCore::LEFT, event.x, event.y);
		}
		else if (event.type == SDL_FINGERUP)
		{
			m_Wagic.onPointerReleased(WagicCore::LEFT, event.x, event.y);
		}
		else
		{
			m_Wagic.onPointerMoved(WagicCore::LEFT, event.x, event.y);
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
	SDL_WM_SetCaption(m_Wagic.GetName(), "");

	m_Wagic.initApp();
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

	g_SdlApp = new SdlApp();

	int result = g_SdlApp->OnExecute();

	if(g_SdlApp)
		delete g_SdlApp;

	return result;
}
