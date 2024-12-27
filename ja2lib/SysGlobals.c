#include "SysGlobals.h"

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

#include "GameLoop.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"

// External globals
char gubErrorText[200];
char gubFilename[200];
BOOLEAN gfEditMode = FALSE;
char gDebugStr[128];
char gSystemDebugStr[128];
int8_t gbFPSDisplay = SHOW_MIN_FPS;
BOOLEAN gfResetInputCheck = FALSE;
BOOLEAN gfGlobalError = FALSE;
BOOLEAN fFirstTimeInGameScreen = TRUE;
uint32_t guiGameCycleCounter = 0;
int16_t gsCurrentActionPoints = 1;

BOOLEAN SET_ERROR(const char *String, ...) {
  va_list ArgPtr;

  va_start(ArgPtr, String);
  vsprintf(gubErrorText, String, ArgPtr);
  va_end(ArgPtr);

  SetPendingNewScreen(ERROR_SCREEN);

  gfGlobalError = TRUE;

  return (FALSE);
}
