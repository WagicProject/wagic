#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QTime>
#include <QtGui/QApplication>
#include <QScopedPointer>
#ifdef QT_WIDGET
#include <QWidget>
#else
#include <QtDeclarative>
#include "qmlapplicationviewer.h"
#endif //QT_WIDGET
#include "filedownloader.h"
#include "GameApp.h"
#include "corewrapper.h"

QWidget* g_glwidget = NULL;

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
    JGE::BindKey(gDefaultBindings[i].keysym, gDefaultBindings[i].keycode);
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

int JGEGetTime()
{
    return (int)WagicCore::g_startTimer.elapsed();
}

int main(int argc, char* argv[])
{
    QScopedPointer<QApplication> app
#ifdef QT_WIDGET
        (new QApplication(argc, argv));
#else
        (createApplication(argc, argv));

#endif //QT_WIDGET
    app->setApplicationName(WagicCore::getApplicationName());
    FileDownloader fileDownloader(USERDIR, WAGIC_RESOURCE_NAME);
#ifdef QT_WIDGET
    g_glwidget = new WagicCore();
    g_glwidget->connect(&fileDownloader, SIGNAL(finished(int)), SLOT(start(int)));
#else
    qmlRegisterType<WagicCore>("CustomComponents", 1, 0, "WagicCore");

    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());
    g_glwidget = viewer.data();

    viewer->rootContext()->setContextProperty("fileDownloader", &fileDownloader);
    viewer->setMainQmlFile(QLatin1String("qml/QmlWagic/main.qml"));

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    QGLWidget *glWidget = new QGLWidget;
    viewer->setViewport(glWidget);

    viewer->showExpanded();
    viewer->setResizeMode(QDeclarativeView::SizeRootObjectToView);
#endif //QT_WIDGET

    return app->exec();
}
