#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QTime>
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

  void wheelEvent(QWheelEvent *event);

protected:
  QPoint lastPos;

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
{
  startTimer( 5 );
  setWindowTitle(g_launcher->GetName());
#ifdef Q_WS_MAEMO_5
  setAttribute(Qt::WA_Maemo5AutoOrientation);
  setAttribute(Qt::WA_Maemo5NonComposited);
#endif
  setAttribute(Qt::WA_AcceptTouchEvents);
  grabGesture(Qt::PanGesture);
  grabGesture(Qt::PinchGesture);
  grabGesture(Qt::SwipeGesture);
}


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
#endif

  glEnable(GL_CULL_FACE);				// do not calculate inside of poly's
  glFrontFace(GL_CCW);					// counter clock-wise polygons are out

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_SCISSOR_TEST);				// Enable Clipping
}

int actualWidth;
int actualHeight;

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)	// Resize The GL Window
{
  actualWidth = width;
  actualHeight = height;

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
  if(event->key() == Qt::Key_F)
  {
    JGEToggleFullscreen();
  }
  g_engine->HoldKey_NoRepeat(event->key());
  event->accept();
  QWidget::keyPressEvent(event);
  return;
}

void JGEQtRenderer::keyReleaseEvent(QKeyEvent *event)
{
  g_engine->ReleaseKey(event->key());
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
  g_glwidget->show();

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
