#ifndef STRUCURE_WRAP_H
#define STRUCURE_WRAP_H

#include "SGP/Types.h"

struct LEVELNODE;
struct STRUCTURE;

BOOLEAN IsTreePresentAtGridno(int16_t sGridNo);
BOOLEAN IsFencePresentAtGridno(int16_t sGridNo);
BOOLEAN IsJumpableFencePresentAtGridno(int16_t sGridNo);
BOOLEAN IsDoorPresentAtGridno(int16_t sGridNo);

// OK, THIS FUNCTION RETURNS A struct LEVELNODE POINTER TO A WALL OF THE SAME ORIENTATION
// AS WAS GIVEN. RETURNS NULL IF NONE FOUND.
struct LEVELNODE *GetWallLevelNodeOfSameOrientationAtGridno(int16_t sGridNo, int8_t ubOrientation);

struct LEVELNODE *IsWallPresentAtGridno(int16_t sGridNo);

BOOLEAN IsDoorVisibleAtGridNo(int16_t sGridNo);

BOOLEAN IsHiddenStructureVisible(int16_t sGridNo, uint16_t usIndex);

BOOLEAN DoesGridnoContainHiddenStruct(int16_t sGridNo, BOOLEAN *pfVisible);

BOOLEAN WallExistsOfTopLeftOrientation(int16_t sGridNo);

BOOLEAN WallExistsOfTopRightOrientation(int16_t sGridNo);

BOOLEAN WallOrClosedDoorExistsOfTopLeftOrientation(int16_t sGridNo);

BOOLEAN WallOrClosedDoorExistsOfTopRightOrientation(int16_t sGridNo);

BOOLEAN OpenRightOrientedDoorWithDoorOnRightOfEdgeExists(int16_t sGridNo);
BOOLEAN OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists(int16_t sGridNo);

struct LEVELNODE *GetWallLevelNodeAndStructOfSameOrientationAtGridno(
    int16_t sGridNo, int8_t ubOrientation, struct STRUCTURE **ppStructure);

BOOLEAN CutWireFence(int16_t sGridNo);
BOOLEAN IsCuttableWireFenceAtGridNo(int16_t sGridNo);
BOOLEAN IsCutWireFenceAtGridNo(int16_t sGridNo);

BOOLEAN IsRepairableStructAtGridNo(int16_t sGridNo, uint8_t *pubID);
BOOLEAN IsRefuelableStructAtGridNo(int16_t sGridNo, uint8_t *pubID);

BOOLEAN IsRoofPresentAtGridno(int16_t sGridNo);

int16_t FindDoorAtGridNoOrAdjacent(int16_t sGridNo);

BOOLEAN IsCorpseAtGridNo(int16_t sGridNo, uint8_t ubLevel);

BOOLEAN SetOpenableStructureToClosed(int16_t sGridNo, uint8_t ubLevel);

#endif
