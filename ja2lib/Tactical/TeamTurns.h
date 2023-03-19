#ifndef TEAMTURNS_H
#define TEAMTURNS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

extern UINT8 gubOutOfTurnPersons;
extern BOOLEAN gfHiddenInterrupt;
extern BOOLEAN gfHiddenTurnbased;

#define INTERRUPT_QUEUED (gubOutOfTurnPersons > 0)

extern BOOLEAN StandardInterruptConditionsMet(struct SOLDIERTYPE* pSoldier, UINT8 ubOpponentID,
                                              INT8 bOldOppList);
extern INT8 CalcInterruptDuelPts(struct SOLDIERTYPE* pSoldier, UINT8 ubOpponentID,
                                 BOOLEAN fUseWatchSpots);
extern void EndAITurn(void);
extern void DisplayHiddenInterrupt(struct SOLDIERTYPE* pSoldier);
extern BOOLEAN InterruptDuel(struct SOLDIERTYPE* pSoldier, struct SOLDIERTYPE* pOpponent);
extern void AddToIntList(UINT8 ubID, BOOLEAN fGainControl, BOOLEAN fCommunicate);
extern void DoneAddingToIntList(struct SOLDIERTYPE* pSoldier, BOOLEAN fChange,
                                UINT8 ubInterruptType);

void ClearIntList(void);

BOOLEAN SaveTeamTurnsToTheSaveGameFile(FileID hFile);

BOOLEAN LoadTeamTurnsFromTheSavedGameFile(FileID hFile);

void EndAllAITurns(void);

BOOLEAN NPCFirstDraw(struct SOLDIERTYPE* pSoldier, struct SOLDIERTYPE* pTargetSoldier);

#endif
