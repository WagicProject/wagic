#ifndef _WResourceManager_H_
#define _WResourceManager_H_

#include "WResource_Fwd.h"
#include <JResourceManager.h>
#include <JSoundSystem.h>

const std::string kGenericCardID = "back";
const std::string kGenericCardThumbnailID = "back_thumb";

const std::string kGenericCard("back.jpg");
const std::string kGenericThumbCard("back_thumb.jpg");

enum ENUM_WRES_INFO
{
    WRES_UNLOCKED = 0,      //Resource is unlocked.
    WRES_MAX_LOCK = 250,    //Maximum number of locks for a resource.
    WRES_PERMANENT = 251,   //Resource is permanent (ie, managed)  
    WRES_UNDERLOCKED = 252, //Resource was released too many times.
};

enum ENUM_RETRIEVE_STYLE
{
	RETRIEVE_EXISTING, 	//Only returns a resource if it already exists. Does not lock or unlock.
    RETRIEVE_NORMAL,    //Returns or creates a resource. Does not change lock status.
    RETRIEVE_LOCK,      //As above, locks cached resource. Not for quads.
    RETRIEVE_UNLOCK,    //As above, unlocks cached resource. Not for quads.
    RETRIEVE_RESOURCE,  //Only retrieves a managed resource. Does not make a new one.
    RETRIEVE_MANAGE,    //Makes resource permanent.
    RETRIEVE_THUMB,     //Retrieve it as a thumbnail.
    CACHE_THUMB = RETRIEVE_THUMB, //Backwards compatibility. 
};

enum ENUM_CACHE_SUBTYPE
{
    CACHE_NORMAL =  (1<<0),    //Use default values. Not really a flag.
    //CACHE_EXISTING = (1<<1),   //Retrieve it only if it already exists

    //Because these bits only modify how a cached resource's Attempt() is called,
    //We can use them over and over for each resource type.
    TEXTURE_SUB_EXACT = (1<<2),   //Don't do any fiddling with the filename.
    TEXTURE_SUB_CARD = (1<<3),    //Retrieve using cardFile, not graphicsFile.
    TEXTURE_SUB_AVATAR = (1<<4),  //Retrieve using avatarFile, not graphicsFile.
    TEXTURE_SUB_THUMB = (1<<5),   //Retrieve prepending "thumbnails\" to the filename.
    TEXTURE_SUB_5551 = (1<<6),    //For textures. If we have to allocate, use RGBA5551.

};

enum ENUM_CACHE_ERROR
{
    CACHE_ERROR_NONE = 0,
    CACHE_ERROR_NOT_CACHED = CACHE_ERROR_NONE,
    CACHE_ERROR_404,
    CACHE_ERROR_BAD, 		//Something went wrong with item->attempt()
    CACHE_ERROR_BAD_ALLOC, 	//Couldn't allocate item
    CACHE_ERROR_LOST,
    CACHE_ERROR_NOT_MANAGED,
};

struct WManagedQuad; 
class WFont;
class MTGCard;
struct hgeParticleSystemInfo;

class WResourceManager
{
public:
    static WResourceManager* Instance();
    static void Terminate();

    virtual ~WResourceManager()
	{
	}

    virtual bool IsThreaded() = 0;

    virtual JQuadPtr RetrieveCard(MTGCard * card, int style = RETRIEVE_NORMAL,int submode = CACHE_NORMAL) = 0;
    virtual JSample * RetrieveSample(const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL) = 0;
    virtual JTexture * RetrieveTexture(const string& filename, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL) = 0;
    virtual JQuadPtr RetrieveQuad(const string& filename, float offX=0.0f, float offY=0.0f, float width=0.0f, float height=0.0f,  string resname="",  int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL, int id = 0) = 0;
    virtual JQuadPtr RetrieveTempQuad(const string& filename, int submode = CACHE_NORMAL) = 0;
    virtual hgeParticleSystemInfo * RetrievePSI(const string& filename, JQuad * texture, int style = RETRIEVE_NORMAL, int submode = CACHE_NORMAL) = 0;
    virtual int RetrieveError() = 0;

    virtual void Release(JTexture * tex) = 0;
    virtual void Release(JSample * sample) = 0;

	//Refreshes all files in cache, for when mode/profile changes.
    virtual void Refresh() = 0;

    virtual unsigned int nowTime() = 0;

    virtual JQuadPtr GetQuad(const string &quadName) = 0;

    //Our file redirect system.
    virtual string graphicsFile(const string& filename) = 0;
    virtual string avatarFile(const string& filename) = 0;
    virtual string cardFile(const string& filename) = 0;
    virtual string musicFile(const string& filename) = 0;
    virtual string sfxFile(const string& filename) = 0;
    virtual int fileOK(const string&, bool relative = false) = 0;
    virtual int dirOK(const string& dirname) = 0;

    //For backwards compatibility with JWResourceManager. Avoid using these, they're not optimal.
    virtual int CreateTexture(const string &textureName) = 0;
    virtual JTexture* GetTexture(const string &textureName) = 0;

    // Font management functions
    virtual void InitFonts(const std::string& inLang) = 0;
    virtual int ReloadWFonts() = 0;
    virtual WFont* LoadWFont(const string& inFontname, int inFontHeight, int inFontID) = 0;
    virtual WFont* GetWFont(int id) = 0;

    //Wrapped from JSoundSystem. TODO: Privatize.
    virtual JMusic * ssLoadMusic(const char *fileName) = 0;

    //Resets the cache limits on when it starts to purge data.
    virtual void ResetCacheLimits() = 0;

    virtual void DebugRender() = 0;

protected:
    /*
    ** Singleton object only accessibly via Instance(), constructor is private
    */
    WResourceManager()
	{
	}

    static WResourceManager* sInstance;
};

#endif
