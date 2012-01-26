#include <qplatformdefs.h>
#include <QtOpenGL>
#include "corewrapper.h"
#include <QElapsedTimer>

#if (defined FORCE_GLES)
#undef GL_ES_VERSION_2_0
#undef GL_VERSION_2_0
#define GL_VERSION_ES_CM_1_1 1
#ifndef GL_OES_VERSION_1_1
#define glOrthof glOrtho
#define glClearDepthf glClearDepth
#endif
#endif

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

// in pixels
#define kHitzonePliancy 50

// tick value equates to ms
#define kTapEventTimeout 250

// swipe duration
#define kSwipeEventMinDuration 250
// swipe distance in pixel (from top to down)
#define kSwipeMinDistance 200


QElapsedTimer WagicCore::g_startTimer;

WagicCore::WagicCore(super *parent) :
    super(parent), m_engine(0), m_app(0), m_launcher(0), m_active(false)
#ifdef Q_WS_MAEMO_5
  , dBusConnection(QDBusConnection::systemBus()), dBusInterface(0)
#endif //Q_WS_MAEMO_5
{
#ifdef QT_WIDGET
#if (defined Q_WS_MAEMO_5)
    setAttribute(Qt::WA_Maemo5AutoOrientation);
    setAttribute(Qt::WA_Maemo5NonComposited);
#endif //Q_WS_MAEMO_5
    setAttribute(Qt::WA_AcceptTouchEvents);
  //  setAttribute(Qt::WA_InputMethodEnabled);
    setMouseTracking(true);
    grabGesture(Qt::TapAndHoldGesture);
    resize(ACTUAL_SCREEN_WIDTH, ACTUAL_SCREEN_HEIGHT);
#else
    setWidth(480);
    setHeight(272);
    setFlag(QGraphicsItem::ItemHasNoContents, false);
#endif //QT_WIDGET
    g_startTimer.restart();
    m_lastTickCount = g_startTimer.elapsed();

#ifdef Q_WS_MAEMO_5
    dBusInterface = new QDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH,
                                        MCE_REQUEST_IF, dBusConnection);

    // Handle screen state on / off
    dBusConnection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_DISPLAY_SIG, this, SLOT(displayStateChanged(const QDBusMessage &)));
#endif
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
#ifdef Q_WS_MAEMO_5
    if(dBusInterface)
        delete dBusInterface;
#endif //Q_WS_MAEMO_5

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

    done();
    update();
}

void WagicCore::setActive(bool active)
{
    if(!m_engine) return;

    if(!m_active && active)
    {
        m_engine->Resume();
    #if (defined Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
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
    #if (defined Q_WS_MAEMO_5) || defined(MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
        killTimer(m_timerId);
    #endif
        m_active = active;
        emit activeChanged();
    }
}

void WagicCore::initializeGL()
{
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
}

#ifndef QT_WIDGET
void WagicCore::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->beginNativePainting();

    initApp();

    resizeGL ( boundingRect().size().width(), boundingRect().size().height());

    initializeGL();

    paintGL();

    painter->endNativePainting();
}
#endif //QT_WIDGET

void WagicCore::paintGL()
{
    if(m_engine)
        m_engine->Render();
}


void WagicCore::resizeGL(int width, int height)
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

void WagicCore::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
#if (defined Q_WS_MAEMO_5)
  case Qt::Key_F7:
    /* interrupt please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    break;
  case Qt::Key_F8:
    /* next phase please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    break;
#endif // Q_WS_MAEMO_5
  case Qt::Key_F:
    JGEToggleFullscreen();
    break;
  default:
    m_engine->HoldKey_NoRepeat((LocalKeySym)event->key());
  }
  event->accept();
  super::keyPressEvent(event);
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
    super::keyReleaseEvent(event);
}

#ifdef QT_WIDGET
void WagicCore::wheelEvent(QWheelEvent *event)
#else
void WagicCore::wheelEvent ( QGraphicsSceneWheelEvent * event)
#endif
{
    if(event->orientation() == Qt::Vertical)
        m_engine->Scroll(0, 3*event->delta(), static_cast<int>(sqrt(9*event->delta()*event->delta())));
    else

        m_engine->Scroll(3*event->delta(), 0, static_cast<int>(sqrt(9*event->delta()*event->delta())));

    event->accept();
}

#ifdef QT_WIDGET
void WagicCore::tapAndHoldTriggered(QTapAndHoldGesture* gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        m_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    }
}

void WagicCore::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    QPoint lastPos = event->pos();
    // this is intended to convert window coordinate into game coordinate.
    // this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
    int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
    int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

    if (lastPos.y() >= m_viewPort.top() &&
      lastPos.y() <= m_viewPort.bottom() &&
      lastPos.x() <= m_viewPort.right() &&
      lastPos.x() >= m_viewPort.left()) {
      m_engine->LeftClicked(
                  ((lastPos.x()-m_viewPort.left())*SCREEN_WIDTH)/actualWidth,
                  ((lastPos.y()-m_viewPort.top())*SCREEN_HEIGHT)/actualHeight);
#if (!defined Q_WS_MAEMO_5) && (!defined MEEGO_EDITION_HARMATTAN) && (!defined Q_WS_ANDROID)
      m_engine->HoldKey_NoRepeat(JGE_BTN_OK);
#else
      mMouseDownX = lastPos.x();
      mMouseDownY = lastPos.y();
      mLastFingerDownTime = g_startTimer.elapsed();
#endif
    } else if(lastPos.y()<m_viewPort.top()) {
      m_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    } else if(lastPos.y()>m_viewPort.bottom()) {
      m_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
    }
    event->accept();
  }
  else if(event->button() == Qt::RightButton)
  { /* next phase please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    event->accept();
  }
  else if(event->button() == Qt::MidButton)
  { /* interrupt please */
    m_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    event->accept();
  }
  else
  {
    super::mousePressEvent(event);
  }
}

