#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QTime>

#ifdef Q_WS_MAEMO_5
// For volume buttons support
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

// For screen on/off events support
#include <mce/dbus-names.h>
#include <mce/mode-names.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>

#endif //Q_WS_MAEMO_5

#include "../include/JGE.h"
#include "../include/JTypes.h"
#include "../include/JApp.h"
#include "../include/JFileSystem.h"
#include "../include/JRenderer.h"
#include "../include/JGameLauncher.h"

#define ACTUAL_SCREEN_WIDTH (SCREEN_WIDTH)
#define ACTUAL_SCREEN_HEIGHT (SCREEN_HEIGHT)
#define ACTUAL_RATIO ((GLfloat)ACTUAL_SCREEN_WIDTH / (GLfloat)ACTUAL_SCREEN_HEIGHT)

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

  void mouseDoubleClickEvent(QMouseEvent *event);

  void mousePressEvent(QMouseEvent *event);

  void mouseReleaseEvent(QMouseEvent *event);

  void mouseMoveEvent(QMouseEvent *event);

  void wheelEvent(QWheelEvent *event);

  void showEvent ( QShowEvent * event );

  void hideEvent ( QHideEvent * event );

#ifdef Q_WS_MAEMO_5
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
  QPoint lastPos;
  int timerId;
  bool timerStarted;

#ifdef Q_WS_MAEMO_5
  QDBusConnection dBusConnection;
  QDBusInterface* dBusInterface;
#endif //Q_WS_MAEMO_5
};

uint64_t	lastTickCount;
JGE* g_engine = NULL;
JApp* g_app = NULL;
JGameLauncher* g_launcher = NULL;
JGEQtRenderer *g_glwidget = NULL;

static const struct { LocalKeySym keysym; JButton keycode; } gDefaultBindings[] =
{
  { Qt::Key_Enter,        JGE_BTN_MENU },
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

int JGEGetTime()
{
  QTime theTime = QTime::currentTime();
  return theTime.second() * 1000 + theTime.msec();
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

  JRenderer::GetInstance()->Enable2D();
  QTime theTime = QTime::currentTime();
  lastTickCount = theTime.second() * 1000 + theTime.msec();

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
#ifdef Q_WS_MAEMO_5
  setAttribute(Qt::WA_Maemo5AutoOrientation);
  setAttribute(Qt::WA_Maemo5NonComposited);
  grabZoomKeys(true);
#endif
  setAttribute(Qt::WA_AcceptTouchEvents);
  setMouseTracking(true);
  grabGesture(Qt::PanGesture);
  grabGesture(Qt::PinchGesture);
  grabGesture(Qt::SwipeGesture);

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
  glClearDepth(1.0f);					// Depth Buffer Setup

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

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)	// Resize The GL Window
{
  JRenderer::GetInstance()->SetActualWidth(width);
  JRenderer::GetInstance()->SetActualHeight(height);

  if ((GLfloat)width / (GLfloat)height < ACTUAL_RATIO)
  {
    glViewport(0, -((width/ACTUAL_RATIO)-height)/2, width, width / ACTUAL_RATIO);			// Reset The Current Viewport
  }
  else
  {
    glViewport(-(height*ACTUAL_RATIO-width)/2, 0, height * ACTUAL_RATIO, height);
  }
  glScissor(0, 0, width, height);
}

void JGEQtRenderer::resizeGL(int width, int height)
{
  ReSizeGLScene(width, height);
}

void JGEQtRenderer::paintGL()
{
  g_engine->Render();
}

void JGEQtRenderer::timerEvent( QTimerEvent* )
{
  QTime theTime = QTime::currentTime();
  static uint64_t tickCount;
  quint32 dt;
  tickCount = theTime.second() * 1000 + theTime.msec();
  dt = (tickCount - lastTickCount);
  lastTickCount = tickCount;

  if(g_engine->IsDone()) close();

  //gPrevControllerState = gControllerState;
  g_engine->SetDelta((float)dt / 1000.0f);
  g_engine->Update((float)dt / 1000.0f);

  updateGL();
}

void JGEQtRenderer::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    lastPos = event->pos();
    // this is intended to convert window coordinate into game coordinate.
    // this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
    int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
    int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();
    g_engine->LeftClicked((lastPos.x()*SCREEN_WIDTH)/actualWidth, (lastPos.y()*SCREEN_HEIGHT)/actualHeight);
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
    QGLWidget::mouseReleaseEvent(event);
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
    QGLWidget::mouseReleaseEvent(event);
  }
}


