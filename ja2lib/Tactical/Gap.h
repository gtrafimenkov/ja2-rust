// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __GAP_H
#define __GAP_H

#include "SGP/Types.h"
#include "Tactical/Faces.h"

typedef uint8_t AudioSample8;
typedef int16_t AudioSample16;

void AudioGapListInit(char *zSoundFile, AudioGapList *pGapList);
void AudioGapListDone(AudioGapList *pGapList);

void PollAudioGap(uint32_t uiSampleNum, AudioGapList *pGapList);

uint32_t PlayJA2GapSample(char *zSoundFile, uint32_t usRate, uint32_t ubVolume, uint32_t ubLoops,
                          uint32_t uiPan, AudioGapList *pData);

#endif
