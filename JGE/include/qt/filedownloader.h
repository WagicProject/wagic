#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#ifdef QT_WIDGET
#include <QProgressDialog>
#else
#include <qdeclarative.h>
#endif //QT_WIDGET

 
class FileDownloader :
#ifdef QT_WIDGET
        public QProgressDialog
#else
        public QObject
#endif //QT_WIDGET
{
    Q_OBJECT

    Q_PROPERTY(qint64 received READ received NOTIFY receivedChanged)
    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(DownloadState state READ getState NOTIFY stateChanged )
    Q_PROPERTY(QString state_string READ getStateString NOTIFY stateStringChanged )
    Q_ENUMS(DownloadState)
public:
    enum DownloadState {
        NETWORK_ERROR,
        DOWNLOADING_HASH,
        DOWNLOADING_FILE,
        DOWNLOADED
    } ;

    explicit FileDownloader(QString localPath, QString targetFile, QObject *parent = 0);
    virtual ~FileDownloader();
    qint64 received() const {return m_received;};
    QString getStatus() {return m_status;};
    DownloadState getState() {
        return m_state;
    };
    QString getStateString() {
        if(m_state == DOWNLOADING_HASH )
            return "DOWNLOADING_HASH";
        else if(m_state == DOWNLOADING_FILE )
            return "DOWNLOADING_FILE";
        else
            return "DOWNLOADED";
    }

signals:
    void receivedChanged(int value);
    void statusChanged();
    void stateChanged(DownloadState state);
    void stateStringChanged();

private slots:
    void fileDownloaded(){
        if(m_tmp.write(m_downloadReply->readAll()) == -1) return;
        if(QFile(m_localPath).exists())
            QFile::remove(m_localPath);

        if(!m_tmp.rename(m_localPath)) return;

        computeLocalHash(m_tmp);
        m_tmp.setAutoRemove(false);
        m_state = DOWNLOADED;
        emit stateChanged(m_state);
    };
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal){
        if(m_tmp.write(m_downloadReply->readAll()) == -1) return;
        m_received = bytesReceived*100/bytesTotal;
        emit receivedChanged(m_received);
    };
    void setDownloadUrl(QUrl url);
    void computeLocalHash(QFile& file);
    void requestHash(QUrl url);
    void computeRemoteHash();
    void handleStateChange(DownloadState state){
#ifdef QT_WIDGET
        switch(state) {
        case DOWNLOADED:
        case NETWORK_ERROR:
            emit finished(0);
            break;
        case DOWNLOADING_HASH:
            break;
        case DOWNLOADING_FILE:
            show();
            break;
        }

#else
        emit stateStringChanged();
#endif //QT_WIDGET
    };


private:
    DownloadState m_state;
    QNetworkAccessManager m_WebCtrl;
    qint64 m_received;
    QTemporaryFile m_tmp;
    QString m_targetFile;
    QString m_localPath;
    QString m_localHash;
    QString m_remoteHash;
    bool m_OK;
    QString m_status;
    QNetworkReply* m_downloadReply;
    QNetworkReply* m_hashReply;
};
#ifndef QT_WIDGET
QML_DECLARE_TYPEINFO(FileDownloader, QML_HAS_ATTACHED_PROPERTIES)
#endif  //QT_WIDGET

#endif // FILEDOWNLOADER_H
