#include "filedownloader.h"
#include <QDir>
#include <QCryptographicHash>
 
FileDownloader::FileDownloader(QString localPath, QString targetFile, QObject *parent) :
#ifdef QT_WIDGET
    QProgressDialog("Downloading resources", "", 0, 100, (QWidget*)parent, 0),
#else
    QObject(parent),
#endif //QT_WIDGET
    m_state(NETWORK_ERROR), m_received(0), m_targetFile(targetFile), m_localHash(""), m_OK(false)

{
#ifdef QT_WIDGET
    setCancelButton(0);
    connect(this, SIGNAL(receivedChanged(int)), SLOT(setValue(int)));
    connect(this, SIGNAL(canceled()), SLOT(handleCancel()));
#endif //QT_WIDGET
    connect(this, SIGNAL(stateChanged(DownloadState)), SLOT(handleStateChange(DownloadState)));

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
        /* a file is already present in the local path */
        computeLocalHash(local);
        m_state = DOWNLOADED;
    }

    if(m_WebCtrl.networkAccessible()) {
        /* Network is OK, we request the remote hash file */
        m_state = DOWNLOADING_HASH;
        requestHash(QUrl("http://code.google.com/p/wagic/downloads/detail?name="+m_targetFile));
    }

    emit stateChanged(m_state);
}

void FileDownloader::setDownloadUrl(QUrl url)
{
    if((!url.isEmpty()))
    {
        if(!m_WebCtrl.networkAccessible()) {
            m_status = "Network not accessible, press retry when the network is accessible.";
            emit statusChanged();
            return;
        }

        QNetworkRequest request(url);
        m_downloadReply = m_WebCtrl.get(request);

        connect(m_downloadReply, SIGNAL(downloadProgress(qint64, qint64)),
                    SLOT(downloadProgress(qint64, qint64)));

        connect(m_downloadReply, SIGNAL(finished()), SLOT(fileDownloaded()));
        m_status = "Downloading Resources";

        m_OK = m_tmp.open();
    }
}


FileDownloader::~FileDownloader()
{

}

void FileDownloader::requestHash(QUrl url)
{
    QNetworkRequest request(url);
    m_hashReply = m_WebCtrl.get(request);

    connect(m_hashReply, SIGNAL(finished()), SLOT(computeRemoteHash()));
}

void FileDownloader::computeRemoteHash()
{
    if(m_hashReply->error() != QNetworkReply::NoError) {
        m_state = NETWORK_ERROR;
    } else {
        QString aString = m_hashReply->readAll();

        int index = aString.indexOf("<span id=\"sha1\">");
        m_remoteHash = aString.mid(index+16, 40);
        if(m_localHash != m_remoteHash)
        {   /* We download the real file */
            m_state = DOWNLOADING_FILE;
            setDownloadUrl(QUrl("http://wagic.googlecode.com/files/" + m_targetFile));
        }
        else
        {
            m_state = DOWNLOADED;
        }
    }
    emit stateChanged(m_state);
}

void FileDownloader::computeLocalHash(QFile& file)
{
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    QFile myFile(file.fileName());
    myFile.open(QFile::ReadOnly);
    while(!myFile.atEnd()){
        crypto.addData(myFile.read(8192));
    }
    QByteArray hash = crypto.result();
    m_localHash = hash.toHex();
}
