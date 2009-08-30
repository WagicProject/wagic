#ifndef _TEXTURES_CACHE_H
#define _TEXTURES_CACHE_H

#define CACHE_SIZE_PIXELS 2000000

#define CACHE_CARD 1
#define CACHE_THUMB 2

#include <JGE.h>
#include <JTypes.h>

#include <map>

using std::map;

#include "MTGDeck.h"

class MTGCard;

class CachedTexture{
 protected:
  JTexture* tex;
  JQuad* quad;
 public:
  int lastTime;
  int type;
  int nbpixels;


  JQuad * getQuad();

  void init(string filename);
  CachedTexture(MTGCard * card, int type);
  CachedTexture(string filename);
  ~CachedTexture();
};


class TexturesCache{
 protected:
  int lastTime;
  int nb_textures;
  int delete_previous;
  int totalsize;
  map<string,CachedTexture *> cache;
 public:
  int isInCache(MTGCard * card, int type=CACHE_CARD);
  TexturesCache();
  ~TexturesCache();
  int removeOldestQuad();
  void removeQuad(string id);
  int cleanup();
  CachedTexture * getCacheByCard(MTGCard * card, int type=CACHE_CARD);
  JQuad * getQuad(MTGCard * card, int type=CACHE_CARD);
  JQuad * getThumb(MTGCard * card){return getQuad(card, CACHE_THUMB);};
  JQuad * getQuad(string path,MTGCard * card = NULL, int type=0);
};
extern TexturesCache cache;


class SampleCached{
public:
  int lastTime;
  JSample * sample;
  SampleCached(int _lastTime, JSample * _sample):lastTime(_lastTime),sample(_sample){};
  ~SampleCached(){SAFE_DELETE(sample);};
};

class SampleCache{
protected:
  int lastTime;
  map<string,  SampleCached *> cache;
  static SampleCache * mInstance;
  void cleanCache();
  void cleanOldest();
  ~SampleCache();
public:
  static SampleCache * GetInstance();
  static void DestroyInstance();
  SampleCache(){lastTime = 0;};
  JSample * getSample(string filename);

};

#endif
