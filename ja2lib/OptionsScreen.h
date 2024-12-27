// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _OPTIONS_SCREEN__H_
#define _OPTIONS_SCREEN__H_

#include "MessageBoxScreen.h"
#include "SGP/Types.h"

#define OPT_BUTTON_FONT FONT14ARIAL
#define OPT_BUTTON_ON_COLOR 73   // FONT_MCOLOR_WHITE
#define OPT_BUTTON_OFF_COLOR 73  // FONT_MCOLOR_WHITE

// Record the previous screen the user was in.
extern uint32_t guiPreviousOptionScreen;

uint32_t OptionsScreenShutdown(void);
uint32_t OptionsScreenHandle(void);
uint32_t OptionsScreenInit(void);

void SetOptionsScreenToggleBoxes();
void GetOptionsScreenToggleBoxes();

BOOLEAN DoOptionsMessageBox(uint8_t ubStyle, wchar_t* zString, uint32_t uiExitScreen,
                            uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback);

#endif
