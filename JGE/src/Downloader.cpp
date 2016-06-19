#include "DebugRoutines.h"
#include "JFileSystem.h"
#include "Downloader.h"

#define RECORDS_DEFAULT_FILE "cache/records.txt"

#ifdef QT_CONFIG
QNetworkAccessManager DownloadRequest::networkAccessManager;
#endif
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

DownloadRequest::DownloadRequest(string localPath,
                    string remoteResourceURL,
                    string ETag,
                    DownloadStatus downloadStatus,
                    size_t totalSize,
                    size_t currentSize):
    mLocalPath(localPath),
    mRemoteResourceURL(remoteResourceURL),
    mRequestedRemoteResourceURL(remoteResourceURL),
    mETag(ETag),
    mDownloadStatus(downloadStatus),
    mUpgradeAvailable(false),
    mTotalSize(totalSize),
    mCurrentSize(currentSize)
{
}

DownloadRequest::~DownloadRequest()
{
}

void DownloadRequest::startHead()
{
#ifdef QT_CONFIG
    QNetworkRequest request(QUrl(QString(mRequestedRemoteResourceURL.c_str())));
    request.setRawHeader("If-None-Match", mETag.c_str());
    mNetworkReply = networkAccessManager.head(request);
    connect(mNetworkReply, SIGNAL(finished()), SLOT(fileDownloaded()));
#endif
}

void DownloadRequest::startGet()
{
#ifdef QT_CONFIG
    mNetworkReply = networkAccessManager.get(QNetworkRequest(QUrl(QString(mRequestedRemoteResourceURL.c_str()))));
#endif
    mFile.close();
    JFileSystem::GetInstance()->Remove(getTempLocalPath());
    JFileSystem::GetInstance()->openForWrite(mFile, getTempLocalPath());
#ifdef QT_CONFIG
    connect(mNetworkReply, SIGNAL(downloadProgress(int64_t, int64_t)),
                SLOT(downloadProgress(int64_t, int64_t)));
    connect(mNetworkReply, SIGNAL(finished()), SLOT(fileDownloaded()));
#endif

#ifdef __EMSCRIPTEN__
     emscripten_async_wget2_data(mRemoteResourceURL.c_str(), "GET", 0, this, 1,
                                 (em_async_wget2_data_onload_func)DownloadRequest::onLoadCb,
                                 (em_async_wget2_data_onerror_func)DownloadRequest::onErrorCb,
                                 (em_async_wget2_data_onprogress_func)DownloadRequest::onProgressCb);

#endif
}


#ifdef __EMSCRIPTEN__
void DownloadRequest::onLoadCb(unsigned int handle, DownloadRequest* req, const char *buffer, unsigned int size)
{

    DebugTrace("DownloadRequest::onLoadCb: " << size);
    req->processBufferDownloaded(size, buffer);
    Downloader::GetInstance()->Update();
}

void DownloadRequest::onErrorCb(unsigned int handle, DownloadRequest* req, int errorCode, const char* errorText)
{
    DebugTrace("DownloadRequest::onErrorCb");
    req->processError(errorCode, errorText);
    Downloader::GetInstance()->Update();
}

void DownloadRequest::onProgressCb(unsigned int handle, DownloadRequest* req, int bytesReceived, int bytesTotal)
{
    DebugTrace("DownloadRequest::onProgressCb");
    req->DownloadRequest::downloadProgress(bytesReceived, bytesTotal);
}
#endif

void DownloadRequest::processError(int errorCode, const char* errorText)
{
  DebugTrace(errorText);
  mDownloadStatus = DownloadRequest::DOWNLOAD_ERROR;
  mFile.close();
  JFileSystem::GetInstance()->Remove(getTempLocalPath());
}

