// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __TIMER_
#define __TIMER_

#include "SGP/Types.h"

BOOLEAN InitializeClockManager(void);
void ShutdownClockManager(void);
uint32_t GetClock(void);

#endif
