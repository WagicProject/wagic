#include "PrecompiledHeader.h"

#include "utils.h"
#include <JFileSystem.h>
#include "WResourceManager.h"
#include "WCachedResource.h"
#include <hge/hgeparticle.h>

#ifdef WITH_FMOD
#endif

const std::string kPlaceholderID("placeholder");

//WResource
WResource::~WResource()
{
}

WResource::WResource()
{
    locks = WRES_UNLOCKED;
    lastTime = WResourceManager::Instance()->nowTime();
    loadedMode = 0;
}

bool WResource::isLocked()
{
    return (locks != WRES_UNLOCKED);
}

bool WResource::isPermanent()
{
    return (locks == WRES_PERMANENT);
}

void WResource::deadbolt()
{
    if (locks <= WRES_MAX_LOCK) locks = WRES_PERMANENT;
}

void WResource::lock()
{
    if (locks < WRES_MAX_LOCK) locks++;
    // watch out for this - if the lock count increases beyond a reasonable threshold, 
    // someone is probably not releasing the texture correctly
    assert(locks < 50);
}

void WResource::unlock(bool force)
{
    if (force)
        locks = 0;
    else if (locks > WRES_UNLOCKED)
    {
        if (locks <= WRES_MAX_LOCK) locks--;
    }
    else
#ifdef DEBUG_CACHE
        locks = WRES_UNDERLOCKED;
#else
        locks = 0;
#endif

}

void WResource::hit()
{
    lastTime = WResourceManager::Instance()->nowTime();
}

WCachedResource::~WCachedResource()
{
    DebugTrace("Destroying WCachedResource: " << mFilename);
}

//WCachedTexture
WCachedTexture::WCachedTexture()
{
    texture = NULL;
}

WCachedTexture::~WCachedTexture()
{
    if (texture)
    	SAFE_DELETE(texture);
}

JTexture * WCachedTexture::Actual()
{
    return texture;
}

bool WCachedTexture::isLocked()
{
    return (locks != WRES_UNLOCKED);
}

JQuadPtr WCachedTexture::GetQuad(float offX, float offY, float width, float height, const string& resname)
{
    if (!texture) return JQuadPtr();

    if (width == 0.0f || width > static_cast<float> (texture->mWidth)) width = static_cast<float> (texture->mWidth);
    if (height == 0.0f || height > static_cast<float> (texture->mHeight)) height = static_cast<float> (texture->mHeight);

    // If we're fetching a card resource, but it's not available yet, we'll be attempting to get the Quad from the temporary back image.
    // If that's the case, don't stash a separate tracked quad entry for each card name in the the Back/BackThumbnail's resource
    string resource(resname);
    if (mFilename == kGenericCard || mFilename == kGenericThumbCard)
    {
        // if we're the back or thumb_back file, but we've been asked for a card ID, then assign it
        // a placeholder ID.  Reason being, hotspots on quads are different (ie centered) for card images, so we'll store a separate quad for cards 
        if (resname != kGenericCardID && resname != kGenericCardThumbnailID)
            resource = kPlaceholderID;
    }

	std::map<string, JQuadPtr>::iterator iter = mTrackedQuads.find(resource);
	if (iter != mTrackedQuads.end())
		return iter->second;

	JQuadPtr quad(NEW JQuad(texture, offX, offY, width, height));

    //Update JQ's values to what we called this with.
    quad->SetTextureRect(offX, offY, width, height);
	mTrackedQuads.insert(std::pair<string, JQuadPtr>(resource, quad));
    return quad;

}

JQuadPtr WCachedTexture::GetQuad(const string& resname)
{
	JQuadPtr result;
 	std::map<string, JQuadPtr>::iterator iter = mTrackedQuads.find(resname);
	if (iter != mTrackedQuads.end())
		result = iter->second;

	return result;
}

JQuadPtr WCachedTexture::GetCard(float offX, float offY, float width, float height, const string& resname)
{
    JQuadPtr jq = GetQuad(offX, offY, width, height, resname);
    if (jq.get())
        jq->SetHotSpot(static_cast<float> (jq->mTex->mWidth / 2), static_cast<float> (jq->mTex->mHeight / 2));

    return jq;
}

unsigned long WCachedTexture::size()
{
    if (!texture) return 0;

    unsigned int pixel_size = 4;
#if defined(PSP)
    pixel_size = JRenderer::GetInstance()->PixelSize(texture->mTextureFormat);
#endif
    return texture->mTexHeight * texture->mTexWidth * pixel_size;
}

bool WCachedTexture::isGood()
{
    return (texture != NULL);
}

