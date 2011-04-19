#include "PrecompiledHeader.h"

#include "GameOptions.h"
#include "CacheEngine.h"
#include "WResourceManager.h"
#include "StyleManager.h"

#if defined (WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "WFont.h"

#define FORCE_LOW_CACHE_MEMORY
const unsigned int kConstrainedCacheLimit = 8 * 1024 * 1024;

extern bool neofont;
int idCounter = OTHERS_OFFSET;

namespace
{
    const std::string kExtension_png(".png");
    const std::string kExtension_gbk(".gbk");
    const std::string kExtension_font(".font");

    const std::string kGenericCard("back.jpg");
    const std::string kGenericThumbCard("back_thumb.jpg");
}

WResourceManager* WResourceManager::sInstance = NULL;

int WResourceManager::RetrieveError()
{
    return lastError;
}

bool WResourceManager::RemoveOldest()
{
    if (sampleWCache.RemoveOldest()) return true;
    if (textureWCache.RemoveOldest()) return true;
    if (psiWCache.RemoveOldest()) return true;

    return false;
}

//WResourceManager
void WResourceManager::DebugRender()
{
    JRenderer* renderer = JRenderer::GetInstance();
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    if (!font || !renderer) return;

    font->SetColor(ARGB(255,255,255,255));
    font->SetScale(DEFAULT_MAIN_FONT_SCALE);
    renderer->FillRect(0, 0, SCREEN_WIDTH, 40, ARGB(128,155,0,0));

    renderer->FillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 40, ARGB(128,155,0,0));
    char buf[512];

    unsigned long man = 0;
    unsigned int misses = 0;

    if (textureWCache.cacheItems < textureWCache.cache.size()) misses = textureWCache.cache.size() - textureWCache.cacheItems;

    if (textureWCache.totalSize > textureWCache.cacheSize) man = textureWCache.totalSize - textureWCache.cacheSize;

    sprintf(buf, "Textures %u+%llu (of %u) items (%u misses), Pixels: %lu (of %lu) + %lu", textureWCache.cacheItems,
        (long long unsigned int) textureWCache.managed.size(), textureWCache.maxCached, misses,
        textureWCache.cacheSize, textureWCache.maxCacheSize, man);
    font->DrawString(buf, 10, 5);

#if PSPENV
    //int maxLinear = ramAvailableLineareMax();
    //int ram = ramAvailable();
   
    //sprintf(buf, "Ram : linear max: %i - total : %i sceSize : %i\n", maxLinear, ram, sceSize);
    font->DrawString(buf, 10, 20);
#endif

    sprintf(buf, "Time: %u. Total Size: %lu (%lu cached, %lu managed). ", lastTime, Size(), SizeCached(), SizeManaged());
    font->DrawString(buf, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 15, JGETEXT_RIGHT);

#ifdef DEBUG_CACHE
    if(debugMessage.size())
        font->DrawString(debugMessage.c_str(), SCREEN_WIDTH-10,SCREEN_HEIGHT-25,JGETEXT_RIGHT);

#endif
}

unsigned long WResourceManager::Size()
{
    unsigned long res = 0;
    res += textureWCache.totalSize;
    res += sampleWCache.totalSize;
    res += psiWCache.totalSize;
    return res;
}

unsigned long WResourceManager::SizeCached()
{
    unsigned long res = 0;
    res += textureWCache.cacheSize;
    res += sampleWCache.cacheSize;
    res += psiWCache.cacheSize;
    return res;
}

unsigned long WResourceManager::SizeManaged()
{
    unsigned long res = 0;
    if (textureWCache.totalSize > textureWCache.cacheSize) res += textureWCache.totalSize - textureWCache.cacheSize;

    if (sampleWCache.totalSize > sampleWCache.cacheSize) res += sampleWCache.totalSize - sampleWCache.cacheSize;

    if (psiWCache.totalSize > psiWCache.cacheSize) res += psiWCache.totalSize - psiWCache.cacheSize;

    return res;
}

unsigned int WResourceManager::Count()
{
    unsigned int count = 0;
    count += textureWCache.cacheItems;
    count += textureWCache.managed.size();
    count += sampleWCache.cacheItems;
    count += sampleWCache.managed.size();
    count += psiWCache.cacheItems;
    count += psiWCache.managed.size();
    return count;
}

unsigned int WResourceManager::CountCached()
{
    unsigned int count = 0;
    count += textureWCache.cacheItems;
    count += sampleWCache.cacheItems;
    count += psiWCache.cacheItems;
    return count;
}
unsigned int WResourceManager::CountManaged()
{
    unsigned int count = 0;
    count += textureWCache.managed.size();
    count += sampleWCache.managed.size();
    count += psiWCache.managed.size();
    return count;
}

unsigned int WResourceManager::nowTime()
{
    if (lastTime > MAX_CACHE_TIME) FlattenTimes();

    return ++lastTime;
}

