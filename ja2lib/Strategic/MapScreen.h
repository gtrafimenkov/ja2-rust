#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "SGP/Types.h"

struct SOLDIERTYPE;
struct path;

extern BOOLEAN fCharacterInfoPanelDirty;
extern BOOLEAN fTeamPanelDirty;
extern BOOLEAN fMapPanelDirty;

extern BOOLEAN fMapInventoryItem;
extern BOOLEAN gfInConfirmMapMoveMode;
extern BOOLEAN gfInChangeArrivalSectorMode;

extern BOOLEAN gfSkyriderEmptyHelpGiven;

BOOLEAN SetInfoChar(uint8_t ubSolId);
void EndMapScreen(BOOLEAN fDuringFade);
void ReBuildCharactersList(void);

BOOLEAN HandlePreloadOfMapGraphics(void);
void HandleRemovalOfPreLoadedMapGraphics(void);

void ChangeSelectedMapSector(INT16 sMapX, INT16 sMapY, int8_t bMapZ);

BOOLEAN CanToggleSelectedCharInventory(void);

BOOLEAN CanExtendContractForCharSlot(int8_t bCharNumber);

void TellPlayerWhyHeCantCompressTime(void);

void ChangeSelectedInfoChar(int8_t bCharNumber, BOOLEAN fResetSelectedList);

void MAPEndItemPointer();

void CopyPathToAllSelectedCharacters(struct path* pPath);
void CancelPathsOfAllSelectedCharacters();

INT32 GetPathTravelTimeDuringPlotting(struct path* pPath);

void AbortMovementPlottingMode(void);

void ExplainWhySkyriderCantFly(void);

BOOLEAN CanChangeSleepStatusForCharSlot(int8_t bCharNumber);
BOOLEAN CanChangeSleepStatusForSoldier(struct SOLDIERTYPE* pSoldier);

BOOLEAN MapCharacterHasAccessibleInventory(int8_t bCharNumber);

#endif
