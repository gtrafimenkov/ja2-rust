#ifndef TEAMTURNS_H
#define TEAMTURNS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

extern uint8_t gubOutOfTurnPersons;
extern BOOLEAN gfHiddenInterrupt;
extern BOOLEAN gfHiddenTurnbased;

#define INTERRUPT_QUEUED (gubOutOfTurnPersons > 0)

extern BOOLEAN StandardInterruptConditionsMet(struct SOLDIERTYPE* pSoldier, uint8_t ubOpponentID,
                                              INT8 bOldOppList);
extern INT8 CalcInterruptDuelPts(struct SOLDIERTYPE* pSoldier, uint8_t ubOpponentID,
                                 BOOLEAN fUseWatchSpots);
extern void EndAITurn(void);
extern void DisplayHiddenInterrupt(struct SOLDIERTYPE* pSoldier);
extern BOOLEAN InterruptDuel(struct SOLDIERTYPE* pSoldier, struct SOLDIERTYPE* pOpponent);
extern void AddToIntList(uint8_t ubID, BOOLEAN fGainControl, BOOLEAN fCommunicate);
extern void DoneAddingToIntList(struct SOLDIERTYPE* pSoldier, BOOLEAN fChange,
                                uint8_t ubInterruptType);

void ClearIntList(void);

BOOLEAN SaveTeamTurnsToTheSaveGameFile(HWFILE hFile);

BOOLEAN LoadTeamTurnsFromTheSavedGameFile(HWFILE hFile);

void EndAllAITurns(void);

BOOLEAN NPCFirstDraw(struct SOLDIERTYPE* pSoldier, struct SOLDIERTYPE* pTargetSoldier);

#endif
