#ifndef __GLOBALS_H
#define __GLOBALS_H

#include "SGP/Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN gfProgramIsRunning;  // Turn this to FALSE to exit program
extern CHAR8 gzCommandLine[100];    // Command line given
extern UINT8 gbPixelDepth;          // GLOBAL RUN-TIME SETTINGS

#ifdef __cplusplus
}
#endif

#endif
