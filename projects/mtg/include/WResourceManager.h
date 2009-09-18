#ifndef _WRESOURCEMANAGER_H_
#define _WRESOURCEMANAGER_H_
#include <JResourceManager.h>
#include <JSoundSystem.h>
#include <JTypes.h>
#include "MTGDeck.h"
#include "MTGCard.h"
#include "WCachedResource.h"

//Soft limits.
#define HUGE_CACHE_LIMIT 10000000
#define HUGE_CACHE_ITEMS 400

#define LARGE_CACHE_LIMIT 5000000
#define LARGE_CACHE_ITEMS 300

#define SMALL_CACHE_LIMIT 1000000
#define SMALL_CACHE_ITEMS 200


//Hard Limits.
#define MAX_CACHE_OBJECTS 400
#define MAX_CACHE_ATTEMPTS 10
#define MAX_CACHE_MISSES 200
#define MAX_CACHED_SAMPLES 0

enum ENUM_WRES_INFO{
  WRES_UNLOCKED = 0,      //Resource is unlocked.
  WRES_MAX_LOCK = 250,    //Maximum number of locks for a resource.
  WRES_PERMANENT = 251,   //Resource is permanent (ie, managed)  
  WRES_UNDERLOCKED = 252, //Resource was released too many times.
};

enum ENUM_RETRIEVE_STYLE{
  RETRIEVE_EXISTING,  //Only returns a resource if it already exists. Does not lock or unlock.
  RETRIEVE_NORMAL,    //Returns or creates a resource. Does not change lock status.
  RETRIEVE_LOCK,      //As above, locks cached resource. Not for quads.
  RETRIEVE_UNLOCK,    //As above, unlocks cached resource. Not for quads.
  RETRIEVE_RESOURCE,  //Only retrieves a managed resource. Does not make a new one.
  RETRIEVE_VRAM,      //Retrieve it, and use vram if have to we create it. Must still remove it.
  RETRIEVE_MANAGE,    //Makes resource permanent.
  RETRIEVE_THUMB,     //Retrieve it as a thumbnail.
  CACHE_THUMB = RETRIEVE_THUMB, //Backwords compatibility. 
};

enum ENUM_CACHE_SUBTYPE{
  CACHE_NORMAL =  (1<<0),          //Use default values. Not really a flag.
  CACHE_EXISTING = (1<<1),   //Retrieve it only if it already exists
  
  //Because these bits only modify how a cached resource's Attempt() is called,
  //We can use them over and over for each resource type.
  TEXTURE_SUB_CARD = (1<<2), //Retrieve using cardFile, not graphicsFile.
  TEXTURE_SUB_AVATAR = (1<<6), //Retrieve using avatarFile, not graphicsFile.
  TEXTURE_SUB_THUMB = (1<<3),//Retrieve prepending "thumbnails\" to the filename.
  TEXTURE_SUB_VRAM = (1<<4), //For textures. If we have to allocate, do it in VRAM.
  TEXTURE_SUB_5551 = (1<<5), //For textures. If we have to allocate, use RGBA5551.

};

enum ENUM_CACHE_ERROR{
  CACHE_ERROR_NONE = 0,
  CACHE_ERROR_NOT_CACHED = CACHE_ERROR_NONE,
  CACHE_ERROR_404,
  CACHE_ERROR_BAD,
  CACHE_ERROR_LOST,
  CACHE_ERROR_NOT_MANAGED,
};

template <class cacheItem, class cacheActual> 
class WCache{
public:
  friend class WResourceManager;

  WCache();
  ~WCache();
  
