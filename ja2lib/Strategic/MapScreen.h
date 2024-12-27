// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "SGP/Types.h"

struct SOLDIERTYPE;
struct path;

extern BOOLEAN fCharacterInfoPanelDirty;
extern BOOLEAN fTeamPanelDirty;

extern BOOLEAN fMapInventoryItem;
extern BOOLEAN gfInConfirmMapMoveMode;
extern BOOLEAN gfInChangeArrivalSectorMode;

extern BOOLEAN gfSkyriderEmptyHelpGiven;

BOOLEAN SetInfoChar(uint8_t ubSolId);
void EndMapScreen(BOOLEAN fDuringFade);
void ReBuildCharactersList(void);

BOOLEAN PreloadMapScreenGraphics(void);
void UnloadMapScreenGraphics(void);

void ChangeSelectedMapSector(u8 sMapX, u8 sMapY, int8_t bMapZ);

BOOLEAN CanToggleSelectedCharInventory(void);

BOOLEAN CanExtendContractForCharSlot(int8_t bCharNumber);

void TellPlayerWhyHeCantCompressTime(void);

void ChangeSelectedInfoChar(int8_t bCharNumber, BOOLEAN fResetSelectedList);

void MAPEndItemPointer();

void CopyPathToAllSelectedCharacters(struct path* pPath);
void CancelPathsOfAllSelectedCharacters();

int32_t GetPathTravelTimeDuringPlotting(struct path* pPath);

void AbortMovementPlottingMode(void);

void ExplainWhySkyriderCantFly(void);

BOOLEAN CanChangeSleepStatusForCharSlot(int8_t bCharNumber);
BOOLEAN CanChangeSleepStatusForSoldier(struct SOLDIERTYPE* pSoldier);

BOOLEAN MapCharacterHasAccessibleInventory(int8_t bCharNumber);

BOOLEAN GetMouseMapXY(u8* psMapWorldX, u8* psMapWorldY);

#endif
