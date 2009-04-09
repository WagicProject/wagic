#include "../include/config.h"
#include "../include/TexturesCache.h"

TexturesCache::TexturesCache(){
  nb_textures = 0;
  totalsize = 0;
  delete_previous = 0;
  lastTime = 0;
  for (int i=0; i<MAX_CACHE_OBJECTS;i++){
    cache[i] = NULL;
  }
#ifdef WIN32
  char buf [4096];
  sprintf(buf, " Init TextureCache : %p\n", this);
  OutputDebugString(buf);
#endif
}

TexturesCache::~TexturesCache(){
  LOG("==Destroying TexturesCache==");
  for (int i = 0; i < nb_textures; i++){
    delete cache[i];
  }
  LOG("==Destroying TexturesCache Successful==");
}

int TexturesCache::isInCache(MTGCard * card, int type){
  int cache_id = getCacheById(card->getId(), type);
  if (cache_id == -1)
    return 0;
  return 1;
}

int TexturesCache::getCacheById(int id, int type){
  for (int i=0; i<nb_textures;i++){
    if (cache[i]->type == type && cache[i]->getId() == id){
      return i;
    }
  }
  return -1;
}

int TexturesCache::getOldestQuad(){
  int oldest = -1;
  int result = -1;
  for (int i= 0; i < nb_textures; i++){
    if (oldest == -1 || oldest > cache[i]->lastTime){
      oldest = cache[i]->lastTime;
      result = i;
    }
  }
  return result;
}

void TexturesCache::removeQuad(int id){
  totalsize -= cache[id]->nbpixels;
  delete cache[id];
  cache[id] = cache[nb_textures - 1];
  cache[nb_textures - 1] = NULL;
  nb_textures--;
}

int TexturesCache::cleanup(){
  while (nb_textures >= MAX_CACHE_OBJECTS - 1 || totalsize > CACHE_SIZE_PIXELS){
    int i = getOldestQuad();
    if (i == -1) return 0;
    removeQuad(i);
  }
  return 1;
}

JQuad * TexturesCache::getQuad(MTGCard * card, int type){
  int cache_id = getCacheById(card->getId(), type);
  if (cache_id == -1){
    
    //Not found in the cache, we have to load the file and put it in the cache
    if (cleanup()){
      cache_id = nb_textures;
      cache[cache_id] = NEW CardTexture(card, type);
      totalsize+= cache[cache_id]->nbpixels;
      fprintf(stderr, "Total Size of cache in pixels:  %i\n", totalsize);
      nb_textures++;
    }
  }
  cache[cache_id]->lastTime = lastTime++;
  return cache[cache_id]->getQuad();
}

int CardTexture::getId(){
  return mtgid;
}

CardTexture::CardTexture(MTGCard * card, int _type): type(_type){
  LOG("==Creating CardTexture Object");
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
#ifdef WIN32
  OutputDebugString(filename);
#endif
  if (fileExists(filename))
    tex = JRenderer::GetInstance()->LoadTexture(filename, false,GU_PSM_5551);
  if (tex){
    quad = NEW JQuad(tex, 0.0f, 0.0f, tex->mWidth, tex->mHeight);
    nbpixels = tex->mTexHeight * tex->mTexWidth;
  }
  mtgid = card->getId();
  LOG("CardTexture Object Creation succesful");
}

JQuad * CardTexture::getQuad(){
  return quad;
}

CardTexture::~CardTexture(){
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
    JSample * sample = JSoundSystem::GetInstance()->LoadSample(filename.c_str());
    if (!sample && fileExists(filename.c_str())){ //Out of Ram ??
      cleanCache();
      sample = JSoundSystem::GetInstance()->LoadSample(filename.c_str());
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