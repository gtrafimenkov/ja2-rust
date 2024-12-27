// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __IMP_PORTRAIT_H
#define __IMP_PORTRAIT_H

#include "SGP/Types.h"

void EnterIMPPortraits(void);
void RenderIMPPortraits(void);
void ExitIMPPortraits(void);
void HandleIMPPortraits(void);
BOOLEAN RenderPortrait(int16_t sX, int16_t sY);

extern int32_t iPortraitNumber;

#endif
