//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#include "../../Dependencies/include/png.h"
#include "../../Dependencies/include/fmod.h"

#include "../../include/JSoundSystem.h"
#include "../../include/JFileSystem.h"


//////////////////////////////////////////////////////////////////////////
JMusic::JMusic()
{
}

void JMusic::Update(){

}

int JMusic::getPlayTime(){
  return FSOUND_GetCurrentPosition(JSoundSystem::GetInstance()->mChannel)/44.1; //todo more generic, here it's only 44kHz
}

JMusic::~JMusic()
{
	JSoundSystem::GetInstance()->StopMusic(this);
	//JSoundSystem::GetInstance()->FreeMusic(this);

	if (mTrack)
		FSOUND_Sample_Free(mTrack);
}


//////////////////////////////////////////////////////////////////////////
JSample::JSample()
{

}

JSample::~JSample()
{
	//JSoundSystem::GetInstance()->FreeSample(this);
	if (mSample)
		FSOUND_Sample_Free(mSample);
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
	FSOUND_Init(44100, 32, 0);
}


void JSoundSystem::DestroySoundSystem()
{
	FSOUND_Close();

}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
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
}


void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
	
	if (music && music->mTrack)
	{
		mChannel = FSOUND_PlaySound(mChannel, music->mTrack);
    SetMusicVolume(mVolume);

		if (looping)
			FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_NORMAL);
		else
			FSOUND_SetLoopMode(mChannel, FSOUND_LOOP_OFF);
		
	}
}


void JSoundSystem::StopMusic(JMusic *music)
{
 	FSOUND_StopSound(mChannel);
}


void JSoundSystem::SetVolume(int volume)
{
		SetMusicVolume(volume);
    SetSfxVolume(volume);
}

void JSoundSystem::SetMusicVolume(int volume)
{
	if (mChannel != FSOUND_FREE) FSOUND_SetVolumeAbsolute(mChannel,volume * 2.55);	
	mVolume = volume;
}

void JSoundSystem::SetSfxVolume(int volume){
  //this sets the volume to all channels then reverts back the volume for music.. 
  //that's a bit dirty but it works
  FSOUND_SetVolumeAbsolute(FSOUND_ALL,volume * 2.55);
  mSampleVolume = volume;
  SetMusicVolume(mVolume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
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
}


void JSoundSystem::PlaySample(JSample *sample)
{
  if (sample && sample->mSample){
		int channel = FSOUND_PlaySound(FSOUND_FREE, sample->mSample);
    FSOUND_SetVolumeAbsolute(channel,mSampleVolume * 2.55);
  }
}

