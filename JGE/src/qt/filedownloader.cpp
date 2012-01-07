#include "filedownloader.h"
#include <QDir>
#include <QCryptographicHash>
 
FileDownloader::FileDownloader(QString localPath, QObject *parent) :
    QObject(parent), m_received(0), m_hash(""), m_OK(false)

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
        computeHash(local);
    }
}

void FileDownloader::setDownloadUrl(QUrl url)
{
    if((!url.isEmpty()) && url.toString() != m_downloadUrl.toString())
    {
        connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                    SLOT(fileDownloaded(QNetworkReply*)));

        QNetworkRequest request(url);
        QNetworkReply* reply = m_WebCtrl.get(request);

        connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
                    SLOT(downloadProgress(qint64, qint64)));

        m_OK = m_tmp.open();

        m_downloadUrl.setUrl(url.toString());
        emit downloadUrlChanged();
    }
}


FileDownloader::~FileDownloader()
{

}

void FileDownloader::computeHash(QFile& file)
{
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    file.open(QFile::ReadOnly);
    while(!file.atEnd()){
    crypto.addData(file.read(8192));
    }
    QByteArray hash = crypto.result();

    m_hash = hash.toHex();
    emit hashChanged();
}
