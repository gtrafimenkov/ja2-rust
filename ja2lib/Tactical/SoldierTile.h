#ifndef __SOLDIER_TILE_H
#define __SOLDIER_TILE_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define MOVE_TILE_CLEAR 1
#define MOVE_TILE_TEMP_BLOCKED -1
#define MOVE_TILE_STATIONARY_BLOCKED -2

int8_t TileIsClear(struct SOLDIERTYPE *pSoldier, int8_t bDirection, INT16 sGridNo, int8_t bLevel);

void MarkMovementReserved(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

void UnMarkMovementReserved(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleNextTile(struct SOLDIERTYPE *pSoldier, int8_t bDirection, INT16 sGridNo,
                       INT16 sFinalDestTile);

BOOLEAN HandleNextTileWaiting(struct SOLDIERTYPE *pSoldier);

BOOLEAN TeleportSoldier(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fForce);

void SwapMercPositions(struct SOLDIERTYPE *pSoldier1, struct SOLDIERTYPE *pSoldier2);

void SetDelayedTileWaiting(struct SOLDIERTYPE *pSoldier, INT16 sCauseGridNo, int8_t bValue);

BOOLEAN CanExchangePlaces(struct SOLDIERTYPE *pSoldier1, struct SOLDIERTYPE *pSoldier2,
                          BOOLEAN fShow);

#endif
