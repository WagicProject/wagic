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
    Q_PROPERTY(qint64 received READ received NOTIFY receivedChanged)
public:
    explicit FileDownloader(QUrl url, QString localPath, QObject *parent = 0);
    virtual ~FileDownloader();
    qint64 received() const {return m_received;};
    bool isDone() {return m_done;};

signals:
    void downloaded();
    void receivedChanged();

private slots:
    void fileDownloaded(QNetworkReply* pReply){
        if(m_tmp.write(pReply->readAll()) == -1) return;

        if(!m_tmp.rename(m_localPath)) return;

        m_tmp.setAutoRemove(false);

        //emit a signal
        emit downloaded();
    };
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
        m_received = bytesReceived*100/bytesTotal;
        emit receivedChanged();
    };

private:

    QNetworkAccessManager m_WebCtrl;
    qint64 m_received;
    QTemporaryFile m_tmp;
    QString m_localPath;
    bool m_OK;
    bool m_done;
};
QML_DECLARE_TYPEINFO(FileDownloader, QML_HAS_ATTACHED_PROPERTIES)

#endif // FILEDOWNLOADER_H
