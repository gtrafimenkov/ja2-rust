#ifndef __EDIT_SYS_H
#define __EDIT_SYS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

#define SMALLBRUSH 0
#define MEDIUMBRUSH 1
#define LARGEBRUSH 2

#define NO_BANKS 0
#define DRAW_BANKS 1
#define DRAW_BANK_WATER 2
#define DRAW_ERASE 3

#define NO_CLIFFS 0
#define DRAW_CLIFFS 1
#define DRAW_CLIFF_LAND 2

extern BOOLEAN gfWarning;

extern BOOLEAN gfDoFill;
extern UINT16 CurrentPaste;
extern UINT16 gDebrisPaste;
extern UINT16 gChangeElevation;
extern UINT16 CurrentStruct;
extern uint32_t gDoBanks;
extern uint32_t gDoCliffs;

void EraseMapTile(uint32_t iMapIndex);
void QuickEraseMapTile(uint32_t iMapIndex);
void DeleteStuffFromMapTile(uint32_t iMapIndex);

void PasteDebris(uint32_t iMapIndex);

void PasteStructure(uint32_t iMapIndex);
void PasteStructure1(uint32_t iMapIndex);
void PasteStructure2(uint32_t iMapIndex);
void PasteStructureCommon(uint32_t iMapIndex);

void PasteSingleWall(uint32_t iMapIndex);
void PasteSingleDoor(uint32_t iMapIndex);
void PasteSingleWindow(uint32_t iMapIndex);
void PasteSingleRoof(uint32_t iMapIndex);
void PasteSingleBrokenWall(uint32_t iMapIndex);
void PasteSingleDecoration(uint32_t iMapIndex);
void PasteSingleDecal(uint32_t iMapIndex);
void PasteSingleFloor(uint32_t iMapIndex);
void PasteSingleToilet(uint32_t iMapIndex);
void PasteRoomNumber(uint32_t iMapIndex, UINT8 ubRoomNumber);

void PasteSingleWallCommon(uint32_t iMapIndex);

UINT16 GetRandomIndexByRange(UINT16 usRangeStart, UINT16 usRangeEnd);
UINT16 GetRandomTypeByRange(UINT16 usRangeStart, UINT16 usRangeEnd);

void PasteFloor(uint32_t iMapIndex, UINT16 usFloorIndex, BOOLEAN fReplace);

void PasteBanks(uint32_t iMapIndex, UINT16 usStructIndex, BOOLEAN fReplace);
void PasteRoads(uint32_t iMapIndex);
void PasteCliffs(uint32_t iMapIndex, UINT16 usStructIndex, BOOLEAN fReplace);

void PasteTexture(uint32_t iMapIndex);
void PasteTextureCommon(uint32_t iMapIndex);

void PasteHigherTexture(uint32_t iMapIndex, uint32_t fNewType);

void RaiseWorldLand();

void EliminateObjectLayerRedundancy();

#endif