void WResourceManager::FlattenTimes()
{
    unsigned int t;
    lastTime = sampleWCache.Flatten();

    t = textureWCache.Flatten();
    if (t > lastTime) lastTime = t;

    t = psiWCache.Flatten();
    if (t > lastTime) lastTime = t;
}

WResourceManager::WResourceManager()
{
    DebugTrace("Init WResourceManager : " << this);
#ifdef DEBUG_CACHE
    menuCached = 0;
#endif

    psiWCache.Resize(PSI_CACHE_SIZE, 20);
    sampleWCache.Resize(SAMPLES_CACHE_SIZE, MAX_CACHED_SAMPLES);
    textureWCache.Resize(TEXTURES_CACHE_MINSIZE, MAX_CACHE_OBJECTS);
    lastTime = 0;
    lastError = CACHE_ERROR_NONE;

    bThemedCards = false;

    LOG("Calling CacheEngine::Create");

#ifdef PSPENV
    CacheEngine::Create<UnthreadedCardRetriever>(textureWCache);
#else
    CacheEngine::Create<ThreadedCardRetriever>(textureWCache);
#endif
}

WResourceManager::~WResourceManager()
{
    LOG("==Destroying WResourceManager==");
    RemoveWFonts();

    CacheEngine::Terminate();
    LOG("==Successfully Destroyed WResourceManager==");
}

bool WResourceManager::IsThreaded()
{
    return CacheEngine::IsThreaded();
}

JQuadPtr WResourceManager::RetrieveCard(MTGCard * card, int style, int submode)
{
    //Cards are never, ever resource managed, so just check cache.
    if (!card || options[Options::DISABLECARDS].number) return JQuadPtr();

    submode = submode | TEXTURE_SUB_CARD;

    string filename = setlist[card->setId] + "/" + card->getImageName();
    int id = card->getMTGId();

    //Aliases.
    if (style == RETRIEVE_THUMB)
    {
        submode = submode | TEXTURE_SUB_THUMB;
        style = RETRIEVE_NORMAL;
    }

    JQuadPtr jq = RetrieveQuad(filename, 0, 0, 0, 0, "", style, submode | TEXTURE_SUB_5551, id);

    lastError = textureWCache.mError;
    if (jq)
    {
        jq->SetHotSpot(static_cast<float> (jq->mTex->mWidth / 2), static_cast<float> (jq->mTex->mHeight / 2));
        return jq;
    }

    return JQuadPtr();
}

int WResourceManager::AddQuadToManaged(const WManagedQuad& inQuad)
{
    int id = mIDLookupMap.size();
    mIDLookupMap.insert(make_pair(id, inQuad.resname));
    mManagedQuads.insert(make_pair(inQuad.resname, inQuad));

    return id;
}

int WResourceManager::CreateQuad(const string &quadName, const string &textureName, float x, float y, float width, float height)
{
    if (!quadName.size() || !textureName.size()) return INVALID_ID;

    if (GetQuad(quadName) != NULL)
    {
        assert(false);
        return ALREADY_EXISTS;
    }

    WCachedTexture * jtex = textureWCache.Retrieve(0, textureName, RETRIEVE_MANAGE);
    lastError = textureWCache.mError;
    int id = INVALID_ID;
    //Somehow, jtex wasn't promoted.
    if (RETRIEVE_MANAGE && jtex && !jtex->isPermanent()) return id;

    if (jtex)
    {
        JQuadPtr quad = jtex->GetQuad(x, y, width, height, quadName);

        if (quad.get())
        {
            jtex->deadbolt();

            WManagedQuad mq;
            mq.resname = quadName;
            mq.texture = jtex;
            id = AddQuadToManaged(mq);
        }
    }

    assert(id != INVALID_ID);
    return id;
}

JQuadPtr WResourceManager::GetQuad(const string &quadName)
{
    JQuadPtr result;
    ManagedQuadMap::const_iterator found = mManagedQuads.find(quadName);
    if (found != mManagedQuads.end())
    {
        result = found->second.texture->GetQuad(quadName);
    }

    return result;
}

JQuadPtr WResourceManager::GetQuad(int id)
{
    JQuadPtr result;
    if (id < 0 || id >= (int) mManagedQuads.size()) return result;

    IDLookupMap::const_iterator key = mIDLookupMap.find(id);
    if (key != mIDLookupMap.end())
    {
        WCachedTexture* jtex = mManagedQuads[key->second].texture;
        if (jtex)
        {
            result = jtex->GetQuad(key->second);
        }
    }

    return result;
}

JQuadPtr WResourceManager::RetrieveTempQuad(const string& filename, int submode)
{
    return RetrieveQuad(filename, 0, 0, 0, 0, "temporary", RETRIEVE_NORMAL, submode);
}

