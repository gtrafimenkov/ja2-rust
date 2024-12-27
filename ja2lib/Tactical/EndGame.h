// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __ENDGAME_H
#define __ENDGAME_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

BOOLEAN DoesO3SectorStatueExistHere(int16_t sGridNo);
void ChangeO3SectorStatue(BOOLEAN fFromExplosion);

void HandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, int16_t sGridNo, int8_t bLevel);
void BeginHandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, int16_t sGridNo, int8_t bLevel);

void HandleDoneLastKilledQueenQuote();

void EndQueenDeathEndgameBeginEndCimenatic();
void EndQueenDeathEndgame();

void HandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, int16_t sGridNo, int8_t bLevel);
void BeginHandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, int16_t sGridNo, int8_t bLevel);

void HandleDoneLastEndGameQuote();

#endif
