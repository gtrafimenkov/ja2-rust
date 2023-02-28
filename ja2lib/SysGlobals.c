#include "SysGlobals.h"

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#include "GameLoop.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"

// External globals
CHAR8 gubErrorText[200];
CHAR8 gubFilename[200];
BOOLEAN gfEditMode = FALSE;
CHAR8 gDebugStr[128];
CHAR8 gSystemDebugStr[128];
INT8 gbFPSDisplay = SHOW_MIN_FPS;
BOOLEAN gfResetInputCheck = FALSE;
BOOLEAN gfGlobalError = FALSE;
BOOLEAN fFirstTimeInGameScreen = TRUE;
UINT32 guiGameCycleCounter = 0;
INT16 gsCurrentActionPoints = 1;

BOOLEAN SET_ERROR(const char *String, ...) {
  va_list ArgPtr;

  va_start(ArgPtr, String);
  vsprintf(gubErrorText, String, ArgPtr);
  va_end(ArgPtr);

  SetPendingNewScreen(ERROR_SCREEN);

  gfGlobalError = TRUE;

  return (FALSE);
}
