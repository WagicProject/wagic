//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// Copyright (c) 2007 Cooleyes
// Copyright (c) 2007 Mr.Cheese
//
//-------------------------------------------------------------------------------------

#ifndef _JAUDIO_H_
#define _JAUDIO_H_


#include <pspiofilemgr.h>
#include <pspaudiolib.h>
#include <malloc.h>
#include <string.h>


///////////////////////////////////////////////////////////////////
#define PW_REPLAY 0x00000001		//
#define PW_DELAY 0x00000010			//
#define PW_FAST 0x00000100			//
#define PW_PAUSE 0x00001000			//

#define NUMBER_WAV_CHANNELS			3

typedef struct _WAVDATA
{
	char fullName[256];				// filename
	unsigned long fileSize;			// size of file
	short headSize;					// size of head
	unsigned short format;			// 
	unsigned short channelCount;	// 
	unsigned long samplePerSecond;  // 
	unsigned long bytePerSecond;	// 
	unsigned short bytePerSample;	// 
	unsigned long soundSize;		// 
	char* buffer;					// sound data
	SceUID fd;						// file id for streaming
	unsigned long bytePosition;		// current read position
	char nSample;					// progress rate
	unsigned long sizeStep;			// 
	unsigned long flag;				// playback flag
	unsigned long delayTime;		// delay time in (us)

} WAVDATA;


///////////////////////////////////////////////////////////////////
char loadWaveData(WAVDATA* p_wav, char* fileName, char memLoad);
void releaseWaveData(WAVDATA* p_wav);
void audioOutCallback(int channel, void* buf, unsigned int length);
void audioOutCallback_0(void* buf, unsigned int length, void *userdata);
void audioOutCallback_1(void* buf, unsigned int length, void *userdata);
void audioOutCallback_2(void* buf, unsigned int length, void *userdata);
//void audioOutCallback_3(void* buf, unsigned int length, void *userdata);
char playWaveFile(int channel, char* fullName, unsigned long flag);
void stopWaveFile(int channel);
int playWaveMem(WAVDATA* p_wav, unsigned long flag);
void stopWaveMem(WAVDATA* p_wav);
void stopWaveMem(int channel);
void audioInit();
void audioDestroy();
void setChannelFlag(int channel, int flag);
void setPspVolume(int volume);

//////////////////////////////////////////////////////////////////////////


#define DECODING_BUFFER_COUNT		2
#define SAMPLE_PER_FRAME			1152
#define MAX_MP3_FILE				2

class JMP3;

void PlayMP3(JMP3 *mp3, bool looping = false);
void StopMP3();
void ResumeMP3(JMP3 *mp3);

void ReleaseMP3Decoder();
void MP3AudioOutCallback(void* buf, unsigned int length, void *userdata);
int decodeThread2(SceSize args, void *argp);
extern bool g_MP3DecoderOK;

#endif
