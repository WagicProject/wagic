//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#include "../include/JSoundSystem.h"
#include "../include/JAudio.h"
#include "../include/JMP3.h"
#include <string>
using std::string;

JMusic::JMusic()
{
	mTrack = NULL;
}

JMusic::~JMusic()
{
	JSoundSystem::GetInstance()->StopMusic(this);

	if (mTrack)
		delete mTrack;
}

void JMusic::Update(){

}

int JMusic::getPlayTime(){
    if (mTrack) return mTrack->playTime();
    return 0;
}

JSample::JSample()
{
	mSample = NULL;
}


JSample::~JSample()
{
	if (mSample)
		releaseWaveData(mSample);
}

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

}


JSoundSystem::~JSoundSystem()
{

}


void JSoundSystem::InitSoundSystem()
{

	audioInit();

}


void JSoundSystem::DestroySoundSystem()
{

	audioDestroy();

}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
#ifdef RESPATH
  string s = RESPATH"/";
#else
  string s = "Res/";
#endif
  s.append(fileName);
  JMusic *music = new JMusic();
  if (music)
    {
      music->mTrack = new JMP3();
      if (!music->mTrack->load(s))
	{
	  free(music->mTrack);
	  music->mTrack = NULL;
	}
    }
  JMP3::mInstance = music->mTrack;
  return music;
}


JSample *JSoundSystem::LoadSample(const char *fileName)
{
	char s[strlen(fileName)+1];
	strcpy(s, fileName);

	JSample *sample = new JSample();
	if (sample)
	{
		sample->mSample = new WAVDATA;
		loadWaveData(sample->mSample, s, 1);
	}

	return sample;
}




void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{

	if (music->mTrack)
		PlayMP3(music->mTrack, looping);
}


void JSoundSystem::PlaySample(JSample *sample)
{

	playWaveMem(sample->mSample, 0);
}


void JSoundSystem::SetVolume(int volume)
{
  JMP3 * mp3 = JMP3::mInstance;
	if (mp3) mp3->setVolume(volume);
}


void JSoundSystem::StopMusic(JMusic *music)
{
	StopMP3();

}


void JSoundSystem::ResumeMusic(JMusic *music)
{
	ResumeMP3(music->mTrack);

}

