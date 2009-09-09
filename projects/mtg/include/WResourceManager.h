#ifndef _WRESOURCEMANAGER_H_
#define _WRESOURCEMANAGER_H_
#include <JResourceManager.h>
#include <JSoundSystem.h>
#include <JTypes.h>
#include "MTGDeck.h"
#include "MTGCard.h"

#define CACHE_SIZE_PIXELS 2000000
#define MAX_CACHE_OBJECTS 200

class WCachedResource{
public:
  friend class WResourceManager;
  bool isLocked(); //Is the resource locked?
  void lock();    //Lock it.
  void unlock(bool force = false);  //Unlock it. If force, then set locks to 0.
  void hit(); //Update resource last used time.

  WCachedResource();

protected:
  unsigned int lastTime;
  unsigned char locks; //Remember to unlock when we're done using locked stuff, or else this'll be useless.
};

class WCachedTexture: public WCachedResource{
public:
  friend class WResourceManager;
  WCachedTexture();
  ~WCachedTexture();

  JTexture * GetTexture(); //Return this texture as is. Does not make a new one.
  JQuad * GetQuad(float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f); //Get us a new/existing quad.
  JQuad * GetCard(float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f); //Same as above, but centered when new.
  bool ReleaseQuad(JQuad* quad); //We're done with this quad, so delete and stop tracking. True if existed.
protected:  
  JTexture * texture;
  bool bVRAM;
  vector<JQuad*> trackedQuads;
};

class WCachedSample: public WCachedResource{
public:
  friend class WResourceManager;
  WCachedSample();
  ~WCachedSample();

  JSample * GetSample(); //Return this sample.
protected:  
  JSample * sample;
};

enum ENUM_RETRIEVE_STYLE{
  RETRIEVE_EXISTING,  //Only returns a resource if it already exists. Does not lock or unlock.
  RETRIEVE_NORMAL,    //Returns or creates a resource. Does not change lock status.
  RETRIEVE_LOCK,      //As above, locks cached resource.
  RETRIEVE_UNLOCK,    //As above, unlocks cached resource.
  RETRIEVE_RESOURCE,  //Only retrieves a managed resource.
  RETRIEVE_VRAM,      //If we create the texture, use vram. Also locks.
  RETRIEVE_MANAGE,    //Permanently adds retrieved resource to resource manager.
};

enum ENUM_CACHE_SUBTYPE{
  CACHE_CARD,
  CACHE_THUMB
};


//This class is a wrapper for JResourceManager
class WResourceManager: public JResourceManager
{
public:
  WResourceManager();
  ~WResourceManager();
  
  JQuad * RetrieveCard(MTGCard * card, int type = CACHE_CARD, int style = RETRIEVE_NORMAL);
  JSample * RetrieveSample(string filename, int style = RETRIEVE_NORMAL);
  JTexture * RetrieveTexture(string filename, int style = RETRIEVE_NORMAL);
  JQuad * RetrieveQuad(string filename, float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f, string resname="", int style = RETRIEVE_NORMAL);
  void Release(JTexture * tex);
  void Release(JQuad * quad);
  void Release(JSample * sample);

  void ClearMisses();
  void ClearUnlocked();
  void ClearSamples();
  void Refresh(); //Refreshes all files in cache, for when mode/profile changes.

  unsigned int nowTime();

  //Our file redirect system.
  string graphicsFile(const string filename, const string specific = "");
  string cardFile(const string filename, const string setname, const string specific = "");
  string musicFile(const string filename, const string specific = "");
  string sfxFile(const string filename, const string specific = "");
  int fileOK(string filename, bool relative = false);

  
  //Not part of our interface, but left public to maintain JResourceManager compatibility
  //These are for managed resources only.
  int CreateTexture(const string &textureName);
  int CreateQuad(const string &quadName, const string &textureName, float x=0.0f, float y=0.0f, float width=0.0f, float height=0.0f);
  int LoadJLBFont(const string &fontName, int height);
  int LoadMusic(const string &musicName);
  int LoadSample(const string &sampleName);
 
  //Wrapped from JSoundSystem. TODO: Privatize.
  JMusic * ssLoadMusic(const char *fileName);

private:
  bool RemoveOldestTexture();
  bool RemoveOldestSample();
  bool cleanup();

  WCachedTexture * getCachedTexture(string filename, bool makenew = true, int mode = 0, int format = TEXTURE_FORMAT);
  WCachedTexture * getCachedCard(MTGCard * card, int type = CACHE_CARD, bool makenew = true);
  WCachedSample * getCachedSample(string filename, bool makenew = true);

  void FlattenTimes(); //To prevent bad cache timing on int overflow
  //For cached stuff
  map<string,WCachedTexture*> textureCache;
  map<string,WCachedSample*> sampleCache;
  
  vector<string> mTextureMissing; //For managed textures.

  //Current access time.
  int lastTime;
  //Statistics of record.
  int nb_textures;
  long totalsize;
};

extern WResourceManager resources;
#endif