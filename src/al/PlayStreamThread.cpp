#include "GLDefines.h"
#include "ALDefines.h"
#include "Framework.h"
#include "CWaves.h"

SoundList PlayStreamThread::Sounds;

void PlayStreamThread::init()
{
	// Initialize Framework
	ALFWInit();
	if(!ALFWInitOpenAL()) {
		ALFWShutdown();
	}

  Sounds.push_back(load("data\\bomb.wav"));
}

Sound * PlayStreamThread::load(const char * filename)
{
  Sound * result = new Sound;
  result->data = 0;
  result->size = 0;
  CWaves * pWaveLoader = NULL;
	WAVEID WaveID;
	WAVEFORMATEX	wfex;

	unsigned long	ulDataSize = 0;
	unsigned long	ulFrequency = 0;
	unsigned long	ulFormat = 0;
	unsigned long	ulBufferSize;
	unsigned long	ulBytesWritten;
	void * pData = 0;

  pWaveLoader = new CWaves();
	if ((pWaveLoader) && (SUCCEEDED(pWaveLoader->OpenWaveFile(filename, &WaveID))))
	{
		pWaveLoader->GetWaveSize(WaveID, &ulDataSize);
		pWaveLoader->GetWaveFrequency(WaveID, &ulFrequency);
		pWaveLoader->GetWaveALBufferFormat(WaveID, &alGetEnumValue, &ulFormat);

		// Queue 250ms of audio data
		pWaveLoader->GetWaveFormatExHeader(WaveID, &wfex);
		pWaveLoader->GetWaveSize(WaveID, &ulBufferSize);

		if(ulFormat != 0) {
			pData = malloc(ulBufferSize);
      
			if(pData) {
				// Set read position to start of audio data
				pWaveLoader->SetWaveDataOffset(WaveID, 0);
        pWaveLoader->ReadWaveData(WaveID, pData, ulBufferSize, &ulBytesWritten);
			}
		}

		// Close Wave Handle
		pWaveLoader->DeleteWaveFile(WaveID);
	}

  result->data = pData;
  result->size = ulBufferSize;
  result->format = ulFormat;
  result->freq = ulFrequency;
  return result;
}

void PlayStreamThread::shutdown()
{
  Sound * s = 0;
  foreach(SoundList, s, Sounds) {
    if(s->data) {
      free(s->data);
    }
    delete s;
  }
	ALFWShutdownOpenAL();
	ALFWShutdown();
}

DWORD WINAPI stream_run(void * data)
{
  Sound * sound = (Sound *) data;
	ALuint uiSource;
	ALuint uiBuffer;
	ALint iState = AL_PLAYING;
  ALuint uiBuffers[1];
	
  alGenBuffers( 1, uiBuffers );
  alGenSources( 1, &uiSource );
  alBufferData(uiBuffers[0], sound->format, sound->data, sound->size, sound->freq);
	alSourceQueueBuffers(uiSource, 1, &uiBuffers[0]);
	alSourcePlay(uiSource);

  while(iState == AL_PLAYING) {
    Sleep(16);
    alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
  }

	alSourceStop(uiSource);
	alSourcei(uiSource, AL_BUFFER, 0);

	alDeleteSources( 1, &uiSource );
	alDeleteBuffers( 1, uiBuffers );

  return 0;
}

bool PlayStreamThread::play(int index)
{
  Sound * s = 0; int i = 0;
  foreach(SoundList, s, Sounds) {
    if(i++ == index) break;
  }
  HANDLE althread = CreateThread(NULL, 0, &stream_run, s, 0, 0);
  if(althread != INVALID_HANDLE_VALUE) {
    return true;
  }
  return false;
}
