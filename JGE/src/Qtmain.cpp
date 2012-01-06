#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QTime>
#include <QtGui/QApplication>
#include <QtDeclarative>
#include "qmlapplicationviewer.h"
#include "filedownloader.h"

#if (defined FORCE_GLES)
#undef GL_ES_VERSION_2_0
#undef GL_VERSION_2_0
#define GL_VERSION_ES_CM_1_1 1
#ifndef GL_OES_VERSION_1_1
#define glOrthof glOrtho
#define glClearDepthf glClearDepth
#endif
#endif

#if (defined Q_WS_MAEMO_5)
// For volume buttons support
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#if (defined Q_WS_MAEMO_5)
// For screen on/off events support
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#endif //Q_WS_MAEMO_5

#if (defined FORCE_GLES)
#undef GL_ES_VERSION_2_0
#undef GL_VERSION_2_0
#define GL_VERSION_ES_CM_1_1 1
#define glOrthof glOrtho
#define glClearDepthf glClearDepth
#endif

#include "corewrapper.h"

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


class JGEQtRenderer : public QGLWidget
{

public:
  JGEQtRenderer(QWidget *parent);
  virtual ~JGEQtRenderer(){
#ifdef Q_WS_MAEMO_5
    delete dBusInterface;
#endif //Q_WS_MAEMO_5
  };

#ifdef Q_WS_MAEMO_5
public slots:
  void displayStateChanged(const QDBusMessage &message);
#endif //Q_WS_MAEMO_5

protected:
  void initializeGL();

  void resizeGL(int w, int h);

  void paintGL();

  void timerEvent( QTimerEvent* );

  void keyPressEvent(QKeyEvent *event);

  void keyReleaseEvent(QKeyEvent *event);

  void mousePressEvent(QMouseEvent *event);

  void mouseReleaseEvent(QMouseEvent *event);

  void mouseMoveEvent(QMouseEvent *event);

  void wheelEvent(QWheelEvent *event);

  void showEvent ( QShowEvent * event );

  void hideEvent ( QHideEvent * event );

  bool event(QEvent *event)
  {
      if (event->type() == QEvent::Gesture)
          return gestureEvent(static_cast<QGestureEvent*>(event));
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
      else if (event->type() == QEvent::WindowActivate)
      {
          JGE::GetInstance()->Resume();
          showEvent(NULL);
      }
      else if (event->type() == QEvent::WindowDeactivate)
      {
          JGE::GetInstance()->Pause();
          hideEvent(NULL);
      }
#endif

      return QGLWidget::event(event);
  }

  bool gestureEvent(QGestureEvent* event)
  {
      if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture))
          tapAndHoldTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));

      return true;
  }

  void tapAndHoldTriggered(QTapAndHoldGesture* gesture);

#if (defined Q_WS_MAEMO_5)
  void grabZoomKeys(bool grab)
  {
      if (!winId()) {
          qWarning("Can't grab keys unless we have a window id");
          return;
      }

      unsigned long val = (grab) ? 1 : 0;
      Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
      if (!atom) {
          qWarning("Unable to obtain _HILDON_ZOOM_KEY_ATOM. This example will only work "
                   "on a Maemo 5 device!");
          return;
      }

      XChangeProperty (QX11Info::display(),
              winId(),
              atom,
              XA_INTEGER,
              32,
              PropModeReplace,
              reinterpret_cast<unsigned char *>(&val),
              1);
  }
#endif //Q_WS_MAEMO_5

protected:
  int timerId;
  bool timerStarted;
  QRect viewPort;

#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
  int mMouseDownX;
  int mMouseDownY;
  qint64 mLastFingerDownTime;
#endif //Q_WS_MAEMO_5


#ifdef Q_WS_MAEMO_5
  QDBusConnection dBusConnection;
  QDBusInterface* dBusInterface;
#endif //Q_WS_MAEMO_5
};

QElapsedTimer g_startTimer;
qint64	lastTickCount;
JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;
QWidget *g_glwidget = NULL;

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

void JGECreateDefaultBindings()
{
  for (signed int i = sizeof(gDefaultBindings)/sizeof(gDefaultBindings[0]) - 1; i >= 0; --i)
    g_engine->BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);
}

