#include "corewrapper.h"

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((float)ACTUAL_SCREEN_WIDTH / (float)ACTUAL_SCREEN_HEIGHT)

// in pixels
#define kHitzonePliancy 50

// tick value equates to ms
#define kTapEventTimeout 250
// swipe duration
#define kSwipeEventMinDuration 250
// swipe distance in pixel (from top to down)
#define kSwipeMinDistance 200

WagicCore* WagicCore::s_instance = 0;

WagicCore::WagicCore() :
    m_engine(0), m_app(0), m_launcher(0), m_active(false)
{
#ifdef QT_CONFIG
    startTime = QTime::currentTime();
#endif //QT_CONFIG
    s_instance = this;
    m_lastTickCount = JGEGetTime();
}

void WagicCore::initApp()
{
    if(!m_engine)
    {
        m_launcher = new JGameLauncher();
        u32 flags = m_launcher->GetInitFlags();
        if ((flags&JINIT_FLAG_ENABLE3D)!=0)
        {
            JRenderer::Set3DFlag(true);
        }

        JGECreateDefaultBindings();

        m_engine = JGE::GetInstance();
        m_app = m_launcher->GetGameApp();
        m_app->Create();
        m_engine->SetApp(m_app);
        JRenderer::GetInstance()->Enable2D();
        setActive(true);
    }
}

WagicCore::~WagicCore()
{
    if(m_launcher)
    {
        delete m_launcher;
        m_launcher = NULL;
    }

    if(m_engine)
        m_engine->SetApp(NULL);

    if (m_app)
    {
      m_app->Destroy();
      delete m_app;
      m_app = NULL;
    }

    JGE::Destroy();

    m_engine = NULL;
}

bool WagicCore::onUpdate()
{
    bool result = false;
	int64_t tickCount;
    int64_t dt;
    tickCount = JGEGetTime();
    dt = (tickCount - m_lastTickCount);
    m_lastTickCount = tickCount;

	if(m_engine && !m_engine->IsDone())
	{
		m_engine->SetDelta((float)dt / 1000.0f);
		m_engine->Update((float)dt / 1000.0f);

		done();
		result = true;
	}

	return result;
}

void WagicCore::setActive(bool active)
{
    if(!m_engine) return;

    if(!m_active && active)
    {
        m_engine->Resume();
        m_active = active;
    }
    else if(m_active && !active)
    {
        m_engine->Pause();
        m_active = active;
    }
}

bool WagicCore::onRender()
{
	bool result = false;
    if(m_engine)
	{
        m_engine->Render();
		result = true;
	}
	return result;
}


void WagicCore::onWindowResize(void* window, float width, float height)
{
	float left, top, right, bottom;

    if ((float)width / (float)height <= ACTUAL_RATIO)
    {
		left = 0;
		top = (float)(-((width/ACTUAL_RATIO)-height)/2);
		right = width;
		bottom = (-((width/ACTUAL_RATIO)-height)/2 + width / ACTUAL_RATIO);
    }
    else
    {
		left = (-(height*ACTUAL_RATIO-width)/2);
		top = (0);
		right = (-(height*ACTUAL_RATIO-width)/2 + height * ACTUAL_RATIO);
		bottom = height;
    }

	JRenderer::GetInstance()->SetViewPort(left, top, right, bottom);
	JRenderer::GetInstance()->OnWindowsSizeChanged(window, width, height);
}

void WagicCore::onKeyDown(LocalKeySym key)
{
	m_engine->HoldKey_NoRepeat(key);
}

void WagicCore::onKeyUp(LocalKeySym key)
{
	m_engine->ReleaseKey(key);
}

void WagicCore::onWheelChanged(int deltaX, int deltaY)
{
	m_engine->Scroll(deltaX, deltaY);
}

bool WagicCore::onPointerPressed(WagicCore::PointerId pointer, int x, int y)
{
	bool result = false;
	float left, top, right, bottom;
	if(pointer == LEFT)
	{
		// this is intended to convert window coordinate into game coordinate.
		// this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
		int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
		int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();
		JRenderer::GetInstance()->GetViewPort(left, top, right, bottom);

		if (  y >= top &&
			  y <= bottom &&
			  x <= right &&
			  x >= left) {
			  m_engine->LeftClicked(
                  (int)((x-left)*SCREEN_WIDTH)/actualWidth,
                  (int)((y-top)*SCREEN_HEIGHT)/actualHeight);
#if (!defined Q_WS_MAEMO_5) && (!defined MEEGO_EDITION_HARMATTAN) && (!defined ANDROID) && (!defined IOS)
			m_engine->HoldKey_NoRepeat(JGE_BTN_OK);
#else
			mMouseDownX = x;
			mMouseDownY = y;
			mLastFingerDownTime = JGEGetTime();
#endif
			result = true;
		} else if(y<top) {
			m_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
			result = true;
		} else if(y>bottom) {
			m_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
			result = true;
		}
	}
	else if(pointer == RIGHT)
	{ /* next phase please */
		m_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
		result = true;
	}
	else if(pointer == MIDLE)
	{ /* interrupt please */
		m_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
		result = true;
	}
	return result;
}

