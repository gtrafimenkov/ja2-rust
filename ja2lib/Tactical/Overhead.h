#ifndef __OVERHEAD_H
#define __OVERHEAD_H

#include <stdio.h>
#include <string.h>

#include "SGP/Types.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/SoldierFind.h"

struct SOLDIERTYPE;

#define ADD_SOLDIER_NO_PROFILE_ID 200

#define MAX_REALTIME_SPEED_VAL 10

// Enums for waiting for mercs to finish codes
enum {
  NO_WAIT_EVENT = 0,
  WAIT_FOR_MERCS_TO_WALKOFF_SCREEN,
  WAIT_FOR_MERCS_TO_WALKON_SCREEN,
  WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO
};

// TACTICAL ENGINE STATUS FLAGS
typedef struct {
  uint8_t bFirstID;
  uint8_t bLastID;
  COLORVAL RadarColor;
  INT8 bSide;
  INT8 bMenInSector;
  uint8_t ubLastMercToRadio;
  INT8 bTeamActive;
  INT8 bAwareOfOpposition;
  INT8 bHuman;
} TacticalTeamType;

// for use with TacticalStatusType.ubEnemyIntention
enum {
  INTENTION_SCOUTING,
  INTENTION_PATROLLING,
  INTENTION_ATTACKING,
  INTENTION_DEFENDING,
  INTENTION_RETREATING
};

// for use with TacticalStatusType.ubEnemyIntendedRetreatDirection
enum { RETREAT_NORTH, RETREAT_EAST, RETREAT_SOUTH, RETREAT_WEST };

#define PANIC_BOMBS_HERE 0x01
#define PANIC_TRIGGERS_HERE 0x02

#define NUM_PANIC_TRIGGERS 3

#define ENEMY_OFFERED_SURRENDER 0x01

typedef struct {
  uint32_t uiFlags;
  TacticalTeamType Team[MAXTEAMS];
  uint8_t ubCurrentTeam;
  INT16 sSlideTarget;
  INT16 sSlideReason;
  uint32_t uiTimeSinceMercAIStart;
  INT8 fPanicFlags;
  INT16 sPanicTriggerGridnoUnused;
  INT16 sHandGrid;
  uint8_t ubSpottersCalledForBy;
  uint8_t ubTheChosenOne;
  uint32_t uiTimeOfLastInput;
  uint32_t uiTimeSinceDemoOn;
  uint32_t uiCountdownToRestart;
  BOOLEAN fGoingToEnterDemo;
  BOOLEAN fNOTDOLASTDEMO;
  BOOLEAN fMultiplayer;
  BOOLEAN fCivGroupHostile[NUM_CIV_GROUPS];
  uint8_t ubLastBattleSectorX;
  uint8_t ubLastBattleSectorY;
  BOOLEAN fLastBattleWon;
  INT8 bOriginalSizeOfEnemyForce;
  INT8 bPanicTriggerIsAlarmUnused;
  BOOLEAN fVirginSector;
  BOOLEAN fEnemyInSector;
  BOOLEAN fInterruptOccurred;
  INT8 bRealtimeSpeed;
  uint8_t ubEnemyIntention;
  uint8_t ubEnemyIntendedRetreatDirection;
  uint8_t ubEnemySightingOnTheirTurnEnemyID;
  uint8_t ubEnemySightingOnTheirTurnPlayerID;
  BOOLEAN fEnemySightingOnTheirTurn;
  BOOLEAN fAutoBandageMode;
  uint8_t ubAttackBusyCount;
  INT8 bNumEnemiesFoughtInBattleUnused;
  uint8_t ubEngagedInConvFromActionMercID;
  uint16_t usTactialTurnLimitCounter;
  BOOLEAN fInTopMessage;
  uint8_t ubTopMessageType;
  CHAR16 zTopMessageString[20];
  uint16_t usTactialTurnLimitMax;
  uint32_t uiTactialTurnLimitClock;
  BOOLEAN fTactialTurnLimitStartedBeep;
  INT8 bBoxingState;
  INT8 bConsNumTurnsNotSeen;
  uint8_t ubArmyGuysKilled;

  INT16 sPanicTriggerGridNo[NUM_PANIC_TRIGGERS];
  INT8 bPanicTriggerIsAlarm[NUM_PANIC_TRIGGERS];
  uint8_t ubPanicTolerance[NUM_PANIC_TRIGGERS];
  BOOLEAN fAtLeastOneGuyOnMultiSelect;
  BOOLEAN fSaidCreatureFlavourQuote;
  BOOLEAN fHaveSeenCreature;
  BOOLEAN fKilledEnemyOnAttack;
  uint8_t ubEnemyKilledOnAttack;
  INT8 bEnemyKilledOnAttackLevel;
  uint16_t ubEnemyKilledOnAttackLocation;
  BOOLEAN fItemsSeenOnAttack;
  BOOLEAN ubItemsSeenOnAttackSoldier;
  BOOLEAN fBeenInCombatOnce;
  BOOLEAN fSaidCreatureSmellQuote;
  uint16_t usItemsSeenOnAttackGridNo;
  BOOLEAN fLockItemLocators;
  uint8_t ubLastQuoteSaid;
  uint8_t ubLastQuoteProfileNUm;
  BOOLEAN fCantGetThrough;
  INT16 sCantGetThroughGridNo;
  INT16 sCantGetThroughSoldierGridNo;
  uint8_t ubCantGetThroughID;
  BOOLEAN fDidGameJustStart;
  BOOLEAN fStatChangeCheatOn;
  uint8_t ubLastRequesterTargetID;
  BOOLEAN fGoodToAllowCrows;
  uint8_t ubNumCrowsPossible;
  uint32_t uiTimeCounterForGiveItemSrc;
  BOOLEAN fUnLockUIAfterHiddenInterrupt;
  INT8 bNumFoughtInBattle[MAXTEAMS];
  uint32_t uiDecayBloodLastUpdate;
  uint32_t uiTimeSinceLastInTactical;
  BOOLEAN fHasAGameBeenStarted;
  INT8 bConsNumTurnsWeHaventSeenButEnemyDoes;
  BOOLEAN fSomeoneHit;
  uint8_t ubPaddingSmall;
  uint32_t uiTimeSinceLastOpplistDecay;
  INT8 bMercArrivingQuoteBeingUsed;
  uint8_t ubEnemyKilledOnAttackKiller;
  BOOLEAN fCountingDownForGuideDescription;
  INT8 bGuideDescriptionCountDown;
  uint8_t ubGuideDescriptionToUse;
  INT8 bGuideDescriptionSectorX;
  INT8 bGuideDescriptionSectorY;
  INT8 fEnemyFlags;
  BOOLEAN fAutoBandagePending;
  BOOLEAN fHasEnteredCombatModeSinceEntering;
  BOOLEAN fDontAddNewCrows;
  uint8_t ubMorePadding;
  uint16_t sCreatureTenseQuoteDelay;
  uint32_t uiCreatureTenseQuoteLastUpdate;

  // PADDING GONE!!!!!

} TacticalStatusType;

