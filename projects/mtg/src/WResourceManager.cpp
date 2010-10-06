#include "../include/config.h"
#include "../include/DebugRoutines.h"
#include "../include/utils.h"
#include "../include/GameOptions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>
#include <JFileSystem.h>
#include <assert.h>
#include "../include/WResourceManager.h"
#include "../include/StyleManager.h"
#if defined (WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "../include/WFont.h"
extern bool neofont;

int idCounter = OTHERS_OFFSET;

WResourceManager resources;
unsigned int vTime = 0;
int WResourceManager::RetrieveError(){
  return lastError;
}
bool WResourceManager::RemoveOldest(){
 if(sampleWCache.RemoveOldest())
   return true;
 if(textureWCache.RemoveOldest())
   return true;
 if(psiWCache.RemoveOldest())
   return true;

 return false;
}

//WResourceManager
void WResourceManager::DebugRender(){
  JRenderer* renderer = JRenderer::GetInstance();
  WFont * font = resources.GetWFont(Constants::MAIN_FONT);
  font->SetColor(ARGB(255,255,255,255));
  
  if(!font || !renderer)
    return;
  
  font->SetScale(DEFAULT_MAIN_FONT_SCALE);
  renderer->FillRect(0,0,SCREEN_WIDTH,40,ARGB(128,155,0,0));

  renderer->FillRect(0,SCREEN_HEIGHT-20,SCREEN_WIDTH,40,ARGB(128,155,0,0));
  char buf[512];

  
  unsigned long man = 0;
  unsigned int misses = 0;

  if(textureWCache.cacheItems < textureWCache.cache.size())
    misses = textureWCache.cache.size()-textureWCache.cacheItems;

  if(textureWCache.totalSize > textureWCache.cacheSize)
    man = textureWCache.totalSize - textureWCache.cacheSize;

  sprintf(buf,"Textures %u+%llu (of %u) items (%u misses), Pixels: %lu (of %lu) + %lu",
          textureWCache.cacheItems, (long long unsigned int)textureWCache.managed.size(),textureWCache.maxCached,
    misses,textureWCache.cacheSize,textureWCache.maxCacheSize,man);
  font->DrawString(buf, 10,5);


#if defined (WIN32) || defined (LINUX)
#else
  int maxLinear = ramAvailableLineareMax();
  int ram = ramAvailable();
  sprintf(buf, "Ram : linear max: %i - total : %i\n",maxLinear, ram);
  font->DrawString(buf,10, 20);
#endif


  sprintf(buf,"Time: %u. Total Size: %lu (%lu cached, %lu managed). ",lastTime,Size(),SizeCached(),SizeManaged());
  font->DrawString(buf, SCREEN_WIDTH-10,SCREEN_HEIGHT-15,JGETEXT_RIGHT);

#ifdef DEBUG_CACHE
  if(debugMessage.size())
    font->DrawString(debugMessage.c_str(), SCREEN_WIDTH-10,SCREEN_HEIGHT-25,JGETEXT_RIGHT);

#endif
}

unsigned long WResourceManager::Size(){
  unsigned long res = 0;
  res += textureWCache.totalSize;
  res += sampleWCache.totalSize;
  res += psiWCache.totalSize;
  return res;
}

unsigned long WResourceManager::SizeCached(){
  unsigned long res = 0;
  res += textureWCache.cacheSize;
  res += sampleWCache.cacheSize;
  res += psiWCache.cacheSize;
  return res;  
}

unsigned long WResourceManager::SizeManaged(){
  unsigned long res = 0;
  if(textureWCache.totalSize > textureWCache.cacheSize)
    res += textureWCache.totalSize - textureWCache.cacheSize;

  if(sampleWCache.totalSize > sampleWCache.cacheSize)
    res += sampleWCache.totalSize - sampleWCache.cacheSize;

  if(psiWCache.totalSize > psiWCache.cacheSize)
    res += psiWCache.totalSize - psiWCache.cacheSize;

  return res;  
}
unsigned int WResourceManager::Count(){
  unsigned int count = 0;
  count += textureWCache.cacheItems;
  count += textureWCache.managed.size();
  count += sampleWCache.cacheItems;
  count += sampleWCache.managed.size();
  count += psiWCache.cacheItems;
  count += psiWCache.managed.size();
  return count;
}
unsigned int WResourceManager::CountCached(){
  unsigned int count = 0;
  count += textureWCache.cacheItems;
  count += sampleWCache.cacheItems;
  count += psiWCache.cacheItems;
  return count;
}
unsigned int WResourceManager::CountManaged(){
  unsigned int count = 0;
  count += textureWCache.managed.size();
  count += sampleWCache.managed.size();
  count += psiWCache.managed.size();
  return count;
}

unsigned int WResourceManager::nowTime(){
  if(lastTime > MAX_CACHE_TIME)
    FlattenTimes();
  
  return ++lastTime;
}

void WResourceManager::FlattenTimes(){
  unsigned int t;
  lastTime = sampleWCache.Flatten();  

  t = textureWCache.Flatten();
  if(t > lastTime)
    lastTime = t;

  t = psiWCache.Flatten();
  if(t > lastTime)
    lastTime = t;
}

