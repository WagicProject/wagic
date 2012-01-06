#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <qdeclarative.h>
#include <QTemporaryFile>

 
class FileDownloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool done READ isDone NOTIFY downloaded)
    Q_PROPERTY(qint64 received READ received NOTIFY receivedChanged)
    Q_PROPERTY(QUrl url READ getDownloadUrl WRITE setDownloadUrl NOTIFY downloadUrlChanged)
public:
    explicit FileDownloader(QString localPath, QUrl url=QUrl(""), QObject *parent = 0);
    virtual ~FileDownloader();
    qint64 received() const {return m_received;};
    bool isDone() {return m_done;};
    QUrl getDownloadUrl() {return m_downloadUrl;};

signals:
    void downloaded();
    void receivedChanged();
    void downloadUrlChanged();

private slots:
    void fileDownloaded(QNetworkReply* pReply){
        if(m_tmp.write(pReply->readAll()) == -1) return;

        if(!m_tmp.rename(m_localPath)) return;

        m_tmp.setAutoRemove(false);

        m_done = true;
        //emit a signal
        emit downloaded();
    };
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
        m_received = bytesReceived*100/bytesTotal;
        emit receivedChanged();
    };
    void setDownloadUrl(QUrl url);

private:

    QNetworkAccessManager m_WebCtrl;
    qint64 m_received;
    QTemporaryFile m_tmp;
    QString m_localPath;
    QUrl m_downloadUrl;
    bool m_OK;
    bool m_done;
};
QML_DECLARE_TYPEINFO(FileDownloader, QML_HAS_ATTACHED_PROPERTIES)

#endif // FILEDOWNLOADER_H
