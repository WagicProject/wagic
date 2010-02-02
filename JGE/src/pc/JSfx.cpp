//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------


#ifdef WITH_FMOD
#include "../../Dependencies/include/fmod.h"
#else
#define FSOUND_FREE 0
#endif

#include "../../include/JSoundSystem.h"
#include "../../include/JFileSystem.h"


//////////////////////////////////////////////////////////////////////////
JMusic::JMusic()
{
}

void JMusic::Update(){

}

int JMusic::getPlayTime(){
#ifdef WITH_FMOD
  return FSOUND_GetCurrentPosition(JSoundSystem::GetInstance()->mChannel)/44.1; //todo more generic, here it's only 44kHz
#else
  return 0;
#endif
}

JMusic::~JMusic()
{
#ifdef WITH_FMOD
  JSoundSystem::GetInstance()->StopMusic(this);
  if (mTrack) FSOUND_Sample_Free(mTrack);
#endif
}


//////////////////////////////////////////////////////////////////////////
JSample::JSample()
{

}

JSample::~JSample()
{
#ifdef WITH_FMOD
  if (mSample) FSOUND_Sample_Free(mSample);
#endif
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
  mChannel = FSOUND_FREE;
}

JSoundSystem::~JSoundSystem()
{
}

void JSoundSystem::InitSoundSystem()
{
#ifdef WITH_FMOD
  FSOUND_Init(44100, 32, 0);
#endif
}


void JSoundSystem::DestroySoundSystem()
{
#ifdef WITH_FMOD
  FSOUND_Close();
#endif
}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
#ifndef WITH_FMOD
  return NULL;
#else
  JMusic* music = new JMusic();
  if (music)
    {
      JFileSystem* fileSystem = JFileSystem::GetInstance();
      if (fileSystem->OpenFile(fileName))
	{
	  int size = fileSystem->GetFileSize();
	  char *buffer = new char[size];
	  fileSystem->ReadFile(buffer, size);
	  music->mTrack = FSOUND_Sample_Load(FSOUND_UNMANAGED, buffer, FSOUND_LOADMEMORY, 0, size);

	  delete[] buffer;
	  fileSystem->CloseFile();
	}
    }
  return music;
#endif
}


void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
#ifdef WITH_FMOD
  if (music && music->mTrack)
    {
      mChannel = FSOUND_PlaySound(mChannel, music->mTrack);
      SetMusicVolume(mVolume);

      if (looping)
	FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_NORMAL);
      else
	FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_OFF);
    }
#endif
}


void JSoundSystem::StopMusic(JMusic *music)
{
#ifdef WITH_FMOD
  FSOUND_StopSound(mChannel);
#endif
}


void JSoundSystem::SetVolume(int volume)
{
  SetMusicVolume(volume);
  SetSfxVolume(volume);
}

void JSoundSystem::SetMusicVolume(int volume)
{
#ifdef WITH_FMOD
  if (mChannel != FSOUND_FREE) FSOUND_SetVolumeAbsolute(mChannel,volume * 2.55);
#endif
  mVolume = volume;
}

void JSoundSystem::SetSfxVolume(int volume){
  //this sets the volume to all channels then reverts back the volume for music..
  //that's a bit dirty but it works
#ifdef WITH_FMOD
  FSOUND_SetVolumeAbsolute(FSOUND_ALL, volume * 2.55);
#endif
  mSampleVolume = volume;
  SetMusicVolume(mVolume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
#ifndef WITH_FMOD
  return NULL;
#else
  JSample* sample = new JSample();
  if (sample)
    {
      JFileSystem* fileSystem = JFileSystem::GetInstance();
      if (fileSystem->OpenFile(fileName))
	{
	  int size = fileSystem->GetFileSize();
	  char *buffer = new char[size];
	  fileSystem->ReadFile(buffer, size);
	  sample->mSample = FSOUND_Sample_Load(FSOUND_UNMANAGED, buffer, FSOUND_LOADMEMORY, 0, size);

	  delete[] buffer;
	  fileSystem->CloseFile();
	}else
	sample->mSample = NULL;

    }
  return sample;
#endif
}


void JSoundSystem::PlaySample(JSample *sample)
{
#ifdef WITH_FMOD
  if (sample && sample->mSample){
    int channel = FSOUND_PlaySound(FSOUND_FREE, sample->mSample);
    FSOUND_SetVolumeAbsolute(channel,mSampleVolume * 2.55);
  }
#endif
}