bool JGEToggleFullscreen()
{
  if(g_glwidget->isFullScreen())
  {
    g_glwidget->showNormal();
  }
  else
  {
    g_glwidget->showFullScreen();
  }
  return true;
}

bool InitGame(void)
{
  g_engine = JGE::GetInstance();
  g_app = g_launcher->GetGameApp();
  g_app->Create();
  g_engine->SetApp(g_app);
  g_startTimer.start();

  JRenderer::GetInstance()->Enable2D();
  lastTickCount = g_startTimer.elapsed();

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


JGEQtRenderer::JGEQtRenderer(QWidget *parent)
  : QGLWidget(parent) /* Seems to go faster without double buffering */
  , timerStarted(false)
#ifdef Q_WS_MAEMO_5
  , dBusConnection(QDBusConnection::systemBus())
#endif //Q_WS_MAEMO_5
{
  setWindowTitle(g_launcher->GetName());
#if (defined Q_WS_MAEMO_5)
  setAttribute(Qt::WA_Maemo5AutoOrientation);
  setAttribute(Qt::WA_Maemo5NonComposited);
#endif
#if (defined Q_WS_MAEMO_5)
  grabZoomKeys(true);
#endif
  setAttribute(Qt::WA_AcceptTouchEvents);
//  setAttribute(Qt::WA_InputMethodEnabled);
  setMouseTracking(true);

  grabGesture(Qt::PanGesture);
  grabGesture(Qt::PinchGesture);
  grabGesture(Qt::SwipeGesture);
  grabGesture(Qt::TapAndHoldGesture);

#ifdef Q_WS_MAEMO_5
  dBusInterface = new QDBusInterface(MCE_SERVICE, MCE_REQUEST_PATH,
                                      MCE_REQUEST_IF, dBusConnection);

  // Handle screen state on / off
  dBusConnection.connect(MCE_SERVICE, MCE_SIGNAL_PATH, MCE_SIGNAL_IF, MCE_DISPLAY_SIG, this, SLOT(displayStateChanged(const QDBusMessage &)));
#endif
}

#ifdef Q_WS_MAEMO_5
void JGEQtRenderer::displayStateChanged(const QDBusMessage &message)
{
  QString state = message.arguments().at(0).toString();
  if (!state.isEmpty()) {
    if (state == MCE_DISPLAY_ON_STRING) {
       showEvent(0);
    }
    else if (state == MCE_DISPLAY_OFF_STRING) {
      hideEvent(0);
    }
  }
}
#endif

void JGEQtRenderer::initializeGL()
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

void JGEQtRenderer::resizeGL(int width, int height)
{
  if ((GLfloat)width / (GLfloat)height <= ACTUAL_RATIO)
  {
    viewPort.setLeft(0);
    viewPort.setTop(-((width/ACTUAL_RATIO)-height)/2);
    viewPort.setRight(width);
    viewPort.setBottom(-((width/ACTUAL_RATIO)-height)/2 + width / ACTUAL_RATIO);
  }
  else
  {
    viewPort.setLeft(-(height*ACTUAL_RATIO-width)/2);
    viewPort.setTop(0);
    viewPort.setRight(-(height*ACTUAL_RATIO-width)/2 + height * ACTUAL_RATIO);
    viewPort.setBottom(height);
  }

  glViewport(viewPort.left(), viewPort.top(), viewPort.right()-viewPort.left(), viewPort.bottom()-viewPort.top());

  JRenderer::GetInstance()->SetActualWidth(viewPort.right()-viewPort.left());
  JRenderer::GetInstance()->SetActualHeight(viewPort.bottom()-viewPort.top());
  glScissor(0, 0, width, height);

#if (!defined GL_ES_VERSION_2_0) && (!defined GL_VERSION_2_0)

  glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
  glLoadIdentity ();													// Reset The Projection Matrix

#if (defined GL_VERSION_ES_CM_1_1 || defined GL_OES_VERSION_1_1)
  glOrthof(0.0f, (float) (viewPort.right()-viewPort.left())-1.0f, 0.0f, (float) (viewPort.bottom()-viewPort.top())-1.0f, -1.0f, 1.0f);
#else
  gluOrtho2D(0.0f, (float) (viewPort.right()-viewPort.left())-1.0f, 0.0f, (float) (viewPort.bottom()-viewPort.top())-1.0f);
#endif

  glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
  glLoadIdentity ();													// Reset The Modelview Matrix

  glDisable (GL_DEPTH_TEST);
#endif
}

void JGEQtRenderer::paintGL()
{
    if(g_engine)
        g_engine->Render();
}

void JGEQtRenderer::timerEvent( QTimerEvent* )
{
  if(this->isVisible()
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
     // This one is funny, this gives us 0% CPU when the app is in background for 1 line of code =)
     && this->isActiveWindow()
#endif
     )
  {
    static qint64 tickCount;
    quint32 dt;
    tickCount = g_startTimer.elapsed();
    dt = (tickCount - lastTickCount);
    lastTickCount = tickCount;

    if(g_engine->IsDone()) close();

    //gPrevControllerState = gControllerState;
    g_engine->SetDelta((float)dt / 1000.0f);
    g_engine->Update((float)dt / 1000.0f);

    // we stop rendering if the window is hidden
    if(!isHidden())
        updateGL();
  }
}

void JGEQtRenderer::tapAndHoldTriggered(QTapAndHoldGesture* gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    }
}

