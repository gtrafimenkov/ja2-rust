#ifndef __EXIT_GRIDS_H
#define __EXIT_GRIDS_H

#include "SGP/Types.h"

struct LEVELNODE;
struct SOLDIERTYPE;

typedef struct  // for exit grids (object level)
{               // if an item pool is also in same gridno, then this would be a separate levelnode
  // in the object level list
  uint16_t usGridNo;  // sweet spot for placing mercs in new sector.
  uint8_t ubGotoSectorX;
  uint8_t ubGotoSectorY;
  uint8_t ubGotoSectorZ;
} EXITGRID;

BOOLEAN ExitGridAtGridNo(uint16_t usMapIndex);
BOOLEAN GetExitGridLevelNode(uint16_t usMapIndex, struct LEVELNODE **ppLevelNode);
BOOLEAN GetExitGrid(uint16_t usMapIndex, EXITGRID *pExitGrid);

void AddExitGridToWorld(int32_t iMapIndex, EXITGRID *pExitGrid);
void RemoveExitGridFromWorld(int32_t iMapIndex);

void SaveExitGrids(FileID fp, uint16_t usNumExitGrids);
void LoadExitGrids(int8_t **hBuffer);

void AttemptToChangeFloorLevel(int8_t bRelativeZLevel);

extern EXITGRID gExitGrid;
extern BOOLEAN gfOverrideInsertionWithExitGrid;

// Finds closest ExitGrid of same type as is at gridno, within a radius.  Checks
// valid paths, destinations, etc.
uint16_t FindGridNoFromSweetSpotCloseToExitGrid(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo,
                                                int8_t ubRadius, uint8_t *pubDirection);

uint16_t FindClosestExitGrid(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t ubRadius);

#endif
