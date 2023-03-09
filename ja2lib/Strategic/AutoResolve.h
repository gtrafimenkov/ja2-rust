#ifndef __AUTO_RESOLVE_H
#define __AUTO_RESOLVE_H

#include "SGP/Types.h"

void EnterAutoResolveMode(UINT8 ubSectorX, UINT8 ubSectorY);

// is the autoresolve active?
BOOLEAN IsAutoResolveActive(void);

void EliminateAllEnemies(UINT8 ubSectorX, UINT8 ubSectorY);

void ConvertTacticalBattleIntoStrategicAutoResolveBattle();

UINT8 GetAutoResolveSectorID();

extern BOOLEAN gfTransferTacticalOppositionToAutoResolve;

// Returns TRUE if autoresolve is active or a sector is loaded.
BOOLEAN GetCurrentBattleSectorXYZ(u8 *psSectorX, u8 *psSectorY, i8 *psSectorZ);

#endif
