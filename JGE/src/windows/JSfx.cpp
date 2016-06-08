#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "DebugRoutines.h"
#include "../../include/JSoundSystem.h"
#include "../../include/JFileSystem.h"

//////////////////////////////////////////////////////////////////////////
JMusic::JMusic()
    :
    mTrack(0)
{
}

void JMusic::Update(){

}

int JMusic::getPlayTime(){
  return 0;
}

JMusic::~JMusic()
{
}

//////////////////////////////////////////////////////////////////////////
JSample::JSample()
{

}

JSample::~JSample()
{
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
}


void JSoundSystem::DestroySoundSystem()
{
}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
    JMusic* music = NULL;
	music = new JMusic();
    if (music)
    {
		music->filename = JFileSystem::GetInstance()->GetResourceFile(fileName);
		std::string aString = "open \"" + music->filename + "\" type mpegvideo alias mp3";
		mciSendString(aString.c_str(), NULL, 0, NULL);
		music->mTrack = (void*)-1;
    }

	return music;
}


void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
    if (music && music->mTrack)
    {
		if(looping)
			mciSendString("play mp3 repeat", NULL, 0, NULL);
		else
			mciSendString("play mp3", NULL, 0, NULL);
    }
}


void JSoundSystem::StopMusic(JMusic *music)
{
    if (music && music->mTrack)
    {
		mciSendString("stop mp3", NULL, 0, NULL);
    }
}


void JSoundSystem::PauseMusic(JMusic *music)
{
	if (music && music->mTrack)
	{
		mciSendString("pause mp3", NULL, 0, NULL);
	}
}


void JSoundSystem::ResumeMusic(JMusic *music)
{
	if (music && music->mTrack)
	{
		mciSendString("resume mp3", NULL, 0, NULL);
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

void JSoundSystem::SetSfxVolume(int volume)
{
  mSampleVolume = volume;
  SetMusicVolume(mVolume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
    JSample* sample = NULL;
    sample = new JSample();
    if (sample)
    {
		sample->filename = JFileSystem::GetInstance()->GetResourceFile(fileName);
    }

	return sample;
}


void JSoundSystem::PlaySample(JSample *sample)
{
    if(sample)
    {
		sndPlaySound(sample->filename.c_str(), SND_FILENAME | SND_ASYNC);
    }
}