void JGEQtRenderer::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    QPoint lastPos = event->pos();
    // this is intended to convert window coordinate into game coordinate.
    // this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
    int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
    int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

    if (lastPos.y() >= viewPort.top() &&
      lastPos.y() <= viewPort.bottom() &&
      lastPos.x() <= viewPort.right() &&
      lastPos.x() >= viewPort.left()) {
      g_engine->LeftClicked(
                  ((lastPos.x()-viewPort.left())*SCREEN_WIDTH)/actualWidth,
                  ((lastPos.y()-viewPort.top())*SCREEN_HEIGHT)/actualHeight);
#if (!defined Q_WS_MAEMO_5) && (!defined MEEGO_EDITION_HARMATTAN) && (!defined Q_WS_ANDROID)
      g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
#else
      mMouseDownX = lastPos.x();
      mMouseDownY = lastPos.y();
      mLastFingerDownTime = g_startTimer.elapsed();
#endif
    } else if(lastPos.y()<viewPort.top()) {
      g_engine->HoldKey_NoRepeat(JGE_BTN_MENU);
    } else if(lastPos.y()>viewPort.bottom()) {
      g_engine->HoldKey_NoRepeat(JGE_BTN_NEXT);
    }

//    g_engine->LeftClicked((lastPos.x()*SCREEN_WIDTH)/actualWidth, (lastPos.y()*SCREEN_HEIGHT)/actualHeight);
    event->accept();
  }
  else if(event->button() == Qt::RightButton)
  { /* next phase please */
    g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    event->accept();
  }
  else if(event->button() == Qt::MidButton)
  { /* interrupt please */
    g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    event->accept();
  }
  else
  {
    QGLWidget::mousePressEvent(event);
  }
}

void JGEQtRenderer::mouseReleaseEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    QPoint lastPos = event->pos();

    if (lastPos.y() >= viewPort.top() &&
      lastPos.y() <= viewPort.bottom() &&
      lastPos.x() <= viewPort.right() &&
      lastPos.x() >= viewPort.left()) {
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
      if(g_startTimer.elapsed() - mLastFingerDownTime <= kTapEventTimeout )
      {
        if(abs(mMouseDownX - lastPos.x()) < kHitzonePliancy &&
           abs(mMouseDownY - lastPos.y()) < kHitzonePliancy)
        {
          g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
        }
      }
      else if (g_startTimer.elapsed() - mLastFingerDownTime >= kSwipeEventMinDuration)
      { // Let's swipe
        g_engine->Scroll(lastPos.x()-mMouseDownX, lastPos.y()-mMouseDownY);
      }
#else
//#if (!defined Q_WS_MAEMO_5) && (!defined MEEGO_EDITION_HARMATTAN)
      g_engine->ReleaseKey(JGE_BTN_OK);
#endif
      g_engine->ReleaseKey(JGE_BTN_MENU);
    } else if(lastPos.y() < viewPort.top()) {
      g_engine->ReleaseKey(JGE_BTN_MENU);
    } else if(lastPos.y() > viewPort.bottom()) {
      g_engine->ReleaseKey(JGE_BTN_NEXT);
    }
    event->accept();
  }
  else if(event->button() == Qt::RightButton)
  { /* next phase please */
    g_engine->ReleaseKey(JGE_BTN_PREV);
    event->accept();
  }
  else if(event->button() == Qt::MidButton)
  { /* interrupt please */
    g_engine->ReleaseKey(JGE_BTN_SEC);
    event->accept();
  }
  else
  {
    QGLWidget::mouseReleaseEvent(event);
  }
}

