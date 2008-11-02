/////////////
// JAudio.c //
/////////////

#include <pspkernel.h>
#include <pspdebug.h> 
#include <pspaudiolib.h>
#include <pspaudio.h>

#include <pspsdk.h> 
#include <pspaudiocodec.h> 
#include <pspmpeg.h>
#include <malloc.h>
#include <string.h>


#include "../include/JGE.h"
#include "../include/JAudio.h"
#include "../include/JCooleyesMP3.h"
#include "../include/JFileSystem.h"
#include "../include/decoder_prx.h"


//////////////////////////////////////////////////////////////////////////
unsigned long g_MP3CodecBuffer[65] __attribute__((aligned(64))); 
short g_DecoderBuffer[SAMPLE_PER_FRAME<<1] __attribute__((aligned(64)));
short g_DecodedDataOutputBuffer[SAMPLE_PER_FRAME<<2] __attribute__((aligned(64)));

bool g_MP3DecoderOK = false;
bool g_GotEDRAM; 

JCooleyesMP3* g_CurrentMP3;
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
WAVDATA* p_currentWav[NUMBER_WAV_CHANNELS];		// 各通道当前的播放
WAVDATA currentWav[NUMBER_WAV_CHANNELS];		// 各通道当前的播放


///////////////////////////////////////////////////////////////////
char loadWaveData(WAVDATA* p_wav, char* fileName, char memLoad)  // WAVE加载, memLoad-是否加载至内存
{

	JFileSystem* fileSystem = JFileSystem::GetInstance();
	if (!fileSystem->OpenFile(fileName)) 
		return 0;
		
	memset(p_wav, 0, sizeof(WAVDATA));
	//SceUID fd = sceIoOpen(fileName, PSP_O_RDONLY, 0777);
	char head[256];
	memset(head, 0, 256);
	//sceIoRead(fd, head, 20);
	fileSystem->ReadFile(head, 20);
	char string[8];
	memset(string, 0, 8);
	memcpy(string, head, 4);
	if (0!=strcmp(string, "RIFF"))
	{
		//sceIoClose(fd);
		fileSystem->CloseFile();
		return 0;
	}
	memset(string, 0, 8);
	memcpy(string, head+8, 4);
	if (0!=strcmp(string, "WAVE"))
	{
		//sceIoClose(fd);
		fileSystem->CloseFile();
		return 0;
	}
	memset(string, 0, 8);
	memcpy(string, head+12, 3);
	if (0!=strcmp(string, "fmt"))
	{
		//sceIoClose(fd);
		fileSystem->CloseFile();
		return 0;
	}
	int fmtSize = 0;
	memcpy(&fmtSize, head+16, 4);
	//sceIoRead(fd, head+20, fmtSize);
	fileSystem->ReadFile(head+20,fmtSize );
	p_wav->headSize = 20+fmtSize;
	while (1)
	{
		//sceIoRead(fd, head+p_wav->headSize, 4);
		fileSystem->ReadFile(head+p_wav->headSize, 4);
		memset(string, 0, 8);
		memcpy(string, head+p_wav->headSize, 4);
		p_wav->headSize += 4;
		if (0!=strcmp(string, "data"))
		{
			//sceIoRead(fd, head+p_wav->headSize, 4);
			fileSystem->ReadFile(head+p_wav->headSize, 4);
			memcpy(&fmtSize, head+p_wav->headSize, 4);
			p_wav->headSize += 4;
			//sceIoRead(fd, head+p_wav->headSize, fmtSize);
			fileSystem->ReadFile(head+p_wav->headSize, fmtSize);
			p_wav->headSize += fmtSize;
		}
		else
		{
			//sceIoRead(fd, head+p_wav->headSize, 4);
			fileSystem->ReadFile(head+p_wav->headSize, 4);
			p_wav->headSize += 4;
			break;
		}
		if (p_wav->headSize>191)
		{
			//sceIoClose(fd);
			fileSystem->CloseFile();
			return 0;
		}
	}
	strcpy(p_wav->fullName, fileName);
	memcpy(&p_wav->fileSize, head+4, 4);
	memcpy(&p_wav->format, head+20, 2);
	memcpy(&p_wav->channelCount, head+22, 2);
	if (p_wav->channelCount!=1 && p_wav->channelCount!=2)
	{
		//sceIoClose(fd);
		fileSystem->CloseFile();
		return 0;
	}
	memcpy(&p_wav->samplePerSecond, head+24, 4);
	memcpy(&p_wav->bytePerSecond, head+28, 4);
	memcpy(&p_wav->bytePerSample, head+32, 2);
	p_wav->bytePerSample = p_wav->bytePerSample / p_wav->channelCount;
	if (p_wav->bytePerSample!=1 && p_wav->bytePerSample!=2)
	{
		//sceIoClose(fd);
		fileSystem->CloseFile();
		return 0;
	}
	p_wav->nSample = 44100 / p_wav->samplePerSecond;
	p_wav->sizeStep = 4096 / p_wav->nSample * p_wav->channelCount / 2 * p_wav->bytePerSample / 2;
	memcpy(&p_wav->soundSize, head+p_wav->headSize-4, 4);
	if (memLoad)
	{
		if (p_wav->soundSize>4096000)
		{
			//sceIoClose(fd);
			fileSystem->CloseFile();
			return 0;
		}
		p_wav->buffer = (char*)malloc(p_wav->soundSize);
		memset(p_wav->buffer, 0, p_wav->soundSize);
		//sceIoRead(fd, p_wav->buffer, p_wav->soundSize);
		fileSystem->ReadFile(p_wav->buffer, p_wav->soundSize);
		p_wav->bytePosition = 0;
		p_wav->fd = -1;
		//sceIoClose(fd);
		fileSystem->CloseFile();
	}
	else
	{
		p_wav->bytePosition = p_wav->headSize;
		//p_wav->fd = fd;	// no file mode...
		p_wav->fd = -1;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////
void releaseWaveData(WAVDATA* p_wav)  // WAVE释放
{
	if (p_wav->fd==-1)
		free(p_wav->buffer);
	else
		sceIoClose(p_wav->fd);
	memset(p_wav, 0, sizeof(WAVDATA));
}

///////////////////////////////////////////////////////////////////
void audioOutCallback(int channel, void* buf, unsigned int length)  // 各通道回调
{
	WAVDATA* p_wav = NULL;
	memset(buf, 0, 4096);
	if (!currentWav[channel].fullName[0])
	{
		if (!p_currentWav[channel])
			return;
		p_wav = p_currentWav[channel];
		if (!p_wav->fullName[0])
			return;
		else
		{
			if (p_wav->bytePosition>=p_wav->soundSize)
			{
				if (p_wav->flag&PW_REPLAY)
					p_wav->bytePosition = 0;
				else
				{
					p_currentWav[channel] = NULL;
					return;
				}
			}
			if (p_wav->bytePosition==0 && p_wav->delayTime)
				sceKernelDelayThread(p_wav->delayTime);
		}
	}
	else
	{
		p_wav = &currentWav[channel];
		if (p_wav->bytePosition>=p_wav->headSize+p_wav->soundSize)
		{
			if (p_wav->flag&PW_REPLAY)
				p_wav->bytePosition = p_wav->headSize;
			else
			{
				releaseWaveData(&currentWav[channel]);
				return;
			}
			if (p_wav->bytePosition==(unsigned long)p_wav->headSize && p_wav->delayTime)
				sceKernelDelayThread(p_wav->delayTime);
		}
	}
	if (p_wav->flag&PW_PAUSE)
		return;
	int sizeStep = p_wav->sizeStep;
	int nSample = p_wav->nSample;
	if (p_wav->flag&PW_FAST)
		sizeStep = p_wav->sizeStep+(p_wav->sizeStep>>1);
	char* tmpBuff;
	char byteRead[4096];
	int actualSize = 0;
	if (p_wav->fd>=0)
	{
		if (p_wav->bytePosition+sizeStep>p_wav->headSize+p_wav->soundSize)
			actualSize = p_wav->headSize+p_wav->soundSize - p_wav->bytePosition;
		else
			actualSize = sizeStep;
		memset(byteRead, 0, 4096);
		sceIoLseek(p_wav->fd, p_wav->bytePosition, PSP_SEEK_SET);
		sceIoRead(p_wav->fd, byteRead, actualSize);
		tmpBuff = byteRead;
	}
	else if (p_wav->buffer)
	{
		tmpBuff = p_wav->buffer + p_wav->bytePosition;
		if (p_wav->bytePosition+sizeStep>p_wav->soundSize)
			actualSize = p_wav->soundSize - p_wav->bytePosition;
		else
			actualSize = sizeStep;
	}
	else
		return;
	length = length*actualSize/sizeStep;
	short val_16;
	int i;
	int j;
	int srcPos;
	for (i=0; i<(int)length; i+=nSample)
	{
		for (j=0; j<nSample; j++)
		{
			if (p_wav->bytePerSample==2)
			{
				if (p_wav->channelCount==2)
				{
					srcPos = (i<<2)/nSample;
					if (p_wav->flag&PW_FAST)
						srcPos = srcPos+(srcPos>>1);
					memcpy((unsigned char*)buf+((i+j)<<2), tmpBuff+srcPos, 2);
					memcpy((unsigned char*)buf+((i+j)<<2)+2, tmpBuff+srcPos+2, 2);
				}
				else if (p_wav->channelCount==1)
				{
					srcPos = (i<<1)/nSample;
					if (p_wav->flag&PW_FAST)
						srcPos = srcPos+(srcPos>>1);
					memcpy((unsigned char*)buf+((i+j)<<2), tmpBuff+srcPos, 2);
					memcpy((unsigned char*)buf+((i+j)<<2)+2, tmpBuff+srcPos, 2);
				}
			}
			else if (p_wav->bytePerSample==1)
			{
				if (p_wav->channelCount==2)
				{
					srcPos = (i<<1)/nSample;
					if (p_wav->flag&PW_FAST)
						srcPos = srcPos+(srcPos>>1);
					val_16 = 0;
					memcpy(&val_16, tmpBuff+srcPos, 1);
					val_16 = (val_16-128)<<8;
					memcpy((unsigned char*)buf+((i+j)<<2), &val_16, 2);
					val_16 = 0;
					memcpy(&val_16, tmpBuff+srcPos+1, 1);
					val_16 = (val_16-128)<<8;
					memcpy((unsigned char*)buf+((i+j)<<2)+2, &val_16, 2);
				}
				else if (p_wav->channelCount==1)
				{
					srcPos = i/nSample;
					if (p_wav->flag&PW_FAST)
						srcPos = srcPos+(srcPos>>1);
					val_16 = 0;
					memcpy(&val_16, tmpBuff+srcPos, 1);
					val_16 = (val_16-128)<<8;
					memcpy((unsigned char*)buf+((i+j)<<2), &val_16, 2);
					memcpy((unsigned char*)buf+((i+j)<<2)+2, &val_16, 2);
				}
			}
		}
	}
	p_wav->bytePosition += sizeStep;
}

///////////////////////////////////////////////////////////////////
void audioOutCallback_0(void* buf, unsigned int length, void *userdata) {audioOutCallback(0, buf, length);}
void audioOutCallback_1(void* buf, unsigned int length, void *userdata) {audioOutCallback(1, buf, length);}
void audioOutCallback_2(void* buf, unsigned int length, void *userdata) {audioOutCallback(2, buf, length);}
//void audioOutCallback_3(void* buf, unsigned int length, void *userdata) {audioOutCallback(3, buf, length);}

///////////////////////////////////////////////////////////////////
char playWaveFile(int channel, char* fullName, unsigned long flag)  // 播放WAVE文件
{
	stopWaveMem(channel);
	if (currentWav[channel].fullName[0])
		releaseWaveData(&currentWav[channel]);
	if (!loadWaveData(&currentWav[channel], fullName, 0))
		return 0;
	currentWav[channel].flag = flag;
	return 1;
	// ...
}

///////////////////////////////////////////////////////////////////
void stopWaveFile(int channel)  // 停止WAVE文件
{
	if (currentWav[channel].fullName[0])
		releaseWaveData(&currentWav[channel]);
}

///////////////////////////////////////////////////////////////////
int playWaveMem(WAVDATA* p_wav, unsigned long flag)  // 播放WAVE
{
	int i;
	for (i=0;i<NUMBER_WAV_CHANNELS;i++)
	{
		if (p_currentWav[i] == NULL)
		{
			//stopWaveFile(channel);
			if (!p_wav)
				return -1;
			if (!p_wav->soundSize)
				return -1;
			p_currentWav[i] = p_wav;
			p_currentWav[i]->flag = flag;
			p_currentWav[i]->bytePosition = 0;
			return i;
		}
	}

	return -1;	
}

///////////////////////////////////////////////////////////////////
void stopWaveMem(int channel)  // 停止WAVE
{
	p_currentWav[channel] = NULL;
}

///////////////////////////////////////////////////////////////////
void setChannelFlag(int channel, int flag)  // 设置播放属性
{
	WAVDATA* p_wav = NULL;
	if (!currentWav[channel].fullName[0])
	{
		if (!p_currentWav[channel])
			return;
		p_wav = p_currentWav[channel];
		if (!p_wav->fullName[0])
			return;
	}
	else
		p_wav = &currentWav[channel];
	p_wav->flag = flag;
}

///////////////////////////////////////////////////////////////////
void audioInit()  // 初始化
{
	int i;
	for (i=0; i<NUMBER_WAV_CHANNELS; i++)
	{
		memset(&currentWav[i], 0, sizeof(WAVDATA));
		p_currentWav[i] = NULL;
	}
	pspAudioInit();
	pspAudioSetChannelCallback(0, audioOutCallback_0, NULL);
	pspAudioSetChannelCallback(1, audioOutCallback_1, NULL);
	pspAudioSetChannelCallback(2, audioOutCallback_2, NULL);
	//pspAudioSetChannelCallback(3, audioOutCallback_3, NULL);

	pspAudioSetChannelCallback(3, MP3AudioOutCallback, NULL);
	InitMP3Decoder();
}

void audioDestroy()
{
	ReleaseMP3Decoder();
	pspAudioEnd();
}


//////////////////////////////////////////////////////////////////////////
void MP3AudioOutCallback(void* buf, unsigned int length, void *userdata)
{
	
	// PSP_NUM_AUDIO_SAMPLES is 1024, 16bit samples (short), 2 channels for stereo sound
	memset(buf, 0, PSP_NUM_AUDIO_SAMPLES<<2);		
	
	if (g_CurrentMP3 != NULL)
		g_CurrentMP3->FeedAudioData(buf, length);
	
}


//////////////////////////////////////////////////////////////////////////
void PlayMP3(JCooleyesMP3 *mp3, bool looping)
{
	if (g_MP3DecoderOK)
	{
		g_CurrentMP3 = mp3;
		mp3->InitBuffers(g_MP3CodecBuffer, g_DecoderBuffer, g_DecodedDataOutputBuffer);
		mp3->Play(looping);
	}
}


//////////////////////////////////////////////////////////////////////////
void StopMP3()
{
	if (g_CurrentMP3 != NULL)
	{
		g_CurrentMP3->Stop();
	}
}


//////////////////////////////////////////////////////////////////////////
void ResumeMP3()
{
	if (g_CurrentMP3 != NULL)
	{
		g_CurrentMP3->Resume();
	}
}


//////////////////////////////////////////////////////////////////////////
bool InitMP3Decoder()
{
	int result;
	
// 	if (sceKernelDevkitVersion() == 0x01050001)
// 	{
// 		LoadStartModule("flash0:/kd/me_for_vsh.prx", PSP_MEMORY_PARTITION_KERNEL);   
// 		LoadStartModule("flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL);
// 	}
// 	else
// 	{
// 		sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
// 	}

// 	result = pspSdkLoadStartModule("flash0:/kd/me_for_vsh.prx", PSP_MEMORY_PARTITION_KERNEL); 
// 	result = pspSdkLoadStartModule("flash0:/kd/videocodec.prx", PSP_MEMORY_PARTITION_KERNEL); 
// 	result = pspSdkLoadStartModule("flash0:/kd/audiocodec.prx", PSP_MEMORY_PARTITION_KERNEL); 
// 	result = pspSdkLoadStartModule("flash0:/kd/mpegbase.prx", PSP_MEMORY_PARTITION_KERNEL); 
// 	result = pspSdkLoadStartModule("flash0:/kd/mpeg_vsh.prx", PSP_MEMORY_PARTITION_USER); 
// 	
// 	pspSdkFixupImports(result); 
	
	prx_static_init();

	sceMpegInit(); 
	
	memset(g_MP3CodecBuffer, 0, sizeof(g_MP3CodecBuffer)); 
	
	if ( sceAudiocodecCheckNeedMem(g_MP3CodecBuffer, 0x1002) < 0 ) 
		return false;
	
	if ( sceAudiocodecGetEDRAM(g_MP3CodecBuffer, 0x1002) < 0 ) 
		return false;
	
	g_GotEDRAM = true; 
	
	if ( sceAudiocodecInit(g_MP3CodecBuffer, 0x1002) < 0 ) 
		return false;
	
	g_MP3DecoderOK = true;
	
	return true;
	
}



//////////////////////////////////////////////////////////////////////////
void ReleaseMP3Decoder()
{
	if ( g_GotEDRAM )  
		sceAudiocodecReleaseEDRAM(g_MP3CodecBuffer); 
	
}


