#ifndef RESOURCEMANAGERIMPLIMPL_H
#define RESOURCEMANAGERIMPLIMPL_H
#include <JTypes.h>
#include "MTGDeck.h"
#include "MTGCard.h"
#include "utils.h"
#include "WResourceManager.h"
#include "WCachedResource.h"
#include "WFont.h"
#include "JLogger.h"
#include <sstream>

#define HUGE_CACHE_LIMIT 20000000  // Size of the cache for Windows and Linux
#define SAMPLES_CACHE_SIZE 1500000  // Size in bytes of the cached samples
#define PSI_CACHE_SIZE 500000  // Size in bytes of the cached particles
#define TEXTURES_CACHE_MINSIZE 2000000  // Minimum size of the cache on the PSP. The program should complain if the cache ever gets smaller than this
#define OPERATIONAL_SIZE 5000000 // Size required by Wagic for operational stuff. 3MB is not enough. The cache will usually try to take (Total Ram - Operational size)
#define MIN_LINEAR_RAM 1500000
#ifdef DEBUG_CACHE
#define MAX_CACHE_TIME 2000 //The threshold above which we try to prevent nowTime() from looping.
#else
#define MAX_CACHE_TIME 2000000000 
#endif


#define THUMBNAILS_OFFSET 100000000
#define OTHERS_OFFSET 2000000000

//Hard Limits.
#define MAX_CACHE_OBJECTS 300
#define MAX_CACHE_ATTEMPTS 10
#define MAX_CACHE_MISSES 200
#define MAX_CACHED_SAMPLES 50
#define MAX_CACHE_GARBAGE 10

struct WCacheSort
{
    bool operator()(const WResource * l, const WResource * r); //Predicate for use in sorting. See flatten().
};

template <class cacheItem, class cacheActual>
class WCache
{
public:
    friend class ResourceManagerImpl;
    friend class ThreadedCardRetriever;
    friend class UnthreadedCardRetriever;

    WCache();
    ~WCache();

    cacheItem* Retrieve(int id, const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);  //Primary interface function.
    bool Release(cacheActual* actual);         //Releases an item, and deletes it if unlocked.
    bool RemoveMiss(int id=0); //Removes a cache miss.
    bool RemoveOldest();    //Remove oldest unlocked item.
    bool Cleanup();         //Repeats RemoveOldest() until cache fits in size limits
    void ClearUnlocked();   //Remove all unlocked items.
    void Refresh();         //Refreshes all cache items.
    unsigned int Flatten(); //Ensures that the times don't loop. Returns new lastTime.
    void Resize(unsigned long size, int items); //Sets new limits, then enforces them. Lock safe, so not a "hard limit".

protected:

    cacheItem* LoadIntoCache(int id, const string& filename, int submode, int style = RETRIEVE_NORMAL);

    /*
    ** Attempts a new cache item, progressively clearing cache if it fails.
    */
    cacheItem* AttemptNew(const string& filename, int submode);

    bool RemoveItem(cacheItem* item, bool force = true); //Removes an item, deleting it. if(force), ignores locks / permanent
    bool UnlinkCache(cacheItem* item); //Removes an item from our cache, does not delete it. Use with care.
    bool Delete(cacheItem* item); //SAFE_DELETE and garbage collect. If maxCached == 0, nullify first. (This means you have to free that cacheActual later!)
    cacheItem* Get(int id, const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL); //Subordinate to Retrieve.

    int makeID(int id, const string& filename, int submode);  //Makes an ID appropriate to the submode.

    inline bool RequiresOldItemCleanup()
    {
        if (cacheItems > MAX_CACHE_OBJECTS || cacheItems > maxCached || cacheSize > maxCacheSize)
        {
            std::ostringstream stream;
            if (cacheItems > MAX_CACHE_OBJECTS)
            {
                stream << "CacheItems exceeded 300, cacheItems = " << cacheItems;
            }
            else if (cacheItems > maxCached)
            {
                stream << "CacheItems (" << cacheItems << ") exceeded arbitrary limit of " << maxCached;
            }
            else
            {
                stream << "Cache size (" << cacheSize << ") exceeded max value of " << maxCacheSize;
            }

            LOG(stream.str().c_str());
            return true;
        }

#if PSPENV
        if (ramAvailableLineareMax() < MIN_LINEAR_RAM)
        {
            DebugTrace("Memory below minimum threshold!!");
            return true;
        }
#endif
        return false;
    }