void JGEQtRenderer::mouseMoveEvent(QMouseEvent *event)
{
  int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
  int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

  QPoint lastPos = event->pos();

  if (lastPos.y() >= viewPort.top() &&
    lastPos.y() <= viewPort.bottom() &&
    lastPos.x() <= viewPort.right() &&
    lastPos.x() >= viewPort.left()) {
    g_engine->LeftClicked(
                ((lastPos.x()-viewPort.left())*SCREEN_WIDTH)/actualWidth,
                ((lastPos.y()-viewPort.top())*SCREEN_HEIGHT)/actualHeight);
    event->accept();
  } else {
    QGLWidget::mouseMoveEvent(event);
  }
}

void JGEQtRenderer::wheelEvent(QWheelEvent *event)
{
    if(event->orientation() == Qt::Vertical)
        g_engine->Scroll(0, 3*event->delta());
    else
        g_engine->Scroll(3*event->delta(), 0);

  event->accept();
}

void JGEQtRenderer::keyPressEvent(QKeyEvent *event)
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
    g_engine->HoldKey_NoRepeat((LocalKeySym)event->key());
  }
  event->accept();
  QWidget::keyPressEvent(event);
  return;
}

void JGEQtRenderer::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
#if (defined Q_WS_MAEMO_5)
    case Qt::Key_F7:
      /* interrupt please */
      g_engine->ReleaseKey(JGE_BTN_SEC);
      break;
    case Qt::Key_F8:
      /* next phase please */
      g_engine->ReleaseKey(JGE_BTN_PREV);
      break;
  #endif // Q_WS_MAEMO_5
    default:
      g_engine->ReleaseKey((LocalKeySym)event->key());
    }

    event->accept();
    QWidget::keyReleaseEvent(event);
    return;
}

void JGEQtRenderer::showEvent ( QShowEvent * event )
{
  if(!timerStarted)
  {
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
    // 30 fps max on mobile
    timerId = startTimer(33);
#else
    // 200 fps max on desktop
    timerId = startTimer(5);
#endif //Q_WS_MAEMO_5
    timerStarted = true;
  }
}

void JGEQtRenderer::hideEvent ( QHideEvent * event )
{
#if (defined Q_WS_MAEMO_5) || (defined MEEGO_EDITION_HARMATTAN) || (defined Q_WS_ANDROID)
  if(timerStarted)
  {
    killTimer(timerId);
    timerStarted = false;
  }
#endif
}

int main(int argc, char* argv[])
{
    qmlRegisterType<WagicCore>("CustomComponents", 1, 0, "WagicCore");

    QScopedPointer<QApplication> app(createApplication(argc, argv));
    app->setApplicationName(g_launcher->GetName());

    FileDownloader fileDownloader(QUrl("http://wagic.googlecode.com/files/core_017.zip"),
                                  QDir::toNativeSeparators(QDir::homePath()) + "/.wagic/core_017.zip", 0);

    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());
    g_glwidget = viewer.data();
    viewer->setMainQmlFile(QLatin1String("qml/QmlWagic/main.qml"));

    viewer->rootContext()->setContextProperty("fileDownloader", &fileDownloader);

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    QGLWidget *glWidget = new QGLWidget;
    viewer->setViewport(glWidget);

    viewer->showExpanded();
    viewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    return app->exec();
}
