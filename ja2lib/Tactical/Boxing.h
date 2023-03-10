#ifndef BOXING_H
#define BOXING_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define BOXING_SECTOR_X 5
#define BOXING_SECTOR_Y 4
#define BOXING_SECTOR_Z 0
#define ROOM_SURROUNDING_BOXING_RING 3
#define BOXING_RING 29

#define BOXING_AI_START_POSITION 11235

#define NUM_BOXERS 3

typedef enum { BOXER_OUT_OF_RING, NON_BOXER_IN_RING, BAD_ATTACK } DisqualificationReasons;

extern INT16 gsBoxerGridNo[NUM_BOXERS];
extern UINT8 gubBoxerID[NUM_BOXERS];
extern BOOLEAN gfBoxerFought[NUM_BOXERS];
extern INT8 gbBoxingState;
extern BOOLEAN gfLastBoxingMatchWonByPlayer;
extern UINT8 gubBoxingMatchesWon;
extern UINT8 gubBoxersRests;
extern BOOLEAN gfBoxersResting;

extern void BoxingPlayerDisqualified(struct SOLDIERTYPE* pOffender, INT8 bReason);
extern BOOLEAN PickABoxer(void);
extern BOOLEAN CheckOnBoxers(void);
extern void EndBoxingMatch(struct SOLDIERTYPE* pLoser);
extern BOOLEAN BoxerAvailable(void);
extern BOOLEAN AnotherFightPossible(void);
extern void TriggerEndOfBoxingRecord(struct SOLDIERTYPE* pSolier);
extern void BoxingMovementCheck(struct SOLDIERTYPE* pSoldier);
extern void ExitBoxing(void);
extern UINT8 BoxersAvailable(void);
extern void SetBoxingState(INT8 bNewState);
extern BOOLEAN BoxerExists(void);
extern UINT8 CountPeopleInBoxingRing(void);
extern void ClearAllBoxerFlags(void);

#endif
