//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// Copyright (c) 2007 Cooleyes
// 
//-------------------------------------------------------------------------------------

#ifndef _COOLEYES_MP3_
#define _COOLEYES_MP3_


#define FORCE_BUFFER_ALIGNMENT

class JCooleyesMP3
{
public:
	JCooleyesMP3();
	~JCooleyesMP3();
	
	bool Load(const char* filename);
	void Release();
	bool Play(bool looping = false);
	void Stop();
	void Resume();
	void FeedAudioData(void* buf, unsigned int length/*, bool mixing = false*/);
	void Decode();
	bool IsPlaying();
	
	void InitBuffers(unsigned long *MP3CodecBuffer, short* decoderBuffer, short* decodedDataOutputBuffer);
	
public:
	bool mPlaying;
	SceUID mp3_handle; 
	u8* mFileBuffer;
	u8* mMP3FirstFramePointer;
	int mFileSize;
	int mUpdateCounter;
	u8* mCurrFramePointer;
	int mDataPointer;
	int mSamplesPending;
	bool mAllMP3DataProcessed;
	
	bool mLooping;
	
	u32 mSamplePerFrame; 
	u32 mChannelCount;
	u32 mSampleRate; 
	
	int mOutputBufferIndex;
	
	short *mDecoderBuffer;
	short *mDecodedDataOutputBuffer;
	unsigned long *mMP3CodecBuffer;
	
};

#endif