WResourceManager::WResourceManager(){	
  DebugTrace("Init WResourceManager : " << addressof(this));
#ifdef DEBUG_CACHE
  menuCached = 0;
#endif
  mTextureList.clear();
	mTextureList.reserve(0);
	mTextureMap.clear();

	mQuadList.clear();
	mQuadList.reserve(0);
	mQuadMap.clear();

	mWFontList.clear();
	mWFontList.reserve(4);
	mWFontMap.clear();

	mWLBFontList.clear();
	mWLBFontList.reserve(4);
	mWLBFontMap.clear();

  psiWCache.Resize(PSI_CACHE_SIZE,20);      
  sampleWCache.Resize(SAMPLES_CACHE_SIZE,MAX_CACHED_SAMPLES);
  textureWCache.Resize(TEXTURES_CACHE_MINSIZE,MAX_CACHE_OBJECTS);
  lastTime = 0;
  lastError = CACHE_ERROR_NONE;

  bThemedCards = false;
}
WResourceManager::~WResourceManager(){
  LOG("==Destroying WResourceManager==");
  RemoveAll();
  RemoveWFonts();
  RemoveWLBFonts();

  for(vector<WManagedQuad*>::iterator it=managedQuads.begin();it!=managedQuads.end();it++){
    WManagedQuad* wm = *it;
    SAFE_DELETE(wm);
  }
  managedQuads.clear();
  LOG("==Successfully Destroyed WResourceManager==");
}

JQuad * WResourceManager::RetrieveCard(MTGCard * card, int style, int submode){
  //Cards are never, ever resource managed, so just check cache.
  if(!card || options[Options::DISABLECARDS].number)
    return NULL;

  submode = submode | TEXTURE_SUB_CARD;

  string filename = setlist[card->setId];
  filename += "/";
  string filename1 = filename + card->getImageName();
  int id = card->getMTGId();

  //Aliases.
  if(style == RETRIEVE_THUMB){
    submode = submode | TEXTURE_SUB_THUMB;
    style = RETRIEVE_NORMAL;
  }

  //Hack to allow either ID or card name as a filename for a given card.
  // When missing the first attempt (for example [id].jpg), the cache assigns a "404" to the card's image cache,
  // Preventing us to try a second time with [name].jpg.
  //To bypass this, we first check if the card was ever marked as "null". If not, it means it's the first time we're looking for it
  // In that case, we "unmiss" it after trying the [id].jpg, in order to give a chance to the [name.jpg]
  bool canUnmiss = false;
  {
    JQuad * tempQuad = RetrieveQuad(filename1,0,0,0,0, "",RETRIEVE_EXISTING,submode|TEXTURE_SUB_5551,id);
    lastError = textureWCache.mError;
    if (!tempQuad && lastError != CACHE_ERROR_404){
      canUnmiss = true;
    }
  }
  JQuad * jq = RetrieveQuad(filename1,0,0,0,0, "",style,submode|TEXTURE_SUB_5551,id);
  if (!jq) {
    if (canUnmiss) {
      int mId = id;
      //To differentiate between cached thumbnails and the real thing.
      if(submode & TEXTURE_SUB_THUMB){
        if (mId < 0)
          mId-=THUMBNAILS_OFFSET;
        else
          mId+=THUMBNAILS_OFFSET;
      }
      textureWCache.RemoveMiss(mId);
    }
    filename1 = filename + card->data->getName() + ".jpg";
    jq = RetrieveQuad(filename1,0,0,0,0, "",style,submode|TEXTURE_SUB_5551,id);
    int i = 0; //TODO remove debug test;
  }
  lastError = textureWCache.mError;
  if(jq){
    jq->SetHotSpot(jq->mTex->mWidth / 2, jq->mTex->mHeight / 2);  
    return jq;
  }
 
  return NULL;
}


int WResourceManager::CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height){
  if(!quadName.size() || !textureName.size())
    return INVALID_ID;
  
  string resname = quadName;

  vector<WManagedQuad*>::iterator it;
  int pos = 0;
  for(it = managedQuads.begin();it!=managedQuads.end();it++,pos++){
    if((*it)->resname == resname)
      return pos;
  }

  WCachedTexture * jtex = textureWCache.Retrieve(0,textureName,RETRIEVE_MANAGE);
  lastError = textureWCache.mError;
  
  //Somehow, jtex wasn't promoted.
  if(RETRIEVE_MANAGE && jtex && !jtex->isPermanent())
    return INVALID_ID; 

  if(jtex){
     WTrackedQuad * tq = jtex->GetTrackedQuad(x,y,width,height,quadName);  
     
    if(tq){
      tq->deadbolt();
      WManagedQuad * mq;
      mq = NEW WManagedQuad();
      mq->resname = resname;
      mq->texture = jtex;
      managedQuads.push_back(mq);
    }

    return (int) (managedQuads.size() - 1);
  }

  return INVALID_ID;
}

JQuad * WResourceManager::GetQuad(const string &quadName){
  string lookup = quadName;
  
  for(vector<WManagedQuad*>::iterator it=managedQuads.begin();it!=managedQuads.end();it++){
    if((*it)->resname == lookup)
      return (*it)->texture->GetQuad(lookup);
  }

  return NULL;
}

JQuad * WResourceManager::GetQuad(int id){
  if(id < 0 || id >= (int) managedQuads.size())
    return NULL;

  WCachedTexture * jtex = managedQuads[id]->texture;
  if(!jtex)
    return NULL;

  return jtex->GetQuad(managedQuads[id]->resname);
}

