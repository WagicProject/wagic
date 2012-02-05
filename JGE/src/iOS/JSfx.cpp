//
//  JSfx.cpp
//  wagic
//
//  Created by Michael Nguyen on 2/3/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "JFileSystem.h"
#include "JSoundSystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#import "SoundManager.h"
//////////////////////////////////////////////////////////////////////////

JMusic::JMusic()
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
    NSLog(@"InitSoundSystem enter");
    [SoundManager sharedSoundManager];
    
    
    NSLog(@"InitSoundSystem leave");
}


void JSoundSystem::DestroySoundSystem()
{

}


JMusic *JSoundSystem::LoadMusic(const char *fileName)
{
    JMusic* music = new JMusic();
    string fullpath = JFileSystem::GetInstance()->GetResourceFile(fileName);
    music->filename = fullpath;
    NSString *filename = [NSString stringWithCString: fileName encoding:NSUTF8StringEncoding];
    NSString *key = [[filename componentsSeparatedByString: @"."] objectAtIndex: 0];
    NSString *fileType = [[key componentsSeparatedByString: @"."] lastObject];
    NSString *path = [NSString stringWithCString: fullpath.c_str() encoding:NSUTF8StringEncoding];
    music->key = [key cStringUsingEncoding:NSUTF8StringEncoding];
    music->ext = [fileType cStringUsingEncoding: NSUTF8StringEncoding];
    
    [[SoundManager sharedSoundManager] loadBackgroundMusicWithKey:key musicFile:path];

    return music;
    
}


void JSoundSystem::ResumeMusic(JMusic *music)
{
    NSLog(@"Resuming Music");
    [[SoundManager sharedSoundManager] resumeMusic];
}


void JSoundSystem::PauseMusic(JMusic *music)
{
    NSLog(@"Pausing Music");
    [[SoundManager sharedSoundManager] pauseMusic];
}


void JSoundSystem::PlayMusic(JMusic *music, bool looping)
{
    NSString *key = [NSString stringWithCString: music->key.c_str() encoding: NSUTF8StringEncoding];
    NSLog(@"Playing music file %@", [NSString stringWithCString: music->filename.c_str() encoding:NSUTF8StringEncoding]);
    [[SoundManager sharedSoundManager] playMusicWithKey: key timesToRepeat: looping? -1 : 1];
}


void JSoundSystem::StopMusic(JMusic *music)
{
    NSLog(@"Stopping Music");
    [[SoundManager sharedSoundManager] stopMusic];
}


void JSoundSystem::SetVolume(int volume)
{
    SetMusicVolume(volume);
    SetSfxVolume(volume);
}

void JSoundSystem::SetMusicVolume(int volume)
{
    mVolume = volume;
    [[SoundManager sharedSoundManager] setMusicVolume: (float ) volume];
}

void JSoundSystem::SetSfxVolume(int volume){
    mSampleVolume = volume;
    [[SoundManager sharedSoundManager] setFxVolume: (float ) volume];
    SetMusicVolume(mVolume);
}

JSample *JSoundSystem::LoadSample(const char *fileName)
{
    JSample* sample = new JSample();
    if (sample)
    {
        NSArray *components = [[NSString stringWithCString:fileName encoding:NSUTF8StringEncoding] componentsSeparatedByString:@"."];
        string fullpath = JFileSystem::GetInstance()->GetResourceFile(fileName);
        NSString *key = [components objectAtIndex:0];
        NSString *musicFile = [NSString stringWithCString: fullpath.c_str() encoding:NSUTF8StringEncoding];
        sample->filename = fullpath;
        sample->ext = [[components lastObject] cStringUsingEncoding: NSUTF8StringEncoding];
        if ([key isEqualToString: @""])
            return sample;
        sample->key = [key cStringUsingEncoding: NSUTF8StringEncoding];
        [[SoundManager sharedSoundManager] loadSoundWithKey: key musicFile: musicFile];
    }
    return sample;
}


void JSoundSystem::PlaySample(JSample *sample)
{
    SoundManager *soundManager = [SoundManager sharedSoundManager];
    NSString *key = [NSString stringWithCString: sample->key.c_str() encoding:NSUTF8StringEncoding];
    [soundManager playSoundWithKey: key gain: 1.0f pitch: 1.0f location:CGPointZero shouldLoop: NO sourceID: -1];
}

