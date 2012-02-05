/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLDefines.h"
#include "aldlist.h"
#include "SoundPlayer.h"
#include "LEFTsettings.h"

#include "Debug.h"

#define LEFT_SOUNDPLAYER_THREADCOUNT 16

DWORD WINAPI stream_run(void * data)
{
  SoundThread * me = (SoundThread *) data;
  while(me->running) {
    if(me->sound) {
      Sound * sound = me->sound;
      ALuint uiSource;
      ALint iState = AL_PLAYING;
      ALuint uiBuffers[1];

      alGenBuffers(1, uiBuffers);
      alGenSources(1, &uiSource);
      alBufferData(uiBuffers[0], sound->format, sound->data, sound->size, sound->freq);
      assert(AL_NO_ERROR == alGetError());
      alSourceQueueBuffers(uiSource, 1, &uiBuffers[0]);
      alSourcef(uiSource, AL_GAIN, sound->volume);
      alSourcePlay(uiSource);

      while(iState == AL_PLAYING) {
        Sleep(16);
        alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
      }

      alSourceStop(uiSource);
      alSourcei(uiSource, AL_BUFFER, 0);

      alDeleteSources(1, &uiSource);
      alDeleteBuffers(1, uiBuffers);

      me->occupied = false;
      me->sound = 0;
    }
    if(me->running) SuspendThread(me->handle);
  }
  return 0;
}

bool SoundPlayer::play(Sound * s)
{
  assert(s); s->volume = gSettings->getf("r_volume");
  SoundThread * t = 0;
  foreach(SoundThreadList, t, mThreads) {
    if(!t->occupied) {
      t->sound = s;
      t->occupied = true;
      ResumeThread(t->handle);
      return true;
    }
  }
  return false;
}

void SoundPlayer::init()
{
  assert(initOpenAL());
  while(mThreads.size() < LEFT_SOUNDPLAYER_THREADCOUNT) {
    SoundThread * current = new SoundThread;
    current->handle = CreateThread(0, 0, &stream_run, current, CREATE_SUSPENDED, 0);
    current->running = true;
    current->occupied = false;
    current->sound = 0;
    mThreads.push_back(current);
  }
}

void SoundPlayer::clear()
{
  SoundThread * t = 0;
  foreach(SoundThreadList, t, mThreads) {
    t->running = false;
    ResumeThread(t->handle);
    WaitForSingleObject(t->handle, INFINITE);
    delete t;
  }
  ALCcontext * pContext = 0;
	ALCdevice * pDevice = 0;
	pContext = alcGetCurrentContext();
	pDevice = alcGetContextsDevice(pContext);
	alcMakeContextCurrent(0);
	alcDestroyContext(pContext);
	alcCloseDevice(pDevice);
}

bool SoundPlayer::initOpenAL()
{
  ALDeviceList * pDeviceList = 0;
  ALCcontext * pContext = 0;
  ALCdevice * pDevice = 0;
  bool result = false;
  pDeviceList = new ALDeviceList();
  if((pDeviceList) && (pDeviceList->GetNumDevices())) {
    pDevice = alcOpenDevice(pDeviceList->GetDeviceName(pDeviceList->GetDefaultDevice()));
    if(pDevice) {
      pContext = alcCreateContext(pDevice, 0);
      if(!pContext) {
        assert(pContext);
        alcCloseDevice(pDevice);
      } else {
        alcMakeContextCurrent(pContext);
        result = true;
      }
    }
    delete pDeviceList;
  }
  return result;
}

Sound * SoundPlayer::load(const char * filename)
{
  Sound * result = new Sound;
  result->data = 0;
  result->size = 0;
  bool ok = true;

  struct _wav_header {
    unsigned int chunkID;      // ChunkID Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form).
    unsigned int chunkSize;    // filesize - 8
    unsigned int fmt;          // Contains the letters "WAVE" (0x57415645 big-endian form).
    // fmt
    unsigned int chunk1ID;     // Contains the letters "fmt " (0x666d7420 big-endian form).
    unsigned int chunk1Size;   // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
    unsigned short format;     // PCM = 1 (i.e. Linear quantization)
    unsigned short channels;   // Mono = 1, Stereo = 2, etc.
    unsigned int samplerate;   // 8000, 44100, etc.
    unsigned int byterate;     // SampleRate * NumChannels * BitsPerSample/8
    unsigned short blockalign; // NumChannels * BitsPerSample/8
    unsigned short bits;       // 8 bits = 8, 16 bits = 16, etc.
    // data
    unsigned int chunk2ID;     // Contains the letters "data" (0x64617461 big-endian form).
    unsigned int chunk2Size;   // NumSamples * NumChannels * BitsPerSample/8
  } waveheader;

  if(ok) {
    FILE * fp = 0; fopen_s(&fp, filename, "rb");
    if(fp) {
      if(fread(&waveheader, 1, sizeof(waveheader), fp) == sizeof(waveheader)) {
        if(waveheader.chunkID    != 0x46464952) ok = false;
        if(waveheader.fmt        != 0x45564157) ok = false;
        if(waveheader.chunk1ID   != 0x20746d66) ok = false;
        if(waveheader.chunk2ID   != 0x61746164) ok = false;
        if(waveheader.format     !=          1) ok = false;
        if(waveheader.chunk1Size !=         16) ok = false;
        if(waveheader.chunk2Size ==          0) ok = false;
        if(waveheader.channels   <           1) ok = false;
        if(waveheader.channels    >          2) ok = false;
        if(waveheader.bits       <           8) ok = false;
        if(waveheader.bits        >         16) ok = false;
      } else {
        ok = false;
      }
      if(ok) {
        result->data = (unsigned char *) malloc(waveheader.chunk2Size);
        if(result->data) {
          if(fread(result->data, 1, waveheader.chunk2Size, fp) < waveheader.chunk2Size) {
            free(result->data);
            ok = false;
          } else {
            result->size = waveheader.chunk2Size;
          }
        } else {
          ok = false;
        }
      }
      fclose(fp);
    } else {
      ok = false;
    }
  }

  if(ok) {
    result->freq = waveheader.samplerate;
    switch(waveheader.channels) {
    case 1: 
      switch(waveheader.bits) {
      case 8:  result->format = AL_FORMAT_MONO8;
      case 16: result->format = AL_FORMAT_MONO16;
      } break;
    case 2: 
      switch(waveheader.bits) {
      case 8:  result->format = AL_FORMAT_STEREO8;
      case 16: result->format = AL_FORMAT_STEREO16;
      } break;
    }
  }

  if(!ok) {
    delete result;
    result = 0;
  }

  assert(result);
  return result;
}