void DownloadRequest::processBufferDownloaded(unsigned int size, const char*buffer)
{
  if(mFile.is_open())
  {
      mTotalSize = size;
      mFile.write(buffer, size);
      mFile.close();
      if(!JFileSystem::GetInstance()->Rename(getTempLocalPath(), mLocalPath)) {
          mDownloadStatus = DownloadRequest::DOWNLOAD_ERROR;
          return;
      }
  }
  mDownloadStatus = DownloadRequest::DOWNLOADED;
}

void DownloadRequest::fileDownloaded()
{
#ifdef QT_CONFIG
  do {
        QByteArray eTagByteArray = mNetworkReply->rawHeader("ETag");
        if(!eTagByteArray.isEmpty()) {
            string oldETag = mETag;
            mETag = QString(eTagByteArray).toStdString();
            if(oldETag!="" && oldETag != mETag)
                mUpgradeAvailable = true;
        }

        // let's check some error
        if(mNetworkReply->error() != QNetworkReply::NoError) {
            processError((int)mNetworkReply->error(), mNetworkReply->errorString().toStdString().c_str());
            break;
        }

        // check if we're getting redirected
        QVariant redirectionTarget = mNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (!redirectionTarget.isNull()) {
            QUrl newUrl = QUrl(mRequestedRemoteResourceURL.c_str()).resolved(redirectionTarget.toUrl());
            DebugTrace(string("Redirect to ")+ newUrl.toString().toStdString());

            mRequestedRemoteResourceURL = newUrl.toString().toStdString();
            mNetworkReply->deleteLater();
            if(mFile.is_open())
                startGet();
            else
                startHead();
            return;
        }

        QByteArray byteArray = mNetworkReply->readAll();
        processBufferDownloaded(byteArray.size(), byteArray.constData());
    } while(0);

    Downloader::GetInstance()->Update();

    mNetworkReply->deleteLater();
    emit statusChanged((int)mDownloadStatus);
#endif 
}

void DownloadRequest::downloadProgress(int64_t bytesReceived, int64_t bytesTotal)
{
#ifdef QT_CONFIG
	QByteArray byteArray = mNetworkReply->readAll();
    mFile.write(byteArray.constData(), byteArray.size());
#endif
    mCurrentSize = bytesReceived;
    mTotalSize = bytesTotal;
    int percent = 0;
    if(bytesTotal)
        percent = (bytesReceived/bytesTotal)*100;
#ifdef QT_CONFIG
    emit percentChanged(percent);
#endif
}

Downloader* Downloader::mInstance = 0;

Downloader::Downloader(string globalRemoteURL, string localcacheRecords):
    mGlobalRemoteURL(globalRemoteURL),
    mLocalCacheRecords(localcacheRecords)
{
    JFileSystem::GetInstance()->MakeDir("cache");

    izfstream downloadRecords;
    if(mLocalCacheRecords.empty())
        mLocalCacheRecords = RECORDS_DEFAULT_FILE;
    if(JFileSystem::GetInstance()->openForRead(downloadRecords, mLocalCacheRecords))
    {// File exists, let's read it.
        downloadRecords >> (*this);
    }
    JFileSystem::GetInstance()->CloseFile();
}

Downloader::~Downloader()
{
    map<string, DownloadRequest*>::iterator ite;
    for(ite = mRequestMap.begin(); ite != mRequestMap.end(); ite++)
    {
        delete (*ite).second;
    }
    mRequestMap.erase(mRequestMap.begin(), mRequestMap.end());
}

Downloader* Downloader::GetInstance()
{
    if(!mInstance)
    {
        mInstance = new Downloader();
    }
    return mInstance;
}

void Downloader::Release()
{
    if(mInstance)
    {
        delete mInstance;
        mInstance = 0;
    }
}

bool DownloadRequest::NetworkIsAccessible()
{
    bool result = false;

#ifdef QT_CONFIG
    networkAccessManager.setNetworkAccessible(QNetworkAccessManager::Accessible);
    result = networkAccessManager.networkAccessible();
#endif

    return result;
}

