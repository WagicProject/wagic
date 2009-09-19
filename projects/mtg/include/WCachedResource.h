#ifndef _WCACHEDRESOURCE_H_
#define _WCACHEDRESOURCE_H_
#include <hge/hgeparticle.h>

#if defined WIN32 || defined LINUX
#define INVALID_MTEX ((GLuint) -1)
#else
#define INVALID_MTEX -1
#endif

class WResource{
public:
  friend class WResourceManager;
  template<class cacheItem,class cacheActual> friend class WCache;

  WResource();
  virtual ~WResource();
  
  virtual void Trash()=0; //Delete the cacheActual.
  virtual void Nullify()=0; //For when our size is 0, so we don't free anything by mistake.
  virtual unsigned long size()=0; //Size of cached item in bytes.
  virtual bool isGood()=0;  //Return true if this has data.
  virtual bool isLocked();    //Is the resource locked?
  virtual bool isTrash();    //Is the resource locked?
  virtual void lock();    //Lock it.
  virtual void unlock(bool force = false);  //Unlock it. Forcing a lock will also remove "permanent" status.

  bool isPermanent(); //Is the resource permanent?
  void deadbolt();    //Make it permanent.
  void hit();     //Update resource's last used time.

protected:
  string id;        //Our lookup value.
  int loadedMode;   //What submode settings were we loaded with? (For refresh)
  unsigned int lastTime;  //When was the last time we were hit?
  unsigned char locks; //Remember to unlock when we're done using locked stuff, or else this'll be useless.
};

class WCachedResource: public WResource {
public:
  friend class WResourceManager;
  template<class cacheItem,class cacheActual> friend class WCache;

  virtual ~WCachedResource() {};

  virtual void Refresh(string filename)=0; //Basically calls Attempt(filename) and remaps in situ.
  virtual bool Attempt(string filename, int submode, int & error)=0;  //Returns true if we've loaded our data and isGood().
};


class WTrackedQuad: public WResource {
public:
  WTrackedQuad(string _resname);
  ~WTrackedQuad();
  void Nullify();
  void Trash();
  unsigned long size();
  bool isGood();

  string resname;
  JQuad * quad;
#ifdef DEBUG_CACHE
  static int totalTracked;
#endif
};

class WCachedTexture: public WCachedResource{
public:
  friend class WResourceManager;
  template<class cacheItem,class cacheActual> friend class WCache;
  WCachedTexture();
  ~WCachedTexture();

  void Refresh(string filename);
  unsigned long size();  
  bool isGood(); 
  bool isLocked();
  bool Attempt(string filename, int submode, int & error);
  bool compare(JTexture * t) {return (t == texture);};

  void Nullify();
  void Trash();
  JTexture * Actual(); //Return this texture as is. Does not make a new one.
  JQuad * GetQuad(string resname);
  
  WTrackedQuad* GetTrackedQuad(float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f,string resname=""); //Get us a new/existing quad.

  JQuad * GetQuad(float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f,string resname=""); //Alias to GetTrackedQuad.
  JQuad * GetCard(float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f,string resname=""); //Same as above, but centered when new.

  bool ReleaseQuad(JQuad* quad); //We're done with this quad, so delete and stop tracking. True if existed.
protected:  
  JTexture * texture;
  bool bVRAM;
  vector<WTrackedQuad*> trackedQuads;
  static vector<WTrackedQuad*> garbageTQs;
};

class WCachedParticles: public WCachedResource{
public:
  friend class WResourceManager;
  template<class cacheItem,class cacheActual> friend class WCache;
  WCachedParticles();
  ~WCachedParticles();

  void Nullify();
  void Trash();
  void Refresh(string filename);
  unsigned long size();  
  bool isGood();
  bool Attempt(string filename, int submode, int & error);
  bool compare(hgeParticleSystemInfo * p) {return (p == particles);};

  hgeParticleSystemInfo * Actual();
protected:  
  hgeParticleSystemInfo * particles;
};

class WCachedSample: public WCachedResource{
public:
  friend class WResourceManager;
  template<class cacheItem,class cacheActual> friend class WCache;
  WCachedSample();
  ~WCachedSample();    
  void Nullify();
  void Trash();
  bool compare(JSample * s) {return (s == sample);};
  unsigned long size();  
  bool isGood();
  void Refresh(string filename);
  bool Attempt(string filename, int submode, int & error);

  JSample * Actual(); //Return this sample.
protected:  
  JSample * sample;
};

#endif