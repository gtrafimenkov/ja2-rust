#ifndef __SGP_H
#define __SGP_H

#include "SGP/Types.h"

struct PlatformInitParams;

extern BOOLEAN InitializeStandardGamingPlatform(struct PlatformInitParams *params);
extern void ShutdownStandardGamingPlatform(void);

#endif