    map<string,int> ids;
    map<int,cacheItem*> cache;
    map<int,cacheItem*> managed;  //Cache can be arbitrarily large, so managed items are separate.
    unsigned long totalSize;
    unsigned long cacheSize;

    //Applies to cacheSize only.
    unsigned long maxCacheSize;
    unsigned int maxCached;

    unsigned int cacheItems;
    int mError;
};

struct WManagedQuad
{
    WCachedTexture* texture;
    string resname;
};

class ResourceManagerImpl : public WResourceManager
{
public:

    ResourceManagerImpl();
    virtual ~ResourceManagerImpl();

    bool IsThreaded();

    JQuadPtr RetrieveCard(MTGCard * card, int style = RETRIEVE_NORMAL,int submode = CACHE_NORMAL);
    JSample * RetrieveSample(const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);
    JTexture * RetrieveTexture(const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);
    JQuadPtr RetrieveQuad(const string& filename, float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f,  string resname="",  int style = RETRIEVE_LOCK, int submode = CACHE_NORMAL, int id = 0);
    JQuadPtr RetrieveTempQuad(const string& filename, int submode = CACHE_NORMAL);
    hgeParticleSystemInfo * RetrievePSI(const string& filename, JQuad * texture, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);
    int RetrieveError();

    void Release(JTexture * tex);
    void Release(JSample * sample);
    bool RemoveOldest();

    void ClearUnlocked(); //Remove unlocked items.
    void Refresh();       //Refreshes all files in cache, for when mode/profile changes.

    unsigned int nowTime();

    unsigned long Size();
    unsigned long SizeCached();
    unsigned long SizeManaged();

    unsigned int Count();
    unsigned int CountCached();
    unsigned int CountManaged();

    int CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height);
    JQuadPtr GetQuad(const string &quadName);
    JQuadPtr GetQuad(int id);

    int AddQuadToManaged(const WManagedQuad& inManagedQuad);

    //Our file redirect system.
    string graphicsFile(const string& filename);
    string avatarFile(const string& filename);
    string cardFile(const string& filename);
    string musicFile(const string& filename);
    string sfxFile(const string& filename);
    int fileOK(const string&, bool relative = false);
    int dirOK(const string& dirname);

    //For backwards compatibility with JResourceManager. Avoid using these, they're not optimal.
    int CreateTexture(const string &textureName);
    JTexture* GetTexture(const string &textureName);
    JTexture* GetTexture(int id);

    // Font management functions
    void InitFonts(const std::string& inLang);
    int ReloadWFonts();
    WFont* LoadWFont(const string& inFontname, int inFontHeight, int inFontID);
    WFont* GetWFont(int id);
    void RemoveWFonts();

    //Wrapped from JSoundSystem. TODO: Privatize.
    JMusic * ssLoadMusic(const char *fileName);

    //Resets the cache limits on when it starts to purge data.
    void ResetCacheLimits();

    void DebugRender();

#ifdef DEBUG_CACHE
    unsigned long menuCached;
    string debugMessage;
#endif

private:
    bool bThemedCards;		//Does the theme have a "sets" directory for overwriting cards?
    void FlattenTimes();	//To prevent bad cache timing on int overflow

    //For cached stuff
    WCache<WCachedTexture,JTexture> textureWCache;
    WCache<WCachedSample,JSample> sampleWCache;
    WCache<WCachedParticles,hgeParticleSystemInfo> psiWCache;

    typedef std::map<std::string, WManagedQuad> ManagedQuadMap;
    ManagedQuadMap mManagedQuads;

    typedef std::map<int, std::string> IDLookupMap;
    IDLookupMap mIDLookupMap;

    //Statistics of record.
    unsigned int lastTime;
    int lastError;

    typedef std::map<int, WFont*> FontMap;
    FontMap mWFontMap;
    std::string mFontFileExtension;

};

#endif
