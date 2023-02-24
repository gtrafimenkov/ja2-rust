#ifndef __SGP_
#define __SGP_

#include "Local.h"
#include "SGP/Debug.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "SysGlobals.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN gfProgramIsRunning;  // Turn this to FALSE to exit program
extern UINT32 giStartMem;
extern CHAR8 gzCommandLine[100];  // Command line given
extern UINT8 gbPixelDepth;        // GLOBAL RUN-TIME SETTINGS

// function prototypes
void SGPExit(void);
void ShutdownWithErrorBox(CHAR8 *pcMessage);

#ifdef __cplusplus
}
#endif

#endif
