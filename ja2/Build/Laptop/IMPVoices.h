#ifndef __IMP_VOICES_H
#define __IMP_VOICES_H

#include "SGP/Types.h"

void EnterIMPVoices(void);
void RenderIMPVoices(void);
void ExitIMPVoices(void);
void HandleIMPVoices(void);
UINT32 PlayVoice(void);

extern INT32 iVoiceId;
#endif
