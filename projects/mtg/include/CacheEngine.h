#include "PrecompiledHeader.h"

#include "Threading.h"
#include <queue>
#include <set>


struct CacheRequest
{
    CacheRequest()
    {
    }

    CacheRequest(std::string inFilename, int inSubmode, int inCacheID)
        : filename(inFilename), submode(inSubmode), cacheID(inCacheID)
    {
    }

    std::string filename;
    int submode;
    int cacheID;
};

const boost::posix_time::milliseconds kIdleTime(100);


class ThreadedCardRetriever
{
public:

    static void Create(WCache<WCachedTexture,JTexture>& inCache)
    {
        LOG("Creating ThreadedCardRetriever");
        sInstance = NEW ThreadedCardRetriever(inCache);
        sInstance->StartProcessing();
    }

    static ThreadedCardRetriever* Instance()
    {
        return sInstance;
    }

    static void Terminate()
    {
        SAFE_DELETE(sInstance);
    }

    ThreadedCardRetriever(WCache<WCachedTexture,JTexture>& inCache)
        : mTextureCache(inCache), mProcessing(true)
    {
    }

    ~ThreadedCardRetriever()
    {
        StopProcessing();
        mWorkerThread.join();
    }

    void QueueRequest(const std::string& inFilePath, int inSubmode, int inCacheID)
    {
        boost::mutex::scoped_lock lock(mMutex);
        // mRequestLookup is used to prevent duplicate requests for the same id
        if (mRequestLookup.find(inCacheID) == mRequestLookup.end() && mTextureCache.cache.find(inCacheID) == mTextureCache.cache.end())
        {
#ifdef DOLOG
            std::ostringstream stream;
            stream << "Queueing request: " << inFilePath;
            LOG(stream.str().c_str());
#endif
            mRequestLookup.insert(inCacheID);
            mRequestQueue.push(CacheRequest(inFilePath, inSubmode, inCacheID));

            // capping the number of queued decodes to 7, as this is 
            // the maximum # of cards we display concurrently in the deck editor.
            if (mRequestQueue.size() > 7)
            {
                int cacheIDToRemove;
                while (mRequestQueue.size() > 7)
                {
                    // pop the older requests out of the queue
                    cacheIDToRemove = mRequestQueue.front().cacheID;
                    mRequestQueue.pop();
                    mRequestLookup.erase(cacheIDToRemove);

                    assert(mRequestLookup.size() - mRequestQueue.size() < 2);
                }
            }
        }
    }

protected:
    ThreadedCardRetriever();

    static void ThreadProc(void* inParam)
    {
        LOG("Entering ThreadedCardRetriever::ThreadProc");
        ThreadedCardRetriever* instance = reinterpret_cast<ThreadedCardRetriever*>(inParam);
        if (instance)
        {
            while (instance->mProcessing)
            {
                while (!instance->mRequestQueue.empty())
                {
                    CacheRequest request;
                    {
                        boost::mutex::scoped_lock lock(instance->mMutex);
                        request = instance->mRequestQueue.front();
                        instance->mRequestQueue.pop();
                    }

                    instance->mTextureCache.LoadIntoCache(request.cacheID, request.filename, request.submode);

                    {
                        boost::mutex::scoped_lock lock(instance->mMutex);
                        instance->mRequestLookup.erase(request.cacheID);
                    }

                    // not sure this is necessary, adding it to potentially prevent SIGHUP on the psp
                    // rumour has it that if a worker thread doesn't allow the main thread a chance to run, it can hang the unit
                    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
                }

                boost::this_thread::sleep(kIdleTime);
            }
        }
    }

    void StartProcessing()
    {
        mWorkerThread = boost::thread(ThreadProc, this);
    }

    void StopProcessing()
    {
        LOG("StopProcessing called");
        mProcessing = false;
    }

    WCache<WCachedTexture,JTexture>& mTextureCache;
    boost::thread mWorkerThread;

    std::queue<CacheRequest> mRequestQueue;
    std::set<int> mRequestLookup;
    boost::mutex mMutex;
    volatile bool mProcessing;

    static ThreadedCardRetriever* sInstance;
};

ThreadedCardRetriever* ThreadedCardRetriever::sInstance = NULL;