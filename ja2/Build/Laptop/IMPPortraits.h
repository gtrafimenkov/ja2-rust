#ifndef __IMP_PORTRAIT_H
#define __IMP_PORTRAIT_H

#include "SGP/Types.h"

void EnterIMPPortraits(void);
void RenderIMPPortraits(void);
void ExitIMPPortraits(void);
void HandleIMPPortraits(void);
BOOLEAN RenderPortrait(INT16 sX, INT16 sY);

extern INT32 iPortraitNumber;

#endif
