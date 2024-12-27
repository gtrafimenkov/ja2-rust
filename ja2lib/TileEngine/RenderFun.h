// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __RENDER_FUN_H
#define __RENDER_FUN_H

#include <stdio.h>

#include "SGP/Types.h"
#include "TileEngine/WorldDef.h"
#include "rust_geometry.h"

#define NO_ROOM 0
#define MAX_ROOMS 250

extern uint8_t gubWorldRoomHidden[MAX_ROOMS];
extern uint8_t gubWorldRoomInfo[WORLD_MAX];

BOOLEAN InitRoomDatabase();
void ShutdownRoomDatabase();

void SetTileRoomNum(int16_t sGridNo, uint8_t ubRoomNum);
void SetTileRangeRoomNum(struct GRect *pSelectRegion, uint8_t ubRoomNum);

void RemoveRoomRoof(uint16_t sGridNo, uint8_t bRoomNum, struct SOLDIERTYPE *pSoldier);
BOOLEAN InARoom(uint16_t sGridNo, uint8_t *pubRoomNo);
BOOLEAN InAHiddenRoom(uint16_t sGridNo, uint8_t *pubRoomNo);

void SetGridNoRevealedFlag(uint16_t sGridNo);

void ExamineGridNoForSlantRoofExtraGraphic(uint16_t sCheckGridNo);

void SetRecalculateWireFrameFlagRadius(int16_t sX, int16_t sY, int16_t sRadius);

#endif