  cacheItem* Retrieve(string filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);  //Primary interface function.
  bool Release(cacheActual* actual);         //Releases an item, and deletes it if unlocked.
  bool RemoveMiss(string id=""); //Removes a cache miss.
  bool RemoveOldest();    //Remove oldest unlocked item.
  bool Cleanup();         //Repeats RemoveOldest() until cache fits in size limits
  void Clear();           //Removes everything cached. Not lock safe, does not remove managed items.
  void ClearUnlocked();   //Remove all unlocked items.
  void ClearMisses();     //Clear all cache misses.
  void Refresh();         //Refreshes all cache items.
  unsigned int Flatten(); //Ensures that the times don't loop. Returns new lastTime.
  void Resize(unsigned long size, int items); //Sets new limits, then enforces them. Lock safe, so not a "hard limit".

protected:
  bool RemoveItem(cacheItem * item, bool force = true); //Removes an item, deleting it. if(force), ignores locks / permanent
  bool UnlinkCache(cacheItem * item); //Removes an item from our cache, does not delete it. Use with care.
  bool Delete(cacheItem * item); //Call SAFE_DELETE on cacheItem. If maxCached == 0, nullify first. (This means you have to free that cacheActual later!)
  cacheItem* Get(string id, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL); //Subordinate to Retrieve.
  cacheItem* AttemptNew(string filename, int submode);   //Attempts a new cache item, progressively clearing cache if it fails.

  string makeID(string filename, int submode);  //Makes an ID appropriate to the submode.
  string makeFilename(string id, int submode);  //Makes a filename from an ID.

  map<string,cacheItem*> cache; 
  map<string,cacheItem*> managed;  //Cache can be arbitrarily large, so managed items are seperate.
  unsigned long totalSize;
  unsigned long cacheSize;
  
  //Applies to cacheSize only.
  unsigned long maxCacheSize;  
  unsigned int maxCached;

  unsigned int cacheItems;
  int mError;

#if defined DEBUG_CACHE
  string lastRemoved;
  string lastReleased;
  string lastExpired;
#endif
};

struct WManagedQuad {
  WCachedTexture * texture;
  string resname;
};

//This class is a wrapper for JResourceManager
class WResourceManager: public JResourceManager
{
public:
  WResourceManager();
  ~WResourceManager();
  
  JQuad * RetrieveCard(MTGCard * card, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL); //RetrieveCard is reversed to match current use.
  JSample * RetrieveSample(string filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);
  JTexture * RetrieveTexture(string filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);
  JQuad * RetrieveQuad(string filename, float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f, string resname="",  int style = RETRIEVE_LOCK, int submode = CACHE_NORMAL);
  JQuad * RetrieveTempQuad(string filename);
  hgeParticleSystemInfo * RetrievePSI(string filename, JQuad * texture, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL);

  void Release(JTexture * tex);
  void Release(JQuad * quad);
  void Release(JSample * sample);

  
  bool Cleanup();       //Force a cleanup. Return false if nothing removed.
  void ClearMisses();   //Remove all cache misses.
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
	JQuad* GetQuad(const string &quadName);
	JQuad* GetQuad(int id);

  //Our file redirect system.
  string graphicsFile(const string filename, const string specific = "");
  string avatarFile(const string filename, const string specific = "");
  string cardFile(const string filename, const string specific = "");
  string musicFile(const string filename, const string specific = "");
  string sfxFile(const string filename, const string specific = "");
  int fileOK(string filename, bool relative = false);

  //For backwards compatibility with JResourceManager. Avoid using these, they're not optimal.
  int CreateTexture(const string &textureName);
 	JTexture* GetTexture(const string &textureName);
	JTexture* GetTexture(int id);

  //Wrapped from JResourceManger. TODO: Privatize
  int LoadJLBFont(const string &fontName, int height);
 
  //Wrapped from JSoundSystem. TODO: Privatize.
  JMusic * ssLoadMusic(const char *fileName);

  void CacheForState(int state);

  void DebugRender();

#ifdef DEBUG_CACHE
  unsigned long menuCached;
  string debugMessage;
#endif

private:  
  void FlattenTimes(); //To prevent bad cache timing on int overflow

  //For cached stuff
  WCache<WCachedTexture,JTexture> textureWCache;
  WCache<WCachedSample,JSample> sampleWCache;
  WCache<WCachedParticles,hgeParticleSystemInfo> psiWCache;
  vector<WManagedQuad*> managedQuads;
  
  //Statistics of record.
  unsigned int lastTime;
};

extern WResourceManager resources;
#endif