JQuadPtr WResourceManager::RetrieveQuad(const string& filename, float offX, float offY, float width, float height, string resname,
    int style, int submode, int id)
{
    //Lookup managed resources, but only with a real resname.
    if (resname.size() && (style == RETRIEVE_MANAGE || style == RETRIEVE_RESOURCE))
    {
       JQuadPtr quad = GetQuad(resname);
        if (quad.get() || style == RETRIEVE_RESOURCE) return quad;
    }

    //Aliases.
    if (style == RETRIEVE_THUMB)
    {
        submode = submode | TEXTURE_SUB_THUMB;
        style = RETRIEVE_NORMAL;
    }

    //Resname defaults to filename.
    if (!resname.size()) resname = filename;

    //No quad, but we have a managed texture for this!
    WCachedTexture* jtex = textureWCache.Retrieve(id, filename, style, submode);

    lastError = textureWCache.mError;

    //Somehow, jtex wasn't promoted.
    if (style == RETRIEVE_MANAGE && jtex && !jtex->isPermanent()) return JQuadPtr();

    //Make this quad, overwriting any similarly resname'd quads.
    if (jtex)
    {
        JQuadPtr quad = jtex->GetQuad(offX, offY, width, height, resname);

        if (!quad.get())
            return quad;

        if (style == RETRIEVE_MANAGE && resname != "")
        {
            WManagedQuad mq;
            mq.resname = resname;
            mq.texture = jtex;
            AddQuadToManaged(mq);
        }

        if (style == RETRIEVE_LOCK)
            jtex->lock();
        else if (style == RETRIEVE_UNLOCK)
            jtex->unlock();
        else if (style == RETRIEVE_MANAGE) jtex->deadbolt();

        return quad;
    }

    //Texture doesn't exist, so no quad.
    return JQuadPtr();
}

void WResourceManager::Release(JTexture * tex)
{
    if (!tex) return;

    //Copied direct from WCache::Release(). This is quick and dirty.
    map<int, WCachedTexture*>::iterator it;
    for (it = textureWCache.cache.begin(); it != textureWCache.cache.end(); it++)
    {
        if (it->second && it->second->compare(tex)) break;
    }

    if (it == textureWCache.cache.end()) return; //Not here, can't release.

    if (it->second)
    {
        it->second->unlock(); //Release one lock.
        if (it->second->locks != WRES_UNLOCKED) //Normally we'd call isLocked, but this way ignores quads.
            return; //Locked
    }

    textureWCache.Delete(it->second);
    textureWCache.cache.erase(it);
    return; //Released!
}

void WResourceManager::Unmiss(string filename)
{
    map<int, WCachedTexture*>::iterator it;
    int id = textureWCache.makeID(0, filename, CACHE_NORMAL);
    textureWCache.RemoveMiss(id);
}

void WResourceManager::ClearUnlocked()
{
    textureWCache.ClearUnlocked();
    sampleWCache.ClearUnlocked();
    psiWCache.ClearUnlocked();
}

void WResourceManager::Release(JSample * sample)
{
    if (!sample) return;

    sampleWCache.Release(sample);
}

