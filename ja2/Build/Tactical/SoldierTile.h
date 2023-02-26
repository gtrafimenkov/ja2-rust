#ifndef __SOLDIER_TILE_H
#define __SOLDIER_TILE_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define MOVE_TILE_CLEAR 1
#define MOVE_TILE_TEMP_BLOCKED -1
#define MOVE_TILE_STATIONARY_BLOCKED -2

INT8 TileIsClear(struct SOLDIERTYPE *pSoldier, INT8 bDirection, INT16 sGridNo, INT8 bLevel);

void MarkMovementReserved(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

void UnMarkMovementReserved(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleNextTile(struct SOLDIERTYPE *pSoldier, INT8 bDirection, INT16 sGridNo,
                       INT16 sFinalDestTile);

BOOLEAN HandleNextTileWaiting(struct SOLDIERTYPE *pSoldier);

BOOLEAN TeleportSoldier(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fForce);

void SwapMercPositions(struct SOLDIERTYPE *pSoldier1, struct SOLDIERTYPE *pSoldier2);

void SetDelayedTileWaiting(struct SOLDIERTYPE *pSoldier, INT16 sCauseGridNo, INT8 bValue);

BOOLEAN CanExchangePlaces(struct SOLDIERTYPE *pSoldier1, struct SOLDIERTYPE *pSoldier2,
                          BOOLEAN fShow);

#endif