JQuad * WResourceManager::RetrieveTempQuad(string filename,int submode){
  return RetrieveQuad(filename,0,0,0,0,"temporary",RETRIEVE_NORMAL,submode);
}

JQuad * WResourceManager::RetrieveQuad(string filename, float offX, float offY, float width, float height,  string resname, int style, int submode, int id){
  JQuad * jq = NULL;

  //Lookup managed resources, but only with a real resname.
  if(resname.size() && (style == RETRIEVE_MANAGE || style == RETRIEVE_RESOURCE)){
   jq = GetQuad(resname); 
   if(jq || style == RETRIEVE_RESOURCE)
     return jq;
  }  

  //Aliases.
  if(style == RETRIEVE_THUMB){
    submode = submode | TEXTURE_SUB_THUMB;
    style = RETRIEVE_NORMAL;
  }

  //Resname defaults to filename.
  if(!resname.size())
    resname = filename;

  //No quad, but we have a managed texture for this!
  WCachedTexture * jtex = NULL;
  if(style == RETRIEVE_MANAGE || style == RETRIEVE_EXISTING)
    jtex = textureWCache.Retrieve(id,filename,style,submode);
  else
    jtex = textureWCache.Retrieve(id, filename,RETRIEVE_NORMAL,submode);

  lastError = textureWCache.mError;

  //Somehow, jtex wasn't promoted.
  if(style == RETRIEVE_MANAGE && jtex && !jtex->isPermanent())
    return NULL;  

  //Make this quad, overwriting any similarly resname'd quads.
  if(jtex){
     WTrackedQuad * tq = jtex->GetTrackedQuad(offX,offY,width,height,resname);  

    if(!tq) return NULL;

    if(style == RETRIEVE_MANAGE && resname != ""){
      WManagedQuad * mq = NEW WManagedQuad();
      mq->resname = resname;
      mq->texture = jtex;
      managedQuads.push_back(mq);
    }

    if(style == RETRIEVE_LOCK)
      tq->lock();
    else if(style == RETRIEVE_UNLOCK)
      tq->unlock();
    else if(style == RETRIEVE_MANAGE)
      tq->deadbolt();

    return tq->quad;
  }

  //Texture doesn't exist, so no quad.
  return NULL;
}
void WResourceManager::Release(JTexture * tex){
  if(!tex)
    return;

  //Copied direct from WCache::Release(). This is quick and dirty.
  map<int,WCachedTexture*>::iterator it;
  for(it=textureWCache.cache.begin();it!=textureWCache.cache.end();it++){    
    if(it->second && it->second->compare(tex))
      break;
  }

  if(it == textureWCache.cache.end())
    return; //Not here, can't release.

  if(it->second){
    it->second->unlock(); //Release one lock.
    if(it->second->locks != WRES_UNLOCKED) //Normally we'd call isLocked, but this way ignores quads.
      return; //Locked
  }

  textureWCache.Delete(it->second);
  textureWCache.cache.erase(it);
  return; //Released!
}

void WResourceManager::Unmiss(string filename){
  map<int,WCachedTexture*>::iterator it;
  int id = textureWCache.makeID(0,filename,CACHE_NORMAL);
  textureWCache.RemoveMiss(id);
}

void WResourceManager::ClearUnlocked(){
  textureWCache.ClearUnlocked();
  sampleWCache.ClearUnlocked();
  psiWCache.ClearUnlocked();
}
bool WResourceManager::Cleanup(){
  int check = 0;

  if(textureWCache.Cleanup())
    check++;
  if(sampleWCache.Cleanup())
    check++;
  if(psiWCache.Cleanup())
    check++;

  return (check > 0);
}
void WResourceManager::Release(JSample * sample){
  if(!sample)
    return;

  sampleWCache.Release(sample);
}

JTexture * WResourceManager::RetrieveTexture(string filename, int style, int submode){
  WCachedTexture * res = NULL;

  //Aliases.
  if(style == RETRIEVE_THUMB){
    submode = submode | TEXTURE_SUB_THUMB;
    style = RETRIEVE_NORMAL;
  }

  res = textureWCache.Retrieve(0,filename,style,submode);
  lastError = textureWCache.mError;

  if(res){ //a non-null result will always be good.
    JTexture * t = res->Actual();
    return t;
  }
#ifdef DEBUG_CACHE
  else{
    switch(textureWCache.mError){
      case CACHE_ERROR_NONE:
        debugMessage = "Not in cache: ";
        break;
      case CACHE_ERROR_404:
        debugMessage = "File not found: ";
        break;
      case CACHE_ERROR_BAD_ALLOC:
        debugMessage = "Out of memory: ";
        break;
      case CACHE_ERROR_BAD:
        debugMessage = "Cache bad: ";
        break;
      case CACHE_ERROR_NOT_MANAGED:
        debugMessage = "Resource not managed: ";
        break;
      case CACHE_ERROR_LOST:
        debugMessage = "Resource went bad, potential memory leak: ";
        break;
      default:
        debugMessage = "Unspecified error: ";
    }
    debugMessage += filename;
  }
#endif

  return NULL;
}

int WResourceManager::CreateTexture(const string &textureName) {
  JTexture * jtex = RetrieveTexture(textureName,RETRIEVE_MANAGE);
  
  if(jtex)
    return (int) jtex->mTexId;  //Because it's unsigned on windows/linux.

  return INVALID_ID;
}

