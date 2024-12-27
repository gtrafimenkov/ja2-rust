// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SAVE_LOAD_SCREEN__H_
#define _SAVE_LOAD_SCREEN__H_

#include "MessageBoxScreen.h"
#include "SGP/Types.h"

#define NUM_SAVE_GAMES 11

// This flag is used to diferentiate between loading a game and saveing a game.
// gfSaveGame=TRUE		For saving a game
// gfSaveGame=FALSE		For loading a game
extern BOOLEAN gfSaveGame;

// if there is to be a slot selected when entering this screen
extern int8_t gbSetSlotToBeSelected;

extern BOOLEAN gbSaveGameArray[NUM_SAVE_GAMES];

extern BOOLEAN gfCameDirectlyFromGame;

uint32_t SaveLoadScreenShutdown(void);
uint32_t SaveLoadScreenHandle(void);
uint32_t SaveLoadScreenInit(void);

BOOLEAN DoSaveLoadMessageBox(uint8_t ubStyle, wchar_t* zString, uint32_t uiExitScreen, uint16_t usFlags,
                             MSGBOX_CALLBACK ReturnCallback);

BOOLEAN InitSaveGameArray();

void DoneFadeOutForSaveLoadScreen(void);
void DoneFadeInForSaveLoadScreen(void);

BOOLEAN DoQuickSave();
BOOLEAN DoQuickLoad();

BOOLEAN IsThereAnySavedGameFiles();

void DeleteSaveGameNumber(uint8_t ubSaveGameSlotID);

#endif
