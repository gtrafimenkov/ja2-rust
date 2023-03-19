// Windows implementation of the sound platform.

/*********************************************************************************
 * SGP Digital Sound Module
 *
 *		This module handles the playing of digital samples, preloaded or streamed.
 *
 * Derek Beland, May 28, 1997
 *********************************************************************************/
#include <stdio.h>
#include <string.h>

#include "Mss.h"
#include "SGP/Debug.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/MemMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "platform.h"
#include "platform_strings.h"
#include "platform_win.h"
#include "rust_debug.h"
#include "rust_fileman.h"

// Sample status flags
#define SAMPLE_ALLOCATED 0x00000001
#define SAMPLE_LOCKED 0x00000002
// #define SAMPLE_RANDOM 0x00000004
#define SAMPLE_RANDOM_MANUAL 0x00000008
#define SAMPLE_3D 0x00000010

// Struct definition for sample slots in the cache
//		Holds the regular sample data, as well as the
//		data for the random samples

typedef struct {
  CHAR8 pName[128];    // Path to sample data
  UINT32 uiSize;       // Size of sample data
  UINT32 uiSoundSize;  // Playable sound size
  UINT32 uiFlags;      // Status flags
  UINT32 uiSpeed;      // Playback frequency
  BOOLEAN fStereo;     // Stereo/Mono
  UINT8 ubBits;        // 8/16 bits
  PTR pData;           // pointer to sample data memory
  PTR pSoundStart;     // pointer to start of sound data
  UINT32 uiCacheHits;

  UINT32 uiTimeNext;  // Random sound data
  UINT32 uiTimeMin, uiTimeMax;
  UINT32 uiSpeedMin, uiSpeedMax;
  UINT32 uiVolMin, uiVolMax;
  UINT32 uiPanMin, uiPanMax;
  UINT32 uiPriority;
  UINT32 uiInstances;
  UINT32 uiMaxInstances;

  UINT32 uiAilWaveFormat;   // AIL wave sample type
  UINT32 uiADPCMBlockSize;  // Block size for compressed files

} SAMPLETAG;

// Structure definition for slots in the sound output
//		These are used for both the cached and double-buffered
//		streams
typedef struct {
  SAMPLETAG *pSample;
  UINT32 uiSample;
  HSAMPLE hMSS;
  HSTREAM hMSSStream;
  H3DSAMPLE hM3D;
  UINT32 uiFlags;
  UINT32 uiSoundID;
  UINT32 uiPriority;
  void (*pCallback)(UINT8 *, UINT32, UINT32, UINT32, void *);
  void *pData;
  void (*EOSCallback)(void *);
  void *pCallbackData;
  UINT32 uiTimeStamp;
  BOOLEAN fLooping;
  FileID hFile;
  BOOLEAN fMusic;
  BOOLEAN fStopAtZero;
  UINT32 uiFadeVolume;
  UINT32 uiFadeRate;
  UINT32 uiFadeTime;
} SOUNDTAG;

// Uncomment this to disable the startup of sound hardware
// #define SOUND_DISABLE

#pragma pack(push, 1)

// WAV file chunk definitions
typedef struct {
  // General chunk header
  CHAR8 cTag[4];
  UINT32 uiChunkSize;
} WAVCHUNK;

typedef struct {
  // WAV header
  CHAR8 cRiff[4];      // "RIFF"
  UINT32 uiChunkSize;  // Chunk length
  CHAR8 cFileType[4];  // "WAVE"
} WAVRIFF;

typedef struct {
  // FMT chunk
  CHAR8 cFormat[4];      // "FMT "
  UINT32 uiChunkSize;    // Chunk length
  UINT16 uiStereo;       // 1 if stereo, 0 if mono (Not reliable, use channels instead)
  UINT16 uiChannels;     // number of channels used 1=mono, 2=stereo, etc.
  UINT32 uiSpeed;        // Sampling Rate (speed)
  UINT32 uiBytesSec;     // Number of bytes per sec
  UINT16 uiBytesSample;  // Number of bytes per sample (1 = 8 bit mono,
                         // 2 = 8 bit stereo or 16 bit mono, 4 = 16 bit stereo
  UINT16 uiBitsSample;   // bits per sample
} WAVFMT;

typedef struct {
  // Data chunk
  CHAR8 cName[4];      // "DATA"
  UINT32 uiChunkSize;  // Chunk length
} WAVDATA;

#pragma pack(pop)

#define WAV_CHUNK_RIFF 0
#define WAV_CHUNK_FMT 1
#define WAV_CHUNK_DATA 2

#define NUM_WAV_CHUNKS 3

CHAR8 *cWAVChunks[3] = {"RIFF", "FMT ", "DATA"};

// global settings
#define SOUND_MAX_CACHED 128  // number of cache slots

#define SOUND_MAX_CHANNELS 16  // number of mixer channels

#pragma message("TEMP!")

#define SOUND_DEFAULT_MEMORY (8048 * 1024)  // default memory limit
#define SOUND_DEFAULT_THRESH (256 * 8024)   // size for sample to be double-buffered
#define SOUND_DEFAULT_STREAM (64 * 1024)    // double-buffered buffer size

/*#define		SOUND_DEFAULT_MEMORY	(2048*1024)		// default memory limit
#define		SOUND_DEFAULT_THRESH	(256*1024)		// size for sample to be
double-buffered #define		SOUND_DEFAULT_STREAM	(64*1024)
// double-buffered buffer size
*/
// playing/random value to indicate default
#define SOUND_PARMS_DEFAULT 0xffffffff

// Sound status flags
#define SOUND_CALLBACK 0x00000008

// Cache control functions
extern BOOLEAN SoundEmptyCache(void);
extern BOOLEAN SoundSampleIsInUse(UINT32 uiSample);

extern BOOLEAN SoundRandomShouldPlay(UINT32 uiSample);
extern UINT32 SoundStartRandom(UINT32 uiSample);

// Sound instance manipulation functions
extern BOOLEAN SoundStopMusic(void);

// New 3D sound priovider
extern BOOLEAN Sound3DInitProvider(CHAR8 *pProviderName);
extern void Sound3DShutdownProvider(void);

// 3D sound control
extern void Sound3DSetListener(FLOAT flX, FLOAT flY, FLOAT flZ);
extern void Sound3DStopAll(void);

// Local Function Prototypes
BOOLEAN SoundInitCache(void);
BOOLEAN SoundShutdownCache(void);
UINT32 SoundLoadSample(STR pFilename);
UINT32 SoundFreeSample(STR pFilename);
UINT32 SoundGetCached(STR pFilename);
UINT32 SoundLoadDisk(STR pFilename);

// Low level
UINT32 SoundGetEmptySample(void);
BOOLEAN SoundProcessWAVHeader(UINT32 uiSample);
UINT32 SoundFreeSampleIndex(UINT32 uiSample);
UINT32 SoundGetIndexByID(UINT32 uiSoundID);
static HDIGDRIVER SoundInitDriver(UINT32 uiRate, UINT16 uiBits, UINT16 uiChans);
BOOLEAN SoundInitHardware(void);
BOOLEAN SoundGetDriverName(HDIGDRIVER DIG, CHAR8 *cBuf);
BOOLEAN SoundShutdownHardware(void);
UINT32 SoundGetFreeChannel(void);
UINT32 SoundStartSample(UINT32 uiSample, UINT32 uiChannel, SOUNDPARMS *pParms);
UINT32 SoundStartStream(STR pFilename, UINT32 uiChannel, SOUNDPARMS *pParms);
UINT32 SoundGetUniqueID(void);
BOOLEAN SoundPlayStreamed(STR pFilename);
BOOLEAN SoundCleanCache(void);
BOOLEAN SoundSampleIsPlaying(UINT32 uiSample);
BOOLEAN SoundIndexIsPlaying(UINT32 uiSound);
BOOLEAN SoundStopIndex(UINT32 uiSound);
UINT32 SoundGetVolumeIndex(UINT32 uiChannel);
BOOLEAN SoundSetVolumeIndex(UINT32 uiChannel, UINT32 uiVolume);

// Global variables
UINT32 guiSoundDefaultVolume = 127;
UINT32 guiSoundMemoryLimit = SOUND_DEFAULT_MEMORY;     // Maximum memory used for sounds
UINT32 guiSoundMemoryUsed = 0;                         // Memory currently in use
UINT32 guiSoundCacheThreshold = SOUND_DEFAULT_THRESH;  // Double-buffered threshold

HDIGDRIVER hSoundDriver;      // Sound driver handle
BOOLEAN fDirectSound = TRUE;  // Using Direct Sound

