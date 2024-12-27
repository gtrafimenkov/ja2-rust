#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#include "SGP/Types.h"

#define SHOW_MIN_FPS 0
#define SHOW_FULL_FPS 1

extern char gubErrorText[200];
extern BOOLEAN gfAniEditMode;
extern BOOLEAN gfEditMode;
extern BOOLEAN fFirstTimeInGameScreen;
extern BOOLEAN fDirtyRectangleMode;
extern char gDebugStr[128];
extern char gSystemDebugStr[128];

extern uint32_t guiDoneButton;

extern BOOLEAN gfMode;
extern int16_t gsCurrentActionPoints;
extern int8_t gbFPSDisplay;
extern BOOLEAN gfResetInputCheck;
extern BOOLEAN gfGlobalError;

extern uint32_t guiGameCycleCounter;

// VIDEO OVERLAYS
extern int32_t giFPSOverlay;
extern int32_t giCounterPeriodOverlay;

extern BOOLEAN SET_ERROR(const char *String, ...);

#endif
