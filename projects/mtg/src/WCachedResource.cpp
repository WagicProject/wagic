#include "../include/config.h"
#include "../include/utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>
#include <JFileSystem.h>
#include "../include/GameOptions.h"
#include "../include/WResourceManager.h"


//WResource
WResource::~WResource(){
  OutputDebugString("~WResource()\n");
  return;
}
WResource::WResource(){
  locks = WRES_UNLOCKED;
  lastTime = resources.nowTime();
  loadedMode = 0;
}

bool WResource::isLocked(){
  return (locks != WRES_UNLOCKED);
}

bool WResource::isPermanent(){
  return (locks == WRES_PERMANENT);
}

void WResource::deadbolt(){
  if(locks <= WRES_MAX_LOCK)
    locks = WRES_PERMANENT;
}

void WResource::lock(){
  if(locks < WRES_MAX_LOCK)
    locks++;
}

void WResource::unlock(bool force){
  if(force)
    locks = 0;
  else if(locks > WRES_UNLOCKED){
    if(locks <= WRES_MAX_LOCK)
     locks--;
  }
  else
#ifdef DEBUG_CACHE
    locks = WRES_UNDERLOCKED;
#else
    locks = 0;
#endif

}

void WResource::hit(){
  lastTime = resources.nowTime();
}
//WCachedTexture
vector<WTrackedQuad*> WCachedTexture::garbageTQs;

WCachedTexture::WCachedTexture(){
  texture = NULL;
}

WCachedTexture::~WCachedTexture(){
  if(texture)
    SAFE_DELETE(texture);

  if(!trackedQuads.size())
    return;

  vector<WTrackedQuad*>::iterator it;
  WTrackedQuad * tq = NULL;

  for(it=trackedQuads.begin();it!=trackedQuads.end();it++){
   tq = (*it);
   SAFE_DELETE(tq);
  }
  trackedQuads.clear();
}

JTexture * WCachedTexture::Actual(){
  return texture;
}
bool WCachedTexture::isLocked(){
  if(locks != WRES_UNLOCKED)
    return true;

  for(vector<WTrackedQuad*>::iterator it=trackedQuads.begin();it!=trackedQuads.end();it++){
    if((*it)->isLocked())
      return true;
  }

  return false;
}

bool WCachedTexture::ReleaseQuad(JQuad* quad){
  if(quad == NULL)
    return false;

 WTrackedQuad * tq = NULL;
 vector<WTrackedQuad*>::iterator nit;
 for(vector<WTrackedQuad*>::iterator it = trackedQuads.begin();it!=trackedQuads.end();it=nit){
   nit = it;
   nit++;
   if((*it) && (*it)->quad == quad ){
     tq = (*it);
     tq->unlock();

     if(!tq->isLocked()){
      if(WCachedTexture::garbageTQs.size() < MAX_CACHE_GARBAGE){
       tq->Trash();
       garbageTQs.push_back(tq);
      }
      else
       SAFE_DELETE(tq);

      trackedQuads.erase(it);
     }

     return true; //Returns true when found.
   }
 }
 return false;
}

WTrackedQuad * WCachedTexture::GetTrackedQuad(float offX, float offY, float width, float height,string resname){
  if(texture == NULL)
    return NULL;

  bool allocated = false;
  WTrackedQuad * tq = NULL;
  JQuad * quad = NULL;

  vector<WTrackedQuad*>::iterator it;
  std::transform(resname.begin(),resname.end(),resname.begin(),::tolower);

  if(width == 0.0f || width > texture->mWidth)
      width = texture->mWidth;
  if(height == 0.0f || height > texture->mHeight)
      height = texture->mHeight;

  for(it = trackedQuads.begin();it!=trackedQuads.end();it++){
    if((*it) && (*it)->resname == resname){
      tq = (*it);
      break;
    }
  }

  if(tq == NULL){
    allocated = true;
    vector<WTrackedQuad*>::iterator gtq = WCachedTexture::garbageTQs.begin();
    if(gtq != WCachedTexture::garbageTQs.end())
    {
      tq = *gtq;
      garbageTQs.erase(gtq);
    }
    else
      tq = NEW WTrackedQuad(resname);
  }

  if(tq == NULL)
    return NULL;

  quad = tq->quad;

  if(quad == NULL){
    quad = NEW JQuad(texture,offX,offY,width,height);
    if(!quad) {
      //Probably out of memory. Try again.
      resources.Cleanup();
      quad = NEW JQuad(texture,offX,offY,width,height);
    }

    if(!quad){
      if(allocated && tq)
        SAFE_DELETE(tq);
      return NULL; //Probably a crash.
    }

    tq->quad = quad;
    trackedQuads.push_back(tq);
    return tq;
  }
  else{
    //Update JQ's values to what we called this with.
    quad->SetTextureRect(offX,offY,width,height);
    return tq;
  }

  return NULL;
}

