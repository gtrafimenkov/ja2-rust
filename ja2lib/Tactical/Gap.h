#ifndef __GAP_H
#define __GAP_H

#include "SGP/Types.h"
#include "Tactical/Faces.h"

typedef uint8_t AudioSample8;
typedef INT16 AudioSample16;

void AudioGapListInit(CHAR8 *zSoundFile, AudioGapList *pGapList);
void AudioGapListDone(AudioGapList *pGapList);

void PollAudioGap(uint32_t uiSampleNum, AudioGapList *pGapList);

uint32_t PlayJA2GapSample(CHAR8 *zSoundFile, uint32_t usRate, uint32_t ubVolume, uint32_t ubLoops,
                          uint32_t uiPan, AudioGapList *pData);

#endif
