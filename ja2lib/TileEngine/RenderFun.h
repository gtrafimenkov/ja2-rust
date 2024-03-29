#ifndef __RENDER_FUN_H
#define __RENDER_FUN_H

#include <stdio.h>

#include "SGP/Types.h"
#include "TileEngine/WorldDef.h"
#include "rust_geometry.h"

#define NO_ROOM 0
#define MAX_ROOMS 250

extern UINT8 gubWorldRoomHidden[MAX_ROOMS];
extern UINT8 gubWorldRoomInfo[WORLD_MAX];

BOOLEAN InitRoomDatabase();
void ShutdownRoomDatabase();

void SetTileRoomNum(INT16 sGridNo, UINT8 ubRoomNum);
void SetTileRangeRoomNum(struct GRect *pSelectRegion, UINT8 ubRoomNum);

void RemoveRoomRoof(UINT16 sGridNo, UINT8 bRoomNum, struct SOLDIERTYPE *pSoldier);
BOOLEAN InARoom(UINT16 sGridNo, UINT8 *pubRoomNo);
BOOLEAN InAHiddenRoom(UINT16 sGridNo, UINT8 *pubRoomNo);

void SetGridNoRevealedFlag(UINT16 sGridNo);

void ExamineGridNoForSlantRoofExtraGraphic(UINT16 sCheckGridNo);

void SetRecalculateWireFrameFlagRadius(INT16 sX, INT16 sY, INT16 sRadius);

#endif
