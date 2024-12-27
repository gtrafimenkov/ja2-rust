// Platform-independent interface to the sound system.

#ifndef __SOUNDMAN_H
#define __SOUNDMAN_H

#include "SGP/Types.h"

// Sample status flags
#define SAMPLE_RANDOM 0x00000004

// Sound error values (they're all the same)
#define NO_SAMPLE 0xffffffff
#define SOUND_ERROR 0xffffffff

// Maximum allowable priority value
#define PRIORITY_MAX 0xfffffffe
#define PRIORITY_RANDOM PRIORITY_MAX - 1

// Structure definition for sound parameters being passed down to
//		the sample playing function
typedef struct {
  uint32_t uiSpeed;
  uint32_t uiPitchBend;  // Random pitch bend range +/-
  uint32_t uiVolume;
  uint32_t uiPan;
  uint32_t uiLoop;
  uint32_t uiPriority;
  void (*EOSCallback)(void*);
  void* pCallbackData;
} SOUNDPARMS;

// Structure definition for parameters to the random sample playing
//		function
typedef struct {
  uint32_t uiTimeMin, uiTimeMax;
  uint32_t uiSpeedMin, uiSpeedMax;
  uint32_t uiVolMin, uiVolMax;
  uint32_t uiPanMin, uiPanMax;
  uint32_t uiPriority;
  uint32_t uiMaxInstances;
} RANDOMPARMS;

// Global startup/shutdown functions
extern BOOLEAN InitializeSoundManager(void);
extern void ShutdownSoundManager(void);

// Configuration functions
extern void* SoundGetDriverHandle(void);

// Cache control functions
extern uint32_t SoundLoadSample(char* pFilename);
extern uint32_t SoundFreeSample(char* pFilename);
extern uint32_t SoundLockSample(char* pFilename);
extern uint32_t SoundUnlockSample(char* pFilename);

// Play/service sample functions
extern uint32_t SoundPlay(char* pFilename, SOUNDPARMS* pParms);
extern uint32_t SoundPlayStreamedFile(char* pFilename, SOUNDPARMS* pParms);

extern uint32_t SoundPlayRandom(char* pFilename, RANDOMPARMS* pParms);
extern BOOLEAN SoundServiceStreams(void);

// Sound instance manipulation functions
extern BOOLEAN SoundStopAll(void);
extern BOOLEAN SoundStopAllRandom(void);
extern BOOLEAN SoundStop(uint32_t uiSoundID);
extern BOOLEAN SoundIsPlaying(uint32_t uiSoundID);
extern BOOLEAN SoundSetVolume(uint32_t uiSoundID, uint32_t uiVolume);
extern BOOLEAN SoundSetPan(uint32_t uiSoundID, uint32_t uiPan);
extern uint32_t SoundGetVolume(uint32_t uiSoundID);
extern uint32_t SoundGetPosition(uint32_t uiSoundID);

extern void SoundRemoveSampleFlags(uint32_t uiSample, uint32_t uiFlags);

extern void SoundEnableSound(BOOLEAN fEnable);

extern BOOLEAN SoundServiceRandom(void);

#endif
