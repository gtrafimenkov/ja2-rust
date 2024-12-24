#ifndef __IMP_HOME_H
#define __IMP_HOME_H

#include "SGP/Types.h"

void EnterImpHomePage(void);
void RenderImpHomePage(void);
void ExitImpHomePage(void);
void HandleImpHomePage(void);

// minimun glow time
#define MIN_GLOW_DELTA 100
#define CURSOR_HEIGHT GetFontHeight(FONT14ARIAL) + 6

extern int32_t GlowColorsList[][3];
#endif
