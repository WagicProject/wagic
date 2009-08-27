#include "../include/config.h"
#include "../include/utils.h"
#include "../include/GameOptions.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <JGE.h>
#include "../include/WResourceManager.h"


WResourceManager::WResourceManager(){
  jrm = NEW JResourceManager();
}
WResourceManager::~WResourceManager(){
  SAFE_DELETE(jrm);
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

    if(theme != "" || theme != "default"){
       sprintf(buf,"themes/%s/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode graphics     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
    sprintf(buf,"modes/graphics/%s",mode,filename.c_str());
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

    if(theme != "" || theme != "default"){
       sprintf(buf,"themes/%s/sound/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);

    if(mode != "" && mode != "defualt"){
    sprintf(buf,"modes/sound/%s",mode,filename.c_str());
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

    if(theme != "" || theme != "default"){
       sprintf(buf,"themes/%s/sound/sfx/%s",theme.c_str(),filename.c_str());
       if(fileOK(buf,true))
         return buf;
    }

    //Failure. Check mode     
    string mode = options[Options::ACTIVE_MODE].str;
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    if(mode != "" && mode != "defualt"){
    sprintf(buf,"modes/sound/sfx/%s",mode,filename.c_str());
    if(fileOK(buf,true))
       return buf;      
    }
       
     //Failure. Check sound       
     char defdir[512];
     sprintf(defdir,"sound/sfx/%s",filename.c_str());
     if(fileOK(defdir,true))
       return defdir;      
    
     //Complete abject failure. Probably a crash...
     return defdir;
}

int WResourceManager::fileOK(string filename, bool relative){
  
  if(relative){
    char buf[512];
    sprintf(buf,RESPATH"/%s",filename.c_str());
    return fileExists(buf);
  }
  
  return fileExists(filename.c_str());
}

int WResourceManager::CreateTexture(const string &textureName) {
  return jrm->CreateTexture(graphicsFile(textureName));
}
JTexture* WResourceManager::GetTexture(const string &textureName) {
  return jrm->GetTexture(graphicsFile(textureName));
}
JTexture* WResourceManager::GetTexture(int id) {
  return jrm->GetTexture(id);
}

int WResourceManager::CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height){
  return jrm->CreateQuad(quadName, graphicsFile(textureName), x, y, width, height);
}
JQuad* WResourceManager::GetQuad(const string &quadName){
  return jrm->GetQuad(quadName);
}
JQuad* WResourceManager::GetQuad(int id){
  return jrm->GetQuad(id);
}

int WResourceManager::LoadJLBFont(const string &fontName, int height){
  return jrm->LoadJLBFont(graphicsFile(fontName), height);
}
JLBFont* WResourceManager::GetJLBFont(const string &fontName){
  return jrm->GetJLBFont(graphicsFile(fontName));
}

JLBFont* WResourceManager::GetJLBFont(int id){
  return jrm->GetJLBFont(id);
}

int WResourceManager::LoadMusic(const string &musicName){
  return jrm->LoadMusic(musicFile(musicName));
}
JMusic* WResourceManager::GetMusic(const string &musicName){
  return jrm->GetMusic(musicFile(musicName));
}
JMusic* WResourceManager::GetMusic(int id){
    return jrm->GetMusic(id);
}

int WResourceManager::LoadSample(const string &sampleName){
  return jrm->LoadSample(sfxFile(sampleName));
}
JSample* WResourceManager::GetSample(const string &sampleName){
  return jrm->GetSample(sfxFile(sampleName));
}
JSample* WResourceManager::GetSample(int id){
  return jrm->GetSample(id);
}

JTexture* WResourceManager::LoadTexture(const char* filename, int mode, int textureFormat){
  return JRenderer::GetInstance()->LoadTexture(graphicsFile(filename).c_str(),mode,textureFormat);
}

JMusic * WResourceManager::ssLoadMusic(const char *fileName){
  return JSoundSystem::GetInstance()->LoadMusic(musicFile(fileName).c_str());
}
JSample * WResourceManager::ssLoadSample(const char *fileName){
  return JSoundSystem::GetInstance()->LoadSample(sfxFile(fileName).c_str());
}
