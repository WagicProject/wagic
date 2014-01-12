#ifndef DOWNLOADER_H
#define DOWNLOADER_H

//-------------------------------------------------------------------------------------
//
// This class handles download of remote resources (any kind of file)
// All the resources are stored locally in the userPath
// For every resources, the downloader verifies if the resource was modifed
// on the server before downloading the update. The Downloader maintains a catalogue
// of resource downloaded to be able to check if they need to be updated.
//
// The interface can be used completly synchronously by the application and some
// context or message loop is needed in the implementation of this interface
//
// Note that the Downloader could in theory by implemented on top of JNetwork.
//
//-------------------------------------------------------------------------------------
#include <string>
#include <ostream>
#include <istream>
#include <fstream>
#include <map>
#include "Threading.h"
#ifdef QT_CONFIG
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#endif

using namespace std;

class DownloadRequest
        #ifdef QT_CONFIG
        : public QObject
        #endif
{
#ifdef QT_CONFIG
    Q_OBJECT

private slots:
#endif
    void fileDownloaded();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

#ifdef QT_CONFIG
signals:
    void percentChanged(int percent);
    void statusChanged(int);
#endif

public:
    typedef enum {
        NOT_PRESENT,
        DOWNLOADING,
        DOWNLOADED,
        DOWNLOAD_ERROR
    } DownloadStatus;

protected:
    string mLocalPath;
    string mRemoteResourceURL;
    // previous one is the original, next one can change after redirection
    string mRequestedRemoteResourceURL;
    string mETag;
    DownloadStatus mDownloadStatus;
    bool mUpgradeAvailable;
    uint32_t mTotalSize;
    uint32_t mCurrentSize;
    ofstream mFile;
#ifdef QT_CONFIG
    QNetworkReply* mNetworkReply;
    static QNetworkAccessManager networkAccessManager;
#endif


public:
    DownloadRequest(string localPath="",
                    string remoteResourceURL="",
                    string ETag = "",
                    DownloadStatus downloadStatus=NOT_PRESENT,
                    uint32_t totalSize = 0,
                    uint32_t currentSize = 0);
    ~DownloadRequest();
    static bool NetworkIsAccessible();

    string getTempLocalPath() const { return (mLocalPath+".tmp"); };
    string getLocalPath() const { return mLocalPath; };
    string getRemoteResource() const { return mRemoteResourceURL; };
    string getETag() const { return mETag; };
    void startGet();
    void startHead();
    DownloadStatus getDownloadStatus() const { return mDownloadStatus; };
    bool upgradeAvailable() const { return mUpgradeAvailable; };
    void getSizes(uint32_t& totalSize, uint32_t&currentSize) {
        totalSize = mTotalSize;
        currentSize = mCurrentSize;
    };

    friend ostream& operator<<(ostream& out, const DownloadRequest& d);
    friend istream& operator>>(istream&, DownloadRequest&);
    friend class Downloader;
};


class Downloader
{
protected:
    Downloader(string globalRemoteURL="", string localCacheRecords="");
    virtual ~Downloader();
    static Downloader* mInstance;
    string mGlobalRemoteURL;
    string mLocalCacheRecords;
    boost::mutex mMutex;
    map<string, DownloadRequest*> mRequestMap;

public:
    static Downloader* GetInstance();
    static void Release();

    void Update();
    DownloadRequest* Get(string localPath, string remoteResourceURL="");

    friend ostream& operator<<(ostream& out, const Downloader& d);
    friend istream& operator>>(istream&, Downloader&);
};

#endif // DOWNLOADER_H