void JGEQtRenderer::mouseMoveEvent(QMouseEvent *event)
{
  // this is intended to convert window coordinate into game coordinate.
  // this is correct only if the game and window have the same aspect ratio, otherwise, it's just wrong
  int actualWidth = (int) JRenderer::GetInstance()->GetActualWidth();
  int actualHeight = (int) JRenderer::GetInstance()->GetActualHeight();

  g_engine->LeftClicked((event->pos().x()*SCREEN_WIDTH)/actualWidth, (event->pos().y()*SCREEN_HEIGHT)/actualHeight);
  event->accept();
}

/*
void JGEQtRenderer::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    lastPos = event->pos();
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
    QPoint currentPos = event->pos();
    int dx = currentPos.x() - lastPos.x();
    int dy = currentPos.y() - lastPos.y();

    if(abs(dx) > abs(dy) && dx > 5)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_RIGHT);
    }
    else if(abs(dx) > abs(dy) && dx < -5)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_LEFT);
    }
    else if(abs(dx) < abs(dy) && dy < -5)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_UP);
    }
    else if(abs(dx) < abs(dy) && dy > 5)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_DOWN);
    }
    event->accept();
  }
  else if(event->button() == Qt::RightButton)
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_PREV);
    event->accept();
  }
  else if(event->button() == Qt::MidButton)
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_SEC);
    event->accept();
  }
  else
  {
    QGLWidget::mouseReleaseEvent(event);
  }
}
*/

void JGEQtRenderer::mouseDoubleClickEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_OK);
    event->accept();
  }
  else
  {
    QGLWidget::mouseDoubleClickEvent(event);
  }
}


void JGEQtRenderer::wheelEvent(QWheelEvent *event)
{
  if(event->orientation() == Qt::Vertical)
  {
    if(event->delta() > 0)
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_UP);
    }
    else
    {
      g_engine->HoldKey_NoRepeat(JGE_BTN_DOWN);
    }
  }
  else if(event->orientation() == Qt::Horizontal)
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_LEFT);
  }
  else
  {
    g_engine->HoldKey_NoRepeat(JGE_BTN_RIGHT);
  }
  event->accept();
}


void JGEQtRenderer::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
#ifdef Q_WS_MAEMO_5
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

void JGEQtRenderer::showEvent ( QShowEvent * event )
{
  if(!timerStarted)
  {
#ifdef Q_WS_MAEMO_5
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
  if(timerStarted)
  {
    killTimer(timerId);
    timerStarted = false;
  }
}

void JGEQtRenderer::keyReleaseEvent(QKeyEvent *event)
{
  g_engine->ReleaseKey((LocalKeySym)event->key());
  event->accept();
  QWidget::keyReleaseEvent(event);
  return;
}


int main(int argc, char* argv[])
{
  QApplication a( argc, argv );
  QDir::setCurrent(QCoreApplication::applicationDirPath () );

  g_launcher = new JGameLauncher();

  u32 flags = g_launcher->GetInitFlags();

  if ((flags&JINIT_FLAG_ENABLE3D)!=0)
  {
    JRenderer::Set3DFlag(true);
  }

  g_glwidget = new JGEQtRenderer(NULL);
  g_glwidget->resize(ACTUAL_SCREEN_WIDTH, ACTUAL_SCREEN_HEIGHT);
#ifdef Q_WS_MAEMO_5
  // We start in fullscreen on mobile
  g_glwidget->showFullScreen();
#else
  // not on desktop
  g_glwidget->show();
#endif

  if (!InitGame())
  {
      qDebug("Could not init the game\n");
      return 1;
  }

  JGECreateDefaultBindings();

  a.exec();

  if (g_launcher)
    delete g_launcher;

  if(g_glwidget)
    delete g_glwidget;

  // Shutdown
  DestroyGame();

  return 0;
}
