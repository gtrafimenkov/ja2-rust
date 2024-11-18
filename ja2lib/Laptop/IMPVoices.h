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
