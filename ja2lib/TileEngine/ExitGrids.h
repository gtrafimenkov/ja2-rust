#ifndef __EXIT_GRIDS_H
#define __EXIT_GRIDS_H

#include "SGP/Types.h"

struct LEVELNODE;
struct SOLDIERTYPE;

typedef struct  // for exit grids (object level)
{               // if an item pool is also in same gridno, then this would be a separate levelnode
  // in the object level list
  uint16_t usGridNo;  // sweet spot for placing mercs in new sector.
  UINT8 ubGotoSectorX;
  UINT8 ubGotoSectorY;
  UINT8 ubGotoSectorZ;
} EXITGRID;

BOOLEAN ExitGridAtGridNo(uint16_t usMapIndex);
BOOLEAN GetExitGridLevelNode(uint16_t usMapIndex, struct LEVELNODE **ppLevelNode);
BOOLEAN GetExitGrid(uint16_t usMapIndex, EXITGRID *pExitGrid);

void AddExitGridToWorld(INT32 iMapIndex, EXITGRID *pExitGrid);
void RemoveExitGridFromWorld(INT32 iMapIndex);

void SaveExitGrids(HWFILE fp, uint16_t usNumExitGrids);
void LoadExitGrids(INT8 **hBuffer);

void AttemptToChangeFloorLevel(INT8 bRelativeZLevel);

extern EXITGRID gExitGrid;
extern BOOLEAN gfOverrideInsertionWithExitGrid;

// Finds closest ExitGrid of same type as is at gridno, within a radius.  Checks
// valid paths, destinations, etc.
uint16_t FindGridNoFromSweetSpotCloseToExitGrid(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                                INT8 ubRadius, UINT8 *pubDirection);

uint16_t FindClosestExitGrid(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 ubRadius);

#endif
