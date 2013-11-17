#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QTime>
#include <QApplication>
#include <QScopedPointer>
#ifdef QT_WIDGET
#include <QWidget>
#else
#include <QtDeclarative>
#include "qmlapplicationviewer.h"
#endif //QT_WIDGET
#include "filedownloader.h"
#include "GameApp.h"
#include "qtcorewrapper.h"

QWidget* g_glwidget = NULL;



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

int main(int argc, char* argv[])
{
    QScopedPointer<QApplication> app
#ifdef QT_WIDGET
        (new QApplication(argc, argv));
#else
        (createApplication(argc, argv));

#endif //QT_WIDGET

    app->setApplicationName(QtWagicCore::getApplicationName());
    FileDownloader fileDownloader(USERDIR, WAGIC_RESOURCE_NAME);
#ifdef QT_WIDGET
    g_glwidget = new QtWagicCore();
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
