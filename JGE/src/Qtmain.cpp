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
#include "Downloader.h"
#include "GameApp.h"
#include "qtcorewrapper.h"

WagicCore* g_glwidget = NULL;



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

    app->setApplicationName(WagicCore::getApplicationName());
    DownloadRequest* downloadRequest = NULL;
#ifdef WAGIC_RESOURCE_URL
    Downloader*downloader = Downloader::GetInstance();
    downloadRequest = downloader->Get(
                "core.zip",
                WAGIC_RESOURCE_URL
                );
#endif
#ifdef QT_WIDGET
    g_glwidget = new WagicCore();
    if(!downloadRequest || downloadRequest->getDownloadStatus() == DownloadRequest::DOWNLOADED)
    {
        g_glwidget->start(0);
    }
    else
    {
        g_glwidget->connect(downloadRequest, SIGNAL(statusChanged(int)), SLOT(start(int)));
    }
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
