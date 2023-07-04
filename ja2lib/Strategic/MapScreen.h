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

BOOLEAN SetInfoChar(UINT8 ubSolId);
void EndMapScreen(BOOLEAN fDuringFade);
void ReBuildCharactersList(void);

BOOLEAN PreloadMapScreenGraphics(void);
void UnloadMapScreenGraphics(void);

void ChangeSelectedMapSector(u8 sMapX, u8 sMapY, INT8 bMapZ);

BOOLEAN CanToggleSelectedCharInventory(void);

BOOLEAN CanExtendContractForCharSlot(INT8 bCharNumber);

void TellPlayerWhyHeCantCompressTime(void);

void ChangeSelectedInfoChar(INT8 bCharNumber, BOOLEAN fResetSelectedList);

void MAPEndItemPointer();

void CopyPathToAllSelectedCharacters(struct path* pPath);
void CancelPathsOfAllSelectedCharacters();

INT32 GetPathTravelTimeDuringPlotting(struct path* pPath);

void AbortMovementPlottingMode(void);

void ExplainWhySkyriderCantFly(void);

BOOLEAN CanChangeSleepStatusForCharSlot(INT8 bCharNumber);
BOOLEAN CanChangeSleepStatusForSoldier(struct SOLDIERTYPE* pSoldier);

BOOLEAN MapCharacterHasAccessibleInventory(INT8 bCharNumber);

BOOLEAN GetMouseMapXY(u8* psMapWorldX, u8* psMapWorldY);

#endif