JTexture* WResourceManager::GetTexture(const string &textureName){
  JTexture * jtex = RetrieveTexture(textureName,RETRIEVE_RESOURCE);
  return jtex;
}

JTexture* WResourceManager::GetTexture(int id){
  map<int,WCachedTexture*>::iterator it;
  JTexture *jtex = NULL;

  if(id == INVALID_ID)
    return NULL;

  for(it = textureWCache.managed.begin();it!= textureWCache.managed.end(); it++){
    if(it->second){
      jtex = it->second->Actual();
      if(id == (int) jtex->mTexId)
        return jtex;
    }
  }

  return jtex;
}

hgeParticleSystemInfo * WResourceManager::RetrievePSI(string filename, JQuad * texture, int style, int submode){
  
  if(!texture)
    return NULL;

  WCachedParticles * res = psiWCache.Retrieve(0,filename,style,submode);
  lastError = psiWCache.mError;

  if(res) //A non-null result will always be good.
  {
    hgeParticleSystemInfo * i = res->Actual();
    i->sprite = texture;
    return i;
  }

  return NULL;
}

JSample * WResourceManager::RetrieveSample(string filename, int style, int submode){
  WCachedSample * tc = NULL;
  tc = sampleWCache.Retrieve(0,filename,style,submode);
  lastError = sampleWCache.mError;

  //Sample exists! Get it.
  if(tc && tc->isGood()){
    JSample * js = tc->Actual();
    return js;  
  }

  return NULL;
}

