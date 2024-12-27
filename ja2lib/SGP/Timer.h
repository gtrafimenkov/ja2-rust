#ifndef __TIMER_
#define __TIMER_

#include "SGP/Types.h"

BOOLEAN InitializeClockManager(void);
void ShutdownClockManager(void);
uint32_t GetClock(void);

#endif
