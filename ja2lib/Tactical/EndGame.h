#ifndef __ENDGAME_H
#define __ENDGAME_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

BOOLEAN DoesO3SectorStatueExistHere(INT16 sGridNo);
void ChangeO3SectorStatue(BOOLEAN fFromExplosion);

void HandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel);
void BeginHandleDeidrannaDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel);

void HandleDoneLastKilledQueenQuote();

void EndQueenDeathEndgameBeginEndCimenatic();
void EndQueenDeathEndgame();

void HandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel);
void BeginHandleQueenBitchDeath(struct SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel);

void HandleDoneLastEndGameQuote();

#endif