JQuad * WCachedTexture::GetQuad(float offX, float offY, float width, float height,string resname){
  WTrackedQuad * tq = GetTrackedQuad(offX,offY,width,height,resname);

  if(tq)
    return tq->quad;

  return NULL;
}


JQuad * WCachedTexture::GetQuad(string resname){
  vector<WTrackedQuad*>::iterator it;
  std::transform(resname.begin(),resname.end(),resname.begin(),::tolower);

  for(it = trackedQuads.begin();it!=trackedQuads.end();it++){
    if((*it) && (*it)->resname == resname){
      return (*it)->quad;
      }
  }

  return NULL;
}
JQuad * WCachedTexture::GetCard(float offX, float offY, float width, float height, string resname){
  JQuad * jq = GetQuad(offX,offY,width,height,resname);
  if(jq)
    jq->SetHotSpot(jq->mTex->mWidth / 2, jq->mTex->mHeight / 2);
  
  return jq;
}

unsigned long WCachedTexture::size(){
  if(!texture)
    return 0;
  return texture->mTexHeight*texture->mTexWidth;
}

bool WCachedTexture::isGood(){
  if(!texture)
    return false;

  return true;
}
void WCachedTexture::Refresh(string filename){
  int error = 0;
  JTexture* old = texture;
  texture = NULL;

  if(!Attempt(filename,loadedMode, error))
    SAFE_DELETE(texture);

  if(!texture)
    texture = old;
  else
    SAFE_DELETE(old);

  for(vector<WTrackedQuad*>::iterator it=trackedQuads.begin();it!=trackedQuads.end();it++){
    if((*it) && (*it)->quad)
      (*it)->quad->mTex = texture;
  }    
}

bool WCachedTexture::Attempt(string filename, int submode, int & error){
  int format = TEXTURE_FORMAT;
  loadedMode = submode;
  string realname;  

  //Form correct filename.
  if(submode & TEXTURE_SUB_CARD){
    if(submode & TEXTURE_SUB_THUMB){
      for(string::size_type i= 0;i < filename.size();i++){
        if(filename[i] == '\\' || filename[i] == '/'){
          filename.insert(i+1,"thumbnails/");
          break;
        }
      } 

    }
    realname = resources.cardFile(filename);
  }
  else{
    if(submode & TEXTURE_SUB_THUMB)
      filename.insert(0,"thumbnails/");
    
    if(submode & TEXTURE_SUB_AVATAR)
      realname = resources.avatarFile(filename);
    else
      realname = resources.graphicsFile(filename);
  }

  //Apply pixel mode
  if(submode & TEXTURE_SUB_5551)
    format = GU_PSM_5551;

  //Use Vram?
  if(submode & TEXTURE_SUB_VRAM){
    texture = JRenderer::GetInstance()->LoadTexture(realname.c_str(),TEX_TYPE_USE_VRAM,format);
  }
  else
    texture = JRenderer::GetInstance()->LoadTexture(realname.c_str(),TEX_TYPE_NORMAL,format);
  
  //Failure.
  if(!texture){
    if(!fileExists(realname.c_str()))
      error = CACHE_ERROR_404;
    return false;
  }

  //Failure of a different sort.
  if(texture->mTexId == INVALID_MTEX){
    SAFE_DELETE(texture);
    error = CACHE_ERROR_BAD;
    return false;
  }
  
  error = CACHE_ERROR_NONE;
  return true;    
} 

