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
    Q_PROPERTY(QUrl url READ getDownloadUrl WRITE setDownloadUrl NOTIFY downloadUrlChanged)
    Q_PROPERTY(QString hash READ getHash NOTIFY hashChanged)
public:
    explicit FileDownloader(QString localPath, QObject *parent = 0);
    virtual ~FileDownloader();
    qint64 received() const {return m_received;};
    QUrl getDownloadUrl() {return m_downloadUrl;};
    QString getHash() {return m_hash;};

signals:
    void receivedChanged();
    void downloadUrlChanged();
    void hashChanged();

private slots:
    void fileDownloaded(QNetworkReply* pReply){
        if(m_tmp.write(pReply->readAll()) == -1) return;
        if(QFile(m_localPath).exists())
            QFile::remove(m_localPath);

        if(!m_tmp.rename(m_localPath)) return;

        computeHash(m_tmp);
        m_tmp.setAutoRemove(false);
    };
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
        m_received = bytesReceived*100/bytesTotal;
        emit receivedChanged();
    };
    void setDownloadUrl(QUrl url);
    void computeHash(QFile& file);

private:

    QNetworkAccessManager m_WebCtrl;
    qint64 m_received;
    QTemporaryFile m_tmp;
    QString m_localPath;
    QUrl m_downloadUrl;
    QString m_hash;
    bool m_OK;
};
QML_DECLARE_TYPEINFO(FileDownloader, QML_HAS_ATTACHED_PROPERTIES)

#endif // FILEDOWNLOADER_H
