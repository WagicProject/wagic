#include "../include/config.h"
#include "../include/TexturesCache.h"
#include "../include/GameOptions.h"
#include <JFileSystem.h>

TexturesCache cache;

TexturesCache::TexturesCache(){
  nb_textures = 0;
  totalsize = 0;
  delete_previous = 0;
  lastTime = 0;
#ifdef WIN32
  char buf [4096];
  sprintf(buf, " Init TextureCache : %p\n", this);
  OutputDebugString(buf);
#endif
}

TexturesCache::~TexturesCache(){
  LOG("==Destroying TexturesCache==");
  for (map<string,CachedTexture*>::iterator it = cache.begin(); it != cache.end(); ++it){
    delete it->second;
  }
  LOG("==Destroying TexturesCache Successful==");
}

int TexturesCache::isInCache(MTGCard * card, int type){
  CachedTexture * tex = getCacheByCard(card, type);
  if (tex) return 1;
  return 0;
}

CachedTexture * TexturesCache::getCacheByCard(MTGCard *card, int type){
  char _filename[512];
  if (type == CACHE_THUMB){
    sprintf(_filename, "sets/%s/thumbnails/%s", card->getSetName(), card->getImageName());
  }else{
    sprintf(_filename, "sets/%s/%s", card->getSetName(), card->getImageName());
  }
  string filename = _filename;
  return cache[filename];
}


int TexturesCache::removeOldestQuad(){
  int oldest = -1;
  string result = "";
  for (map<string,CachedTexture*>::iterator it = cache.begin(); it != cache.end(); ++it){
    if (it->second && (oldest == -1 || oldest > it->second->lastTime)){
      oldest = it->second->lastTime;
      result = it->first;
    }
  }
  if (oldest != -1){
    removeQuad(result);
    return 1;
  }
  return 0;
}

void TexturesCache::removeQuad(string id){
  totalsize -= cache[id]->nbpixels;
  delete cache[id];
  cache.erase(id);
  nb_textures--;
}

int TexturesCache::cleanup(){
  int maxSize = options[Options::CACHESIZE].number * 100000;
  if (!maxSize) maxSize = CACHE_SIZE_PIXELS;
  while (totalsize > maxSize){
    int result = removeOldestQuad();
    if (!result) return 0;
  }
  return 1;
}

JQuad * TexturesCache::getQuad(string filename,MTGCard * card, int type){
  CachedTexture * ctex = cache[filename];
  if (!ctex){
    if (cleanup()){
      if (card) cache[filename] = NEW CachedTexture(card,type);
      else cache[filename] = NEW CachedTexture(filename);
      totalsize+= cache[filename]->nbpixels;
      fprintf(stderr, "Total Size of cache in pixels:  %i\n", totalsize);
      nb_textures++;
    }else{
      //Error
      return NULL;
    }
  }
  cache[filename]->lastTime = lastTime++;
  return cache[filename]->getQuad();
}

JQuad * TexturesCache::getQuad(MTGCard * card, int type){ 
  char _filename[512];
  if (type == CACHE_THUMB){
    sprintf(_filename, "sets/%s/thumbnails/%s", card->getSetName(), card->getImageName());
  }else{
    sprintf(_filename, "sets/%s/%s", card->getSetName(), card->getImageName());
  }
  string filename = _filename;
  return getQuad(filename,card,type);
}


CachedTexture::CachedTexture(string filename){
  quad = NULL;
  tex = NULL;
  nbpixels = 0;
  lastTime = 0;
    if (fileExists(filename.c_str())) init(filename);
}

CachedTexture::CachedTexture(MTGCard * card, int _type){
  LOG("==Creating CardTexture Object");
  JFileSystem* fs = JFileSystem::GetInstance();
  char filename[100];
  quad = NULL;
  tex = NULL;
  nbpixels = 0;
  lastTime = 0;
  if (type == CACHE_THUMB){
    sprintf(filename, "sets/%s/thumbnails/%s", card->getSetName(), card->getImageName());
  }else{
    sprintf(filename, "sets/%s/%s", card->getSetName(), card->getImageName());
  }

  if (fileExists(filename)){
    fs->DetachZipFile();
    init(filename);
  }else{
    char zipname[100];
    sprintf(zipname, "Res/sets/%s/%s.zip", card->getSetName(),card->getSetName());
    if (fileExists(zipname)){
      fs->AttachZipFile(zipname);
      if (type == CACHE_THUMB){
        sprintf(filename, "thumbnails/%s", card->getImageName());
      }else{
        sprintf(filename, "%s", card->getImageName());
      }
      init(filename);      
    }
  }

  LOG("CardTexture Object Creation succesful");
}

void CachedTexture::init(string filename){
  tex = JRenderer::GetInstance()->LoadTexture(filename.c_str(), false,GU_PSM_5551);
  if (tex){
    quad = NEW JQuad(tex, 0.0f, 0.0f, tex->mWidth, tex->mHeight);
    quad->SetHotSpot(tex->mWidth / 2, tex->mHeight / 2);
    nbpixels = tex->mTexHeight * tex->mTexWidth;
  }
}
JQuad * CachedTexture::getQuad(){
  return quad;
}

CachedTexture::~CachedTexture(){
  LOG("==Deleting CardTexture Object");
  SAFE_DELETE(quad);
  SAFE_DELETE(tex);
  LOG("CardTexture Object deletion Succesful");
}


SampleCache * SampleCache::mInstance = NULL;

SampleCache * SampleCache::GetInstance(){
  if (!mInstance) mInstance = NEW SampleCache();
  return mInstance;
}

JSample * SampleCache::getSample(string filename){
  lastTime++;
  map<string,SampleCached *>::iterator it = cache.find(filename);
  if (it == cache.end()){
    if (cache.size() >10) cleanOldest(); //Poor man's limit
    JSample * sample = GameApp::CommonRes->ssLoadSample(filename.c_str());
    if (!sample && fileExists(GameApp::CommonRes->sfxFile(filename).c_str())){ //Out of Ram ??
      cleanCache();
      sample = GameApp::CommonRes->ssLoadSample(filename.c_str());
    }

    cache[filename] = NEW SampleCached(lastTime, sample);
    return sample;
  }else{
    it->second->lastTime = lastTime;
    return (it->second->sample);
  }
}

void SampleCache::cleanOldest(){
  int smallest = lastTime;
  map<string,SampleCached *>::iterator found = cache.end();
  map<string,SampleCached *>::iterator it;
  for (it = cache.begin(); it != cache.end(); it++){
    if(it->second->lastTime <= smallest){
      smallest = it->second->lastTime;
      found = it;
    }
  }
  if (found != cache.end()){
    delete (found->second);
    cache.erase(found);
  }
}

void SampleCache::cleanCache(){
  map<string,SampleCached *>::iterator it;
  for (it = cache.begin(); it != cache.end(); it++){
    delete(it->second);
  }
  cache.clear();
}

SampleCache::~SampleCache(){
  cleanCache();
}

void SampleCache::DestroyInstance(){
  SAFE_DELETE(mInstance);
}