// Local module variables
BOOLEAN fSoundSystemInit = FALSE;  // Startup called T/F
BOOLEAN gfEnableStartup = TRUE;    // Allow hardware to starup

// Sample cache list for files loaded
SAMPLETAG pSampleList[SOUND_MAX_CACHED];
// Sound channel list for output channels
SOUNDTAG pSoundList[SOUND_MAX_CHANNELS];

// 3D sound globals
CHAR8 *gpProviderName = NULL;
HPROVIDER gh3DProvider = 0;
H3DPOBJECT gh3DListener = 0;
BOOLEAN gfUsingEAX = TRUE;
UINT32 guiRoomTypeIndex = 0;

//*******************************************************************************
// High Level Interface
//*******************************************************************************

//*******************************************************************************
// SoundEnableSound
//
//	Allows or disallows the startup of the sound hardware.
//
//	Returns:	Nothing.
//
//*******************************************************************************
void SoundEnableSound(BOOLEAN fEnable) { gfEnableStartup = fEnable; }

//*******************************************************************************
// InitializeSoundManager
//
//	Zeros out the structs for the system info, and initializes the cache.
//
//	Returns:	TRUE always
//
//*******************************************************************************
BOOLEAN InitializeSoundManager(void) {
  UINT32 uiCount;

  if (fSoundSystemInit) ShutdownSoundManager();

  for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
    memset(&pSoundList[uiCount], 0, sizeof(SOUNDTAG));

#ifndef SOUND_DISABLE
  if (gfEnableStartup && SoundInitHardware()) fSoundSystemInit = TRUE;
#endif

  SoundInitCache();

  guiSoundMemoryLimit = SOUND_DEFAULT_MEMORY;
  guiSoundMemoryUsed = 0;
  guiSoundCacheThreshold = SOUND_DEFAULT_THRESH;

  if (gpProviderName && !gh3DProvider) Sound3DInitProvider(gpProviderName);

  return (TRUE);
}

//*******************************************************************************
// ShutdownSoundManager
//
//		Silences all currently playing sound, deallocates any memory allocated,
//	and releases the sound hardware.
//
//*******************************************************************************
void ShutdownSoundManager(void) {
  if (gh3DProvider) Sound3DShutdownProvider();

  SoundStopAll();
  SoundStopMusic();
  SoundShutdownCache();
  Sleep(1000);
  SoundShutdownHardware();
  // Sleep(1000);
  fSoundSystemInit = FALSE;
}

//*******************************************************************************
// SoundPlay
//
//		Starts a sample playing. If the sample is not loaded in the cache, it will
//	be found and loaded. The pParms structure is used to
//	override the attributes of the sample such as playback speed, and to specify
//	a volume. Any entry containing SOUND_PARMS_DEFAULT will be set by the system.
//
//	Returns:	If the sound was started, it returns a sound ID unique to that
//						instance of the sound
//						If an error occured, SOUND_ERROR will be returned
//
//
//	!!Note:  Can no longer play streamed files
//
//*******************************************************************************

UINT32 SoundPlay(STR pFilename, SOUNDPARMS *pParms) {
  UINT32 uiSample, uiChannel;

  if (fSoundSystemInit) {
    if (!SoundPlayStreamed(pFilename)) {
      if ((uiSample = SoundLoadSample(pFilename)) != NO_SAMPLE) {
        if ((uiChannel = SoundGetFreeChannel()) != SOUND_ERROR) {
          return (SoundStartSample(uiSample, uiChannel, pParms));
        }
      }
    } else {
      // Trying to play a sound which is bigger then the 'guiSoundCacheThreshold'

      // This line was causing a page fault in the Wiz 8 project, so
      // I changed it to the second line, which works OK. -- DB

      // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("\n*******\nSoundPlay():  ERROR:  trying to play
      // %s which is bigger then the 'guiSoundCacheThreshold', use SoundPlayStreamedFile()
      // instead\n", pFilename ) );

      FastDebugMsg(
          String("SoundPlay: ERROR: Trying to play %s sound is too lardge to load into cache, use "
                 "SoundPlayStreamedFile() instead\n",
                 pFilename));
    }
  }

  return (SOUND_ERROR);
}

//*******************************************************************************
// SoundPlayStreamedFile
//
//		The sample will
//	be played as a double-buffered sample. The pParms structure is used to
//	override the attributes of the sample such as playback speed, and to specify
//	a volume. Any entry containing SOUND_PARMS_DEFAULT will be set by the system.
//
//	Returns:	If the sound was started, it returns a sound ID unique to that
//						instance of the sound
//						If an error occured, SOUND_ERROR will be returned
//
//*******************************************************************************
UINT32 SoundPlayStreamedFile(STR pFilename, SOUNDPARMS *pParms) {
  UINT32 uiChannel;
  CHAR8 pFileHandlefileName[128];
  UINT32 uiRetVal = FALSE;

  if (fSoundSystemInit) {
    if ((uiChannel = SoundGetFreeChannel()) != SOUND_ERROR) {
      // Open the file
      FileID hFile = File_OpenForReading(pFilename);
      if (!hFile) {
        FastDebugMsg(
            String("\n*******\nSoundPlayStreamedFile():  ERROR:  Couldnt open '%s' in "
                   "SoundPlayStreamedFile()\n",
                   pFilename));
        return (SOUND_ERROR);
      }

      // MSS cannot determine which provider to play if you don't give it a real filename
      // so if the file isn't in a library, play it normally
      if (!File_IsInsideArchive(hFile)) {
        File_Close(hFile);
        return (SoundStartStream(pFilename, uiChannel, pParms));
      }

      // Get the real file handle of the file
      u64 hRealFileHandle = File_GetWinHandleToReadFile(hFile);
      if (hRealFileHandle == 0) {
        FastDebugMsg(
            String("\n*******\nSoundPlayStreamedFile():  ERROR:  Couldnt get a real file handle "
                   "for '%s' in SoundPlayStreamedFile()\n",
                   pFilename));
        return (SOUND_ERROR);
      }

      // Convert the file handle into a 'name'
      sprintf(pFileHandlefileName, "\\\\\\\\%lld", hRealFileHandle);

      // Start the sound stream
      uiRetVal = SoundStartStream(pFileHandlefileName, uiChannel, pParms);

      // if it succeeded, record the file handle
      if (uiRetVal != SOUND_ERROR)
        pSoundList[uiChannel].hFile = hFile;
      else
        File_Close(hFile);

      return (uiRetVal);
    }
  }

  return (SOUND_ERROR);
}