JTexture * WResourceManager::RetrieveTexture(const string& filename, int style, int submode)
{
    //Aliases.
    if (style == RETRIEVE_THUMB)
    {
        submode = submode | TEXTURE_SUB_THUMB;
        style = RETRIEVE_NORMAL;
    }

    WCachedTexture* res = textureWCache.Retrieve(0, filename, style, submode);
    lastError = textureWCache.mError;

    if (res)
    { //a non-null result will always be good.
        JTexture * t = res->Actual();
        return t;
    }
#ifdef DEBUG_CACHE
    else
    {
        switch(textureWCache.mError)
        {
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

int WResourceManager::CreateTexture(const string &textureName)
{
    JTexture * jtex = RetrieveTexture(textureName, RETRIEVE_MANAGE);

    if (jtex) return (int) jtex->mTexId; //Because it's unsigned on windows/linux.

    return INVALID_ID;
}

JTexture* WResourceManager::GetTexture(const string &textureName)
{
    JTexture * jtex = RetrieveTexture(textureName, RETRIEVE_RESOURCE);
    return jtex;
}

JTexture* WResourceManager::GetTexture(int id)
{
    map<int, WCachedTexture*>::iterator it;
    JTexture *jtex = NULL;

    if (id == INVALID_ID) return NULL;

    for (it = textureWCache.managed.begin(); it != textureWCache.managed.end(); it++)
    {
        if (it->second)
        {
            jtex = it->second->Actual();
            if (id == (int) jtex->mTexId) return jtex;
        }
    }

    return jtex;
}

hgeParticleSystemInfo * WResourceManager::RetrievePSI(const string& filename, JQuad * texture, int style, int submode)
{
    if (!texture) return NULL;

    WCachedParticles * res = psiWCache.Retrieve(0, filename, style, submode);
    lastError = psiWCache.mError;

    if (res) //A non-null result will always be good.
    {
        hgeParticleSystemInfo * i = res->Actual();
        i->sprite = texture;
        return i;
    }

    return NULL;
}

JSample * WResourceManager::RetrieveSample(const string& filename, int style, int submode)
{
    WCachedSample * tc = NULL;
    tc = sampleWCache.Retrieve(0, filename, style, submode);
    lastError = sampleWCache.mError;

    //Sample exists! Get it.
    if (tc && tc->isGood())
    {
        JSample * js = tc->Actual();
        return js;
    }

    return NULL;
}

string WResourceManager::graphicsFile(const string& filename)
{
    char buf[512];

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    //Check for a theme style renaming:
    if (filename != "style.txt")
    {
        WStyle * ws = options.getStyle();
        if (ws)
        {
            sprintf(buf, "themes/%s/%s", theme.c_str(), ws->stylized(filename).c_str());
            if (fileOK(buf, true)) return buf;
        }
    }

    if (theme != "" && theme != "Default")
    {
        sprintf(buf, "themes/%s/%s", theme.c_str(), filename.c_str());
        if (fileOK(buf, true)) return buf;
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
    sprintf(graphdir, "graphics/%s", filename.c_str());
    if (fileOK(graphdir, true)) return graphdir;

    //Failure. Check sets.
    sprintf(buf, "sets/%s", filename.c_str());
    if (fileOK(buf, true)) return buf;

    //Failure. Check raw faile.
    sprintf(buf, "%s", filename.c_str());
    if (fileOK(buf, true)) return buf;

    //Complete abject failure. Probably a crash...
    return graphdir;
}

string WResourceManager::avatarFile(const string& filename)
{
    char buf[512];

    //Check the profile folder.
    string profile = options[Options::ACTIVE_PROFILE].str;

    if (profile != "" && profile != "Default")
    {
        sprintf(buf, "profiles/%s/%s", profile.c_str(), filename.c_str());
        if (fileOK(buf, true)) return buf;
    }
    else
    {
        sprintf(buf, "player/%s", filename.c_str());
        if (fileOK(buf, true)) return buf;
    }

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if (theme != "" && theme != "Default")
    {
        sprintf(buf, "themes/%s/%s", theme.c_str(), filename.c_str());
        if (fileOK(buf, true)) return buf;
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
    sprintf(buf, "ai/baka/avatars/%s", filename.c_str());
    if (fileOK(buf, true)) return buf;

    //Failure. Check graphics
    sprintf(buf, "graphics/%s", filename.c_str());
    if (fileOK(buf, true)) return buf;

    //Failure. Check raw faile.
    sprintf(buf, "%s", filename.c_str());
    if (fileOK(buf, true)) return buf;

    //Complete abject failure. Probably a crash...
    return "";
}

string WResourceManager::cardFile(const string& filename)
{
    char buf[512];
    string::size_type i = 0;
    string set;
    JFileSystem* fs = JFileSystem::GetInstance();

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if (theme != "" && theme != "Default")
    {
        //Does this theme use custom cards?
        if (bThemedCards)
        {
            //Check zipped first. Discover set name.
            for (i = 0; i < filename.size(); i++)
            {
                if (filename[i] == '\\' || filename[i] == '/') break;
            }

            if (i != filename.size()) set = filename.substr(0, i);

            if (set.size())
            {
                char zipname[512];
                sprintf(zipname, JGE_GET_RES("themes/%s/sets/%s/%s.zip").c_str(), theme.c_str(), set.c_str(), set.c_str());
                if (fs->AttachZipFile(zipname)) return filename.substr(i + 1);
            }

            sprintf(buf, "themes/%s/sets/%s", theme.c_str(), filename.c_str());
            if (fileOK(buf, true)) return buf; //Themed, unzipped.
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
    if (!set.size())
    { //Didn't fill "set" string, so do it now.
        for (i = 0; i < filename.size(); i++)
        {
            if (filename[i] == '\\' || filename[i] == '/') break;
        }

        if (i != filename.size()) set = filename.substr(0, i);
    }

    if (set.size())
    {
        char zipname[512];
        sprintf(zipname, JGE_GET_RES("sets/%s/%s.zip").c_str(), set.c_str(), set.c_str());
        if (fs->AttachZipFile(zipname)) return filename.substr(i + 1);
    }

    //Failure. Check for unzipped file in sets
    char defdir[512];
    sprintf(defdir, "sets/%s", filename.c_str());
    if (fileOK(defdir, true)) return defdir;

    //Complete failure.
    return "";
}

string WResourceManager::musicFile(const string& filename)
{
    char buf[512];

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if (theme != "" && theme != "Default")
    {
        sprintf(buf, "themes/%s/sound/%s", theme.c_str(), filename.c_str());
        if (fileOK(buf, true)) return buf;
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
    sprintf(defdir, "sound/%s", filename.c_str());
    if (fileOK(defdir, true)) return defdir;

    //Failure. Check raw faile.
    sprintf(defdir, "%s", filename.c_str());
    if (fileOK(defdir, true)) return defdir;

    //Complete abject failure. Probably a crash...
    return "";
}

string WResourceManager::sfxFile(const string& filename)
{
    char buf[512];

    //Check the theme folder.
    string theme = options[Options::ACTIVE_THEME].str;

    if (theme != "" && theme != "Default")
    {
        sprintf(buf, "themes/%s/sound/sfx/%s", theme.c_str(), filename.c_str());
        if (fileOK(buf, true)) return buf;
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
    sprintf(defdir, "sound/sfx/%s", filename.c_str());
    if (fileOK(defdir, true)) return defdir;

    //Complete abject failure. Probably a crash...
    return "";
}

int WResourceManager::dirOK(const string& dirname)
{
    char fname[512];

#if defined (WIN32)
    sprintf(fname,JGE_GET_RES(dirname).c_str());

    struct _stat statBuffer;
    return (_stat(fname, &statBuffer) >= 0 && // make sure it exists
        statBuffer.st_mode & S_IFDIR); // and it's not a file
#else
    sprintf(fname, "%s", JGE_GET_RES(dirname).c_str());
    struct stat st;
    if (stat(fname, &st) == 0) return 1;
#endif
    return 0;
}

int WResourceManager::fileOK(const string& filename, bool relative)
{
    wagic::ifstream * fp = NULL;
    if (relative)
    {
        fp = NEW wagic::ifstream(JGE_GET_RES(filename).c_str());
    }
    else
        fp = NEW wagic::ifstream(filename.c_str());

    int result = 0;
    if (fp)
    {
        if (*fp) result = 1;
        fp->close();
        delete fp;
    }

    return result;
}

void WResourceManager::InitFonts(const std::string& inLang)
{
    unsigned int idOffset = 0;

    if (inLang.compare("cn") == 0)
    {
        mFontFileExtension = kExtension_gbk;
        LoadWFont("simon", 12, Fonts::MAIN_FONT);
        LoadWFont("f3", 16, Fonts::MENU_FONT);
        LoadWFont("magic", 16, Fonts::MAGIC_FONT);
        LoadWFont("smallface", 12, Fonts::SMALLFACE_FONT);

        idOffset = Fonts::kSingleByteFontOffset;
    }

    if (inLang.compare("jp") == 0)
    {
        mFontFileExtension = kExtension_font;
        LoadWFont("simon", 12, Fonts::MAIN_FONT);
        LoadWFont("f3", 16, Fonts::MENU_FONT);
        LoadWFont("magic", 16, Fonts::MAGIC_FONT);
        LoadWFont("smallface", 12, Fonts::SMALLFACE_FONT);

        idOffset = Fonts::kSingleByteFontOffset;
    }

    mFontFileExtension = kExtension_png;
    LoadWFont("simon", 11, Fonts::MAIN_FONT + idOffset);
    GetWFont(Fonts::MAIN_FONT)->SetTracking(-1);
    LoadWFont("f3", 16, Fonts::MENU_FONT + idOffset);
    LoadWFont("magic", 16, Fonts::MAGIC_FONT + idOffset);
    LoadWFont("smallface", 7, Fonts::SMALLFACE_FONT + idOffset);
}

int WResourceManager::ReloadWFonts()
{
    RemoveWFonts();

    string lang = options[Options::LANG].str;
    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    InitFonts(lang);

    return 1;
}

WFont* WResourceManager::LoadWFont(const string& inFontname, int inFontHeight, int inFontID)
{
    WFont* font = GetWFont(inFontID);
    if (font)
    {
        return font;
    }

    string mFontName = inFontname + mFontFileExtension;
    string path = graphicsFile(mFontName);

    if (mFontFileExtension == kExtension_font)
        font = NEW WUFont(inFontID, path.c_str(), inFontHeight, true);
    else if (mFontFileExtension == kExtension_gbk)
        font = NEW WGBKFont(inFontID, path.c_str(), inFontHeight, true);
    else
        font = NEW WLBFont(inFontID, path.c_str(), inFontHeight, true);
    mWFontMap[inFontID] = font;

    return font;
}

WFont* WResourceManager::GetWFont(int id)
{
    WFont* font = NULL;
    FontMap::iterator iter = mWFontMap.find(id);
    if (iter != mWFontMap.end())
    {
        font = iter->second;
    }
    return font;
}

void WResourceManager::RemoveWFonts()
{
    for (FontMap::iterator font = mWFontMap.begin(); font != mWFontMap.end(); ++font)
    {
        delete font->second;
    }
    mWFontMap.clear();
}

void WResourceManager::ResetCacheLimits()
{
#if defined WIN32 || defined LINUX || defined (IOS)
#ifdef FORCE_LOW_CACHE_MEMORY
    textureWCache.Resize(kConstrainedCacheLimit, MAX_CACHE_OBJECTS);
#else
    textureWCache.Resize(HUGE_CACHE_LIMIT,MAX_CACHE_OBJECTS);
#endif
#else
    unsigned int ram = ramAvailable();
    unsigned int myNewSize = ram - OPERATIONAL_SIZE + textureWCache.totalSize;
    if (myNewSize < TEXTURES_CACHE_MINSIZE)
    {
        DebugTrace( "Error, Not enough RAM for Cache: " << myNewSize << " - total Ram: " << ram);
    }
    textureWCache.Resize(MIN(myNewSize, HUGE_CACHE_LIMIT), MAX_CACHE_OBJECTS);

    DebugTrace("Texture cache resized to " << myNewSize);
#endif
    return;
}

JMusic * WResourceManager::ssLoadMusic(const char *fileName)
{
    string file = musicFile(fileName);
    if (!file.size()) return NULL;
    return JSoundSystem::GetInstance()->LoadMusic(file.c_str());
}

void WResourceManager::Refresh()
{
    //Really easy cache relinking.
    ReloadWFonts();
    sampleWCache.Refresh();
    textureWCache.Refresh();
    psiWCache.Refresh();

    //Check for card images in theme.
    bThemedCards = false;
    if (!options[Options::ACTIVE_THEME].isDefault())
    {
        char buf[512];
        sprintf(buf, "themes/%s/sets", options[Options::ACTIVE_THEME].str.c_str());

        if (dirOK(buf)) bThemedCards = true;
    }
}

//WCache
template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveOldest()
{
    typename map<int, cacheItem*>::iterator oldest = cache.end();

    for (typename map<int, cacheItem*>::iterator it = cache.begin(); it != cache.end(); ++it)
    {
        if (it->second && !it->second->isLocked() && (oldest == cache.end() || it->second->lastTime < oldest->second->lastTime)) oldest
            = it;
    }

    if (oldest != cache.end() && oldest->second && !oldest->second->isLocked())
    {
#ifdef DEBUG_CACHE
        std::ostringstream stream;
        stream << "erasing from cache: "  << oldest->second->mFilename << " " << oldest->first;
        LOG(stream.str().c_str());
#endif
        Delete(oldest->second);
        cache.erase(oldest);
        return true;
    }

    return false;

}
template<class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::ClearUnlocked()
{
    typename map<int, cacheItem*>::iterator it, next;

    for (it = cache.begin(); it != cache.end(); it = next)
    {
        next = it;
        next++;

        if (it->second && !it->second->isLocked())
        {
            Delete(it->second);
            cache.erase(it);
        }
        else if (!it->second)
        {
            cache.erase(it);
        }
    }
}
template<class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Resize(unsigned long size, int items)
{
    maxCacheSize = size;

#ifdef DEBUG_CACHE
    std::ostringstream stream;
    stream << "Max cache limit resized to " << size << ", items limit reset to " << items;
    LOG(stream.str().c_str());
#endif

    if (items > MAX_CACHE_OBJECTS || items < 1)
        maxCached = MAX_CACHE_OBJECTS;
    else
        maxCached = items;
}

template<class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::AttemptNew(const string& filename, int submode)
{
    cacheItem* item = NEW cacheItem;
    if (!item)
    {
        mError = CACHE_ERROR_BAD_ALLOC;
        return NULL;
    }

    mError = CACHE_ERROR_NONE;

    if (!item->Attempt(filename, submode, mError) || !item->isGood())
    {
        //No such file. Fail
        if (mError == CACHE_ERROR_404)
        {
            DebugTrace("AttemptNew failed to load. Deleting cache item " << ToHex(item));
            SAFE_DELETE(item);
            return NULL;
        }
		else
        {
            DebugTrace("AttemptNew failed to load (not a 404 error). Deleting cache item " << ToHex(item));
            SAFE_DELETE(item);
            mError = CACHE_ERROR_BAD;
        	return NULL;
        }
    }

    //Success! Enforce cache limits, then return.
    mError = CACHE_ERROR_NONE;

    return item;
}

template<class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::Retrieve(int id, const string& filename, int style, int submode)
{
    //Check cache.
    mError = CACHE_ERROR_NONE; //Reset error status.
    cacheItem* tc = Get(id, filename, style, submode);

    //Retrieve resource only works on permanent items.
    if (style == RETRIEVE_RESOURCE && tc && !tc->isPermanent())
    {
        mError = CACHE_ERROR_NOT_MANAGED;
        return NULL;
    }

    //Perform lock or unlock on entry.
    if (tc)
    {
        switch (style)
        {
        case RETRIEVE_LOCK:
            tc->lock();
            break;
        case RETRIEVE_UNLOCK:
            tc->unlock();
            break;
        case RETRIEVE_MANAGE:
            if (!tc->isPermanent())
            {
                //Unlink the managed resource from the cache.
                UnlinkCache(tc);

                //Post it in managed resources.
                managed[makeID(id, filename, submode)] = tc;
                tc->deadbolt();
            }
            break;
        }
    }

    //Resource exists!
    if (tc)
    {
        if (tc->isGood())
        {
            tc->hit();
            return tc; //Everything fine.
        }
        //Something went wrong.
        RemoveItem(tc);
        mError = CACHE_ERROR_BAD;
    }

    //Record managed failure. Cache failure is recorded in Get().
    if ((style == RETRIEVE_MANAGE || style == RETRIEVE_RESOURCE) && mError == CACHE_ERROR_404)
        managed[makeID(id, filename, submode)] = NULL;

    return NULL;
}

template<class cacheItem, class cacheActual>
int WCache<cacheItem, cacheActual>::makeID(int id, const string& filename, int submode)
{
    int mId = id;
    if (!mId)
    {
        mId = ids[filename];
        if (!mId)
        {
            mId = idCounter++;
            ids[filename] = mId;
        }
    }

    if (submode & TEXTURE_SUB_THUMB)
        mId += THUMBNAILS_OFFSET;

    return mId;
}

template<class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::Get(int id, const string& filename, int style, int submode)
{
    typename map<int, cacheItem*>::iterator it;
    int lookup = makeID(id, filename, submode);

    //Check for managed resources first. Always
    it = managed.find(lookup);

    //Something is managed.
    if (it != managed.end())
    {
        return it->second; //A hit.
    }

    //Not managed, so look in cache.
    if (style != RETRIEVE_MANAGE)
    {
        boost::mutex::scoped_lock lock(mCacheMutex);
        //DebugTrace("Cache lock acquired, looking up index " << lookup);
        it = cache.find(lookup);
        //Well, we've found something...
        if (it != cache.end())
        {
            if (!it->second)
            {
                mError = CACHE_ERROR_404;
                DebugTrace("cache hit, no item??");
                //assert(false);
            }
            return it->second; //A hit, or maybe a miss.
        }
    }

    // not hit in the cache, respect the RETRIEVE_EXISTING flag if present
    if (style == RETRIEVE_EXISTING)
    {
        return NULL;
    }

    // no hit in cache, clear space before attempting to load a new one
    // note: Cleanup() should ONLY be ever called on the main (UI) thread!
    Cleanup();

    // check if we're doing a card lookup
    if (submode & TEXTURE_SUB_CARD)
    {
        // processing a cache miss, return a generic card & queue up an async read

        // side note:  using a string reference here to a global predefined string, as basic_string is not thread safe for allocations!
        const std::string& cardPath = (submode & TEXTURE_SUB_THUMB) ? kGenericThumbCard : kGenericCard;
        int genericCardId = makeID(0, cardPath, CACHE_NORMAL);
        it = managed.find(genericCardId);
        assert(it != managed.end());

        CacheEngine::Instance()->QueueRequest(filename, submode, lookup);
        return it->second;
    }

    //Space in cache, make new texture
    return LoadIntoCache(lookup, filename, submode, style);
}

template<class cacheItem, class cacheActual>
cacheItem* WCache<cacheItem, cacheActual>::LoadIntoCache(int id, const string& filename, int submode, int style)
{
    // note: my original implementation only had one lock (the cache mutex lock) - I eventually
    // added this second one, as locking at the Get() call means that the main thread is blocked on doing a simple cache
    // check.  If you're hitting the system hard (like, paging up in the deck editor which forces 7 cards to load simultaneously),
    // we'd block the UI thread for a long period at this point.  So I moved the cache mutex to lock specifically only attempts to touch
    // the shared cache container, and this separate lock was added to insulate us against thread safety issues in JGE. In particular,
    // JFileSystem is particularly unsafe, as it assumes that we have only one zip loaded at a time... rather than add mutexes in JGE,
    // I've kept it local to here.
    boost::mutex::scoped_lock functionLock(mLoadFunctionMutex);
    cacheItem* item = AttemptNew(filename, submode);
    //assert(item);
    if (style == RETRIEVE_MANAGE)
    {
        if (mError == CACHE_ERROR_404 || item)
        {
            managed[id] = item; //Record a hit or miss.
        }
        if (item)
        {
            item->deadbolt(); //Make permanent.
        }
    }
    else
    {
        if (mError == CACHE_ERROR_404 || item)
        {
            boost::mutex::scoped_lock lock(mCacheMutex);
            cache[id] = item;
            DebugTrace("inserted item ptr " << ToHex(item) << " at index " << id);
        }
    }

    if (item == NULL)
    {
        DebugTrace("Can't locate ");
        if (submode & TEXTURE_SUB_THUMB)
        {
            DebugTrace("thumbnail ");
        }
        DebugTrace(filename);
        
        return NULL; //Failure
    }

    //Succeeded in making a new item.
    unsigned long isize = item->size();
    totalSize += isize;

    mError = CACHE_ERROR_NONE;
    if (style != RETRIEVE_MANAGE)
    {
        cacheItems++;
        cacheSize += isize;
    }

#ifdef DEBUG_CACHE
    std::ostringstream stream;
    stream << "Cache insert: " << filename << " " << id << ", cacheItem count: " << cacheItems << ", cacheSize is now: " << cacheSize;
    LOG(stream.str().c_str());
#endif

    return item;
}

template<class cacheItem, class cacheActual>
void WCache<cacheItem, cacheActual>::Refresh()
{
    typename map<int, cacheItem*>::iterator it;
    ClearUnlocked();

    for (it = cache.begin(); it != cache.end(); it++)
    {
        if (it->second)
        {
            it->second->Refresh();
        }
    }
    for (it = managed.begin(); it != managed.end(); it++)
    {
        if (it->second)
        {
            it->second->Refresh();
        }
    }
}

template<class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::WCache()
{
    cacheSize = 0;
    totalSize = 0;

    maxCacheSize = TEXTURES_CACHE_MINSIZE;

    maxCached = MAX_CACHE_OBJECTS;
    cacheItems = 0;
    mError = CACHE_ERROR_NONE;
}

template<class cacheItem, class cacheActual>
WCache<cacheItem, cacheActual>::~WCache()
{
    typename map<int, cacheItem*>::iterator it;

    //Delete from cache & managed
    for (it = cache.begin(); it != cache.end(); it++)
    {
        SAFE_DELETE(it->second);
    }

    for (it = managed.begin(); it != managed.end(); it++)
    {
        SAFE_DELETE(it->second);
    }
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Cleanup()
{
    bool result = true;
    // this looks redundant, but the idea is, don't grab the mutex if there's no work to do
    if (RequiresMissCleanup())
    {
        boost::mutex::scoped_lock lock(mCacheMutex);
        while (RequiresMissCleanup())
        {
            RemoveMiss();
        }
    }

    if (RequiresOldItemCleanup())
    {
        boost::mutex::scoped_lock lock(mCacheMutex);
        while (RequiresOldItemCleanup())
        {
            if (!RemoveOldest())
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

bool WCacheSort::operator()(const WResource * l, const WResource * r)
{
    if (!l || !r) return false;
    return (l->lastTime < r->lastTime);
}

template<class cacheItem, class cacheActual>
unsigned int WCache<cacheItem, cacheActual>::Flatten()
{
    vector<cacheItem*> items;
    unsigned int oldest = 0;
    unsigned int lastSet = 0;

    if (!cache.size()) return 0;

    for (typename map<int, cacheItem*>::iterator it = cache.begin(); it != cache.end(); ++it)
    {
        if (!it->second) continue;
        items.push_back(it->second);
    }

    sort(items.begin(), items.end(), WCacheSort());

    for (typename vector<cacheItem*>::iterator it = items.begin(); it != items.end(); ++it)
    {
        assert((*it) && (*it)->lastTime > lastSet);
        lastSet = (*it)->lastTime;
        (*it)->lastTime = ++oldest;
    }

    return oldest + 1;
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveMiss(int id)
{
    typename map<int, cacheItem*>::iterator it = cache.end();

    for (it = cache.begin(); it != cache.end(); it++)
    {
        if ((id == 0 || it->first == id) && it->second == NULL) break;
    }

    if (it != cache.end())
    {
        cache.erase(it);
        return true;
    }

    return false;
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::RemoveItem(cacheItem * item, bool force)
{
    typename map<int, cacheItem*>::iterator it;

    if (item == NULL) return false; //Use RemoveMiss to remove cache misses, not this.

    for (it = cache.begin(); it != cache.end(); it++)
    {
        if (it->second == item) break;
    }
    if (it != cache.end() && it->second && (force || !it->second->isLocked()))
    {
        Delete(it->second);
        cache.erase(it);
        return true;
    }

    return false;
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::UnlinkCache(cacheItem * item)
{
    typename map<int, cacheItem*>::iterator it = cache.end();

    if (item == NULL) return false; //Use RemoveMiss to remove cache misses, not this.

    for (it = cache.begin(); it != cache.end(); it++)
    {
        if (it->second == item) break;
    }
    if (it != cache.end() && it->second)
    {
        it->second = NULL;
        unsigned long isize = item->size();

        cacheSize -= isize;
        cacheItems--;
        cache.erase(it);
        return true;
    }

    return false;
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Delete(cacheItem * item)
{
    if (!item) return false;

    unsigned long isize = item->size();
    totalSize -= isize;
    cacheSize -= isize;
#ifdef DEBUG_CACHE
    if(cacheItems == 0)
        DebugTrace("cacheItems out of sync.");
#endif

    cacheItems--;

    DebugTrace("Deleting cache item " << ToHex(item));
    SAFE_DELETE(item);
    return true;
}

template<class cacheItem, class cacheActual>
bool WCache<cacheItem, cacheActual>::Release(cacheActual* actual)
{
    if (!actual) return false;

    typename map<int, cacheItem*>::iterator it;
    for (it = cache.begin(); it != cache.end(); it++)
    {
        if (it->second && it->second->compare(actual)) break;
    }

    if (it == cache.end()) return false; //Not here, can't release.

    if (it->second)
    {
        it->second->unlock(); //Release one lock.
        if (it->second->isLocked()) return true; //Still locked, won't delete, not technically a failure.
    }

    //Released!
    Delete(it->second);
    cache.erase(it);
    return true;
}
