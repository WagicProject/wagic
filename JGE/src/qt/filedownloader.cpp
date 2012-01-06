#include "filedownloader.h"
#include <QDir>
 
FileDownloader::FileDownloader(QString localPath, QUrl url, QObject *parent) :
    QObject(parent), m_received(0), m_OK(false), m_done(false)

{
    QDir dir(QDir::homePath());
    if(!dir.mkpath(localPath))
    {
        m_OK = false;
        return;
    }
    dir.cd(localPath);
    m_localPath = dir.filePath("core.zip");

    QFile local(m_localPath);
    if(local.exists()) {
        m_done = true;
        return;
    }
    if(!url.isEmpty())
        setDownloadUrl(url);
}

void FileDownloader::setDownloadUrl(QUrl url)
{
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                SLOT(fileDownloaded(QNetworkReply*)));

    QNetworkRequest request(url);
    QNetworkReply* reply = m_WebCtrl.get(request);

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
                SLOT(downloadProgress(qint64, qint64)));

    m_OK = m_tmp.open();
}


FileDownloader::~FileDownloader()
{

}
