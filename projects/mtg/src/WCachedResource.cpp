#include "PrecompiledHeader.h"

#include "utils.h"
#include <JFileSystem.h>
#include "GameOptions.h"
#include "WResourceManager.h"
#include <hge/hgeparticle.h>

#ifdef WITH_FMOD
#endif

//WResource
WResource::~WResource()
{
}

WResource::WResource()
{
    locks = WRES_UNLOCKED;
    lastTime = resources.nowTime();
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
    lastTime = resources.nowTime();
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

    if (!trackedQuads.size()) return;

    vector<WTrackedQuad*>::iterator it;
    WTrackedQuad * tq = NULL;

    for (it = trackedQuads.begin(); it != trackedQuads.end(); it++)
    {
        tq = (*it);
        SAFE_DELETE(tq);
    }
    trackedQuads.clear();
}

JTexture * WCachedTexture::Actual()
{
    return texture;
}

bool WCachedTexture::isLocked()
{
    if (locks != WRES_UNLOCKED) return true;

    for (vector<WTrackedQuad*>::iterator it = trackedQuads.begin(); it != trackedQuads.end(); it++)
    {
        if ((*it)->isLocked()) return true;
    }

    return false;
}

bool WCachedTexture::ReleaseQuad(JQuad* quad)
{
    if (quad == NULL) return false;

    WTrackedQuad * tq = NULL;
    vector<WTrackedQuad*>::iterator nit;
    for (vector<WTrackedQuad*>::iterator it = trackedQuads.begin(); it != trackedQuads.end(); it = nit)
    {
        nit = it;
        nit++;
        if ((*it) && (*it)->quad == quad)
        {
            tq = (*it);
            tq->unlock();

            if (!tq->isLocked())
            {
                SAFE_DELETE(tq);
                trackedQuads.erase(it);
            }

            return true; //Returns true when found.
        }
    }
    return false;
}

WTrackedQuad * WCachedTexture::GetTrackedQuad(float offX, float offY, float width, float height, string resname)
{
    if (!texture) return NULL;

    bool allocated = false;
    WTrackedQuad * tq = NULL;
    JQuad * quad = NULL;

    vector<WTrackedQuad*>::iterator it;

    if (width == 0.0f || width > static_cast<float> (texture->mWidth)) width = static_cast<float> (texture->mWidth);
    if (height == 0.0f || height > static_cast<float> (texture->mHeight)) height = static_cast<float> (texture->mHeight);

    for (it = trackedQuads.begin(); it != trackedQuads.end(); it++)
    {
        if ((*it) && (*it)->resname == resname)
        {
            tq = (*it);
            break;
        }
    }

    if (!tq)
    {
        allocated = true;
        tq = NEW WTrackedQuad(resname);
        if (!tq) return NULL;
    }

    quad = tq->quad;

    if (!quad)
    {
        quad = NEW JQuad(texture, offX, offY, width, height);
        /*
         There's a risk this erases the texture calling the quad creation.... Erwan 2010/03/13
         if(!quad) {
         //Probably out of memory. Try again.
         resources.Cleanup();
         quad = NEW JQuad(texture,offX,offY,width,height);
         }
         */
        if (!quad)
        {
            if (allocated && tq)
            SAFE_DELETE(tq);
            fprintf(stderr, "WCACHEDRESOURCE:GetTrackedQuad -  Quad is null\n");
            return NULL; //Probably a crash.
        }

        tq->quad = quad;
        if (allocated) trackedQuads.push_back(tq);
        return tq;
    }

    //Update JQ's values to what we called this with.
    quad->SetTextureRect(offX, offY, width, height);
    return tq;

}

JQuad * WCachedTexture::GetQuad(float offX, float offY, float width, float height, string resname)
{
    WTrackedQuad * tq = GetTrackedQuad(offX, offY, width, height, resname);

    if (tq) return tq->quad;

    return NULL;
}

JQuad * WCachedTexture::GetQuad(string resname)
{
    vector<WTrackedQuad*>::iterator it;

    for (it = trackedQuads.begin(); it != trackedQuads.end(); it++)
    {
        if ((*it) && (*it)->resname == resname)
        {
            return (*it)->quad;
        }
    }

    return NULL;
}
JQuad * WCachedTexture::GetCard(float offX, float offY, float width, float height, string resname)
{
    JQuad * jq = GetQuad(offX, offY, width, height, resname);
    if (jq) jq->SetHotSpot(static_cast<float> (jq->mTex->mWidth / 2), static_cast<float> (jq->mTex->mHeight / 2));

    return jq;
}

unsigned long WCachedTexture::size()
{
    if (!texture) return 0;

    unsigned int pixel_size = 4;
#if defined WIN32 || defined LINUX || defined IOS
#else
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

    for (vector<WTrackedQuad*>::iterator it = trackedQuads.begin(); it != trackedQuads.end(); it++)
    {
        if ((*it) && (*it)->quad) (*it)->quad->mTex = texture;
    }
}

bool WCachedTexture::Attempt(string filename, int submode, int & error)
{
    mFilename = filename;
    int format = TEXTURE_FORMAT;
    loadedMode = submode;
    string realname;

    //Form correct filename.
    if (submode & TEXTURE_SUB_EXACT)
        realname = filename;
    else if (submode & TEXTURE_SUB_CARD)
    {
        if (submode & TEXTURE_SUB_THUMB)
        {
            for (string::size_type i = 0; i < filename.size(); i++)
            {
                if (filename[i] == '\\' || filename[i] == '/')
                {
                    filename.insert(i + 1, "thumbnails/");
                    break;
                }
            }

        }
        realname = resources.cardFile(filename);
    }
    else
    {
        if (submode & TEXTURE_SUB_THUMB) filename.insert(0, "thumbnails/");

        if (submode & TEXTURE_SUB_AVATAR)
            realname = resources.avatarFile(filename);
        else
            realname = resources.graphicsFile(filename);
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

bool WCachedSample::Attempt(string filename, int submode, int & error)
{
    loadedMode = submode;

    sample = JSoundSystem::GetInstance()->LoadSample(resources.sfxFile(filename).c_str());

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

bool WCachedParticles::Attempt(string filename, int submode, int & error)
{

    JFileSystem* fileSys = JFileSystem::GetInstance();

    if (!fileSys->OpenFile(resources.graphicsFile(filename)))
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
    fileSys->ReadFile(&(particles->nEmission), sizeof(hgeParticleSystemInfo));
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

//WTrackedQuad
unsigned long WTrackedQuad::size()
{
    return sizeof(JQuad);
}

bool WTrackedQuad::isGood()
{
    return (quad != NULL);
}

WTrackedQuad::WTrackedQuad(string _resname)
{
    quad = NULL;
    resname = _resname;
}

WTrackedQuad::~WTrackedQuad()
{
    if (quad)
    SAFE_DELETE(quad);
}