extern uint8_t gbPlayerNum;
extern INT8 gbShowEnemies;

extern uint16_t gusSelectedSoldier;
extern uint16_t gusOldSelectedSoldier;

extern CHAR8 gzAlertStr[][30];
extern CHAR8 gzActionStr[][30];
extern CHAR8 gzDirectionStr[][30];

// TEMP FOR E3
extern uint8_t gubCurrentScene;
extern CHAR8 *GetSceneFilename();
extern INT8 ubLevelMoveLink[10];

// Soldier List used for all soldier overhead interaction
extern struct SOLDIERTYPE *MercPtrs[TOTAL_SOLDIERS];

// MERC SLOTS - A LIST OF ALL ACTIVE MERCS
extern struct SOLDIERTYPE *MercSlots[TOTAL_SOLDIERS];
extern uint32_t guiNumMercSlots;

extern BOOLEAN gfMovingAnimation;

extern TacticalStatusType gTacticalStatus;

BOOLEAN InitTacticalEngine();
void ShutdownTacticalEngine();

BOOLEAN InitOverhead();
BOOLEAN ShutdownOverhead();
BOOLEAN GetSoldier(struct SOLDIERTYPE **ppSoldier, uint16_t usSoldierIndex);

INT16 NewOKDestination(struct SOLDIERTYPE *pCurrSoldier, INT16 sGridNo, BOOLEAN fPeopleToo,
                       INT8 bLevel);

// Simple check to see if a (one-tiled) soldier can occupy a given location on the ground or roof.
extern BOOLEAN IsLocationSittable(INT32 iMapIndex, BOOLEAN fOnRoof);
extern BOOLEAN IsLocationSittableExcludingPeople(INT32 iMapIndex, BOOLEAN fOnRoof);
extern BOOLEAN FlatRoofAboveGridNo(INT32 iMapIndex);

BOOLEAN ExecuteOverhead();
BOOLEAN ResetAllAnimationCache();

void EndTurn(uint8_t ubNextTeam);
void StartPlayerTeamTurn(BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode);
void EndTacticalDemo();

void SelectSoldier(uint16_t usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect);

void LocateGridNo(uint16_t sGridNo);
void LocateSoldier(uint16_t usID, BOOLEAN fSetLocator);

void BeginTeamTurn(uint8_t ubTeam);
void SlideTo(INT16 sGridno, uint16_t usSoldierID, uint16_t usReasonID, BOOLEAN fSetLocator);
void SlideToLocation(uint16_t usReasonID, INT16 sDestGridNo);

