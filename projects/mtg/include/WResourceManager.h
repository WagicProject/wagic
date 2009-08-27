#ifndef _WRESOURCEMANAGER_H_
#define _WRESOURCEMANAGER_H_
#include <JResourceManager.h>
#include <JTypes.h>

//This class is a wrapper for JResourceManager
class WResourceManager
{
public:
  WResourceManager();
  ~WResourceManager();

  //Wrapped from JResourceManager
	 int CreateTexture(const string &textureName);
	 JTexture* GetTexture(const string &textureName);
	 JTexture* GetTexture(int id);

	 int CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height);
	 JQuad* GetQuad(const string &quadName);
	 JQuad* GetQuad(int id);

   int LoadJLBFont(const string &fontName, int height);
	 JLBFont* GetJLBFont(const string &fontName);
	 JLBFont* GetJLBFont(int id);

	 int LoadMusic(const string &musicName);
	 JMusic* GetMusic(const string &musicName);
	 JMusic* GetMusic(int id);

	 int LoadSample(const string &sampleName);
	 JSample* GetSample(const string &sampleName);
	 JSample* GetSample(int id);

  //Wrapped from other bits, if we want them.
   JTexture* LoadTexture(const char* filename, int mode = 0, int textureFormat = TEXTURE_FORMAT);
   //Wrapped from JSoundSystem
   JMusic * ssLoadMusic(const char *fileName);
   JSample * ssLoadSample(const char *fileName); 

  //Our new redirect system.
  string graphicsFile(const string filename, const string specific = "");
  string musicFile(const string filename, const string specific = "");
  string sfxFile(const string filename, const string specific = "");
  bool fileOK(string filename, bool relative = false);

private:
  JResourceManager * jrm;	
};
 
#endif