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
  int8_t bSide;
  int8_t bMenInSector;
  uint8_t ubLastMercToRadio;
  int8_t bTeamActive;
  int8_t bAwareOfOpposition;
  int8_t bHuman;
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
  int16_t sSlideTarget;
  int16_t sSlideReason;
  uint32_t uiTimeSinceMercAIStart;
  int8_t fPanicFlags;
  int16_t sPanicTriggerGridnoUnused;
  int16_t sHandGrid;
  uint8_t ubSpottersCalledForBy;
  uint8_t ubTheChosenOne;
  uint32_t uiTimeOfLastInput;
  uint32_t uiTimeSinceDemoOn;
  uint32_t uiCountdownToRestart;
  BOOLEAN fGoingToEnterDemo;
  BOOLEAN fNOTDOLASTDEMO;
  BOOLEAN fMultiplayer;
  BOOLEAN __only_storage_fCivGroupHostile[20 /* NUM_CIV_GROUPS */];
  uint8_t ubLastBattleSectorX;
  uint8_t ubLastBattleSectorY;
  BOOLEAN fLastBattleWon;
  int8_t bOriginalSizeOfEnemyForce;
  int8_t bPanicTriggerIsAlarmUnused;
  BOOLEAN fVirginSector;
  BOOLEAN fEnemyInSector;
  BOOLEAN fInterruptOccurred;
  int8_t bRealtimeSpeed;
  uint8_t ubEnemyIntention;
  uint8_t ubEnemyIntendedRetreatDirection;
  uint8_t ubEnemySightingOnTheirTurnEnemyID;
  uint8_t ubEnemySightingOnTheirTurnPlayerID;
  BOOLEAN fEnemySightingOnTheirTurn;
  BOOLEAN fAutoBandageMode;
  uint8_t ubAttackBusyCount;
  int8_t bNumEnemiesFoughtInBattleUnused;
  uint8_t ubEngagedInConvFromActionMercID;
  uint16_t usTactialTurnLimitCounter;
  BOOLEAN fInTopMessage;
  uint8_t ubTopMessageType;
  wchar_t zTopMessageString[20];
  uint16_t usTactialTurnLimitMax;
  uint32_t uiTactialTurnLimitClock;
  BOOLEAN fTactialTurnLimitStartedBeep;
  int8_t bBoxingState;
  int8_t bConsNumTurnsNotSeen;
  uint8_t ubArmyGuysKilled;

  int16_t sPanicTriggerGridNo[NUM_PANIC_TRIGGERS];
  int8_t bPanicTriggerIsAlarm[NUM_PANIC_TRIGGERS];
  uint8_t ubPanicTolerance[NUM_PANIC_TRIGGERS];
  BOOLEAN fAtLeastOneGuyOnMultiSelect;
  BOOLEAN fSaidCreatureFlavourQuote;
  BOOLEAN fHaveSeenCreature;
  BOOLEAN fKilledEnemyOnAttack;
  uint8_t ubEnemyKilledOnAttack;
  int8_t bEnemyKilledOnAttackLevel;
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
  int16_t sCantGetThroughGridNo;
  int16_t sCantGetThroughSoldierGridNo;
  uint8_t ubCantGetThroughID;
  BOOLEAN fDidGameJustStart;
  BOOLEAN fStatChangeCheatOn;
  uint8_t ubLastRequesterTargetID;
  BOOLEAN fGoodToAllowCrows;
  uint8_t ubNumCrowsPossible;
  uint32_t uiTimeCounterForGiveItemSrc;
  BOOLEAN fUnLockUIAfterHiddenInterrupt;
  int8_t bNumFoughtInBattle[MAXTEAMS];
  uint32_t uiDecayBloodLastUpdate;
  uint32_t uiTimeSinceLastInTactical;
  BOOLEAN fHasAGameBeenStarted;
  int8_t bConsNumTurnsWeHaventSeenButEnemyDoes;
  BOOLEAN fSomeoneHit;
  uint8_t ubPaddingSmall;
  uint32_t uiTimeSinceLastOpplistDecay;
  int8_t bMercArrivingQuoteBeingUsed;
  uint8_t ubEnemyKilledOnAttackKiller;
  BOOLEAN fCountingDownForGuideDescription;
  int8_t bGuideDescriptionCountDown;
  uint8_t ubGuideDescriptionToUse;
  int8_t bGuideDescriptionSectorX;
  int8_t bGuideDescriptionSectorY;
  int8_t fEnemyFlags;
  BOOLEAN fAutoBandagePending;
  BOOLEAN fHasEnteredCombatModeSinceEntering;
  BOOLEAN fDontAddNewCrows;
  uint8_t ubMorePadding;
  uint16_t sCreatureTenseQuoteDelay;
  uint32_t uiCreatureTenseQuoteLastUpdate;

  // PADDING GONE!!!!!

} TacticalStatusType;

extern uint8_t gbPlayerNum;
extern int8_t gbShowEnemies;

