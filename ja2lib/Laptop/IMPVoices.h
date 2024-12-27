// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __IMP_VOICES_H
#define __IMP_VOICES_H

#include "SGP/Types.h"

void EnterIMPVoices(void);
void RenderIMPVoices(void);
void ExitIMPVoices(void);
void HandleIMPVoices(void);
uint32_t PlayVoice(void);

extern int32_t iVoiceId;
#endif