void WagicCore::mouseReleaseEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    QPoint lastPos = event->pos();

    if (lastPos.y() >= m_viewPort.top() &&
      lastPos.y() <= m_viewPort.bottom() &&
      lastPos.x() <= m_viewPort.right() &&
      lastPos.x() >= m_viewPort.left()) {
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
      if(g_startTimer.elapsed() - mLastFingerDownTime <= kTapEventTimeout )
      {
        if(abs(mMouseDownX - lastPos.x()) < kHitzonePliancy &&
           abs(mMouseDownY - lastPos.y()) < kHitzonePliancy)
        {
          m_engine->HoldKey_NoRepeat(JGE_BTN_OK);
        }
      }
      else if (g_startTimer.elapsed() - mLastFingerDownTime >= kSwipeEventMinDuration)
      { // Let's swipe
        m_engine->Scroll(lastPos.x()-mMouseDownX, lastPos.y()-mMouseDownY);
      }
#else
//#if (!defined Q_WS_MAEMO_5) && (!defined MEEGO_EDITION_HARMATTAN)
      m_engine->ReleaseKey(JGE_BTN_OK);
#endif
      m_engine->ReleaseKey(JGE_BTN_MENU);
    } else if(lastPos.y() < m_viewPort.top()) {
      m_engine->ReleaseKey(JGE_BTN_MENU);
    } else if(lastPos.y() > m_viewPort.bottom()) {
      m_engine->ReleaseKey(JGE_BTN_NEXT);
    }
    event->accept();
  }
  else if(event->button() == Qt::RightButton)
  { /* next phase please */
    m_engine->ReleaseKey(JGE_BTN_PREV);
    event->accept();
  }
  else if(event->button() == Qt::MidButton)
  { /* interrupt please */
    m_engine->ReleaseKey(JGE_BTN_SEC);
    event->accept();
  }
  else
  {
    super::mouseReleaseEvent(event);
  }
}

void WagicCore::mouseMoveEvent(QMouseEvent *event)
{
  int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
  int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

  QPoint lastPos = event->pos();

  if (lastPos.y() >= m_viewPort.top() &&
    lastPos.y() <= m_viewPort.bottom() &&
    lastPos.x() <= m_viewPort.right() &&
    lastPos.x() >= m_viewPort.left()) {
    m_engine->LeftClicked(
                ((lastPos.x()-m_viewPort.left())*SCREEN_WIDTH)/actualWidth,
                ((lastPos.y()-m_viewPort.top())*SCREEN_HEIGHT)/actualHeight);
    event->accept();
  } else {
    super::mouseMoveEvent(event);
  }
}

void WagicCore::showEvent ( QShowEvent * event )
{
  setActive(true);
}

void WagicCore::hideEvent ( QHideEvent * event )
{
  setActive(false);
}

bool WagicCore::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
    else if (event->type() == QEvent::WindowActivate)
    {
        showEvent(NULL);
    }
    else if (event->type() == QEvent::WindowDeactivate)
    {
        hideEvent(NULL);
    }
#endif

    return QGLWidget::event(event);
}

bool WagicCore::gestureEvent(QGestureEvent* event)
{
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture))
        tapAndHoldTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));

    return true;
}

void WagicCore::start(int)
{
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
    showFullScreen();
#else
    show();
#endif
    initApp();
}

#endif //QT_WIDGET

#ifdef Q_WS_MAEMO_5
void WagicCore::displayStateChanged(const QDBusMessage &message)
{
  QString state = message.arguments().at(0).toString();
  if (!state.isEmpty()) {
    if (state == MCE_DISPLAY_ON_STRING && isActiveWindow()) {
       setActive(true);
    }
    else if (state == MCE_DISPLAY_OFF_STRING) {
        setActive(false);
    }
  }
}
#endif
