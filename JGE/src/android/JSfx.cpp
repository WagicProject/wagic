#include "DebugRoutines.h"

// for native audio
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

#include "../../include/JSoundSystem.h"
#include "../../include/JFileSystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////////

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
//////////////////////////////////////////////////////////////////////////

JMusic::JMusic()
    : playerObject(0), playInterface(0), seekInterface(0)
{
}

void JMusic::Update(){

}

int JMusic::getPlayTime(){
  return 0;
}

JMusic::~JMusic()
{
    // destroy file descriptor audio player object, and invalidate all associated interfaces
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playInterface = NULL;
        seekInterface = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
JSample::JSample()
    : playerObject(0), playInterface(0)
{

}

JSample::~JSample()
{
    // destroy file descriptor audio player object, and invalidate all associated interfaces
    if (playerObject != NULL) {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playInterface = NULL;
    }
}

unsigned long JSample::fileSize()
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////
JSoundSystem* JSoundSystem::mInstance = NULL;

JSoundSystem* JSoundSystem::GetInstance()
{
  if (mInstance == NULL)
    {
      mInstance = new JSoundSystem();
      mInstance->InitSoundSystem();
    }
  return mInstance;
}


void JSoundSystem::Destroy()
{
  if (mInstance)
    {
      mInstance->DestroySoundSystem();
      delete mInstance;
      mInstance = NULL;
    }
}


JSoundSystem::JSoundSystem()
{
  mVolume = 0;
  mSampleVolume = 0;
}

JSoundSystem::~JSoundSystem()
{
}

void JSoundSystem::InitSoundSystem()
{
    DebugTrace("InitSoundSystem enter");
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    DebugTrace("result " << result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    DebugTrace("result " << result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    DebugTrace("result " << result);

    // create output mix
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    DebugTrace("result " << result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    DebugTrace("result " << result);

    DebugTrace("InitSoundSystem leave");
}


void JSoundSystem::DestroySoundSystem()
{
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
    JMusic* music = new JMusic();
    if (music)
    {
        // we should use the native asset manager instead
        string fullpath = JFileSystem::GetInstance()->GetResourceFile(fileName);
        int fd = open(fullpath.c_str(), O_RDONLY);
        FILE* file = fdopen(fd, "r");
        off_t start = 0;
        off_t length;
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        SLresult result;

        // configure audio source
        SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
        SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
        SLDataSource audioSrc = {&loc_fd, &format_mime};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids[2] = {SL_IID_SEEK, SL_IID_VOLUME};
        const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        result = (*engineEngine)->CreateAudioPlayer(engineEngine, &music->playerObject, &audioSrc, &audioSnk,
                2, ids, req);
        DebugTrace("result " << result);

        // realize the player
        result = (*music->playerObject)->Realize(music->playerObject, SL_BOOLEAN_FALSE);
        DebugTrace("result " << result);

        // get the play interface
        result = (*music->playerObject)->GetInterface(music->playerObject, SL_IID_PLAY, &music->playInterface);
        DebugTrace("result " << result);

        // get the seek interface
        result = (*music->playerObject)->GetInterface(music->playerObject, SL_IID_SEEK, &music->seekInterface);
        DebugTrace("result " << result);

        // get the volume interface
        //result = (*music->playerObject)->GetInterface(music->playerObject, SL_IID_VOLUME, &musicVolumeInterface);
        DebugTrace("result " << result);
    }
    return music;
}


void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
    if(music && music->playerObject && music->playInterface && music->seekInterface)
    {
        SLresult result;

        // enable whole file looping
        result = (*music->seekInterface)->SetLoop(music->seekInterface,
                                                  looping?SL_BOOLEAN_TRUE:SL_BOOLEAN_FALSE, 0, SL_TIME_UNKNOWN);
        assert(SL_RESULT_SUCCESS == result);

        result = (*music->playInterface)->SetPlayState(music->playInterface,
                                                       SL_PLAYSTATE_PLAYING);
        assert(SL_RESULT_SUCCESS == result);
    }
}


void JSoundSystem::StopMusic(JMusic *music)
{
    if(music && music->playerObject && music->playInterface && music->seekInterface)
    {
        SLresult result;

        result = (*music->playInterface)->SetPlayState(music->playInterface,
                                                       SL_PLAYSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
    }
}


void JSoundSystem::SetVolume(int volume)
{
    SetMusicVolume(volume);
    SetSfxVolume(volume);
}

void JSoundSystem::SetMusicVolume(int volume)
{
    mVolume = volume;
}

void JSoundSystem::SetSfxVolume(int volume){
  mSampleVolume = volume;
  SetMusicVolume(mVolume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
    JSample* sample = new JSample();
    if (sample)
    {
        string fullpath = JFileSystem::GetInstance()->GetResourceFile(fileName);
        int fd = open(fullpath.c_str(), O_RDONLY);
        FILE* file = fdopen(fd, "r");
        off_t start = 0;
        off_t length;
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        SLresult result;

        // configure audio source
        SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
        SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
        SLDataSource audioSrc = {&loc_fd, &format_mime};

        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids[1] = {SL_IID_VOLUME};
        const SLboolean req[1] = {SL_BOOLEAN_TRUE};
        result = (*engineEngine)->CreateAudioPlayer(engineEngine, &sample->playerObject, &audioSrc, &audioSnk,
              1, ids, req);
        DebugTrace("result " << result);

        // realize the player
        result = (*sample->playerObject)->Realize(sample->playerObject, SL_BOOLEAN_FALSE);
        DebugTrace("result " << result);

        // get the play interface
        result = (*sample->playerObject)->GetInterface(sample->playerObject, SL_IID_PLAY, &sample->playInterface);
        DebugTrace("result " << result);

        // get the volume interface
        //result = (*sample->playerObject)->GetInterface(sample->playerObject, SL_IID_VOLUME, &sampleVolumeInterface);
    }
    return sample;
}


void JSoundSystem::PlaySample(JSample *sample)
{
    if(sample && sample->playerObject && sample->playInterface)
    {
        SLresult result;

        result = (*sample->playInterface)->SetPlayState(sample->playInterface,
                                                       SL_PLAYSTATE_PLAYING);
    }
}

