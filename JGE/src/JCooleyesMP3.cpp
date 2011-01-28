#include <pspsdk.h> 
#include <pspaudiocodec.h> 
#include <pspaudiolib.h>
#include <pspmpeg.h>
#include <malloc.h>
#include <string.h>

#include "../include/JAudio.h"
#include "../include/JFileSystem.h"
#include "../include/JCooleyesMP3.h"


JCooleyesMP3::JCooleyesMP3()
{
	mPlaying = false;
	mp3_handle = 0;
	mFileBuffer = NULL;
	mMP3FirstFramePointer = NULL;
	mFileSize = 0;
	mCurrFramePointer = NULL;
	mDataPointer = 0;
	mSamplesPending = 0;
	mAllMP3DataProcessed = true;
	mUpdateCounter = 0;

	mLooping = false;

	mChannelCount = 2; 
	mSampleRate = 44100;					// this is mp3 file's sample rate, also can be 48000,.... 
	mSamplePerFrame = SAMPLE_PER_FRAME;		// default value for MPEG1, Layer3

	mOutputBufferIndex = 0;
}

JCooleyesMP3::~JCooleyesMP3()
{

}


bool JCooleyesMP3::IsPlaying()
{
	return mPlaying;
}


void JCooleyesMP3::InitBuffers(unsigned long *MP3CodecBuffer, short* decoderBuffer, short* decodedDataOutputBuffer)
{

	mMP3CodecBuffer = MP3CodecBuffer;
	mDecoderBuffer = decoderBuffer;
	mDecodedDataOutputBuffer = decodedDataOutputBuffer;

}


bool JCooleyesMP3::Load(const char *filename)
{
	if (!g_MP3DecoderOK)
		return false;

	bool ret = true;

	JFileSystem* fileSys = JFileSystem::GetInstance();

	//mp3_handle = sceIoOpen(filename, PSP_O_RDONLY, 0777); 
	if (fileSys->OpenFile(filename))
	{
		mFileSize = fileSys->GetFileSize();// sceIoLseek32(mp3_handle, 0, PSP_SEEK_END); 
		//sceIoLseek32(mp3_handle, 0, PSP_SEEK_SET); 

		mFileBuffer = (u8*)memalign(64, mFileSize); 
		if (mFileBuffer)
		{
			if (fileSys->ReadFile(mFileBuffer, mFileSize ) != mFileSize)
				ret = false;
		}
		else
			ret = false;

		//sceIoClose(mp3_handle);
		fileSys->CloseFile();

		if (ret)
		{
			mMP3FirstFramePointer = mFileBuffer;

			// skip ID3v2.x header
			if (mFileBuffer[0]=='I' && mFileBuffer[1]=='D' && mFileBuffer[2]=='3')
			{
				u32 size = mFileBuffer[9];
				u32 n = mFileBuffer[8];
				size |= (n<<7);

				n = mFileBuffer[7];
				size |= (n<<14);

				n = mFileBuffer[6];
				size |= (n<<21);

				size += 10;

				mMP3FirstFramePointer += size;

			}
		}
	}

	mAllMP3DataProcessed = !ret;

	return ret;
}

void JCooleyesMP3::Release()
{
	mAllMP3DataProcessed = true;

	if (mFileBuffer)
	{
		free(mFileBuffer);
		mFileBuffer = NULL;
	}
}


bool JCooleyesMP3::Play(bool looping)
{
	if (!mAllMP3DataProcessed)
	{
		mOutputBufferIndex = 0;
		mLooping = looping;

		mUpdateCounter = 0;
		mSamplesPending = 0;
		mDataPointer = 0;
		mCurrFramePointer = mMP3FirstFramePointer;

		mPlaying = true;

		
		

		Decode();
		//MP3Decode();
		//MP3Decode();
		//MP3Decode();

		

	}
	else
		mPlaying = false;

	return mPlaying;
}


// bool JCooleyesMP3::PlaybackDone()
// {
// 	return (mAllMP3DataProcessed && mSamplesPending==0);
// }


