//-------------------------------------------------------------------------------------
//
// JGE is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// Copyright (c) 2008 Alexander Berl <raphael@fx-world.org>
// Copyright (c) 2008 WilLoW :--) <wagic.the.homebrew@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _JMP3_
#define _JMP3_

#include <string>




class JMP3
{
protected:
  static bool loadModules();
  int m_volume;
  int m_samplesPlayed;
  int m_inBufferSize, m_outBufferSize;
  char m_inBuffer[16*1024] __attribute__((aligned(64))); // ?
  short m_outBuffer[16*(1152/2)] __attribute__((aligned(64))); //?
  int m_numChannels;
  int m_samplingRate;
  bool m_loop;
  int m_lastDecoded;
  int m_playTime;
  int GetID3TagSize(char *fname);
  static bool init_done;
public:
  int m_paused;
  int m_channel;
  int m_mp3Handle;
  int m_fileHandle;
  int m_fileSize;
  char m_fileName[256];
  static JMP3* mInstance;
  JMP3();
  ~JMP3();
  static void init();
  bool fillBuffers();
  bool load(const std::string& filename, int inBufferSize = 16*1024, int outBufferSize = 16 * (1152/2));
  bool unload();
  bool update();
  bool play();
  bool pause();
  bool setLoop(bool loop);
  int setVolume(int volume);
  int playTime() const;
  int playTimeMinutes();
  int playTimeSeconds();

};

#endif