string WResourceManager::graphicsFile(const string filename){
    char buf[512];
   
    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    //Check for a theme style renaming:
    if(filename != "style.txt"){
      WStyle * ws = options.getStyle();
      if(ws){
         sprintf(buf,"themes/%s/%s",theme.c_str(),ws->stylized(filename).c_str());
         if(fileOK(buf,true))
          return buf;
      }
    }

    if(theme != "" && theme != "Default"){
      sprintf(buf,"themes/%s/%s",theme.c_str(),filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }
/*
    //FIXME Put back when we're using modes.
    //Failure. Check mode graphics     
    string mode = options[Options::ACTIVE_MODE].str;

    if(mode != "" && mode != "Default"){
      sprintf(buf,"modes/%s/graphics/%s",mode.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }
*/       
     //Failure. Check graphics       
     char graphdir[512];
     sprintf(graphdir,"graphics/%s",filename.c_str());
      if(fileOK(graphdir,true))
        return graphdir;
    
     //Failure. Check sets.       
     sprintf(buf,"sets/%s",filename.c_str());
      if(fileOK(buf,true))
        return buf;

     //Failure. Check raw faile.       
     sprintf(buf,"%s",filename.c_str());
      if(fileOK(buf,true))
        return buf;

     //Complete abject failure. Probably a crash...
     return graphdir;
}


string WResourceManager::avatarFile(const string filename){
    char buf[512];
   
    //Check the profile folder.
    string profile = options[Options::ACTIVE_PROFILE].str;

    if(profile != "" && profile != "Default"){
      sprintf(buf,"profiles/%s/%s",profile.c_str(),filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }else{
      sprintf(buf,"player/%s",filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if(theme != "" && theme != "Default"){
      sprintf(buf,"themes/%s/%s",theme.c_str(),filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }
/*
    //FIXME Put back when we're using modes. 
    //Failure. Check mode graphics     
    string mode = options[Options::ACTIVE_MODE].str;

    if(mode != "" && mode != "Default"){
      sprintf(buf,"modes/%s/graphics/%s",mode.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }
*/

    //Failure. Check Baka
    sprintf(buf,"ai/baka/avatars/%s",filename.c_str());
     if(fileOK(buf,true))
        return buf;
       
     //Failure. Check graphics       
     sprintf(buf,"graphics/%s",filename.c_str());
      if(fileOK(buf,true))
        return buf;

     //Failure. Check raw faile.       
     sprintf(buf,"%s",filename.c_str());
      if(fileOK(buf,true))
        return buf;

     //Complete abject failure. Probably a crash...
     return "";
}

string WResourceManager::cardFile(const string filename){
    char buf[512];
    string::size_type i = 0;
    string set;
    JFileSystem* fs = JFileSystem::GetInstance();
  
    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if(theme != "" && theme != "Default"){
       //Does this theme use custom cards?
       if(bThemedCards){
           //Check zipped first. Discover set name.
           for(i = 0;i < filename.size();i++){
            if(filename[i] == '\\' || filename[i] == '/')
                break;
           }

           if(i != filename.size())
             set = filename.substr(0,i);

            if(set.size()){
              char zipname[512];
              sprintf(zipname, "Res/themes/%s/sets/%s/%s.zip", theme.c_str(), set.c_str(),set.c_str());
              if (fs->AttachZipFile(zipname))
                return filename.substr(i+1);
            }

         sprintf(buf,"themes/%s/sets/%s",theme.c_str(),filename.c_str());
         if(fileOK(buf,true)) 
           return buf; //Themed, unzipped.
       }
    }
    
//FIXME Put back when we're using modes.
/*
    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;

    if(mode != "" && mode != "Default"){
      sprintf(buf,"modes/%s/sets/%s",mode.c_str(),filename.c_str());
      if(fileOK(buf,true))
       return buf;      
    }
*/
     //Failure. Assume it's in a zip file?
     if(!set.size()){ //Didn't fill "set" string, so do it now.
       for(i = 0;i < filename.size();i++){
        if(filename[i] == '\\' || filename[i] == '/')
            break;
       }

       if(i != filename.size())
         set = filename.substr(0,i);
     }

     if(set.size()){
        char zipname[512];
        sprintf(zipname, "Res/sets/%s/%s.zip", set.c_str(),set.c_str());
        if (fs->AttachZipFile(zipname))
            return filename.substr(i+1);
     }

     //Failure. Check for unzipped file in sets       
     char defdir[512];
     sprintf(defdir,"sets/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      

     //Complete failure.
     return "";
}

string WResourceManager::musicFile(const string filename){
    char buf[512];

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if(theme != "" && theme != "Default"){
       sprintf(buf,"themes/%s/sound/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    /*
    //FIXME Put back when we're using modes.
    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;

    if(mode != "" && mode != "Default"){
      sprintf(buf,"modes/%s/sound/%s",mode.c_str(),filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }*/
       
     //Failure. Check sound       
     char defdir[512];
     sprintf(defdir,"sound/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      

     //Failure. Check raw faile.       
     sprintf(defdir,"%s",filename.c_str());
      if(fileOK(defdir,true))
        return defdir;


     //Complete abject failure. Probably a crash...
     return "";
}

string WResourceManager::sfxFile(const string filename){
    char buf[512];

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if(theme != "" && theme != "Default"){
       sprintf(buf,"themes/%s/sound/sfx/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

/*
    //FIXME: Put back when we're using modes.
    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    if(mode != "" && mode != "Default"){
      sprintf(buf,"modes/%s/sound/sfx/%s",mode.c_str(),filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }
*/       
     //Failure. Check sound       
     char defdir[512];
     sprintf(defdir,"sound/sfx/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      
    
     //Complete abject failure. Probably a crash...
     return "";
}

int WResourceManager::dirOK(string dirname){
char fname[512];

#if defined (WIN32)
  sprintf(fname,RESPATH"/%s",dirname.c_str());

  struct _stat statBuffer;
  return (_stat(fname, &statBuffer) >= 0 && // make sure it exists
  statBuffer.st_mode & S_IFDIR); // and it's not a file
#else
  sprintf(fname,RESPATH"/%s",dirname.c_str());
  struct stat st;
  if(stat(fname,&st) == 0)
    return 1;
#endif
  return 0;
}


int WResourceManager::fileOK(string filename, bool relative){
  char fname[512];
  std::ifstream * fp = NULL;
  if(relative){
    sprintf(fname,RESPATH"/%s",filename.c_str());
    fp = NEW std::ifstream(fname);
  }
  else
    fp = NEW std::ifstream(filename.c_str());

  int result = 0;
  if(fp){
    if(*fp) result = 1;
    fp->close();
    delete fp;
  }

  return result;
}

int WResourceManager::reloadWLBFonts(){
  vector<string> fontNames;
  vector<float> fontSizes;

  fontNames.resize(mWLBFontList.size());
  fontSizes.resize(mWLBFontList.size());
  for ( map<string, int>::iterator itr = mWLBFontMap.begin(); itr != mWLBFontMap.end(); ++itr){
    fontNames[itr->second] = itr->first;
    fontSizes[itr->second] = mWLBFontList[itr->second]->GetHeight();
  }
  RemoveWLBFonts();
  for(size_t i = 0; i < fontNames.size(); ++i){
    LoadWLBFont(fontNames[i],fontSizes[i]);
  }

  return 1;
}

int WResourceManager::reloadWFonts(){
  string lang = options[Options::LANG].str;
  std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

  if (lang.compare("cn") != 0) 
    RemoveWFonts();
  else if (mWFontList.size() == 0){
    resources.LoadWFBFont("simon",12);
    resources.LoadWFBFont("f3",16);
    resources.LoadWFBFont("magic",16);
    resources.LoadWFBFont("smallface",12);
  }

  return 1;
}


WFont * WResourceManager::LoadWLBFont(const string &fontName, int height) {
  map<string, int>::iterator itr = mWLBFontMap.find(fontName);

  if (itr != mWLBFontMap.end()) return mWLBFontList[itr->second];

  string mFontName = fontName + ".png";
  string path = graphicsFile(mFontName);
  if (path.size() > 4 ) path = path.substr(0, path.size() - 4); //some stupid manipulation because of the way Font works in JGE
  int id = mWLBFontList.size();
  mWLBFontList.push_back(NEW WLBFont(path.c_str(), height, true));
  mWLBFontMap[fontName] = id;
  mWLBFontList[id]->id = id;

  return mWLBFontList[id];
}

WFont * WResourceManager::LoadWFBFont(const string &fontName, int height) {
  map<string, int>::iterator itr = mWFontMap.find(fontName);
  if (itr != mWFontMap.end()) return mWFontList[itr->second];

  string mFontName = fontName + ".gbk";
  string path = graphicsFile(mFontName);
  if (path.size() > 4 ) path = path.substr(0, path.size() - 4); //some stupid manipulation because of the way WFont works in JGE

  int id = mWFontList.size();
  mWFontList.push_back(NEW WFBFont(path.c_str(), height, true));
  mWFontMap[fontName] = id;
  mWFontList[id]->id = id;

  return mWFontList[id];
}

WFont * WResourceManager::GetWFont(const string &fontName) {
  map<string, int>::iterator itr;

  if (neofont) {
    itr = mWFontMap.find(fontName);
    if (itr != mWFontMap.end())
        return mWFontList[itr->second];
    else
      return NULL;
  } else {
    itr = mWLBFontMap.find(fontName);
    if (itr != mWLBFontMap.end())
      return mWLBFontList[itr->second];
    else
      return NULL;
  }
}

WFont * WResourceManager::GetWFont(int id) {
  if (neofont) {
      return mWFontList[id];
  } else {
    if (id >=0 && id < (int)mWLBFontList.size())
      return mWLBFontList[id];
    else
      return NULL;
  }
}

WFont * WResourceManager::GetWLBFont(int id) {
  if (id >=0 && id < (int)mWLBFontList.size())
    return mWLBFontList[id];
  else
    return NULL;
}

void WResourceManager::RemoveWLBFonts() {
  for (vector<WLBFont *>::iterator font = mWLBFontList.begin(); font != mWLBFontList.end(); ++font)
    delete *font;
  mWLBFontList.clear();
  mWLBFontMap.clear();
}

void WResourceManager::RemoveWFonts() {
  for (vector<WFont *>::iterator font = mWFontList.begin(); font != mWFontList.end(); ++font)
    delete *font;
  mWFontList.clear();
  mWFontMap.clear();
}

void WResourceManager::autoResize(){
#if defined WIN32 || defined LINUX
    textureWCache.Resize(HUGE_CACHE_LIMIT,MAX_CACHE_OBJECTS);
#else
    unsigned int ram = ramAvailable();
    unsigned int myNewSize = ram - OPERATIONAL_SIZE + textureWCache.totalSize;
    if (myNewSize < TEXTURES_CACHE_MINSIZE){
      fprintf(stderr, "Error, Not enough RAM for Cache: %i - total Ram: %i\n", myNewSize, ram);
    }
    textureWCache.Resize(myNewSize,MAX_CACHE_OBJECTS);
#endif
    return;
}

JMusic * WResourceManager::ssLoadMusic(const char *fileName){
  string file = musicFile(fileName);
  if (!file.size()) return NULL;
  return JSoundSystem::GetInstance()->LoadMusic(file.c_str());
}


void WResourceManager::Refresh(){
  //Really easy cache relinking.
  reloadWFonts();
  reloadWLBFonts();
  sampleWCache.Refresh();
  textureWCache.Refresh();
  psiWCache.Refresh();

  map<string,WCachedTexture*>::iterator it;
  vector<JQuad*>::iterator q;

  //Now do some juggling so that managed resources also reload. 
  map<JTexture *,JTexture *> oldTextures;
  map<JTexture *,string> newNames;
  map<JTexture *,JTexture *>::iterator oldIt;
  vector<JTexture*>::iterator jtex;
  map<string, int>::iterator mapping;
  JTexture * newtex;
  JTexture * oldtex = NULL;

  //Store old mappings.
  for(mapping = mTextureMap.begin();mapping != mTextureMap.end();mapping++){
    if(oldTextures[mTextureList[mapping->second]] == NULL){
      newtex = JRenderer::GetInstance()->LoadTexture(graphicsFile(mapping->first).c_str(),0,TEXTURE_FORMAT);
      oldtex = mTextureList[mapping->second];
      if(!newtex)
        newNames[oldtex] = mapping->first;
      else{
        newNames[newtex] = mapping->first;
      }

      oldTextures[oldtex] = newtex;
    }
  }

  //Remap quads.
  for(q = mQuadList.begin();q!=mQuadList.end();q++){
    newtex = oldTextures[(*q)->mTex];
    if(newtex != NULL)
      (*q)->mTex = newtex;
  }

  //Rebuild mTextureList and mapping.
  mTextureList.clear();
  mTextureMap.clear();
  int x = 0;
  for(oldIt = oldTextures.begin();oldIt!=oldTextures.end();oldIt++){
    
    if(oldIt->second)
      newtex = oldIt->second;
    else
      newtex = oldIt->first;
    
    mTextureList.push_back(newtex);
    mTextureMap[newNames[newtex]] = x;
    x++;
  }
  
  //Rebuild mapping.
  for(mapping = mTextureMap.begin();mapping != mTextureMap.end();mapping++){
    if(oldTextures[mTextureList[mapping->second]] == NULL)
      continue;
  }

  //Delete unused textures.
  for(oldIt = oldTextures.begin();oldIt!=oldTextures.end();oldIt++){
    if(!oldIt->second || !oldIt->first )
      continue;

    SAFE_DELETE(oldtex);
  }

  //Check for card images in theme.
  bThemedCards = false;
  if(!options[Options::ACTIVE_THEME].isDefault()){
    char buf[512];
    sprintf(buf,"themes/%s/sets",options[Options::ACTIVE_THEME].str.c_str());
    
    if(dirOK(buf)) 
      bThemedCards = true;
  }
}

//WCache
template <class cacheItem,class cacheActual> 
bool WCache<cacheItem, cacheActual>::RemoveOldest(){
  typename map<int,cacheItem*> ::iterator oldest = cache.end();

  for(typename map<int,cacheItem *>::iterator it = cache.begin();it!=cache.end();it++){
    if(it->second && !it->second->isLocked() 
      && (oldest == cache.end() || it->second->lastTime < oldest->second->lastTime))
      oldest = it;
  }

  if(oldest != cache.end() && oldest->second && !oldest->second->isLocked()){
    Delete(oldest->second);
    cache.erase(oldest);
    return true;
  }

  return false;

}
template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::ClearUnlocked(){
  typename map<int,cacheItem*>::iterator it, next;

  for(it = cache.begin(); it != cache.end();it=next){
    next = it;
    next++;

      if(it->second && !it->second->isLocked()){
        Delete(it->second);
        cache.erase(it);
      }
      else if(!it->second){
        cache.erase(it);
      }
  }
}
template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Resize(unsigned long size, int items){
    maxCacheSize = size;

  if(items > MAX_CACHE_OBJECTS || items < 1)
    maxCached = MAX_CACHE_OBJECTS;
  else
    maxCached = items;
}


template <class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::AttemptNew(string filename, int submode){
  if(submode & CACHE_EXISTING){ //Should never get this far.
    mError = CACHE_ERROR_NOT_CACHED;
    return NULL;
  }

  cacheItem* item = NEW cacheItem;
  if(!item) {
    mError = CACHE_ERROR_BAD_ALLOC;
    return NULL;
  }

  mError = CACHE_ERROR_NONE;

  if(!item->Attempt(filename,submode,mError) || !item->isGood()) {
    //No such file. Fail
    if(mError == CACHE_ERROR_404){
      SAFE_DELETE(item); 
      return NULL;
    }
    //Probably not enough memory: cleanup and try again
    Cleanup();
    mError = CACHE_ERROR_NONE;
    if(!item->Attempt(filename,submode,mError) || !item->isGood()) {
      SAFE_DELETE(item);
      mError = CACHE_ERROR_BAD;
      return NULL;
    }
  }
  
  //Success! Enforce cache limits, then return.
  mError = CACHE_ERROR_NONE;
  item->lock();
  Cleanup();  
  item->unlock();
  return item;
}

template <class cacheItem, class cacheActual>
cacheItem * WCache<cacheItem, cacheActual>::Retrieve(int id, string filename, int style, int submode){
  //Check cache.
  cacheItem * tc = NULL;
  mError = CACHE_ERROR_NONE; //Reset error status.

  if(style == RETRIEVE_EXISTING || style == RETRIEVE_RESOURCE)
    tc = Get(id,filename,style,submode|CACHE_EXISTING);
  else
    tc = Get(id, filename,style,submode);

  //Retrieve resource only works on permanent items.
  if(style == RETRIEVE_RESOURCE && tc && !tc->isPermanent()){
    mError = CACHE_ERROR_NOT_MANAGED;
    return NULL;
  }
  
  //Perform lock or unlock on entry.
  if(tc){
    switch(style){
    case RETRIEVE_LOCK: tc->lock(); break;
    case RETRIEVE_UNLOCK: tc->unlock(); break;
    case RETRIEVE_MANAGE:
      if (!tc->isPermanent()) {
        //Unlink the managed resource from the cache.
        UnlinkCache(tc);
        
        //Post it in managed resources.
        managed[makeID(id,filename,submode)] = tc;
        tc->deadbolt();
      }
      break;
    }
  }

  //Resource exists! 
  if(tc){
    if(tc->isGood()){
      tc->hit();    
      return tc;  //Everything fine.
    }
    //Something went wrong.
    RemoveItem(tc);
    mError = CACHE_ERROR_BAD;
  }

  //Record managed failure. Cache failure is recorded in Get().
  if((style == RETRIEVE_MANAGE || style == RETRIEVE_RESOURCE) && mError == CACHE_ERROR_404)
   managed[makeID(id,filename,submode)] = NULL; 

  return NULL;
}
template <class cacheItem, class cacheActual>
int WCache<cacheItem, cacheActual>::makeID(int id, string filename, int submode){
  int mId = id;
  if (!mId) {
    mId = ids[filename];
    if (!mId){
      mId = idCounter++;
      ids[filename] = mId;
    }
  }
  
  //To differentiate between cached thumbnails and the real thing.
  if(submode & TEXTURE_SUB_THUMB){
    if (mId < 0)
      mId-=THUMBNAILS_OFFSET;
    else
      mId+=THUMBNAILS_OFFSET;
  }
  return mId;
}

template <class cacheItem, class cacheActual>
cacheItem * WCache<cacheItem, cacheActual>::Get(int id, string filename, int style, int submode){
  typename map<int,cacheItem*>::iterator it;
  int lookup = makeID(id,filename, submode);  

  //Check for managed resources first. Always
  it = managed.find(lookup);

  //Something is managed.
  if(it != managed.end()) {
    return it->second; //A hit.
  }

  //Failed to find managed resource and won't create one. Record a miss.
  if(style == RETRIEVE_RESOURCE){
    managed[lookup] = NULL;
    return NULL;
  }

  //Not managed, so look in cache.
  if(style != RETRIEVE_MANAGE){
    it = cache.find(lookup);
    //Well, we've found something...
    if(it != cache.end()){
      if(!it->second)
        mError = CACHE_ERROR_404;
      return it->second; //A hit, or maybe a miss.
    }
  }  

  //Didn't exist in cache.
  if(submode & CACHE_EXISTING ){  
    mError = CACHE_ERROR_NOT_CACHED;
    return NULL;
  }

  //Space in cache, make new texture
  cacheItem * item = AttemptNew(filename,submode);       
  
  if(style == RETRIEVE_MANAGE){
    if(mError == CACHE_ERROR_404 || item)
      managed[lookup] = item; //Record a hit or miss.
    if(item){
      item->deadbolt(); //Make permanent.
    }
  } 
  else {
    if(mError == CACHE_ERROR_404 || item)
      cache[lookup] = item;
  }

  if (!item) return NULL; //Failure

  //Succeeded in making a new item.
  unsigned long isize = item->size();   
  totalSize += isize;
    
  mError = CACHE_ERROR_NONE;
  if(style != RETRIEVE_MANAGE){
    cacheItems++;
    cacheSize += isize;
  }

  return item;
}

template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Refresh(){
  typename map<int,cacheItem*>::iterator it;
  ClearUnlocked();

  for(it = cache.begin();it!=cache.end();it++){
    if(it->second){
      it->second->Refresh();
    }
  } 
  for(it = managed.begin();it!=managed.end();it++){
    if(it->second){
      it->second->Refresh();
    }
  }
}

template <class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::WCache(){
  cacheSize = 0;
  totalSize = 0;

  maxCacheSize = TEXTURES_CACHE_MINSIZE;

  maxCached = MAX_CACHE_OBJECTS;
  cacheItems = 0;
  mError = CACHE_ERROR_NONE;
}

template <class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::~WCache(){
  typename map<int,cacheItem*>::iterator it;

  //Delete from cache & managed
  for(it=cache.begin();it!=cache.end();it++){
    SAFE_DELETE(it->second);
  }

  for(it=managed.begin();it!=managed.end();it++){
    SAFE_DELETE(it->second);
  }

}


template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Cleanup(){
  while(cacheItems < cache.size() && cache.size() - cacheItems > MAX_CACHE_MISSES){
    RemoveMiss();
  }

  while (cacheItems > MAX_CACHE_OBJECTS || cacheItems > maxCached || cacheSize > maxCacheSize
#if defined WIN32 || defined LINUX
#else
  || ramAvailableLineareMax() < MIN_LINEAR_RAM   
#endif
    ){
      if (!RemoveOldest()) {
        return false;
      }
  } 
  return true;
}

bool WCacheSort::operator()(const WResource * l, const WResource * r){
  if(!l || !r)
    return false;
  return (l->lastTime < r->lastTime);
}

template <class cacheItem, class cacheActual>
unsigned int WCache<cacheItem, cacheActual>::Flatten(){
  vector<cacheItem*> items;
  unsigned int oldest = 0;
  unsigned int lastSet = 0;

  if(!cache.size())
    return 0;

  for (typename map<int,cacheItem*>::iterator it = cache.begin(); it != cache.end(); ++it){
    if(!it->second) continue;
    items.push_back(it->second);
  }

  sort(items.begin(), items.end(), WCacheSort());

  for (typename vector<cacheItem*>::iterator it = items.begin(); it != items.end(); ++it){
    assert((*it) && (*it)->lastTime > lastSet);
    lastSet = (*it)->lastTime; 
    (*it)->lastTime = ++oldest;
  }

  return oldest + 1;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveMiss(int id){
  typename map<int,cacheItem*>::iterator it = cache.end();

  for(it = cache.begin();it!=cache.end();it++){
    if((id == 0 || it->first == id) && it->second == NULL)
          break;
  }

  if(it != cache.end())
  {
    cache.erase(it);
    return true; 
  }

  return false;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveItem(cacheItem * item, bool force){
  typename map<int,cacheItem*>::iterator it;

  if(item == NULL)
    return false;   //Use RemoveMiss to remove cache misses, not this.

  for(it = cache.begin();it!=cache.end();it++){
    if(it->second == item)
      break;
  }
  if(it != cache.end() && it->second && (force || !it->second->isLocked())){
    Delete(it->second);
    cache.erase(it);
    return true;
  }

  return false;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::UnlinkCache(cacheItem * item){
  typename map<int,cacheItem*>::iterator it = cache.end();

  if(item == NULL)
    return false;   //Use RemoveMiss to remove cache misses, not this.

  for(it = cache.begin();it!=cache.end();it++){
    if(it->second == item)
      break;
  }
  if(it != cache.end() && it->second){
    it->second = NULL;  
    unsigned long isize = item->size();

    cacheSize -= isize; 
    cacheItems--;
    cache.erase(it);
    return true;
  }

  return false;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Delete(cacheItem * item){
  if(!item)
    return false;

  unsigned long isize = item->size();
  totalSize -= isize;
  cacheSize -= isize;
#ifdef DEBUG_CACHE
  if(cacheItems == 0)
    DebugTrace("cacheItems out of sync.");
#endif

  cacheItems--;

  SAFE_DELETE(item);
  return true;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Release(cacheActual* actual){
  if(!actual)
    return false;

  typename map<int,cacheItem*>::iterator it;
  for(it=cache.begin();it!=cache.end();it++){    
    if(it->second && it->second->compare(actual))
      break;
  }
  
  if(it == cache.end())
    return false; //Not here, can't release.

  if(it->second){
    it->second->unlock(); //Release one lock.
    if(it->second->isLocked())
      return true; //Still locked, won't delete, not technically a failure.
  }

  //Released!
  Delete(it->second);
  cache.erase(it);
  return true;
}