extern uint16_t gusSelectedSoldier;
extern uint16_t gusOldSelectedSoldier;

extern char gzAlertStr[][30];
extern char gzActionStr[][30];
extern char gzDirectionStr[][30];

// TEMP FOR E3
extern uint8_t gubCurrentScene;
extern char *GetSceneFilename();
extern int8_t ubLevelMoveLink[10];

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

int16_t NewOKDestination(struct SOLDIERTYPE *pCurrSoldier, int16_t sGridNo, BOOLEAN fPeopleToo,
                         int8_t bLevel);

// Simple check to see if a (one-tiled) soldier can occupy a given location on the ground or roof.
extern BOOLEAN IsLocationSittable(int32_t iMapIndex, BOOLEAN fOnRoof);
extern BOOLEAN IsLocationSittableExcludingPeople(int32_t iMapIndex, BOOLEAN fOnRoof);
extern BOOLEAN FlatRoofAboveGridNo(int32_t iMapIndex);

BOOLEAN ExecuteOverhead();
BOOLEAN ResetAllAnimationCache();

void EndTurn(uint8_t ubNextTeam);
void StartPlayerTeamTurn(BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode);
void EndTacticalDemo();

void SelectSoldier(uint16_t usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect);

void LocateGridNo(uint16_t sGridNo);
void LocateSoldier(uint16_t usID, BOOLEAN fSetLocator);

void BeginTeamTurn(uint8_t ubTeam);
void SlideTo(int16_t sGridno, uint16_t usSoldierID, uint16_t usReasonID, BOOLEAN fSetLocator);
void SlideToLocation(uint16_t usReasonID, int16_t sDestGridNo);

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

int16_t FindAdjacentGridEx(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t *pubDirection,
                           int16_t *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);
int16_t FindNextToAdjacentGridEx(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                 uint8_t *pubDirection, int16_t *psAdjustedGridNo,
                                 BOOLEAN fForceToPerson, BOOLEAN fDoor);

void SelectNextAvailSoldier(struct SOLDIERTYPE *pSoldier);
BOOLEAN TeamMemberNear(int8_t bTeam, int16_t sGridNo, int32_t iRange);
BOOLEAN IsValidTargetMerc(uint8_t ubSoldierID);

// FUNCTIONS FOR MANIPULATING MERC SLOTS - A LIST OF ALL ACTIVE MERCS
int32_t GetFreeMercSlot(void);
void RecountMercSlots(void);
int32_t AddMercSlot(struct SOLDIERTYPE *pSoldier);
BOOLEAN RemoveMercSlot(struct SOLDIERTYPE *pSoldier);

int32_t AddAwaySlot(struct SOLDIERTYPE *pSoldier);
BOOLEAN RemoveAwaySlot(struct SOLDIERTYPE *pSoldier);
int32_t MoveSoldierFromMercToAwaySlot(struct SOLDIERTYPE *pSoldier);
int32_t MoveSoldierFromAwayToMercSlot(struct SOLDIERTYPE *pSoldier);

void EnterCombatMode(uint8_t ubStartingTeam);
void ExitCombatMode();

void HandleTeamServices(uint8_t ubTeamNum);
void HandlePlayerServices(struct SOLDIERTYPE *pTeamSoldier);

void SetEnemyPresence();

void CycleThroughKnownEnemies();

BOOLEAN CheckForEndOfCombatMode(BOOLEAN fIncrementTurnsNotSeen);

struct SOLDIERTYPE *FreeUpAttacker(uint8_t ubID);

BOOLEAN PlayerTeamFull();

void SetActionToDoOnceMercsGetToLocation(uint8_t ubActionCode, int8_t bNumMercsWaiting,
                                         uint32_t uiData1, uint32_t uiData2, uint32_t uiData3);

void ResetAllMercSpeeds();

BOOLEAN HandleGotoNewGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving,
                            BOOLEAN fInitialMove, uint16_t usAnimState);

struct SOLDIERTYPE *ReduceAttackBusyCount(uint8_t ubID, BOOLEAN fCalledByAttacker);

void CommonEnterCombatModeCode();

void CheckForPotentialAddToBattleIncrement(struct SOLDIERTYPE *pSoldier);

void CencelAllActionsForTimeCompression(void);

BOOLEAN CheckForEndOfBattle(BOOLEAN fAnEnemyRetreated);

void AddManToTeam(int8_t bTeam);

void RemoveManFromTeam(int8_t bTeam);

void RemoveSoldierFromTacticalSector(struct SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected);

void MakeCivHostile(struct SOLDIERTYPE *pSoldier, int8_t bNewSide);

#define REASON_NORMAL_ATTACK 1
#define REASON_EXPLOSION 2

BOOLEAN ProcessImplicationsOfPCAttack(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE **ppTarget,
                                      int8_t bReason);

int16_t FindAdjacentPunchTarget(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                                int16_t *psAdjustedTargetGridNo, uint8_t *pubDirection);

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
