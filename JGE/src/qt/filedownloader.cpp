#include "filedownloader.h"
 
FileDownloader::FileDownloader(QUrl url, QString localPath, QObject *parent) :
    QObject(parent), m_received(0), m_localPath(localPath), m_OK(false), m_done(false)

{
    QFile local(m_localPath);
    if(local.exists()) {
        m_done = true;
        return;
    }
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