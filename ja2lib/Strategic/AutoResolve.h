#ifndef __AUTO_RESOLVE_H
#define __AUTO_RESOLVE_H

#include "SGP/Types.h"

void EnterAutoResolveMode(uint8_t ubSectorX, uint8_t ubSectorY);

// is the autoresolve active?
BOOLEAN IsAutoResolveActive(void);

void EliminateAllEnemies(uint8_t ubSectorX, uint8_t ubSectorY);

void ConvertTacticalBattleIntoStrategicAutoResolveBattle();

uint8_t GetAutoResolveSectorID();

extern BOOLEAN gfTransferTacticalOppositionToAutoResolve;

// Returns TRUE if autoresolve is active or a sector is loaded.
BOOLEAN GetCurrentBattleSectorXYZ(INT16 *psSectorX, INT16 *psSectorY, INT16 *psSectorZ);

#endif