//*******************************************************************************
// SoundPlayRandom
//
//		Registers a sample to be played randomly within the specified parameters.
//	Parameters are passed in through pParms. Any parameter containing
//	SOUND_PARMS_DEFAULT will be set by the system. Only the uiTimeMin entry may
//	NOT be defaulted.
//
//	* Samples designated "random" are ALWAYS loaded into the cache, and locked
//	in place. They are never double-buffered, and this call will fail if they
//	cannot be loaded. *
//
//	Returns:	If successful, it returns the sample index it is loaded to, else
//						SOUND_ERROR is returned.
//
//*******************************************************************************
UINT32 SoundPlayRandom(STR pFilename, RANDOMPARMS *pParms) {
  UINT32 uiSample, uiTicks;

  if (fSoundSystemInit) {
    if ((uiSample = SoundLoadSample(pFilename)) != NO_SAMPLE) {
      pSampleList[uiSample].uiFlags |= (SAMPLE_RANDOM | SAMPLE_LOCKED);

      if (pParms->uiTimeMin == SOUND_PARMS_DEFAULT)
        return (SOUND_ERROR);
      else
        pSampleList[uiSample].uiTimeMin = pParms->uiTimeMin;

      if (pParms->uiTimeMax == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiTimeMax = pParms->uiTimeMin;
      else
        pSampleList[uiSample].uiTimeMax = pParms->uiTimeMax;

      pSampleList[uiSample].uiSpeedMin = pParms->uiSpeedMin;

      pSampleList[uiSample].uiSpeedMax = pParms->uiSpeedMax;

      if (pParms->uiVolMin == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiVolMin = guiSoundDefaultVolume;
      else
        pSampleList[uiSample].uiVolMin = pParms->uiVolMin;

      if (pParms->uiVolMax == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiVolMax = guiSoundDefaultVolume;
      else
        pSampleList[uiSample].uiVolMax = pParms->uiVolMax;

      if (pParms->uiPanMin == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiPanMin = 64;
      else
        pSampleList[uiSample].uiPanMin = pParms->uiPanMin;

      if (pParms->uiPanMax == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiPanMax = 64;
      else
        pSampleList[uiSample].uiPanMax = pParms->uiPanMax;

      if (pParms->uiMaxInstances == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiMaxInstances = 1;
      else
        pSampleList[uiSample].uiMaxInstances = pParms->uiMaxInstances;

      if (pParms->uiPriority == SOUND_PARMS_DEFAULT)
        pSampleList[uiSample].uiPriority = PRIORITY_RANDOM;
      else
        pSampleList[uiSample].uiPriority = pParms->uiPriority;

      pSampleList[uiSample].uiInstances = 0;

      uiTicks = Plat_GetTickCount();
      pSampleList[uiSample].uiTimeNext =
          Plat_GetTickCount() + pSampleList[uiSample].uiTimeMin +
          Random(pSampleList[uiSample].uiTimeMax - pSampleList[uiSample].uiTimeMin);
      return (uiSample);
    }
  }

  return (SOUND_ERROR);
}

//*******************************************************************************
// SoundIsPlaying
//
//		Returns TRUE/FALSE that an instance of a sound is still playing.
//
//*******************************************************************************
BOOLEAN SoundIsPlaying(UINT32 uiSoundID) {
  UINT32 uiSound;

  if (fSoundSystemInit) {
    uiSound = SoundGetIndexByID(uiSoundID);
    if (uiSound != NO_SAMPLE) return (SoundIndexIsPlaying(uiSound));
  }

  return (FALSE);
}

//*****************************************************************************************
// SoundIndexIsPlaying
//
// Returns TRUE/FALSE whether a sound channel's sample is currently playing.
//
// Returns BOOLEAN            - TRUE = playing, FALSE = stopped or nothing allocated
//
// UINT32 uiSound             - Channel number of sound
//
// Created:  2/24/00 Derek Beland
//*****************************************************************************************
BOOLEAN SoundIndexIsPlaying(UINT32 uiSound) {
  INT32 iStatus = SMP_DONE;

  if (fSoundSystemInit) {
    if (pSoundList[uiSound].hMSS != NULL) iStatus = AIL_sample_status(pSoundList[uiSound].hMSS);

    if (pSoundList[uiSound].hMSSStream != NULL)
      iStatus = AIL_stream_status(pSoundList[uiSound].hMSSStream);

    if (pSoundList[uiSound].hM3D != NULL) iStatus = AIL_3D_sample_status(pSoundList[uiSound].hM3D);

    return ((iStatus != SMP_DONE) && (iStatus != SMP_STOPPED));
  }

  return (FALSE);
}

//*******************************************************************************
// SoundStop
//
//		Stops the playing of a sound instance, if still playing.
//
//	Returns:	TRUE if the sample was actually stopped, FALSE if it could not be
//						found, or was not playing.
//
//*******************************************************************************
BOOLEAN SoundStop(UINT32 uiSoundID) {
  UINT32 uiSound;

  if (fSoundSystemInit) {
    if (SoundIsPlaying(uiSoundID)) {
      uiSound = SoundGetIndexByID(uiSoundID);
      if (uiSound != NO_SAMPLE) {
        SoundStopIndex(uiSound);
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

//*******************************************************************************
// SoundGetSystemInfo
//
//		Returns information about the capabilities of the hardware. Currently does
//	nothing.
//
//	Returns:	FALSE, always
//
//*******************************************************************************
BOOLEAN SoundGetSystemInfo(void) { return (FALSE); }

//*******************************************************************************
// SoundStopAll
//
//		Stops all currently playing sounds.
//
//	Returns:	TRUE, always
//
//*******************************************************************************
BOOLEAN SoundStopAll(void) {
  UINT32 uiCount;

  if (fSoundSystemInit) {
    for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
      if (!pSoundList[uiCount].fMusic) SoundStopIndex(uiCount);
  }

  return (TRUE);
}

//*******************************************************************************
// SoundSetVolume
//
//		Sets the volume on a currently playing sound.
//
//	Returns:	TRUE if the volume was actually set on the sample, FALSE if the
//						sample had already expired or couldn't be found
//
//*******************************************************************************
BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume) {
  UINT32 uiSound, uiVolCap;

  if (fSoundSystemInit) {
    uiVolCap = min(uiVolume, 127);

    if ((uiSound = SoundGetIndexByID(uiSoundID)) != NO_SAMPLE) {
      pSoundList[uiSound].uiFadeVolume = uiVolume;
      return (SoundSetVolumeIndex(uiSound, uiVolume));
    }
  }

  return (FALSE);
}

//*****************************************************************************************
// SoundSetVolumeIndex
//
// Sounds the volume on a sound channel.
//
// Returns BOOLEAN            - TRUE if the volume was set
//
// UINT32 uiChannel           - Sound channel
// UINT32 uiVolume            - New volume 0-127
//
// Created:  3/17/00 Derek Beland
//*****************************************************************************************
BOOLEAN SoundSetVolumeIndex(UINT32 uiChannel, UINT32 uiVolume) {
  UINT32 uiVolCap;

  if (fSoundSystemInit) {
    uiVolCap = min(uiVolume, 127);

    if (pSoundList[uiChannel].hMSS != NULL)
      AIL_set_sample_volume(pSoundList[uiChannel].hMSS, uiVolCap);

    if (pSoundList[uiChannel].hMSSStream != NULL)
      AIL_set_stream_volume(pSoundList[uiChannel].hMSSStream, uiVolCap);

    if (pSoundList[uiChannel].hM3D != NULL)
      AIL_set_3D_sample_volume(pSoundList[uiChannel].hM3D, uiVolCap);

    return (TRUE);
  }

  return (FALSE);
}

//*******************************************************************************
// SoundSetPan
//
//		Sets the pan on a currently playing sound.
//
//	Returns:	TRUE if the pan was actually set on the sample, FALSE if the
//						sample had already expired or couldn't be found
//
//*******************************************************************************
BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan) {
  UINT32 uiSound, uiPanCap;

  if (fSoundSystemInit) {
    uiPanCap = min(uiPan, 127);

    if ((uiSound = SoundGetIndexByID(uiSoundID)) != NO_SAMPLE) {
      if (pSoundList[uiSound].hMSS != NULL) AIL_set_sample_pan(pSoundList[uiSound].hMSS, uiPanCap);

      if (pSoundList[uiSound].hMSSStream != NULL)
        AIL_set_stream_pan(pSoundList[uiSound].hMSSStream, uiPanCap);

      return (TRUE);
    }
  }

  return (FALSE);
}

//*******************************************************************************
// SoundGetVolume
//
//		Returns the current volume setting of a sound that is playing. If the sound
//	has expired, or could not be found, SOUND_ERROR is returned.
//
//*******************************************************************************
UINT32 SoundGetVolume(UINT32 uiSoundID) {
  UINT32 uiSound;

  if (fSoundSystemInit) {
    if ((uiSound = SoundGetIndexByID(uiSoundID)) != NO_SAMPLE)
      return (SoundGetVolumeIndex(uiSound));
  }

  return (SOUND_ERROR);
}

//*****************************************************************************************
// SoundGetVolumeIndex
//
// Returns the current volume of a sound channel.
//
// Returns UINT32             - Volume 0-127
//
// UINT32 uiChannel           - Channel
//
// Created:  3/17/00 Derek Beland
//*****************************************************************************************
UINT32 SoundGetVolumeIndex(UINT32 uiChannel) {
  if (fSoundSystemInit) {
    if (pSoundList[uiChannel].hMSS != NULL)
      return ((UINT32)AIL_sample_volume(pSoundList[uiChannel].hMSS));

    if (pSoundList[uiChannel].hMSSStream != NULL)
      return ((UINT32)AIL_stream_volume(pSoundList[uiChannel].hMSSStream));

    if (pSoundList[uiChannel].hM3D != NULL)
      return ((UINT32)AIL_3D_sample_volume(pSoundList[uiChannel].hM3D));
  }

  return (SOUND_ERROR);
}

//*******************************************************************************
// SoundServiceRandom
//
//		This function should be polled by the application if random samples are
//	used. The time marks on each are checked and if it is time to spawn a new
//	instance of the sound, the number already in existance are checked, and if
//	there is room, a new one is made and the count updated.
//		If random samples are not being used, there is no purpose in polling this
//	function.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
BOOLEAN SoundServiceRandom(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++) {
    if (!(pSampleList[uiCount].uiFlags & SAMPLE_RANDOM_MANUAL) && SoundRandomShouldPlay(uiCount))
      SoundStartRandom(uiCount);
  }

  return (FALSE);
}

//*******************************************************************************
// SoundRandomShouldPlay
//
//	Determines whether a random sound is ready for playing or not.
//
//	Returns:	TRUE if a the sample should be played.
//
//*******************************************************************************
BOOLEAN SoundRandomShouldPlay(UINT32 uiSample) {
  UINT32 uiTicks;

  uiTicks = Plat_GetTickCount();
  if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM)
    if (pSampleList[uiSample].uiTimeNext <= Plat_GetTickCount())
      if (pSampleList[uiSample].uiInstances < pSampleList[uiSample].uiMaxInstances) return (TRUE);

  return (FALSE);
}

//*******************************************************************************
// SoundStartRandom
//
//	Starts an instance of a random sample.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
UINT32 SoundStartRandom(UINT32 uiSample) {
  UINT32 uiChannel, uiSoundID;
  SOUNDPARMS spParms;

  if ((uiChannel = SoundGetFreeChannel()) != SOUND_ERROR) {
    memset(&spParms, 0xff, sizeof(SOUNDPARMS));

    //		spParms.uiSpeed=pSampleList[uiSample].uiSpeedMin+Random(pSampleList[uiSample].uiSpeedMax-pSampleList[uiSample].uiSpeedMin);
    spParms.uiVolume = pSampleList[uiSample].uiVolMin +
                       Random(pSampleList[uiSample].uiVolMax - pSampleList[uiSample].uiVolMin);
    spParms.uiPan = pSampleList[uiSample].uiPanMin +
                    Random(pSampleList[uiSample].uiPanMax - pSampleList[uiSample].uiPanMin);
    spParms.uiLoop = 1;
    spParms.uiPriority = pSampleList[uiSample].uiPriority;

    if ((uiSoundID = SoundStartSample(uiSample, uiChannel, &spParms)) != SOUND_ERROR) {
      pSampleList[uiSample].uiTimeNext =
          Plat_GetTickCount() + pSampleList[uiSample].uiTimeMin +
          Random(pSampleList[uiSample].uiTimeMax - pSampleList[uiSample].uiTimeMin);
      pSampleList[uiSample].uiInstances++;
      return (uiSoundID);
    }
  }
  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundStopAllRandom
//
//		This function should be polled by the application if random samples are
//	used. The time marks on each are checked and if it is time to spawn a new
//	instance of the sound, the number already in existance are checked, and if
//	there is room, a new one is made and the count updated.
//		If random samples are not being used, there is no purpose in polling this
//	function.
//
//	Returns:	TRUE if a new random sound was created, FALSE if nothing was done.
//
//*******************************************************************************
BOOLEAN SoundStopAllRandom(void) {
  UINT32 uiChannel, uiSample;

  // Stop all currently playing random sounds
  for (uiChannel = 0; uiChannel < SOUND_MAX_CHANNELS; uiChannel++) {
    if ((pSoundList[uiChannel].hMSS != NULL) || (pSoundList[uiChannel].hM3D != NULL)) {
      uiSample = pSoundList[uiChannel].uiSample;

      // if this was a random sample, decrease the iteration count
      if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM) SoundStopIndex(uiChannel);
    }
  }

  // Unlock all random sounds so they can be dumped from the cache, and
  // take the random flag off so they won't be serviced/played
  for (uiSample = 0; uiSample < SOUND_MAX_CACHED; uiSample++) {
    if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM)
      pSampleList[uiSample].uiFlags &= (~(SAMPLE_RANDOM | SAMPLE_LOCKED));
  }

  return (FALSE);
}

//*******************************************************************************
// SoundServiceStreams
//
//		Can be polled in tight loops where sound buffers might starve due to heavy
//	hardware use, etc. Streams DO NOT normally need to be serviced manually, but
//	in some cases (heavy file loading) it might be desirable.
//
//		If you are using the end of sample callbacks, you must call this function
//	periodically to check the sample's status.
//
//	Returns:	TRUE always.
//
//*******************************************************************************
BOOLEAN SoundServiceStreams(void) {
  UINT32 uiCount, uiSpeed, uiBuffLen, uiBytesPerSample;
  UINT8 *pBuffer;
  void *pData;

  if (fSoundSystemInit) {
    for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
      if (pSoundList[uiCount].hMSSStream != NULL) {
        if (AIL_service_stream(pSoundList[uiCount].hMSSStream, 0)) {
          if (pSoundList[uiCount].uiFlags & SOUND_CALLBACK) {
            uiSpeed = pSoundList[uiCount].hMSSStream->datarate;
            uiBuffLen = pSoundList[uiCount].hMSSStream->bufsize;
            pBuffer = pSoundList[uiCount].hMSSStream->bufs[pSoundList[uiCount].hMSSStream->buf1];
            uiBytesPerSample = pSoundList[uiCount].hMSSStream->samp->format;
            pData = pSoundList[uiCount].pData;
            pSoundList[uiCount].pCallback(pBuffer, uiBuffLen, uiSpeed, uiBytesPerSample, pData);
          }
        }
      }

      if (pSoundList[uiCount].hMSS || pSoundList[uiCount].hMSSStream || pSoundList[uiCount].hM3D) {
        // If a sound has a handle, but isn't playing, stop it and free up the handle
        if (!SoundIsPlaying(pSoundList[uiCount].uiSoundID))
          SoundStopIndex(uiCount);
        else {  // Check the volume fades on currently playing sounds
          UINT32 uiVolume = SoundGetVolumeIndex(uiCount);
          UINT32 uiTime = Plat_GetTickCount();

          if ((uiVolume != pSoundList[uiCount].uiFadeVolume) &&
              (uiTime >= (pSoundList[uiCount].uiFadeTime + pSoundList[uiCount].uiFadeRate))) {
            if (uiVolume < pSoundList[uiCount].uiFadeVolume)
              SoundSetVolumeIndex(uiCount, ++uiVolume);
            else if (uiVolume > pSoundList[uiCount].uiFadeVolume) {
              uiVolume--;
              if (!uiVolume && pSoundList[uiCount].fStopAtZero)
                SoundStopIndex(uiCount);
              else
                SoundSetVolumeIndex(uiCount, uiVolume);
            }

            pSoundList[uiCount].uiFadeTime = uiTime;
          }
        }
      }
    }
  }

  return (TRUE);
}

//*******************************************************************************
// SoundGetPosition
//
//	Reports the current time position of the sample.
//
//	Note: You should be checking SoundIsPlaying very carefully while
//	calling this function.
//
//	Returns:	The current time of the sample in milliseconds.
//
//*******************************************************************************
UINT32 SoundGetPosition(UINT32 uiSoundID) {
  // UINT32 uiSound, uiFreq=0, uiPosition=0, uiBytesPerSample=0, uiFormat=0;
  UINT32 uiSound, uiTime, uiPosition;

  if (fSoundSystemInit) {
    if ((uiSound = SoundGetIndexByID(uiSoundID)) != NO_SAMPLE) {
      /*			if(pSoundList[uiSound].hMSSStream!=NULL)
                              {
                                      uiPosition=(UINT32)AIL_stream_position(pSoundList[uiSound].hMSSStream);
                                      uiFreq=(UINT32)pSoundList[uiSound].hMSSStream->samp->playback_rate;
                                      uiFormat=(UINT32)pSoundList[uiSound].hMSSStream->samp->format;

                              }
                              else if(pSoundList[uiSound].hMSS!=NULL)
                              {
                                      uiPosition=(UINT32)AIL_sample_position(pSoundList[uiSound].hMSS);
                                      uiFreq=(UINT32)pSoundList[uiSound].hMSS->playback_rate;
                                      uiFormat=(UINT32)pSoundList[uiSound].hMSS->format;
                              }
                      }

                      switch(uiFormat)
                      {
                              case DIG_F_MONO_8:		uiBytesPerSample=1;
                                                                                                                      break;
                              case DIG_F_MONO_16:		uiBytesPerSample=2;
                                                                                                                      break;
                              case DIG_F_STEREO_8:	uiBytesPerSample=2;
                                                                                                                      break;
                              case DIG_F_STEREO_16:	uiBytesPerSample=4;
                                                                                                                      break;
                      }

                      if(uiFreq)
                      {
                              return((uiPosition/uiBytesPerSample)/(uiFreq/1000));
                      }
              }
      */
      uiTime = Plat_GetTickCount();
      // check for rollover
      if (uiTime < pSoundList[uiSound].uiTimeStamp)
        uiPosition = (0 - pSoundList[uiSound].uiTimeStamp) + uiTime;
      else
        uiPosition = (uiTime - pSoundList[uiSound].uiTimeStamp);

      return (uiPosition);
    }
  }

  return (0);
}

//*******************************************************************************
// Cacheing Subsystem
//*******************************************************************************

//*******************************************************************************
// SoundInitCache
//
//		Zeros out the structures of the sample list.
//
//*******************************************************************************
BOOLEAN SoundInitCache(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++)
    memset(&pSampleList[uiCount], 0, sizeof(SAMPLETAG));

  return (TRUE);
}

//*******************************************************************************
// SoundShutdownCache
//
//		Empties out the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
BOOLEAN SoundShutdownCache(void) {
  SoundEmptyCache();
  return (TRUE);
}

//*******************************************************************************
// SoundEmptyCache
//
//		Frees up all samples in the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
BOOLEAN SoundEmptyCache(void) {
  UINT32 uiCount;

  SoundStopAll();

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++) SoundFreeSampleIndex(uiCount);

  return (TRUE);
}

//*******************************************************************************
// SoundLoadSample
//
//		Frees up all samples in the cache.
//
//	Returns: TRUE, always
//
//*******************************************************************************
UINT32 SoundLoadSample(STR pFilename) {
  UINT32 uiSample = NO_SAMPLE;

  if ((uiSample = SoundGetCached(pFilename)) != NO_SAMPLE) return (uiSample);

  return (SoundLoadDisk(pFilename));
}

//*******************************************************************************
// SoundLockSample
//
//		Locks a sample into cache memory, so the cacheing system won't release it
//	when it needs room.
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
UINT32 SoundLockSample(STR pFilename) {
  UINT32 uiSample;

  if ((uiSample = SoundGetCached(pFilename)) != NO_SAMPLE) {
    pSampleList[uiSample].uiFlags |= SAMPLE_LOCKED;
    return (uiSample);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundUnlockSample
//
//		Removes the lock on a sample so the cache is free to dump it when necessary.
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
UINT32 SoundUnlockSample(STR pFilename) {
  UINT32 uiSample;

  if ((uiSample = SoundGetCached(pFilename)) != NO_SAMPLE) {
    pSampleList[uiSample].uiFlags &= (~SAMPLE_LOCKED);
    return (uiSample);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundFreeSample
//
//		Releases the resources associated with a sample from the cache.
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
UINT32 SoundFreeSample(STR pFilename) {
  UINT32 uiSample;

  if ((uiSample = SoundGetCached(pFilename)) != NO_SAMPLE) {
    if (!SoundSampleIsPlaying(uiSample)) {
      SoundFreeSampleIndex(uiSample);
      return (uiSample);
    }
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundGetCached
//
//		Tries to locate a sound by looking at what is currently loaded in the
//	cache.
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
UINT32 SoundGetCached(STR pFilename) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++) {
    if (strcasecmp(pSampleList[uiCount].pName, pFilename) == 0) return (uiCount);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundLoadDisk
//
//		Loads a sound file from disk into the cache, allocating memory and a slot
//	for storage.
//
//
//	Returns: The sample index if successful, NO_SAMPLE if the file wasn't found
//						in the cache.
//
//*******************************************************************************
UINT32 SoundLoadDisk(STR pFilename) {
  FileID hFile;
  UINT32 uiSize, uiSample;
  BOOLEAN fRemoved = TRUE;

  Assert(pFilename != NULL);

  if ((hFile = File_OpenForReading(pFilename)) != 0) {
    uiSize = File_GetSize(hFile);

    // if insufficient memory, start unloading old samples until either
    // there's nothing left to unload, or we fit
    fRemoved = TRUE;
    while (((uiSize + guiSoundMemoryUsed) > guiSoundMemoryLimit) && (fRemoved))
      fRemoved = SoundCleanCache();

    // if we still don't fit
    if ((uiSize + guiSoundMemoryUsed) > guiSoundMemoryLimit) {
      FastDebugMsg(
          String("SoundLoadDisk:  ERROR:  trying to play %s, not enough memory\n", pFilename));
      File_Close(hFile);
      return (NO_SAMPLE);
    }

    // if all the sample slots are full, unloading one
    if ((uiSample = SoundGetEmptySample()) == NO_SAMPLE) {
      SoundCleanCache();
      uiSample = SoundGetEmptySample();
    }

    // if we still don't have a sample slot
    if (uiSample == NO_SAMPLE) {
      FastDebugMsg(
          String("SoundLoadDisk:  ERROR: Trying to play %s, sound channels are full\n", pFilename));
      File_Close(hFile);
      return (NO_SAMPLE);
    }

    memset(&pSampleList[uiSample], 0, sizeof(SAMPLETAG));

    if ((pSampleList[uiSample].pData = AIL_mem_alloc_lock(uiSize)) == NULL) {
      FastDebugMsg(
          String("SoundLoadDisk:  ERROR: Trying to play %s, AIL channels are full\n", pFilename));
      File_Close(hFile);
      return (NO_SAMPLE);
    }

    guiSoundMemoryUsed += uiSize;

    File_Read(hFile, pSampleList[uiSample].pData, uiSize, NULL);
    File_Close(hFile);

    strcpy(pSampleList[uiSample].pName, pFilename);
    strupr(pSampleList[uiSample].pName);
    pSampleList[uiSample].uiSize = uiSize;
    pSampleList[uiSample].uiFlags |= SAMPLE_ALLOCATED;

    /*		if(!strstr(pFilename, ".MP3"))
                            SoundProcessWAVHeader(uiSample);
    */
    return (uiSample);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundCleanCache
//
//		Removes the least-used sound from the cache to make room.
//
//	Returns:	TRUE if a sample was freed, FALSE if none
//
//*******************************************************************************
BOOLEAN SoundCleanCache(void) {
  UINT32 uiCount, uiLowestHits = NO_SAMPLE, uiLowestHitsCount = 0;

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++) {
    if ((pSampleList[uiCount].uiFlags & SAMPLE_ALLOCATED) &&
        !(pSampleList[uiCount].uiFlags & SAMPLE_LOCKED)) {
      if ((uiLowestHits == NO_SAMPLE) || (uiLowestHitsCount < pSampleList[uiCount].uiCacheHits)) {
        if (!SoundSampleIsPlaying(uiCount)) {
          uiLowestHits = uiCount;
          uiLowestHitsCount = pSampleList[uiCount].uiCacheHits;
        }
      }
    }
  }

  if (uiLowestHits != NO_SAMPLE) {
    SoundFreeSampleIndex(uiLowestHits);
    return (TRUE);
  }

  return (FALSE);
}

//*******************************************************************************
// Low Level Interface (Local use only)
//*******************************************************************************

//*******************************************************************************
// SoundSampleIsPlaying
//
//		Returns TRUE/FALSE that a sample is currently in use for playing a sound.
//
//*******************************************************************************
BOOLEAN SoundSampleIsPlaying(UINT32 uiSample) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
    if (pSoundList[uiCount].uiSample == uiSample) return (TRUE);
  }

  return (FALSE);
}

//*******************************************************************************
// SoundGetEmptySample
//
//		Returns the slot number of an available sample index.
//
//	Returns:	A free sample index, or NO_SAMPLE if none are left.
//
//*******************************************************************************
UINT32 SoundGetEmptySample(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CACHED; uiCount++) {
    if (!(pSampleList[uiCount].uiFlags & SAMPLE_ALLOCATED)) return (uiCount);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundProcessWAVHeader
//
//		Reads the information contained in the header of a loaded WAV file, and
//	transfers it to the system structures for that slot.
//
//	Returns:	TRUE if a good header was processed, FALSE if an error occurred.
//
//*******************************************************************************
BOOLEAN SoundProcessWAVHeader(UINT32 uiSample) {
  CHAR8 *pChunk;
  AILSOUNDINFO ailInfo;

  pChunk = (CHAR8 *)pSampleList[uiSample].pData;
  if (!AIL_WAV_info((void *)pChunk, &ailInfo)) return (FALSE);

  pSampleList[uiSample].uiSpeed = ailInfo.rate;
  pSampleList[uiSample].fStereo = (BOOLEAN)(ailInfo.channels == 2);
  pSampleList[uiSample].ubBits = (UINT8)ailInfo.bits;

  pSampleList[uiSample].pSoundStart = (PTR)ailInfo.data_ptr;
  pSampleList[uiSample].uiSoundSize = ailInfo.data_len;

  pSampleList[uiSample].uiAilWaveFormat = ailInfo.format;
  pSampleList[uiSample].uiADPCMBlockSize = ailInfo.block_size;

  return (TRUE);
}

//*******************************************************************************
// SoundFreeSampleIndex
//
//		Frees up a sample referred to by it's index slot number.
//
//	Returns:	Slot number if something was free, NO_SAMPLE otherwise.
//
//*******************************************************************************
UINT32 SoundFreeSampleIndex(UINT32 uiSample) {
  if (pSampleList[uiSample].uiFlags & SAMPLE_ALLOCATED) {
    if (pSampleList[uiSample].pData != NULL) {
      guiSoundMemoryUsed -= pSampleList[uiSample].uiSize;
      AIL_mem_free_lock(pSampleList[uiSample].pData);
    }

    memset(&pSampleList[uiSample], 0, sizeof(SAMPLETAG));
    return (uiSample);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundGetIndexByID
//
//		Searches out a sound instance referred to by it's ID number.
//
//	Returns:	If the instance was found, the slot number. NO_SAMPLE otherwise.
//
//*******************************************************************************
UINT32 SoundGetIndexByID(UINT32 uiSoundID) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
    if (pSoundList[uiCount].uiSoundID == uiSoundID) return (uiCount);
  }

  return (NO_SAMPLE);
}

//*******************************************************************************
// SoundInitHardware
//
//		Initializes the sound hardware through Windows/DirectX. THe highest possible
//	mixing rate and capabilities set are searched out and used.
//
//	Returns:	TRUE if the hardware was initialized, FALSE otherwise.
//
//*******************************************************************************
BOOLEAN SoundInitHardware(void) {
  UINT32 uiCount;
  CHAR8 cDriverName[128];

  // Try to start up the Miles Sound System
  if (!AIL_startup()) return (FALSE);

  // Initialize the driver handle
  hSoundDriver = NULL;

  // Set up preferences, to try to use DirectSound and to set the
  // maximum number of handles that we are allowed to allocate. Note
  // that this is not the number we may have playing at one time--
  // that number is set by SOUND_MAX_CHANNELS
  AIL_set_preference(DIG_MIXER_CHANNELS, SOUND_MAX_CHANNELS);

  fDirectSound = TRUE;

  AIL_set_preference(DIG_USE_WAVEOUT, NO);
  // startup with DirectSound
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(44100, 16, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(44100, 8, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(22050, 8, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(11025, 8, 1);

  if (hSoundDriver) {
    // Detect if the driver is emulated or not
    SoundGetDriverName(hSoundDriver, cDriverName);
    _strlwr(cDriverName);
    // If it is, we don't want to use it, since the extra
    // code layer can slow us down by up to 40% under NT
    if (strstr(cDriverName, "emulated")) {
      AIL_waveOutClose(hSoundDriver);
      hSoundDriver = NULL;
    }
  }

  // nothing in DirectSound worked, so try waveOut
  if (hSoundDriver == NULL) {
    fDirectSound = FALSE;
    AIL_set_preference(DIG_USE_WAVEOUT, YES);
  }

  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(44100, 16, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(44100, 8, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(22050, 8, 2);
  if (hSoundDriver == NULL) hSoundDriver = SoundInitDriver(11025, 8, 1);

  if (hSoundDriver != NULL) {
    for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++)
      memset(&pSoundList[uiCount], 0, sizeof(SOUNDTAG));

    return (TRUE);
  }

  return (FALSE);

  /*
          // midi startup
          if (hSoundDriver!=NULL)
          {
                  soundMDI = MIDI_init_driver();
                  if (soundMDI==NULL)
                  {
                          _RPT1(_CRT_WARN, "MIDI: %s", AIL_last_error());
                  }
                  else
                  {
                          soundSEQ = MIDI_load_sequence(soundMDI, "SOUNDS\\DEMO.XMI");
                          if (soundSEQ==NULL)
                          _RPT1(_CRT_WARN, "MIDI: %s", AIL_last_error());
                  }
          }
  */
}

//*******************************************************************************
// SoundShutdownHardware
//
//		Shuts down the system hardware.
//
//	Returns:	TRUE always.
//
//*******************************************************************************
BOOLEAN SoundShutdownHardware(void) {
  if (fSoundSystemInit) AIL_shutdown();

  return (TRUE);
}

//*******************************************************************************
// SoundInitDriver
//
//		Tries to initialize the sound driver using the specified settings.
//
//	Returns:	Pointer to the driver if successful, NULL otherwise.
//
//*******************************************************************************
static HDIGDRIVER SoundInitDriver(UINT32 uiRate, UINT16 uiBits, UINT16 uiChans) {
  static PCMWAVEFORMAT sPCMWF;
  HDIGDRIVER DIG;
  CHAR8 cBuf[128];

  memset(&sPCMWF, 0, sizeof(PCMWAVEFORMAT));
  sPCMWF.wf.wFormatTag = WAVE_FORMAT_PCM;
  sPCMWF.wf.nChannels = uiChans;
  sPCMWF.wf.nSamplesPerSec = uiRate;
  sPCMWF.wf.nAvgBytesPerSec = uiRate * (uiBits / 8) * uiChans;
  sPCMWF.wf.nBlockAlign = (uiBits / 8) * uiChans;
  sPCMWF.wBitsPerSample = uiBits;

  if (AIL_waveOutOpen(&DIG, NULL, 0, (LPWAVEFORMAT)&sPCMWF)) return (NULL);

  memset(cBuf, 0, 128);
  AIL_digital_configuration(DIG, 0, 0, cBuf);
  FastDebugMsg(String("Sound Init: %dKHz, %d uiBits, %s %s\n", uiRate, uiBits,
                      (uiChans == 1) ? "Mono" : "Stereo", cBuf));

  return (DIG);
}

//*******************************************************************************
// SoundGetDriverName
//
//		Returns the name of the AIL device.
//
//	Returns:	TRUE or FALSE if the string was filled.
//
//*******************************************************************************
BOOLEAN SoundGetDriverName(HDIGDRIVER DIG, CHAR8 *cBuf) {
  if (DIG) {
    cBuf[0] = '\0';
    AIL_digital_configuration(DIG, NULL, NULL, cBuf);
    return (TRUE);
  } else
    return (FALSE);
}

//*******************************************************************************
// SoundGetFreeChannel
//
//		Finds an unused sound channel in the channel list.
//
//	Returns:	Index of a sound channel if one was found, SOUND_ERROR if not.
//
//*******************************************************************************
UINT32 SoundGetFreeChannel(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
    if (!SoundIsPlaying(pSoundList[uiCount].uiSoundID)) {
      SoundStopIndex(uiCount);
    }

    if ((pSoundList[uiCount].hMSS == NULL) && (pSoundList[uiCount].hMSSStream == NULL) &&
        (pSoundList[uiCount].hM3D == NULL))
      return (uiCount);
  }

  return (SOUND_ERROR);
}

//*******************************************************************************
// SoundStartSample
//
//		Starts up a sample on the specified channel. Override parameters are passed
//	in through the structure pointer pParms. Any entry with a value of 0xffffffff
//	will be filled in by the system.
//
//	Returns:	Unique sound ID if successful, SOUND_ERROR if not.
//
//*******************************************************************************
UINT32 SoundStartSample(UINT32 uiSample, UINT32 uiChannel, SOUNDPARMS *pParms) {
  UINT32 uiSoundID;
  CHAR8 AILString[200];

  if (!fSoundSystemInit) return (SOUND_ERROR);

  if ((pSoundList[uiChannel].hMSS = AIL_allocate_sample_handle(hSoundDriver)) == NULL) {
    sprintf(AILString, "Sample Error: %s", AIL_last_error());
    FastDebugMsg(AILString);
    return (SOUND_ERROR);
  }

  AIL_init_sample(pSoundList[uiChannel].hMSS);

  if (!AIL_set_named_sample_file(pSoundList[uiChannel].hMSS, pSampleList[uiSample].pName,
                                 pSampleList[uiSample].pData, pSampleList[uiSample].uiSize, 0)) {
    AIL_release_sample_handle(pSoundList[uiChannel].hMSS);
    pSoundList[uiChannel].hMSS = NULL;

    sprintf(AILString, "AIL Set Sample Error: %s", AIL_last_error());
    DbgMessage(TOPIC_GAME, DBG_LEVEL_0, AILString);
    return (SOUND_ERROR);
  }

  // Store the natural playback rate before we modify it below
  pSampleList[uiSample].uiSpeed = AIL_sample_playback_rate(pSoundList[uiChannel].hMSS);

  if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM) {
    if ((pSampleList[uiSample].uiSpeedMin != SOUND_PARMS_DEFAULT) &&
        (pSampleList[uiSample].uiSpeedMin != SOUND_PARMS_DEFAULT)) {
      UINT32 uiSpeed = pSampleList[uiSample].uiSpeedMin +
                       Random(pSampleList[uiSample].uiSpeedMax - pSampleList[uiSample].uiSpeedMin);

      AIL_set_sample_playback_rate(pSoundList[uiChannel].hMSS, uiSpeed);
    }
  } else {
    if ((pParms != NULL) && (pParms->uiSpeed != SOUND_PARMS_DEFAULT)) {
      Assert((pParms->uiSpeed > 0) && (pParms->uiSpeed <= 60000));
      AIL_set_sample_playback_rate(pSoundList[uiChannel].hMSS, pParms->uiSpeed);
    }
  }

  if ((pParms != NULL) && (pParms->uiPitchBend != SOUND_PARMS_DEFAULT)) {
    UINT32 uiRate = AIL_sample_playback_rate(pSoundList[uiChannel].hMSS);
    UINT32 uiBend = uiRate * pParms->uiPitchBend / 100;
    AIL_set_sample_playback_rate(pSoundList[uiChannel].hMSS,
                                 uiRate + (Random(uiBend * 2) - uiBend));
  }

  if ((pParms != NULL) && (pParms->uiVolume != SOUND_PARMS_DEFAULT))
    AIL_set_sample_volume(pSoundList[uiChannel].hMSS, pParms->uiVolume);
  else
    AIL_set_sample_volume(pSoundList[uiChannel].hMSS, guiSoundDefaultVolume);

  if ((pParms != NULL) && (pParms->uiLoop != SOUND_PARMS_DEFAULT)) {
    AIL_set_sample_loop_count(pSoundList[uiChannel].hMSS, pParms->uiLoop);

    // If looping infinately, lock the sample so it can't be unloaded
    // and mark it as a looping sound
    if (pParms->uiLoop == 0) {
      pSampleList[uiSample].uiFlags |= SAMPLE_LOCKED;
      pSoundList[uiChannel].fLooping = TRUE;
    }
  }

  if ((pParms != NULL) && (pParms->uiPan != SOUND_PARMS_DEFAULT))
    AIL_set_sample_pan(pSoundList[uiChannel].hMSS, pParms->uiPan);

  if ((pParms != NULL) && (pParms->uiPriority != SOUND_PARMS_DEFAULT))
    pSoundList[uiChannel].uiPriority = pParms->uiPriority;
  else
    pSoundList[uiChannel].uiPriority = PRIORITY_MAX;

  if ((pParms != NULL) && ((UINT32)pParms->EOSCallback != SOUND_PARMS_DEFAULT)) {
    pSoundList[uiChannel].EOSCallback = pParms->EOSCallback;
    pSoundList[uiChannel].pCallbackData = pParms->pCallbackData;
  } else {
    pSoundList[uiChannel].EOSCallback = NULL;
    pSoundList[uiChannel].pCallbackData = NULL;
  }

  uiSoundID = SoundGetUniqueID();
  pSoundList[uiChannel].uiSoundID = uiSoundID;
  pSoundList[uiChannel].uiSample = uiSample;
  pSoundList[uiChannel].uiTimeStamp = Plat_GetTickCount();
  pSoundList[uiChannel].uiFadeVolume = SoundGetVolumeIndex(uiChannel);

  pSampleList[uiSample].uiCacheHits++;

  AIL_start_sample(pSoundList[uiChannel].hMSS);

  return (uiSoundID);
}

//*******************************************************************************
// SoundStartStream
//
//		Starts up a stream on the specified channel. Override parameters are passed
//	in through the structure pointer pParms. Any entry with a value of 0xffffffff
//	will be filled in by the system.
//
//	Returns:	Unique sound ID if successful, SOUND_ERROR if not.
//
//*******************************************************************************
UINT32 SoundStartStream(STR pFilename, UINT32 uiChannel, SOUNDPARMS *pParms) {
  UINT32 uiSoundID, uiSpeed;
  CHAR8 AILString[200];

  if (!fSoundSystemInit) return (SOUND_ERROR);

  if ((pSoundList[uiChannel].hMSSStream =
           AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM)) == NULL) {
    SoundCleanCache();
    pSoundList[uiChannel].hMSSStream =
        AIL_open_stream(hSoundDriver, pFilename, SOUND_DEFAULT_STREAM);
  }

  if (pSoundList[uiChannel].hMSSStream == NULL) {
    sprintf(AILString, "Stream Error: %s", AIL_last_error());
    DbgMessage(TOPIC_GAME, DBG_LEVEL_0, AILString);
    return (SOUND_ERROR);
  }

  if ((pParms != NULL) && (pParms->uiSpeed != SOUND_PARMS_DEFAULT))
    uiSpeed = pParms->uiSpeed;
  else
    uiSpeed = AIL_stream_playback_rate(pSoundList[uiChannel].hMSSStream);

  if ((pParms != NULL) && (pParms->uiPitchBend != SOUND_PARMS_DEFAULT)) {
    UINT32 uiBend = uiSpeed * pParms->uiPitchBend / 100;
    uiSpeed += (Random(uiBend * 2) - uiBend);
  }

  AIL_set_stream_playback_rate(pSoundList[uiChannel].hMSSStream, uiSpeed);

  if ((pParms != NULL) && (pParms->uiVolume != SOUND_PARMS_DEFAULT))
    AIL_set_stream_volume(pSoundList[uiChannel].hMSSStream, pParms->uiVolume);
  else
    AIL_set_stream_volume(pSoundList[uiChannel].hMSSStream, guiSoundDefaultVolume);

  if (pParms != NULL) {
    if (pParms->uiLoop != SOUND_PARMS_DEFAULT)
      AIL_set_stream_loop_count(pSoundList[uiChannel].hMSSStream, pParms->uiLoop);
  }

  if ((pParms != NULL) && (pParms->uiPan != SOUND_PARMS_DEFAULT))
    AIL_set_stream_pan(pSoundList[uiChannel].hMSSStream, pParms->uiPan);

  AIL_start_stream(pSoundList[uiChannel].hMSSStream);

  uiSoundID = SoundGetUniqueID();
  pSoundList[uiChannel].uiSoundID = uiSoundID;
  if (pParms)
    pSoundList[uiChannel].uiPriority = pParms->uiPriority;
  else
    pSoundList[uiChannel].uiPriority = SOUND_PARMS_DEFAULT;

  if ((pParms != NULL) && ((UINT32)pParms->EOSCallback != SOUND_PARMS_DEFAULT)) {
    pSoundList[uiChannel].EOSCallback = pParms->EOSCallback;
    pSoundList[uiChannel].pCallbackData = pParms->pCallbackData;
  } else {
    pSoundList[uiChannel].EOSCallback = NULL;
    pSoundList[uiChannel].pCallbackData = NULL;
  }

  pSoundList[uiChannel].uiTimeStamp = Plat_GetTickCount();
  pSoundList[uiChannel].uiFadeVolume = SoundGetVolumeIndex(uiChannel);

  return (uiSoundID);
}

//*******************************************************************************
// SoundGetUniqueID
//
//		Returns a unique ID number with every call. Basically it's just a 32-bit
// static value that is incremented each time.
//
//*******************************************************************************
UINT32 SoundGetUniqueID(void) {
  static UINT32 uiNextID = 0;

  if (uiNextID == SOUND_ERROR) uiNextID++;

  return (uiNextID++);
}

//*******************************************************************************
// SoundPlayStreamed
//
//		Returns TRUE/FALSE whether a sound file should be played as a streamed
//	sample, or loaded into the cache. The decision is based on the size of the
//	file compared to the guiSoundCacheThreshold.
//
//	Returns:	TRUE if it should be streamed, FALSE if loaded.
//
//*******************************************************************************
BOOLEAN SoundPlayStreamed(STR pFilename) {
  FileID hDisk;
  UINT32 uiFilesize;

  if ((hDisk = File_OpenForReading(pFilename)) != 0) {
    uiFilesize = File_GetSize(hDisk);
    File_Close(hDisk);
    return (uiFilesize >= guiSoundCacheThreshold);
  }

  return (FALSE);
}

//*******************************************************************************
// SoundStopIndex
//
//		Stops a sound referred to by it's slot number. This function is the only
//	one that should be deallocating sample handles. The random sounds have to have
//	their counters maintained, and using this as the central function ensures
//	that they stay in sync.
//
//	Returns:	TRUE if the sample was stopped, FALSE if it could not be found.
//
//*******************************************************************************
BOOLEAN SoundStopIndex(UINT32 uiChannel) {
  UINT32 uiSample;

  if (fSoundSystemInit) {
    if (uiChannel != NO_SAMPLE) {
      if (pSoundList[uiChannel].hMSS != NULL) {
        AIL_stop_sample(pSoundList[uiChannel].hMSS);
        AIL_release_sample_handle(pSoundList[uiChannel].hMSS);
        pSoundList[uiChannel].hMSS = NULL;
        uiSample = pSoundList[uiChannel].uiSample;

        // if this was a random sample, decrease the iteration count
        if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM) pSampleList[uiSample].uiInstances--;

        if (pSoundList[uiChannel].EOSCallback != NULL)
          pSoundList[uiChannel].EOSCallback(pSoundList[uiChannel].pCallbackData);

        if (pSoundList[uiChannel].fLooping && !SoundSampleIsInUse(uiChannel))
          SoundRemoveSampleFlags(uiSample, SAMPLE_LOCKED);

        pSoundList[uiChannel].uiSample = NO_SAMPLE;
      }

      if (pSoundList[uiChannel].hMSSStream != NULL) {
        AIL_close_stream(pSoundList[uiChannel].hMSSStream);
        pSoundList[uiChannel].hMSSStream = NULL;
        if (pSoundList[uiChannel].EOSCallback != NULL)
          pSoundList[uiChannel].EOSCallback(pSoundList[uiChannel].pCallbackData);

        pSoundList[uiChannel].uiSample = NO_SAMPLE;
      }

      if (pSoundList[uiChannel].hM3D != NULL) {
        AIL_stop_3D_sample(pSoundList[uiChannel].hM3D);
        AIL_release_3D_sample_handle(pSoundList[uiChannel].hM3D);
        pSoundList[uiChannel].hM3D = NULL;
        uiSample = pSoundList[uiChannel].uiSample;

        // if this was a random sample, decrease the iteration count
        if (pSampleList[uiSample].uiFlags & SAMPLE_RANDOM) pSampleList[uiSample].uiInstances--;

        if (pSoundList[uiChannel].EOSCallback != NULL)
          pSoundList[uiChannel].EOSCallback(pSoundList[uiChannel].pCallbackData);

        if (pSoundList[uiChannel].fLooping && !SoundSampleIsInUse(uiChannel))
          SoundRemoveSampleFlags(uiSample, SAMPLE_LOCKED);

        pSoundList[uiChannel].uiSample = NO_SAMPLE;
      }

      if (pSoundList[uiChannel].hFile != 0) {
        File_Close(pSoundList[uiChannel].hFile);
        pSoundList[uiChannel].hFile = 0;

        pSoundList[uiChannel].uiSample = NO_SAMPLE;
      }

      return (TRUE);
    }
  }

  return (FALSE);
}

//*******************************************************************************
// SoundGetDriverHandle
//
//	Returns:	Pointer to the current sound driver
//
//*******************************************************************************
void *SoundGetDriverHandle(void) { return (hSoundDriver); }

void SoundRemoveSampleFlags(UINT32 uiSample, UINT32 uiFlags) {
  // CHECK FOR VALID SAMPLE
  if ((pSampleList[uiSample].uiFlags & SAMPLE_ALLOCATED)) {
    // REMOVE
    pSampleList[uiSample].uiFlags &= (~uiFlags);
  }
}

//*******************************************************************************
// SoundSampleIsInUse
//
//	Returns:	TRUE if the sample index is currently being played by the system.
//
//*******************************************************************************
BOOLEAN SoundSampleIsInUse(UINT32 uiSample) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
    if ((pSoundList[uiCount].uiSample == uiSample) && SoundIsPlaying(uiCount)) return (TRUE);
  }

  return (FALSE);
}

//*****************************************************************************************
// SoundStopMusic
//
// Stops any sound instance with the music flag.
//
// Returns nothing.
//
// Created:  3/16/00 Derek Beland
//*****************************************************************************************
BOOLEAN SoundStopMusic(void) {
  UINT32 uiCount;
  BOOLEAN fStopped = FALSE;

  if (fSoundSystemInit) {
    for (uiCount = 0; uiCount < SOUND_MAX_CHANNELS; uiCount++) {
      if ((pSoundList[uiCount].hMSS != NULL) || (pSoundList[uiCount].hMSSStream != NULL) ||
          (pSoundList[uiCount].hM3D != NULL)) {
        if (pSoundList[uiCount].fMusic) {
          SoundStop(pSoundList[uiCount].uiSoundID);
          fStopped = TRUE;
        }
      }
    }
  }

  return (fStopped);
}

//*****************************************************************************************
//
//
//
//
// New 3D Sound Code
//
//
//
//
//*****************************************************************************************

//*****************************************************************************************
// Sound3DInitProvider
//
// Attempt
//
// Returns BOOLEAN            -
//
// CHAR8 *pProviderName       -
//
// Created:  8/17/99 Derek Beland
//*****************************************************************************************
BOOLEAN Sound3DInitProvider(CHAR8 *pProviderName) {
  HPROENUM hEnum = HPROENUM_FIRST;
  HPROVIDER hProvider = 0;
  BOOLEAN fDone = FALSE;
  CHAR8 *pName;
  INT32 iResult;

  // 3D sound providers depend on the 2D sound system being initialized first
  if (!fSoundSystemInit || !pProviderName) return (FALSE);

  // We're already booted up
  if (gh3DProvider) return (TRUE);

  while (!fDone) {
    if (!AIL_enumerate_3D_providers(&hEnum, &hProvider, &pName))
      fDone = TRUE;
    else if (hProvider) {
      if (strcmp(pProviderName, pName) == 0) {
        fDone = TRUE;
        if (AIL_open_3D_provider(hProvider) == M3D_NOERR) {
          gh3DProvider = hProvider;

          // Create a "listener" which represents our position in space
          gh3DListener = AIL_open_3D_listener(gh3DProvider);
          if (!gh3DListener) {
            AIL_close_3D_provider(gh3DProvider);
            return (FALSE);
          }
          Sound3DSetListener(0.0f, 0.0f, 0.0f);

          AIL_3D_provider_attribute(gh3DProvider, "EAX environment selection", &iResult);
          if (iResult != (-1)) gfUsingEAX = TRUE;

          return (TRUE);
        }
      }
    }
  }

  // We didn't find a provider with a matching name that would boot up
  return (FALSE);
}

//*****************************************************************************************
// Sound3DShutdownProvider
//
// Shuts down and deallocates the 3D sound system
//
// Returns nothing.
//
// Created:  8/17/99 Derek Beland
//*****************************************************************************************
void Sound3DShutdownProvider(void) {
  Sound3DStopAll();

  if (gh3DListener) {
    AIL_close_3D_listener(gh3DListener);
    gh3DListener = 0;
  }

  if (gh3DProvider) {
    AIL_close_3D_provider(gh3DProvider);
    gh3DProvider = 0;
  }
}

//*****************************************************************************************
// Sound3DSetListener
//
// Sets the listener location. This should be set to the current camera location.
//
// Returns nothing.
//
// FLOAT flX                  - X coordinate
// FLOAT flY                  - Y coordinate
// FLOAT flZ                  - Z coordinate
//
// Created:  8/17/99 Derek Beland
//*****************************************************************************************
void Sound3DSetListener(FLOAT flX, FLOAT flY, FLOAT flZ) {
  if (fSoundSystemInit && gh3DListener) AIL_set_3D_position(gh3DListener, flX, flY, flZ);
}

//*****************************************************************************************
// Sound3DActiveSounds
//
// Returns the number of active sounds.
//
// Returns INT32              - Number of 3D sounds playing
//
// Created:  8/17/99 Derek Beland
//*****************************************************************************************
INT32 Sound3DActiveSounds(void) { return ((INT32)AIL_active_3D_sample_count(gh3DProvider)); }

//*****************************************************************************************
// Sound3DStopAll
//
// Stops all currently playing 3D samples.
//
// Returns nothing.
//
// Created:  8/17/99 Derek Beland
//*****************************************************************************************
void Sound3DStopAll(void) {
  UINT32 uiChannel;

  // Stop all currently playing random sounds
  for (uiChannel = 0; uiChannel < SOUND_MAX_CHANNELS; uiChannel++) {
    if (pSoundList[uiChannel].hM3D != NULL) SoundStopIndex(uiChannel);
  }
}
