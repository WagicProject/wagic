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
#include "../include/JCooleyesMP3.h"


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
	//char s[strlen(fileName)+1];
	//strcpy(s, fileName);

	JMusic *music = new JMusic();
	if (music)
	{
		music->mTrack = new JCooleyesMP3();
		if (music->mTrack)
			music->mTrack->Load(fileName);
	}

	return music;
}

// void JSoundSystem::FreeMusic(JMusic *music)
// {
// 	if (music)
// 	{
// 		if (music->mTrack)
// 			delete music->mTrack;
// 
// 	}
// }

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

// 
// void JSoundSystem::FreeMusic(JMusic *music)
// {
// 	if (music)
// 	{
// 		if (music->mTrack)
// 		{
// 			music->mTrack->Release();
// 			delete music->mTrack;
// 		}
// 		delete music;
// 	}
// }


// void JSoundSystem::FreeSample(JSample *sample)
// {
// 	if (sample)
// 	{
// 		releaseWaveData(sample->mSample);
// 		delete sample;
// 	}
// }


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

	mVolume = volume;
}


void JSoundSystem::StopMusic(JMusic *music)
{
	StopMP3();

}


void JSoundSystem::ResumeMusic(JMusic *music)
{
	ResumeMP3();

}

