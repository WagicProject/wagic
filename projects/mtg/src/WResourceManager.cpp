#include "../include/config.h"
#include "../include/utils.h"
#include "../include/GameOptions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>
#include <JFileSystem.h>
#include "../include/WResourceManager.h"

WResourceManager resources;
unsigned int vTime = 0;

void handle_new_failure(){
  OutputDebugString("NEW failed. Attempting to clear cache.");
#ifdef DEBUG_CACHE
  resources.debugMessage = "Emergency cache cleanup!";
#endif
  if(!resources.RemoveOldest()){
    OutputDebugString("Nothing to clear from cache. Abort.");
    abort();
  }
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
int WResourceManager::RetrieveError(){
  return lastError;
}
void WResourceManager::DebugRender(){
  JRenderer* renderer = JRenderer::GetInstance();
  JLBFont * font = resources.GetJLBFont(Constants::MAIN_FONT);
  
  if(!font || !renderer)
    return;
  
  font->SetScale(DEFAULT_MAIN_FONT_SCALE);
  renderer->FillRect(0,0,SCREEN_WIDTH,20,ARGB(128,155,0,0));
#ifdef DEBUG_CACHE
  if(debugMessage.size())
    renderer->FillRect(0,SCREEN_HEIGHT-30,SCREEN_WIDTH,40,ARGB(128,155,0,0));
  else
#endif
    renderer->FillRect(0,SCREEN_HEIGHT-20,SCREEN_WIDTH,40,ARGB(128,155,0,0));
  char buf[512];

  
  unsigned long man = 0;
  
  if(textureWCache.totalSize > textureWCache.cacheSize)
    man = textureWCache.totalSize - textureWCache.cacheSize;

  sprintf(buf,"Textures %u+%u (of %u) items (%u misses), Pixels: %lu (of %lu) + %lu",
    textureWCache.cacheItems, textureWCache.mManaged.size(),textureWCache.maxCached,
    textureWCache.mMisses.size(),textureWCache.cacheSize,textureWCache.maxCacheSize,man);
  font->DrawString(buf, 10,5);



  sprintf(buf,"Total Size: %lu (%lu cached, %lu managed)",Size(),SizeCached(),SizeManaged());
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
  count += textureWCache.mManaged.size();
  count += sampleWCache.cacheItems;
  count += sampleWCache.mManaged.size();
  count += psiWCache.cacheItems;
  count += psiWCache.mManaged.size();
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
  count += textureWCache.mManaged.size();
  count += sampleWCache.mManaged.size();
  count += psiWCache.mManaged.size();
  return count;
}

unsigned int WResourceManager::nowTime(){
  if(lastTime == 65535)
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

  #ifdef WIN32
  char buf [4096];
  sprintf(buf, " Init WResourceManager : %p\n", this);
  OutputDebugString(buf);
  #endif
#ifdef DEBUG_CACHE
  menuCached = 0;
#endif
  mTextureList.clear();
	mTextureList.reserve(0);
	mTextureMap.clear();

	mQuadList.clear();
	mQuadList.reserve(0);
	mQuadMap.clear();

	mFontList.clear();
	mFontList.reserve(4);
	mFontMap.clear();

  psiWCache.Resize(SMALL_CACHE_LIMIT,6);      //Plenty of room for mana symbols, or whatever.
  sampleWCache.Resize(SMALL_CACHE_LIMIT,MAX_CACHED_SAMPLES);
  textureWCache.Resize(LARGE_CACHE_LIMIT,MAX_CACHE_OBJECTS);
  lastTime = 0;
  lastError = CACHE_ERROR_NONE;
}
WResourceManager::~WResourceManager(){
  LOG("==Destroying WResourceManager==");
  RemoveAll();

  for(vector<WManagedQuad*>::iterator it=managedQuads.begin();it!=managedQuads.end();it++){
    WManagedQuad* wm = *it;
    SAFE_DELETE(wm);
  }
  managedQuads.clear();

  //Remove all our reserved WTrackedQuads from WCachedTexture
  vector<WTrackedQuad*>::iterator g;
  for(g=WCachedTexture::garbageTQs.begin();g!=WCachedTexture::garbageTQs.end();g++){
    WTrackedQuad * tq = *g;
    SAFE_DELETE(tq);
  }
  LOG("==Successfully Destroyed WResourceManager==");
}

JQuad * WResourceManager::RetrieveCard(MTGCard * card, int style, int submode){
  //Cards are never, ever resource managed, so just check cache.
  if(!card)
    return NULL;

  submode = submode | TEXTURE_SUB_CARD;

  string filename = card->getSetName();
  filename += "/";
  filename += card->getImageName();
  JQuad * jq = RetrieveQuad(filename,0,0,0,0,"",style,submode|TEXTURE_SUB_5551);

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
  std::transform(resname.begin(),resname.end(),resname.begin(),::tolower);

  vector<WManagedQuad*>::iterator it;
  int pos = 0;
  for(it = managedQuads.begin();it!=managedQuads.end();it++,pos++){
    if((*it)->resname == resname)
      return pos;
  }

  WCachedTexture * jtex = textureWCache.Retrieve(textureName,RETRIEVE_MANAGE);
  
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
  std::transform(lookup.begin(),lookup.end(),lookup.begin(),::tolower);
  
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

JQuad * WResourceManager::RetrieveTempQuad(string filename){
  return RetrieveQuad(filename,0,0,0,0,"",RETRIEVE_NORMAL);
}

JQuad * WResourceManager::RetrieveQuad(string filename, float offX, float offY, float width, float height, string resname, int style, int submode){
  JQuad * jq = NULL;

  //Lookup managed resources, but only with a real resname.
  if(resname.size() && (style == RETRIEVE_MANAGE || style == RETRIEVE_RESOURCE)){
   jq = GetQuad(resname); 
   if(jq || style == RETRIEVE_RESOURCE)
     return jq;
  }  

  //Aliases.
  if(style == RETRIEVE_VRAM){
    submode = submode | TEXTURE_SUB_VRAM;
    style = RETRIEVE_LOCK;
  } 
  else if(style == RETRIEVE_THUMB){
    submode = submode | TEXTURE_SUB_THUMB;
    style = RETRIEVE_NORMAL;
  }

  //Resname defaults to filename.
  if(!resname.size())
    resname = filename;

  //No quad, but we have a managed texture for this!
  WCachedTexture * jtex = NULL;
  if(style == RETRIEVE_MANAGE)
    jtex = textureWCache.Retrieve(filename,RETRIEVE_MANAGE,submode);
  else if(style == RETRIEVE_EXISTING)
    jtex = textureWCache.Retrieve(filename,RETRIEVE_EXISTING,submode);
  else
    jtex = textureWCache.Retrieve(filename,RETRIEVE_NORMAL,submode);

  lastError = textureWCache.mError;

  //Somehow, jtex wasn't promoted.
  if(style == RETRIEVE_MANAGE && jtex && !jtex->isPermanent())
    return NULL;  

  //Make this quad, overwriting any similarly resname'd quads.
  if(jtex){
     WTrackedQuad * tq = jtex->GetTrackedQuad(offX,offY,width,height,resname);  

    if(tq == NULL)
      return NULL;

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

  textureWCache.Release(tex);
}

void WResourceManager::Release(JQuad * quad){
  if(!quad)
    return;

  int i;
  for(i = 0; i < MAX_CACHE_OBJECTS;i++){
    if(textureWCache.mCached[i].isGood() && textureWCache.mCached[i].ReleaseQuad(quad))
      break;
  }

  if(i != MAX_CACHE_OBJECTS)
   textureWCache.RemoveItem(&textureWCache.mCached[i],false); //won't remove locked.
}

void WResourceManager::ClearMisses(){
  textureWCache.ClearMisses();
  sampleWCache.ClearMisses();
  psiWCache.ClearMisses();
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
  if(style == RETRIEVE_VRAM){
    submode = submode | TEXTURE_SUB_VRAM;
    style = RETRIEVE_LOCK;
  } 
  else if(style == RETRIEVE_THUMB){
    submode = submode | TEXTURE_SUB_THUMB;
    style = RETRIEVE_NORMAL;
  }

  res = textureWCache.Retrieve(filename,style,submode);

  if(res){ //a non-null result will always be good.
    JTexture * t = res->Actual();
    JRenderer::GetInstance()->BindTexture(t);
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
      case CACHE_ERROR_NOT_CACHED:
        debugMessage = "Resource not cached: ";
        break;
      case CACHE_ERROR_LOST:
        debugMessage = "Resource went bad: ";
        break;
      default:
        debugMessage = "Unspecified error: ";
    }
    debugMessage += filename;
  }
#endif
  lastError = textureWCache.mError;

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

  if(id == INVALID_ID || id < 0 || id > (int) textureWCache.mManaged.size())
    return NULL;

  int pos = 0;
  list<WCachedTexture>::iterator i;
  for(i = textureWCache.mManaged.begin();i!=textureWCache.mManaged.end();i++){
    if(pos == id)
      break;
    pos++;
  }

  if(i == textureWCache.mManaged.end())
    return NULL;

  WCachedTexture * wct = &(*i);
  if(wct->isGood())
    return wct->Actual();

  return NULL;
}

hgeParticleSystemInfo * WResourceManager::RetrievePSI(string filename, JQuad * texture, int style, int submode){
  
  if(!texture)
    return NULL;

  WCachedParticles * res = psiWCache.Retrieve(filename,style,submode);

  if(res) //A non-null result will always be good.
  {
    hgeParticleSystemInfo * i = res->Actual();
    i->sprite = texture;
    return i;
  }

  lastError = psiWCache.mError;
  return NULL;
}

JSample * WResourceManager::RetrieveSample(string filename, int style, int submode){
  //Check cache. This just tracks misses.
  return NULL;
  WCachedSample * it;
  it = sampleWCache.Get(filename,submode);

  if(it == NULL)
    return NULL;

  //Sample exists! Get it.
  if(it->isGood()){
    JSample * js = it->Actual();

    it->Nullify(); //Samples are freed when played, not managed by us.
    it->Trash();

    //Adjust sizes accordingly.
    sampleWCache.cacheSize = 0;
    sampleWCache.totalSize = 0;
    return js;  
  }

  lastError = sampleWCache.mError;
  return NULL;
}

string WResourceManager::graphicsFile(const string filename, const string specific){
    char buf[512];
   
    //Check the specific location, if any.
    if(specific != ""){
      sprintf(buf,"%s/%s",specific.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);

    if(theme != "" && theme != "default"){
      sprintf(buf,"themes/%s/%s",theme.c_str(),filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }

    //Failure. Check mode graphics     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
      sprintf(buf,"modes/%s/graphics/%s",mode.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }
       
     //Failure. Check graphics       
     char graphdir[512];
     sprintf(graphdir,"graphics/%s",filename.c_str());
      if(fileOK(graphdir,true))
        return graphdir;
    
     //Failure. Check sets.       
     sprintf(buf,"sets/%s",filename.c_str());
      if(fileOK(buf,true))
        return buf;

     //Complete abject failure. Probably a crash...
     return graphdir;
}


string WResourceManager::avatarFile(const string filename, const string specific){
    char buf[512];
   
    //Check the specific location, if any.
    if(specific != ""){
      sprintf(buf,"%s/%s",specific.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }
  
    //Check the profile folder.
    string profile = options[Options::ACTIVE_PROFILE].str;
    std::transform(profile.begin(), profile.end(), profile.begin(), ::tolower);
    if(profile != "" && profile != "default"){
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
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);

    if(theme != "" && theme != "default"){
      sprintf(buf,"themes/%s/%s",theme.c_str(),filename.c_str());
     if(fileOK(buf,true))
        return buf;
    }

    //Failure. Check mode graphics     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
      sprintf(buf,"modes/%s/graphics/%s",mode.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }

    //Failure. Check Baka
    sprintf(buf,"ai/baka/avatars/%s",filename.c_str());
     if(fileOK(buf,true))
        return buf;
       
     //Failure. Check graphics       
     char graphdir[512];
     sprintf(graphdir,"graphics/%s",filename.c_str());
      if(fileOK(graphdir,true))
        return graphdir;
    
     //Complete abject failure. Probably a crash...
     return graphdir;
}

string WResourceManager::cardFile(const string filename, const string specific){
    JFileSystem* fs = JFileSystem::GetInstance();
    char buf[512];

    fs->DetachZipFile();

    //Check the specific location, if any.
    if(specific != ""){
      sprintf(buf,"%s/sets/%s",specific.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);

    if(theme != "" && theme != "default"){
       sprintf(buf,"themes/%s/sets/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
      sprintf(buf,"modes/%s/sets/%s",mode.c_str(),filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }
       
     //Failure. Check sets       
     char defdir[512];
     sprintf(defdir,"sets/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      

     //Failure. Assume it's in a zip file?
     string::size_type i;
     for(i = 0;i < filename.size();i++){
      if(filename[i] == '\\' || filename[i] == '/')
          break;
     } 

     if(i != filename.size()){
       string set = filename.substr(0,i);

       if(set.size()){
          char zipname[512];
          sprintf(zipname, "Res/sets/%s/%s.zip", set.c_str(),set.c_str());
          if (fileOK(zipname)){
            fs->AttachZipFile(zipname);
            return filename.substr(i+1);
          }
       }
     }

     //Complete failure.
     return "";
}

string WResourceManager::musicFile(const string filename, const string specific){
    char buf[512];

    //Check the specific location, if any.
    if(specific != ""){
      sprintf(buf,"%s/%s",specific.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);

    if(theme != "" && theme != "default"){
       sprintf(buf,"themes/%s/sound/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
      sprintf(buf,"modes/%s/sound/%s",mode.c_str(),filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }
       
     //Failure. Check sound       
     char defdir[512];
     sprintf(defdir,"sound/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      
    
     //Complete abject failure. Probably a crash...
     return defdir;
}

string WResourceManager::sfxFile(const string filename, const string specific){
    char buf[512];

    //Check the specific location, if any.
    if(specific != ""){
      sprintf(buf,"%s/%s",specific.c_str(),filename.c_str());
      if(fileOK(buf,true))
        return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;
    std::transform(theme.begin(), theme.end(), theme.begin(), ::tolower);

    if(theme != "" && theme != "default"){
       sprintf(buf,"themes/%s/sound/sfx/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    if(mode != "" && mode != "defualt"){
      sprintf(buf,"modes/%s/sound/sfx/%s",mode.c_str(),filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }
       
     //Failure. Check sound       
     char defdir[512];
     sprintf(defdir,"sound/sfx/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      
    
     //Complete abject failure. Probably a crash...
     return "";
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

int WResourceManager::LoadJLBFont(const string &fontName, int height){
  map<string, int>::iterator itr = mFontMap.find(fontName);

	if (itr == mFontMap.end())
	{
		string path = graphicsFile(fontName);

		printf("creating font:%s\n", path.c_str());

		int id = mFontList.size();
		mFontList.push_back(NEW JLBFont(path.c_str(), height, true));

		mFontMap[fontName] = id;

		return id;
	}
	else
		return itr->second;
}
void WResourceManager::CacheForState(int state){
#if (defined WIN32 || defined LINUX) && !defined DEBUG_CACHE
  textureWCache.Resize(HUGE_CACHE_LIMIT,HUGE_CACHE_ITEMS);
  return;
#else 
  switch(state){
    //Default is not to change cache sizes.
    case GAME_STATE_MENU:
    case GAME_STATE_OPTIONS:
      break;
    //Duels use a smaller cache, so there's more room for game stuff.
    case GAME_STATE_DUEL:
      textureWCache.Resize(SMALL_CACHE_LIMIT,SMALL_CACHE_ITEMS);
      sampleWCache.Resize(SMALL_CACHE_LIMIT,MAX_CACHED_SAMPLES);
      Cleanup();
      break;
      //Shop is almost entirely cache safe, so give it near infinite resources.
    case GAME_STATE_SHOP:
      textureWCache.Resize(HUGE_CACHE_LIMIT,HUGE_CACHE_ITEMS); 
      break;
    case GAME_STATE_DECK_VIEWER:
      textureWCache.Resize(HUGE_CACHE_LIMIT,HUGE_CACHE_ITEMS); 
      break;
      //Anything unknown, use large cache.
    default:
      textureWCache.Resize(LARGE_CACHE_LIMIT,LARGE_CACHE_ITEMS);
      break;
  }
#endif
}

JMusic * WResourceManager::ssLoadMusic(const char *fileName){
  return JSoundSystem::GetInstance()->LoadMusic(musicFile(fileName).c_str());
}


void WResourceManager::Refresh(){
  //Really easy cache relinking.
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
        JRenderer::GetInstance()->BindTexture(newtex);
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
}

//WCache
template <class cacheItem,class cacheActual> 
bool WCache<cacheItem, cacheActual>::RemoveOldest(){
  int oldest = -1;

  for(int i = 0;i< MAX_CACHE_OBJECTS;i++){
    if(mCached[i].isGood() && !mCached[i].isLocked() 
      && (oldest == -1 || mCached[i].lastTime < mCached[oldest].lastTime))
      oldest = i;
  }

  if(oldest != -1 && mCached[oldest].isGood() && !mCached[oldest].isLocked()){
#if defined DEBUG_CACHE
    lastExpired = mCached[oldest].id;
#endif
    Delete(&mCached[oldest]);
    return true;
  }

  return false;
}
template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Clear(){
  typename list<cacheItem*>::iterator it;

  for(int i = 0; i < MAX_CACHE_OBJECTS;i++){
    if(!mCached[i].isTrash())
      Delete(&mCached[i]);
  }
}

template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::ClearUnlocked(){
  typename list<cacheItem>::iterator it;

  for(int i = 0; i < MAX_CACHE_OBJECTS;i++){
    if(mCached[i].isGood() && !mCached[i].isLocked())
      Delete(&mCached[i]);
  }
}

template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::ClearMisses(){
  mMisses.clear();
}
template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Resize(unsigned long size, int items){
    maxCacheSize = size;

  if(items > MAX_CACHE_OBJECTS || items < 0)
    maxCached = MAX_CACHE_OBJECTS;
  else
    maxCached = items;
}

template <class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::Recycle(){

  int i;
  for(i = 0; i < MAX_CACHE_OBJECTS;i++){
    if(mCached[i].isTrash())
      break;
  }

  if(i == MAX_CACHE_OBJECTS){    
    mError = CACHE_ERROR_FULL;
    return NULL;
  }

  return &mCached[i];  
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::AttemptNew(cacheItem* item, int submode){
  if(submode & CACHE_EXISTING){ //Should never get this far.
    mError = CACHE_ERROR_NOT_CACHED;
    return false;
  }

  //We weren't passed a valid item, so get one.
  if(item == NULL || !item->isTrash())
    item = Recycle();

  //Weren't able to get an item. Fail.
  if(item == NULL){
    mError = CACHE_ERROR_BAD_ALLOC;
    return false;
  }

  string filename = makeFilename(item->id,submode);
  
  if(!item->Attempt(filename,submode,mError)){
    //No such file. Fail.
    if(!item->isGood() && mError == CACHE_ERROR_404){
      item->Trash();
      return false;
    }

    //Make several attempts, removing bits of the cache.
    for(int attempt=0;attempt<MAX_CACHE_ATTEMPTS;attempt++){
      if(!RemoveOldest()) 
        break;
      
      if(item->Attempt(filename,submode,mError) && item->isGood())
        break;

      //Failed attempt. Trash this.
      item->Trash();
    }

    if(!item->isGood()){
      item->Trash();
      Cleanup();
      item->Attempt(filename,submode,mError);
    }
    //Still no result, so clear cache entirely, then try again.
    if(!item->isGood()){
      item->Trash();
      ClearUnlocked();
      item->Attempt(filename,submode,mError);
    }
  }

  //Final failure. Trash it.
  if(item && !item->isGood()){
    item->Trash();
    mError = CACHE_ERROR_BAD;
    return false;
  }
  else
    mError = CACHE_ERROR_NONE;

  //Strictly enforce limits.
  item->lock();
  Cleanup();  
  item->unlock();

  return true;
}

template <class cacheItem, class cacheActual>
cacheItem * WCache<cacheItem, cacheActual>::Retrieve(string filename, int style, int submode){
  //Check cache.
  cacheItem* item;

  if(style == RETRIEVE_EXISTING || style == RETRIEVE_RESOURCE)
    item = Get(filename,style,submode|CACHE_EXISTING);
  else
    item = Get(filename,style,submode);

  //Get failed.
  if(item == NULL || mError != CACHE_ERROR_NONE){
    return NULL;
  }

  //Retrieve resource only works on permanent items.
  if(style == RETRIEVE_RESOURCE && !item->isPermanent()){
    mError = CACHE_ERROR_NOT_MANAGED;
    return NULL;
  }

  //Perform lock or unlock on entry.
  if(style == RETRIEVE_LOCK) item->lock();
  else if(style == RETRIEVE_UNLOCK) item->unlock();
  else if(style == RETRIEVE_MANAGE) item->deadbolt();

  //All is well!
  item->hit();
  return item;
}
template <class cacheItem, class cacheActual>
string WCache<cacheItem, cacheActual>::makeID(string id, int submode){
  string lookup = id;
  
  //To differentiate between cached thumbnails and the real thing.
  if(submode & TEXTURE_SUB_THUMB)
    lookup.insert(0,"T");
  
  return lookup;
}

template <class cacheItem, class cacheActual>
string WCache<cacheItem, cacheActual>::makeFilename(string id, int submode){
  //To differentiate between cached thumbnails and the real thing.
  if(submode & TEXTURE_SUB_THUMB)
    return id.substr(1);
  
  return id;
}

template <class cacheItem, class cacheActual> 
void WCache<cacheItem, cacheActual>::RecordMiss(string miss){
  list<string>::iterator it;
  for(it=mMisses.begin();it!=mMisses.end();it++){
    if(*it == miss)
      return;
  }

  mMisses.push_back(miss);
}

template <class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::Get(string id, int style, int submode){
  typename list<cacheItem>::iterator it;
  cacheItem * item = NULL;
  int i = 0;
  string lookup = makeID(id,submode);  
  
  //Start with no errors.
  mError = CACHE_ERROR_NONE;

  
  //Check for misses.
  list<string>::iterator miss;
  for(miss = mMisses.begin();miss != mMisses.end();miss++){
    if(*miss == lookup){
      mError = CACHE_ERROR_404;
      return NULL;
    }
  }

  //Check for managed resources first. Always
  for(it = mManaged.begin();it!=mManaged.end();it++){
    if(it->id == lookup)
      break;
  }  
  //Something is managed.
  if(it != mManaged.end() && it->isGood()) {
     mError = CACHE_ERROR_NONE;
     return &(*it);
  }
  //Failed to find managed resource and won't create one. Record a miss.
  else if(style == RETRIEVE_RESOURCE){
    mError = CACHE_ERROR_NOT_MANAGED;
    return NULL;
  }

  //Not managed, so look in cache.
  if(style != RETRIEVE_RESOURCE ){
    for(i= 0;i<MAX_CACHE_OBJECTS;i++){
      item = &mCached[i];
      if(item->id == lookup)
        break;
    }

    if(i >= MAX_CACHE_OBJECTS)
      item = NULL;

    //Well, we've found something...
    if(item != NULL && item->isGood()) {
       mError = CACHE_ERROR_NONE;
       return item; //A hit.
    }
    //Didn't find anything. Die here, if we must.
    else if(style == RETRIEVE_EXISTING || submode & CACHE_EXISTING){
      mError = CACHE_ERROR_NOT_CACHED; 
      return NULL;
    }
  }  
  
  //If we've got an item that went bad, trash it so we can recycle it.
  if(item != NULL && !item->isGood() && !item->isTrash()){
    item->Trash();
  }

  //Give us a managed item
  if(style == RETRIEVE_MANAGE){
    //This was formerly in cache, so promote it
    if(item != NULL){
      if(item->isGood()){
        mManaged.push_front(*item);
        item->Trash();
        mError = CACHE_ERROR_NONE;
        it = mManaged.begin();
        return &(*it);
      }
      else{
        //It went bad, so trash it and manage something new.
        item->Trash();
      }
    }
    else{
      cacheItem temp;
      mManaged.push_front(temp);
      it = mManaged.begin();
      item = &(*it);
    }
  }
  //Give us a cached item.
  else{
    if(item != NULL && !item->isTrash()) {
      mError = CACHE_ERROR_LOST;
      return NULL; //Something went wrong. TODO: Proper error message.
    }

    item = Recycle();
  }

  if(mError != CACHE_ERROR_NONE)
    return NULL;
 
  //If we've reached this point, we've got an item to fill.
  item->id = lookup; //Asign the new lookup value.
  AttemptNew(item,submode);  //Try to fill it.

  //No errors, so good!
  if(item->isGood()){
    unsigned long isize = item->size();   
    totalSize += isize;
    
    if(style != RETRIEVE_MANAGE){
      cacheItems++;
      cacheSize += isize;
    }

    mError = CACHE_ERROR_NONE;  
    return item;
  }
  //Failed. Record miss.
  else if(mError == CACHE_ERROR_404){
    RecordMiss(lookup); 
  }    
 
  //Failure.
  if(item && !item->isGood())
    item->Trash();

  mError = CACHE_ERROR_BAD;
  return NULL;
}

template <class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Refresh(){
  ClearUnlocked();


  for(int i = 0;i < MAX_CACHE_OBJECTS;i++){
    cacheItem * item = &mCached[i];
    if(item->isGood()){
      item->Refresh(makeFilename(item->id,item->loadedMode));
    }
  } 
  typename list<cacheItem>::iterator it;
  for(it = mManaged.begin();it!=mManaged.end();it++){
    if(it->isGood()){
      it->Refresh(makeFilename(it->id,it->loadedMode));
    }
  }
  mMisses.clear();
}

template <class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::WCache(){
  cacheSize = 0;
  totalSize = 0;

  maxCacheSize = SMALL_CACHE_LIMIT;

  maxCached = MAX_CACHE_OBJECTS;
  cacheItems = 0;
  mError = CACHE_ERROR_NONE;
}

template <class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::~WCache(){
  //Vectors take care of themselves.
}


template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Cleanup(){
  while(mMisses.size() > MAX_CACHE_MISSES){
    RemoveMiss();
  }

  while (cacheItems > MAX_CACHE_OBJECTS - 1 || cacheItems > maxCached || cacheSize > maxCacheSize ){
    if (!RemoveOldest()) 
      return false;
  } 
  return true;
}

template <class cacheItem, class cacheActual>
unsigned int WCache<cacheItem, cacheActual>::Flatten(){
  unsigned int youngest = 65535;
  unsigned int oldest = 0;

  for (int i = 0; i < MAX_CACHE_OBJECTS; i++){
    cacheItem * it = &mCached[i];
    if(!it->isGood()) continue;
    if(it->lastTime < youngest) youngest = it->lastTime;
    if(it->lastTime > oldest) oldest = it->lastTime;
  }

  for (int i = 0; i < MAX_CACHE_OBJECTS; i++){
    cacheItem * it = &mCached[i];

    if(!it->isGood()) continue;
      it->lastTime -= youngest;
  }

  return (oldest - youngest);
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveMiss(string id){
  typename list<string>::iterator it = mMisses.end();

  for(it = mMisses.begin();it!=mMisses.end();it++){
    if((id == "" || *it == id))
          break;
  }

  if(it != mMisses.end())
  {
    mMisses.erase(it);
    return true; 
  }

  return false;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveItem(cacheItem* item, bool force){
  if(item && !item->isLocked()){
    Delete(item);
    return true;
  }

  return false;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Delete(cacheItem* item){
//No need to delete this.
  if(item == NULL || item->isTrash())
    return false;

#ifdef DEBUG_CACHE
  char buf[512];
  sprintf(buf,"WCache::Delete %s\n",item->id.c_str());
  OutputDebugString(buf);
#endif

  if(maxCached == 0)
    item->Nullify();

  unsigned long isize = item->size();
  totalSize -= isize;
  cacheSize -= isize;

#ifdef DEBUG_CACHE
    if(cacheItems == 0)
      OutputDebugString("cacheItems out of sync.\n");
#endif

  cacheItems--;

  item->Trash();
  return true;
}

template <class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Release(cacheActual* actual){
  if(!actual)
    return false;

  int i;

  for(i = 0;i < MAX_CACHE_OBJECTS;i++){    
    if(!mCached[i].isGood())
      continue;

    if(mCached[i].compare(actual))
      break;
  }
  
  if(i == MAX_CACHE_OBJECTS)
    return false; //Not here, can't release.

  //Release one lock.
  mCached[i].unlock(); 

  //Still locked, won't delete, not technically a failure.
  if(mCached[i].isLocked())
    return true; 

#if defined DEBUG_CACHE
  lastReleased = mCached[i].id;
#endif
  Delete(&mCached[i]);
  return true;
}