bool WagicCore::onPointerReleased(WagicCore::PointerId pointer, int x, int y)
{
	bool result = false;
	float left, top, right, bottom;
	JRenderer::GetInstance()->GetViewPort(left, top, right, bottom);

	if(pointer == LEFT)
	{
		if (y >= top &&
			y <= bottom &&
			x <= right &&
			x >= left) {
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined ANDROID) || (defined IOS)
			if(JGEGetTime() - mLastFingerDownTime <= kTapEventTimeout )
			{
				if(abs(mMouseDownX - lastPos.x()) < kHitzonePliancy &&
					abs(mMouseDownY - lastPos.y()) < kHitzonePliancy)
				{
					m_engine->HoldKey_NoRepeat(JGE_BTN_OK);
				}
			}
			else if (JGEGetTime() - mLastFingerDownTime >= kSwipeEventMinDuration)
			{ // Let's swipe
				m_engine->Scroll(lastPos.x()-mMouseDownX, lastPos.y()-mMouseDownY);
			}
#else
			m_engine->ReleaseKey(JGE_BTN_OK);
#endif
			m_engine->ReleaseKey(JGE_BTN_MENU);
		} else if(y < top) {
			m_engine->ReleaseKey(JGE_BTN_MENU);
		} else if(y > bottom) {
			m_engine->ReleaseKey(JGE_BTN_NEXT);
		}
		result = true;
	}
	else if(pointer == RIGHT)
	{ /* next phase please */
		m_engine->ReleaseKey(JGE_BTN_PREV);
		result = true;
	}
	else if(pointer == MIDLE)
	{ /* interrupt please */
		m_engine->ReleaseKey(JGE_BTN_SEC);
		result = true;
	}

	return result;
}

bool WagicCore::onPointerMoved(WagicCore::PointerId pointer, int x, int y)
{
	bool result = false;
	float left, top, right, bottom;
	JRenderer::GetInstance()->GetViewPort(left, top, right, bottom);

	int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
	int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

	if (y >= top &&
		y <= bottom &&
		x <= right &&
		x >= left) {
		m_engine->LeftClicked(
                (int)((x-left)*SCREEN_WIDTH)/actualWidth,
                (int)((y-top)*SCREEN_HEIGHT)/actualHeight);
		result = true;
	}
	return result;
}


static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
#ifdef SDL_CONFIG
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

	/* Android customs */
	{ SDLK_AC_BACK,       JGE_BTN_MENU },
	/* Android/maemo volume button mapping */
	{ SDLK_VOLUMEUP,      JGE_BTN_PREV },
	{ SDLK_VOLUMEDOWN,    JGE_BTN_SEC},
};
#elif defined QT_CONFIG
{
    { Qt::Key_Enter,        JGE_BTN_MENU },
    { Qt::Key_Return,       JGE_BTN_MENU },
    { Qt::Key_Escape,       JGE_BTN_MENU },
    { Qt::Key_Backspace,    JGE_BTN_CTRL },
    { Qt::Key_Up,           JGE_BTN_UP },
    { Qt::Key_Down,         JGE_BTN_DOWN },
    { Qt::Key_Left,         JGE_BTN_LEFT },
    { Qt::Key_Right,        JGE_BTN_RIGHT },
    { Qt::Key_Space,        JGE_BTN_OK },
    { Qt::Key_Tab,          JGE_BTN_CANCEL },
    { Qt::Key_J,            JGE_BTN_PRI },
    { Qt::Key_K,            JGE_BTN_SEC },
    { Qt::Key_Q,            JGE_BTN_PREV },
    { Qt::Key_A,            JGE_BTN_NEXT },
// fullscreen management seems somehow broken in JGE, it works fine with Qt directly
//  { Qt::Key_F,            JGE_BTN_FULLSCREEN },
};
#else
{
    { VK_CONTROL,	JGE_BTN_CTRL },
    { VK_RETURN,	JGE_BTN_MENU },
    { VK_ESCAPE,	JGE_BTN_MENU },
    { VK_UP,		JGE_BTN_UP },
    { VK_RIGHT,		JGE_BTN_RIGHT },
    { VK_DOWN,		JGE_BTN_DOWN },
    { VK_LEFT,		JGE_BTN_LEFT },
    { 'Z',			JGE_BTN_UP },
    { 'D',			JGE_BTN_RIGHT },
    { 'S',			JGE_BTN_DOWN },
    { 'Q',			JGE_BTN_LEFT },
    { 'A',			JGE_BTN_PREV },
    { 'E',			JGE_BTN_NEXT },
    { 'I',			JGE_BTN_CANCEL },
    { 'L',			JGE_BTN_OK },
    { VK_SPACE,		JGE_BTN_OK },
    { 'K',			JGE_BTN_SEC },
    { 'J',			JGE_BTN_PRI },
    { 'F',			JGE_BTN_FULLSCREEN },
};
#endif

void WagicCore::registerDefaultBindings()
{
    for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
        m_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);

}

void JGECreateDefaultBindings()
{
	WagicCore::getInstance()->registerDefaultBindings();
}

int JGEGetTime()
{
#ifdef SDL_CONFIG
	return (int)SDL_GetTicks();
#elif defined QT_CONFIG
    return WagicCore::s_instance->startTime.msecsTo(QTime::currentTime());
#elif defined WP8
	return (int)GetTickCount64();
#endif
}
