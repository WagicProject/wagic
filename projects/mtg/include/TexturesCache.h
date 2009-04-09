#ifndef _TEXTURES_CACHE_H
#define _TEXTURES_CACHE_H

#define MAX_CACHE_OBJECTS 100
#define CACHE_SIZE_PIXELS 6000000

#define CACHE_CARD 1
#define CACHE_THUMB 2

#include <JGE.h>
#include <JTypes.h>

#include <map>

using std::map;

#include "MTGDeck.h"

class MTGCard;

class CardTexture{
 protected:
  int mtgid;

  JTexture* tex;
  JQuad* quad;
 public:
  int lastTime;
  int type;
  int nbpixels;
  int getId();

  JQuad * getQuad();

  CardTexture(MTGCard * card, int type);
  ~CardTexture();
};


class TexturesCache{
 protected:
  int lastTime;
  int nb_textures;
  int delete_previous;
  int totalsize;
  CardTexture * cache[MAX_CACHE_OBJECTS];
 public:
  int isInCache(MTGCard * card, int type=CACHE_CARD);
  TexturesCache();
  ~TexturesCache();
  int getOldestQuad();
  void removeQuad(int id);
  int cleanup();
  int getCacheById(int id, int type=CACHE_CARD);
  JQuad * getQuad(MTGCard * card, int type=CACHE_CARD);
  JQuad * getThumb(MTGCard * card){return getQuad(card, CACHE_THUMB);};

};


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
