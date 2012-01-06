#include "corewrapper.h"

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((GLfloat)ACTUAL_SCREEN_WIDTH / (GLfloat)ACTUAL_SCREEN_HEIGHT)

static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
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
//    { Qt::Key_F,            JGE_BTN_FULLSCREEN },
};

int JGEGetTime()
{
    return (int)WagicCore::g_startTimer.elapsed();
}

QElapsedTimer WagicCore::g_startTimer;

WagicCore::WagicCore(QDeclarativeItem *parent) :
    QDeclarativeItem(parent), m_engine(0), m_app(0), m_launcher(0), m_active(false)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
    setWidth(480);
    setHeight(272);

    // BindKey is a static method, m_engine is not even initialized
    for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
      m_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);

    g_startTimer.restart();
    m_lastTickCount = g_startTimer.elapsed();
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

        // BindKey is a static method, m_engine is not even initialized
        for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
          m_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);

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

void WagicCore::pixelInput(int x, int y)
{
    if(m_engine)
        m_engine->LeftClicked(x, y);
}

void WagicCore::timerEvent( QTimerEvent* )
{
    qint64 tickCount;
    quint32 dt;
    tickCount = g_startTimer.elapsed();
    dt = (tickCount - m_lastTickCount);
    m_lastTickCount = tickCount;

    if(!m_engine)
        return;
    if(m_engine->IsDone())
        QApplication::instance()->quit();

    m_engine->SetDelta((float)dt / 1000.0f);
    m_engine->Update((float)dt / 1000.0f);

    update();
}

void WagicCore::setActive(bool active)
{
    if(!m_engine) return;

    if(!m_active && active)
    {
        m_engine->Resume();
    #if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
        // 30 fps max on mobile
        m_timerId = startTimer(33);
    #else
        // 200 fps max on desktop
        m_timerId = startTimer(5);
    #endif //Q_WS_MAEMO_5
        m_active = active;
        emit activeChanged();
    }
    else if(m_active && !active)
    {
        m_engine->Pause();
    #if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
        killTimer(timerId);
    #endif
        m_active = active;
        emit activeChanged();
    }
}

void WagicCore::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->beginNativePainting();

    initApp();

    QRectF rectf = boundingRect();
    resize ( rectf);

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
  #if (defined GL_VERSION_ES_CM_1_1 || defined GL_OES_VERSION_1_1)
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

    render();

    painter->endNativePainting();
}

void WagicCore::resize ( const QRectF &rect)
{
    int width = rect.size().width();
    int height= rect.size().height();
    if(width && height)
    {
        if ((GLfloat)width / (GLfloat)height <= ACTUAL_RATIO)
        {
          m_viewPort.setLeft(0);
          m_viewPort.setTop(-((width/ACTUAL_RATIO)-height)/2);
          m_viewPort.setRight(width);
          m_viewPort.setBottom(-((width/ACTUAL_RATIO)-height)/2 + width / ACTUAL_RATIO);
        }
        else
        {
          m_viewPort.setLeft(-(height*ACTUAL_RATIO-width)/2);
          m_viewPort.setTop(0);
          m_viewPort.setRight(-(height*ACTUAL_RATIO-width)/2 + height * ACTUAL_RATIO);
          m_viewPort.setBottom(height);
        }

        glViewport(m_viewPort.left(), m_viewPort.top(), m_viewPort.right()-m_viewPort.left(), m_viewPort.bottom()-m_viewPort.top());

        JRenderer::GetInstance()->SetActualWidth(m_viewPort.right()-m_viewPort.left());
        JRenderer::GetInstance()->SetActualHeight(m_viewPort.bottom()-m_viewPort.top());
        glScissor(0, 0, width, height);

#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

        glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
        glLoadIdentity ();													// Reset The Projection Matrix

#if (defined GL_VERSION_ES_CM_1_1 || defined GL_OES_VERSION_1_1)
        glOrthof(0.0f, (float) (m_viewPort.right()-m_viewPort.left())-1.0f, 0.0f, (float) (m_viewPort.bottom()-m_viewPort.top())-1.0f, -1.0f, 1.0f);
#else
        gluOrtho2D(0.0f, (float) (m_viewPort.right()-m_viewPort.left())-1.0f, 0.0f, (float) (m_viewPort.bottom()-m_viewPort.top())-1.0f);
#endif

        glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
        glLoadIdentity ();													// Reset The Modelview Matrix

        glDisable (GL_DEPTH_TEST);
#endif

    }
}

void WagicCore::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
#if (defined Q_WS_MAEMO_5)
  case Qt::Key_F7:
    /* interrupt please */
    g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    break;
  case Qt::Key_F8:
    /* next phase please */
    g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    break;
#endif // Q_WS_MAEMO_5
  case Qt::Key_F:
    JGEToggleFullscreen();
    break;
  default:
    m_engine->HoldKey_NoRepeat((LocalKeySym)event->key());
  }
  event->accept();
  QGraphicsItem::keyPressEvent(event);
  return;
}

void WagicCore::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
#if (defined Q_WS_MAEMO_5)
    case Qt::Key_F7:
      /* interrupt please */
      m_engine->ReleaseKey(JGE_BTN_SEC);
      break;
    case Qt::Key_F8:
      /* next phase please */
      m_engine->ReleaseKey(JGE_BTN_PREV);
      break;
  #endif // Q_WS_MAEMO_5
    default:
      m_engine->ReleaseKey((LocalKeySym)event->key());
    }

    event->accept();
    QGraphicsItem::keyReleaseEvent(event);
    return;
}

