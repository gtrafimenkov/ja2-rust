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
extern UINT32 gDoBanks;
extern UINT32 gDoCliffs;

void EraseMapTile(UINT32 iMapIndex);
void QuickEraseMapTile(UINT32 iMapIndex);
void DeleteStuffFromMapTile(UINT32 iMapIndex);

void PasteDebris(UINT32 iMapIndex);

void PasteStructure(UINT32 iMapIndex);
void PasteStructure1(UINT32 iMapIndex);
void PasteStructure2(UINT32 iMapIndex);
void PasteStructureCommon(UINT32 iMapIndex);

void PasteSingleWall(UINT32 iMapIndex);
void PasteSingleDoor(UINT32 iMapIndex);
void PasteSingleWindow(UINT32 iMapIndex);
void PasteSingleRoof(UINT32 iMapIndex);
void PasteSingleBrokenWall(UINT32 iMapIndex);
void PasteSingleDecoration(UINT32 iMapIndex);
void PasteSingleDecal(UINT32 iMapIndex);
void PasteSingleFloor(UINT32 iMapIndex);
void PasteSingleToilet(UINT32 iMapIndex);
void PasteRoomNumber(UINT32 iMapIndex, UINT8 ubRoomNumber);

void PasteSingleWallCommon(UINT32 iMapIndex);

UINT16 GetRandomIndexByRange(UINT16 usRangeStart, UINT16 usRangeEnd);
UINT16 GetRandomTypeByRange(UINT16 usRangeStart, UINT16 usRangeEnd);

void PasteFloor(UINT32 iMapIndex, UINT16 usFloorIndex, BOOLEAN fReplace);

void PasteBanks(UINT32 iMapIndex, UINT16 usStructIndex, BOOLEAN fReplace);
void PasteRoads(UINT32 iMapIndex);
void PasteCliffs(UINT32 iMapIndex, UINT16 usStructIndex, BOOLEAN fReplace);

void PasteTexture(UINT32 iMapIndex);
void PasteTextureCommon(UINT32 iMapIndex);

void PasteHigherTexture(UINT32 iMapIndex, UINT32 fNewType);

void RaiseWorldLand();

void EliminateObjectLayerRedundancy();

#endif
