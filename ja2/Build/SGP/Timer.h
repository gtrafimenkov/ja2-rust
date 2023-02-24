#ifndef __TIMER_
#define __TIMER_

#include "SGP/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN InitializeClockManager(void);
void ShutdownClockManager(void);
UINT32 GetClock(void);

#ifdef __cplusplus
}
#endif

#endif