void WCachedTexture::Nullify(){
  if(texture)
    texture = NULL;
}
void WCachedTexture::Trash(){
    SAFE_DELETE(texture);
    
  vector<WTrackedQuad*>::iterator it;
  WTrackedQuad * tq = NULL;

  for(it=trackedQuads.begin();it!=trackedQuads.end();it++){
   tq = (*it);
   if(WCachedTexture::garbageTQs.size() > MAX_CACHE_GARBAGE)
    SAFE_DELETE(tq);
   else{
    tq->Trash();
    WCachedTexture::garbageTQs.push_back(tq);
   }
  }
  trackedQuads.clear();
}

//WCachedSample
void WCachedSample::Nullify(){
  if(sample){
    sample = NULL;
  }
}

void WCachedSample::Trash(){
  SAFE_DELETE(sample);
}

WCachedSample::WCachedSample(){
  sample = NULL;
}

WCachedSample::~WCachedSample(){
  SAFE_DELETE(sample);
}

JSample * WCachedSample::Actual(){
  return sample;
}

unsigned long WCachedSample::size(){
  if(!sample || !sample->mSample)
    return 0;

#if defined WIN32 || defined LINUX
  return FSOUND_Sample_GetLength(sample->mSample);
#else
  return sample->mSample->fileSize;
#endif
} 

bool WCachedSample::isGood(){
  if(!sample || !sample->mSample)
    return false;

  return true;
}
void WCachedSample::Refresh(string filename){
  return;
}

bool WCachedSample::Attempt(string filename, int submode, int & error){
  loadedMode = submode;

  sample = JSoundSystem::GetInstance()->LoadSample(resources.sfxFile(filename).c_str());

  if(!isGood()){
    SAFE_DELETE(sample);
    if(!fileExists(filename.c_str()))
      error = CACHE_ERROR_404;
    else
      error = CACHE_ERROR_BAD;

    return false;
  }

  return true;
}

//WCachedParticles

bool WCachedParticles::isGood(){
  if(!particles)
    return false;
  return true;
}
unsigned long WCachedParticles::size(){
  if(!particles)
    return 0; //Sizeof(pointer)

  return sizeof(hgeParticleSystemInfo);
}

//Only effects future particle systems, of course.
void WCachedParticles::Refresh(string filename){ 
  hgeParticleSystemInfo * old = particles;

  int error = 0;
  Attempt(filename,loadedMode,error);

  if(isGood())
    SAFE_DELETE(old);
  else{
    SAFE_DELETE(particles);
    particles = old;
  }

  return;
}

bool WCachedParticles::Attempt(string filename, int submode, int & error){

  JFileSystem* fileSys = JFileSystem::GetInstance();
  
  if(!fileSys->OpenFile(resources.graphicsFile(filename))){
    error = CACHE_ERROR_404;
    return false;
  }

  SAFE_DELETE(particles);

  particles = NEW hgeParticleSystemInfo;
	fileSys->ReadFile(particles, sizeof(hgeParticleSystemInfo));
	fileSys->CloseFile();

	particles->sprite=NULL;
  error = CACHE_ERROR_NONE;
  return true;
}

hgeParticleSystemInfo * WCachedParticles::Actual(){
  return particles;
}

WCachedParticles::WCachedParticles(){
  particles = NULL;  
}
WCachedParticles::~WCachedParticles(){
  SAFE_DELETE(particles);  
}

void WCachedParticles::Nullify(){
  if(particles)
    particles = NULL;
}

void WCachedParticles::Trash(){
  SAFE_DELETE(particles);
}

//WTrackedQuad
void WTrackedQuad::Nullify() {
  quad = NULL;
}

void WTrackedQuad::Trash(){
  resname.clear();
  SAFE_DELETE(quad);
}

unsigned long WTrackedQuad::size() {
  return sizeof(JQuad);
}
bool WTrackedQuad::isGood(){
  return (quad != NULL);
}
WTrackedQuad::WTrackedQuad(string _resname) {
  quad = NULL; resname = _resname;
}
WTrackedQuad::~WTrackedQuad() {
  if(quad) SAFE_DELETE(quad);
}