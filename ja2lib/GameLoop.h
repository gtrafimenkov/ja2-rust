// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __GAMELOOP_
#define __GAMELOOP_

#include "SGP/Types.h"

// main game loop systems
#define INIT_SYSTEM 0

#define NO_PENDING_SCREEN 0xFFFF

extern BOOLEAN InitializeGame(void);
extern void ShutdownGame(void);
extern void GameLoop(void);

// handle exit from game due to shortcut key
void HandleShortCutExitState(void);

void SetPendingNewScreen(uint32_t uiNewScreen);

#endif
