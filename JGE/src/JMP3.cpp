#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>

#include "../include/JAudio.h"
#include "../include/JFileSystem.h"
#include "../include/JMP3.h"


JMP3* JMP3::mInstance = NULL;



void JMP3::init() {
   loadModules();
}

JMP3::JMP3() :
  m_volume(PSP_AUDIO_VOLUME_MAX), m_samplesPlayed(0), m_paused(true) {
}

JMP3::~JMP3() {
   unload();
}

bool JMP3::loadModules() {
   int loadAvCodec = sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
   if (loadAvCodec < 0) {
      return false;
   }

   int loadMp3 = sceUtilityLoadModule(PSP_MODULE_AV_MP3);
   if (loadMp3 < 0) {
      return false;
   }

   return true;
}

bool JMP3::fillBuffers() {
   SceUChar8* dest;
   SceInt32 length;
   SceInt32 pos;

   int ret = sceMp3GetInfoToAddStreamData(m_mp3Handle, &dest, &length, &pos);
   if (ret < 0)
      return false;

    if (sceIoLseek32(m_fileHandle, pos, SEEK_SET) < 0) {
      // Re-open the file because file handel can be invalidated by suspend/resume.
      sceIoClose(m_fileHandle);
      m_fileHandle = sceIoOpen(m_fileName, PSP_O_RDONLY, 0777);
      if (m_fileHandle < 0)
         return false;
      if (sceIoLseek32(m_fileHandle, 0, SEEK_END) != m_fileSize
            || sceIoLseek32(m_fileHandle, pos, SEEK_SET) < 0) {
         sceIoClose(m_fileHandle);
         m_fileHandle = -1;
         return false;
      }
    }

    int readLength = sceIoRead(m_fileHandle, dest, length);

   if (readLength < 0)
      return false;

   ret = sceMp3NotifyAddStreamData(m_mp3Handle, readLength);
   if (ret < 0)
      return false;

   return true;
}

bool JMP3::load(const std::string& filename, int inBufferSize, int outBufferSize) {
      m_inBufferSize = inBufferSize;
      //m_inBuffer = new char[m_inBufferSize];
      //if (!m_inBuffer)
      //   return false;

      m_outBufferSize = outBufferSize;
      //m_outBuffer = new short[outBufferSize];
      //if (!m_outBuffer)
      //   return false;

      m_fileHandle = sceIoOpen(filename.c_str(), PSP_O_RDONLY, 0777);
       if (m_fileHandle < 0)
          return false;

     // Memorise the full path for reloading with decode thread.
      if ( getcwd(m_fileName, sizeof(m_fileName)) ){
         int len = strnlen(m_fileName, sizeof(m_fileName));
         if (len + filename.size() <= sizeof(m_fileName) - 2){
            m_fileName[len++] = '/';
            strcpy(m_fileName + len, filename.c_str());
         }else{
            m_fileName[0] = NULL;
         }
      }

       int ret = sceMp3InitResource();
       if (ret < 0)
          return false;

      SceMp3InitArg initArgs;

      int fileSize = sceIoLseek32(m_fileHandle, 0, SEEK_END);
      sceIoLseek32(m_fileHandle, 0, SEEK_SET);
	  m_fileSize = fileSize;

      initArgs.unk1 = 0;
      initArgs.unk2 = 0;
      initArgs.mp3StreamStart = 0;
      initArgs.mp3StreamEnd = fileSize;
      initArgs.mp3BufSize = m_inBufferSize;
      initArgs.mp3Buf = (SceVoid*) m_inBuffer;
      initArgs.pcmBufSize = m_outBufferSize;
      initArgs.pcmBuf = (SceVoid*) m_outBuffer;

      m_mp3Handle = sceMp3ReserveMp3Handle(&initArgs);
      if (m_mp3Handle < 0)
         return false;

      // Alright we are all set up, let's fill the first buffer.
      bool _filled= fillBuffers();
      if (! _filled) return false;


      // Start this bitch up!
      int start = sceMp3Init(m_mp3Handle);
      if (start < 0)
         return false;

      m_numChannels = sceMp3GetMp3ChannelNum(m_mp3Handle);
      m_samplingRate = sceMp3GetSamplingRate(m_mp3Handle);

   return true;
}

bool JMP3::unload() {
   if (m_channel >= 0)
      sceAudioSRCChRelease();

   sceMp3ReleaseMp3Handle(m_mp3Handle);

   sceMp3TermResource();

   sceIoClose(m_fileHandle);

   //delete[] m_inBuffer;

   //delete[] m_outBuffer;

   return true;
}

bool JMP3::update() {
	int retry = 8;//FIXME:magic number
	JMP3_update_start:

   if (!m_paused) {
      if (sceMp3CheckStreamDataNeeded(m_mp3Handle) > 0) {
         fillBuffers();
      }

      short* tempBuffer;
      int numDecoded = 0;

      while (true) {
         numDecoded = sceMp3Decode(m_mp3Handle, &tempBuffer);
         if (numDecoded > 0)
            break;

         int ret = sceMp3CheckStreamDataNeeded(m_mp3Handle);
         if (ret <= 0)
            break;

         fillBuffers();
      }

      // Okay, let's see if we can't get something outputted :/
      if (numDecoded == 0 || ((unsigned)numDecoded == 0x80671402)) {
	  if (retry-- > 0){
           //give me a recovery chance after suspend/resume...
            sceKernelDelayThread(1);
            goto JMP3_update_start;
         }

         sceMp3ResetPlayPosition(m_mp3Handle);
         if (!m_loop)
            m_paused = true;

         m_samplesPlayed = 0;
      } else {
         if (m_channel < 0 || m_lastDecoded != numDecoded) {
            if (m_channel >= 0)
               sceAudioSRCChRelease();

            m_channel = sceAudioSRCChReserve(numDecoded / (2 * m_numChannels), m_samplingRate, m_numChannels);
         }

         // Output
         m_samplesPlayed += sceAudioSRCOutputBlocking(m_volume, tempBuffer);
         m_playTime = (m_samplingRate > 0) ? (m_samplesPlayed / (m_samplingRate/1000)) : 0;
		 m_lastDecoded = numDecoded;

      }
   }

   return true;
}

bool JMP3::play() {
   return (m_paused = false);
}

bool JMP3::pause() {
   return (m_paused = true);
}

bool JMP3::setLoop(bool loop) {
   sceMp3SetLoopNum(m_mp3Handle, (loop == true) ? -1 : 0);
   return (m_loop = loop);
}

int JMP3::setVolume(int volume) {
   return (m_volume = volume);
}

int JMP3::playTime() const {
   return m_playTime;
}

int JMP3::playTimeMinutes() {
   return (m_playTime / 1000) / 60;
}

int JMP3::playTimeSeconds() {
   return (m_playTime/1000) % 60;
}