void JCooleyesMP3::FeedAudioData(void* buf, unsigned int length/*, bool mixing*/)
{
	if (mPlaying)
	{
		if (mSamplesPending > 0)
		{
			short *dest = (short *)buf;

			if ((int)length > mSamplesPending)
				length = mSamplesPending;

			int i;
			int count = length<<1;//*2
			int bufferMax = mSamplePerFrame<<2;	//1152 * 2 * 4
			for (i=0;i<count;i++)
			{
// 				if (mixing)
// 				{
// 					dest[i] >>= 1;
// 					dest[i] |= (mDecodedDataOutputBuffer[mDataPointer]>>1);
// 				}
// 				else
					
				dest[i] = mDecodedDataOutputBuffer[mDataPointer];

				mDataPointer++;
				if (mDataPointer >= bufferMax)
					mDataPointer = 0;
			}

			mSamplesPending -= length;

		}

		Decode();

		if (mAllMP3DataProcessed && mSamplesPending<=0)
		{
			if (mLooping)
			{
				mSamplesPending = 0;
				mAllMP3DataProcessed = false;
				mUpdateCounter = 0;
				mCurrFramePointer = mMP3FirstFramePointer;

				Decode();

			}
			else
				mPlaying = false;
		}
	}
}



void JCooleyesMP3::Decode()
{
	if (mAllMP3DataProcessed)
		return;


	// get offset to current decoding buffer
	int dest = mOutputBufferIndex*(mSamplePerFrame<<1);//*2;

	if (mSamplesPending == 0 || dest > mDataPointer || (dest + (int)(mSamplePerFrame<<1) < mDataPointer))	//1152*2;
	{

		memset(mDecoderBuffer, 0, mSamplePerFrame<<2);//*2*2);  

		unsigned int mp3_header = mCurrFramePointer[0]; 
		mp3_header = (mp3_header<<8) | mCurrFramePointer[1]; 
		mp3_header = (mp3_header<<8) | mCurrFramePointer[2]; 
		mp3_header = (mp3_header<<8) | mCurrFramePointer[3]; 

		int bitrate = (mp3_header & 0xf000) >> 12; 
		int padding = (mp3_header & 0x200) >> 9; 

		int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
		int frame_size = 144000*bitrates[bitrate]/mSampleRate + padding; 


		// skip 1 frame

// 		mCurrFramePointer += frame_size;
// 		mUpdateCounter += frame_size;
// 
// 		mp3_header = mCurrFramePointer[0]; 
// 		mp3_header = (mp3_header<<8) | mCurrFramePointer[1]; 
// 		mp3_header = (mp3_header<<8) | mCurrFramePointer[2]; 
// 		mp3_header = (mp3_header<<8) | mCurrFramePointer[3]; 
// 
// 		bitrate = (mp3_header & 0xf000) >> 12; 
// 		padding = (mp3_header & 0x200) >> 9; 
// 		
// 		frame_size = 144000*bitrates[bitrate]/mSampleRate + padding; 



		//////////////////////////////////////////////////////////////////////////
		#if defined (FORCE_BUFFER_ALIGNMENT)

			u8* mp3_data_buffer = (u8*)memalign(64, frame_size); 
			memcpy(mp3_data_buffer, mCurrFramePointer, frame_size);
			mMP3CodecBuffer[6] = (unsigned long)mp3_data_buffer; 

		//////////////////////////////////////////////////////////////////////////
		#else

			mMP3CodecBuffer[6] = (unsigned long)mCurrFramePointer; 

		#endif
		//////////////////////////////////////////////////////////////////////////

		mMP3CodecBuffer[8] = (unsigned long)mDecoderBuffer; 

		mMP3CodecBuffer[7] = mMP3CodecBuffer[10] = frame_size; 
		mMP3CodecBuffer[9] = mSamplePerFrame << 2;//* 4; 

		int res = sceAudiocodecDecode(mMP3CodecBuffer, 0x1002);

		//////////////////////////////////////////////////////////////////////////
		#if defined (FORCE_BUFFER_ALIGNMENT)

			free (mp3_data_buffer);

		#endif
		//////////////////////////////////////////////////////////////////////////

		if ( res < 0 ) 
		{ 

			mAllMP3DataProcessed = true; 
			return; 
		}  
		short *buffer = mDecodedDataOutputBuffer+dest;//mOutputBufferIndex*1152*2;

		memcpy(buffer, mDecoderBuffer, mSamplePerFrame<<2);//1152*4);

		mOutputBufferIndex = (mOutputBufferIndex+1)%DECODING_BUFFER_COUNT;

		mSamplesPending += mSamplePerFrame; 

		mCurrFramePointer += frame_size;
		mUpdateCounter += frame_size;

		if (mUpdateCounter >= mFileSize)
		{

			mAllMP3DataProcessed = true;
		}

	}

}


void JCooleyesMP3::Stop()
{
	mPlaying = false;
}


void JCooleyesMP3::Resume()
{
	if (!mAllMP3DataProcessed)
		mPlaying = true;
}
