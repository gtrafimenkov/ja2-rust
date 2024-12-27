#ifndef __ENDGAME_H
#define __ENDGAME_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

BOOLEAN DoesO3SectorStatueExistHere(INT16 sGridNo);
void ChangeO3SectorStatue(BOOLEAN fFromExplosion);

void HandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, int8_t bLevel);
void BeginHandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, int8_t bLevel);

void HandleDoneLastKilledQueenQuote();

void EndQueenDeathEndgameBeginEndCimenatic();
void EndQueenDeathEndgame();

void HandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, int8_t bLevel);
void BeginHandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, int8_t bLevel);

void HandleDoneLastEndGameQuote();

#endif