void WCachedTexture::Refresh()
{
    int error = 0;
    JTexture* old = texture;
    texture = NULL;

    if (!Attempt(mFilename, loadedMode, error))
        SAFE_DELETE(texture);

    if (!texture)
        texture = old;
    else
        SAFE_DELETE(old);

    JRenderer::GetInstance()->TransferTextureToGLContext(*texture);

    for (map<string, JQuadPtr>::iterator it = mTrackedQuads.begin(); it != mTrackedQuads.end(); ++it)
    {
        if (it->second.get())
            it->second->mTex = texture;
    }
}

bool WCachedTexture::Attempt(const string& filename, int submode, int & error)
{
    mFilename = filename;
    int format = TEXTURE_FORMAT;
    loadedMode = submode;
    string realname = filename;

    //Form correct filename.
	if (submode & TEXTURE_SUB_CARD)
    {
        if (submode & TEXTURE_SUB_THUMB)
        {
            for (string::size_type i = 0; i < realname.size(); i++)
            {
                if (realname[i] == '\\' || realname[i] == '/')
                {
                    realname.insert(i + 1, "thumbnails/");
                    break;
                }
            }

        }
        realname = WResourceManager::Instance()->cardFile(realname);
    }
    else
    {
        if (submode & TEXTURE_SUB_THUMB) realname.insert(0, "thumbnails/");

        if (submode & TEXTURE_SUB_AVATAR)
            realname = WResourceManager::Instance()->avatarFile(realname);
        else
            realname = WResourceManager::Instance()->graphicsFile(realname);
    }

    //Apply pixel mode
    if (submode & TEXTURE_SUB_5551) format = GU_PSM_5551;

    if (!realname.size())
    {
        error = CACHE_ERROR_404;
        return false;
    }

    texture = JRenderer::GetInstance()->LoadTexture(realname.c_str(), TEX_TYPE_USE_VRAM, format);

    //Failure.
    if (!texture)
    {
        error = CACHE_ERROR_BAD;
        if (!fileExists(realname.c_str())) error = CACHE_ERROR_404;
        return false;
    }

    error = CACHE_ERROR_NONE;
    return true;
}

//WCachedSample
WCachedSample::WCachedSample()
{
    sample = NULL;
}

WCachedSample::~WCachedSample()
{
    SAFE_DELETE(sample);
}

JSample * WCachedSample::Actual()
{
    return sample;
}

unsigned long WCachedSample::size()
{
    if (!sample || !sample->mSample) return 0;
    return sample->fileSize();
}

bool WCachedSample::isGood()
{
    if (!sample || !sample->mSample) return false;

    return true;
}

void WCachedSample::Refresh()
{
    return;
}

bool WCachedSample::Attempt(const string& filename, int submode, int & error)
{
    loadedMode = submode;
    string sfxFile = WResourceManager::Instance()->sfxFile(filename);
    sample = JSoundSystem::GetInstance()->LoadSample(sfxFile.c_str());

    if (!isGood())
    {
        SAFE_DELETE(sample);
        if (!fileExists(filename.c_str()))
            error = CACHE_ERROR_404;
        else
            error = CACHE_ERROR_BAD;
        return false;
    }

    error = CACHE_ERROR_NONE;
    return true;
}

//WCachedParticles

bool WCachedParticles::isGood()
{
    if (!particles) return false;
    return true;
}

unsigned long WCachedParticles::size()
{
    if (!particles) return 0; //Sizeof(pointer)

    return sizeof(hgeParticleSystemInfo);
}

//Only effects future particle systems, of course.
void WCachedParticles::Refresh()
{
    hgeParticleSystemInfo * old = particles;

    int error = 0;
    Attempt(mFilename, loadedMode, error);

    if (isGood())
        SAFE_DELETE(old);
    else
    {
        SAFE_DELETE(particles);
        particles = old;
    }

    return;
}

bool WCachedParticles::Attempt(const string& filename, int submode, int & error)
{

    JFileSystem* fileSys = JFileSystem::GetInstance();

    if (!fileSys->OpenFile(WResourceManager::Instance()->graphicsFile(filename)))
    {
        error = CACHE_ERROR_404;
        return false;
    }

    SAFE_DELETE(particles);

    particles = NEW hgeParticleSystemInfo;
    // We Skip reading the pointer as it may be larger than 4 bytes in the structure
    void *dummyPointer;
    fileSys->ReadFile(&dummyPointer, 4);
    // we're actually trying to read more than the file size now, but it's no problem.
    // Note that this fix is only to avoid the largest problems, filling a structure
    // by directly reading a file, is really a bad idea ...
    fileSys->ReadFile(&(particles->nEmission), sizeof(hgeParticleSystemInfo) - sizeof(void*));
    fileSys->CloseFile();

    particles->sprite = NULL;
    error = CACHE_ERROR_NONE;
    return true;
}

hgeParticleSystemInfo * WCachedParticles::Actual()
{
    return particles;
}

WCachedParticles::WCachedParticles()
{
    particles = NULL;
}
WCachedParticles::~WCachedParticles()
{
    SAFE_DELETE(particles);
}