void RebuildAllSoldierShadeTables();
void HandlePlayerTeamMemberDeath(struct SOLDIERTYPE *pSoldier);
uint8_t LastActiveTeamMember(uint8_t ubTeam);
BOOLEAN SoldierOnVisibleWorldTile(struct SOLDIERTYPE *pSoldier);

uint8_t FindNextActiveAndAliveMerc(struct SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,
                                   BOOLEAN fOnlyRegularMercs);
uint8_t FindPrevActiveAndAliveMerc(struct SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,
                                   BOOLEAN fOnlyRegularMercs);

BOOLEAN CheckForPlayerTeamInMissionExit();
void HandleNPCTeamMemberDeath(struct SOLDIERTYPE *pSoldier);

void StopMercAnimation(BOOLEAN fStop);

uint32_t EnterTacticalDemoMode();

BOOLEAN UIOKMoveDestination(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos);

INT16 FindAdjacentGridEx(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t *pubDirection,
                         INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);
INT16 FindNextToAdjacentGridEx(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, uint8_t *pubDirection,
                               INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);

void SelectNextAvailSoldier(struct SOLDIERTYPE *pSoldier);
BOOLEAN TeamMemberNear(INT8 bTeam, INT16 sGridNo, INT32 iRange);
BOOLEAN IsValidTargetMerc(uint8_t ubSoldierID);

// FUNCTIONS FOR MANIPULATING MERC SLOTS - A LIST OF ALL ACTIVE MERCS
INT32 GetFreeMercSlot(void);
void RecountMercSlots(void);
INT32 AddMercSlot(struct SOLDIERTYPE *pSoldier);
BOOLEAN RemoveMercSlot(struct SOLDIERTYPE *pSoldier);

INT32 AddAwaySlot(struct SOLDIERTYPE *pSoldier);
BOOLEAN RemoveAwaySlot(struct SOLDIERTYPE *pSoldier);
INT32 MoveSoldierFromMercToAwaySlot(struct SOLDIERTYPE *pSoldier);
INT32 MoveSoldierFromAwayToMercSlot(struct SOLDIERTYPE *pSoldier);

void EnterCombatMode(uint8_t ubStartingTeam);
void ExitCombatMode();

void HandleTeamServices(uint8_t ubTeamNum);
void HandlePlayerServices(struct SOLDIERTYPE *pTeamSoldier);

void SetEnemyPresence();

void CycleThroughKnownEnemies();

BOOLEAN CheckForEndOfCombatMode(BOOLEAN fIncrementTurnsNotSeen);

struct SOLDIERTYPE *FreeUpAttacker(uint8_t ubID);

BOOLEAN PlayerTeamFull();

void SetActionToDoOnceMercsGetToLocation(uint8_t ubActionCode, INT8 bNumMercsWaiting,
                                         uint32_t uiData1, uint32_t uiData2, uint32_t uiData3);

void ResetAllMercSpeeds();

BOOLEAN HandleGotoNewGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving,
                            BOOLEAN fInitialMove, uint16_t usAnimState);

struct SOLDIERTYPE *ReduceAttackBusyCount(uint8_t ubID, BOOLEAN fCalledByAttacker);

void CommonEnterCombatModeCode();

void CheckForPotentialAddToBattleIncrement(struct SOLDIERTYPE *pSoldier);

void CencelAllActionsForTimeCompression(void);

BOOLEAN CheckForEndOfBattle(BOOLEAN fAnEnemyRetreated);

void AddManToTeam(INT8 bTeam);

void RemoveManFromTeam(INT8 bTeam);

void RemoveSoldierFromTacticalSector(struct SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected);

void MakeCivHostile(struct SOLDIERTYPE *pSoldier, INT8 bNewSide);

#define REASON_NORMAL_ATTACK 1
#define REASON_EXPLOSION 2

BOOLEAN ProcessImplicationsOfPCAttack(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE **ppTarget,
                                      INT8 bReason);

INT16 FindAdjacentPunchTarget(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                              INT16 *psAdjustedTargetGridNo, uint8_t *pubDirection);

struct SOLDIERTYPE *CivilianGroupMemberChangesSides(struct SOLDIERTYPE *pAttacked);
void CivilianGroupChangesSides(uint8_t ubCivilianGroup);

void CycleVisibleEnemies(struct SOLDIERTYPE *pSrcSoldier);
uint8_t CivilianGroupMembersChangeSidesWithinProximity(struct SOLDIERTYPE *pAttacked);

void PauseAITemporarily(void);
void PauseAIUntilManuallyUnpaused(void);
void UnPauseAI(void);

void DoPOWPathChecks(void);

BOOLEAN HostileCiviliansWithGunsPresent(void);
BOOLEAN HostileCiviliansPresent(void);
BOOLEAN HostileBloodcatsPresent(void);
uint8_t NumPCsInSector(void);

void SetSoldierNonNeutral(struct SOLDIERTYPE *pSoldier);
void SetSoldierNeutral(struct SOLDIERTYPE *pSoldier);

#endif
