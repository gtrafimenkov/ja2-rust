#ifndef __TIMER_
#define __TIMER_

#include "SGP/Types.h"

BOOLEAN InitializeClockManager(void);
void ShutdownClockManager(void);
UINT32 GetClock(void);

#endif