DownloadRequest* Downloader::Get(string localPath, string remoteResourceURL)
{
    map<string, DownloadRequest*>::iterator ite = mRequestMap.find(localPath);
    if(ite == mRequestMap.end())
    { // request does not exist, let's create it
        DownloadRequest* request = new DownloadRequest(localPath, remoteResourceURL);
        std::pair<std::map<string,DownloadRequest*>::iterator,bool> ret;
        ret = mRequestMap.insert ( std::pair<string,DownloadRequest*>(localPath, request) );
        if (ret.second==false) {
            DebugTrace("Downloader::Get Error inserting request in Map");
            return 0;
        }
        ite = ret.first;
    }

    // Now, we can check the server
    if((*ite).second->getDownloadStatus() == DownloadRequest::NOT_PRESENT ||
            (*ite).second->upgradeAvailable())
    {   // File is not here or an update is available, let's get it
        (*ite).second->startGet();
        (*ite).second->mDownloadStatus = DownloadRequest::DOWNLOADING;
    }
    else if ((*ite).second->getDownloadStatus() == DownloadRequest::DOWNLOADED)
    {   // File is here, let's check if there is some update without blocking the playback
        (*ite).second->startHead();
    }

    return (*ite).second;
}

void Downloader::Update()
{
    ofstream downloadRecords;
    if(JFileSystem::GetInstance()->openForWrite(downloadRecords, mLocalCacheRecords))
    {
        downloadRecords << (*this);
    }
    downloadRecords.close();
}

ostream& operator<<(ostream& out, const DownloadRequest& d)
{
//    HEAD request fails, so this line erase cache record after upgrade check :(
//    if(d.getDownloadStatus() == DownloadRequest::DOWNLOADED)
    {
        out << "localPath=" << d.mLocalPath << endl;
        out << "remoteResource=" << d.mRemoteResourceURL << endl;
        out << "ETag=" << d.mETag << endl;
        out << "upgradeAvailable=" << d.mUpgradeAvailable <<endl;
    }

    return out;
}

istream& operator>>(istream& in, DownloadRequest& d)
{
    string s;

    while(std::getline(in, s))
    {
        size_t limiter = s.find("=");
        string areaS;
        if (limiter != string::npos)
        {
            areaS = s.substr(0, limiter);
            if (areaS.compare("localPath") == 0)
            {
                d.mLocalPath = s.substr(limiter + 1);
            }
            else if (areaS.compare("remoteResource") == 0)
            {
                d.mRemoteResourceURL = s.substr(limiter + 1);
                d.mRequestedRemoteResourceURL = d.mRemoteResourceURL;
            }
            else if (areaS.compare("ETag") == 0)
            {
                d.mETag = s.substr(limiter + 1);
                d.mDownloadStatus = DownloadRequest::DOWNLOADED;
            }
            else if (areaS.compare("upgradeAvailable") == 0)
            {
                d.mUpgradeAvailable = (bool)atoi(s.substr(limiter + 1).c_str());
                break;
            }
        }
    }

    return in;
}

ostream& operator<<(ostream& out, const Downloader& d)
{
    map<string, DownloadRequest*>::const_iterator ite;
    for(ite = d.mRequestMap.begin(); ite != d.mRequestMap.end(); ite++)
    {
        out << (*(*ite).second) << endl;
    }
    return out;
}

istream& operator>>(istream& in, Downloader& d)
{
    while(!in.eof())
    {
        DownloadRequest* downloadRequest = new DownloadRequest();
        in >> (*downloadRequest);

        if(!downloadRequest->getLocalPath().empty() &&
           !downloadRequest->getRemoteResource().empty() &&
           !downloadRequest->getETag().empty()) {
            d.mRequestMap[downloadRequest->getLocalPath()] = downloadRequest;
        } else {
            delete downloadRequest;
        }
    }
    return in;
}

