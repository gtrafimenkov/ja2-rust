// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/Points.h"
#include "TileEngine/IsometricUtils.h"

extern BOOLEAN gfTurnBasedAI;

// THIS IS AN ITEM #  - AND FOR NOW JUST COMPLETELY FAKE...

#define MAX_TOSS_SEARCH_DIST 1    // must throw within this of opponent
#define NPC_TOSS_SAFETY_MARGIN 4  // all friends must be this far away

#define ACTING_ON_SCHEDULE(p) ((p)->fAIFlags & AI_CHECK_SCHEDULE)

// the AI should try to have this many APs before climbing a roof, if possible
#define AI_AP_CLIMBROOF 15

#define TEMPORARILY 0
#define FOREVER 1

#define IGNORE_PATH 0
#define ENSURE_PATH 1
#define ENSURE_PATH_COST 2

// Kris:  November 10, 1997
// Please don't change this value from 10.  It will invalidate all of the maps and soldiers.
#define MAXPATROLGRIDS 10

#define NOWATER 0
#define WATEROK 1

#define DONTADDTURNCOST 0
#define ADDTURNCOST 1

enum { URGENCY_LOW = 0, URGENCY_MED, URGENCY_HIGH, NUM_URGENCY_STATES };

#define NOWATER 0
#define WATEROK 1

#define IGNORE_PATH 0
#define ENSURE_PATH 1
#define ENSURE_PATH_COST 2

#define DONTFORCE 0
#define FORCE 1

#define MAX_ROAMING_RANGE WORLD_COLS

#define PTR_CIV_OR_MILITIA (PTR_CIVILIAN || (pSoldier->bTeam == MILITIA_TEAM))

#define REALTIME_AI_DELAY (10000 + Random(1000))
#define REALTIME_CIV_AI_DELAY                                \
  (1000 * (gTacticalStatus.Team[MILITIA_TEAM].bMenInSector + \
           gTacticalStatus.Team[CIV_TEAM].bMenInSector) +    \
   5000 + 2000 * Random(3))
#define REALTIME_CREATURE_AI_DELAY (10000 + 1000 * Random(3))

// #define PLAYINGMODE             0
// #define CAMPAIGNLENGTH          1
// #define LASTUSABLESLOT          2
// #define RANDOMMERCS             3
// #define AVAILABLEMERCS          4
// #define HIRINGKNOWLEDGE         5
// #define EQUIPMENTLEVEL          6
// #define ENEMYTEAMSIZE           7
#define ENEMYDIFFICULTY 8  // this is being used in this module
// #define FOG_OF_WAR              9
// #define TURNLENGTH              10
// #define INCREASEDAP             11
// #define BLOODSTAINS             12
// #define STARTINGBALANCE         13
#define MAXGAMEOPTIONS 14

#define NOSHOOT_WAITABIT -1
#define NOSHOOT_WATER -2
#define NOSHOOT_MYSELF -3
#define NOSHOOT_HURT -4
#define NOSHOOT_NOAMMO -5
#define NOSHOOT_NOLOAD -6
#define NOSHOOT_NOWEAPON -7

#define PERCENT_TO_IGNORE_THREAT 50  // any less, use threat value
#define ACTION_TIMEOUT_CYCLES 50     // # failed cycles through AI
#define MAX_THREAT_RANGE 400         // 30 tiles worth
#define MIN_PERCENT_BETTER 5         // 5% improvement in cover is good

#define TOSSES_PER_10TURNS 18  // max # of grenades tossable in 10 turns
#define SHELLS_PER_10TURNS 13  // max # of shells   firable  in 10 turns

#define SEE_THRU_COVER_THRESHOLD 5  // min chance to get through

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

#undef max
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  struct SOLDIERTYPE *pOpponent;
  int16_t sGridNo;
  int32_t iValue;
  int32_t iAPs;
  int32_t iCertainty;
  int32_t iOrigRange;
} THREATTYPE;

// define for bAimTime for bursting
#define BURSTING 5

typedef struct {
  uint8_t ubPossible;           // is this attack form possible?  T/F
  uint8_t ubOpponent;           // which soldier is the victim?
  uint8_t ubAimTime;            // how many extra APs to spend on aiming
  uint8_t ubChanceToReallyHit;  // chance to hit * chance to get through cover
  int32_t iAttackValue;         // relative worthiness of this type of attack
  int16_t sTarget;              // target gridno of this attack
  int8_t bTargetLevel;          // target level of this attack
  uint8_t ubAPCost;             // how many APs the attack will use up
  int8_t bWeaponIn;             // the inv slot of the weapon in question
} ATTACKTYPE;

extern THREATTYPE Threat[MAXMERCS];
extern int ThreatPercent[10];
extern uint8_t SkipCoverCheck;
extern int8_t GameOption[MAXGAMEOPTIONS];

typedef enum { SEARCH_GENERAL_ITEMS, SEARCH_AMMO, SEARCH_WEAPONS } ItemSearchReasons;

// go as far as possible flags
#define FLAG_CAUTIOUS 0x01
#define FLAG_STOPSHORT 0x02

#define STOPSHORTDIST 5

int16_t AdvanceToFiringRange(struct SOLDIERTYPE *pSoldier, int16_t sClosestOpponent);

BOOLEAN AimingGun(struct SOLDIERTYPE *pSoldier);
void CalcBestShot(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestShot);
void CalcBestStab(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab, BOOLEAN fBladeAttack);
void CalcBestThrow(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow);
void CalcTentacleAttack(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab);

int16_t CalcSpreadBurst(struct SOLDIERTYPE *pSoldier, int16_t sFirstTarget, int8_t bTargetLevel);
int32_t CalcManThreatValue(struct SOLDIERTYPE *pSoldier, int16_t sMyGrid, uint8_t ubReduceForCover,
                           struct SOLDIERTYPE *pMe);
int8_t CanNPCAttack(struct SOLDIERTYPE *pSoldier);
void CheckIfTossPossible(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow);
BOOLEAN ClimbingNecessary(struct SOLDIERTYPE *pSoldier, int16_t sDestGridNo, int8_t bDestLevel);
int8_t ClosestPanicTrigger(struct SOLDIERTYPE *pSoldier);
int16_t ClosestReachableDisturbance(struct SOLDIERTYPE *pSoldier, uint8_t ubUnconsciousOK,
                                    BOOLEAN *pfChangeLevel);
int16_t ClosestReachableFriendInTrouble(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfClimbingNecessary);
int16_t ClosestSeenOpponent(struct SOLDIERTYPE *pSoldier, int16_t *psGridNo, int8_t *pbLevel);
void CreatureCall(struct SOLDIERTYPE *pCaller);
int8_t CreatureDecideAction(struct SOLDIERTYPE *pCreature);
void CreatureDecideAlertStatus(struct SOLDIERTYPE *pCreature);
int8_t CrowDecideAction(struct SOLDIERTYPE *pSoldier);
void DecideAlertStatus(struct SOLDIERTYPE *pSoldier);
int8_t DecideAutoBandage(struct SOLDIERTYPE *pSoldier);
uint16_t DetermineMovementMode(struct SOLDIERTYPE *pSoldier, int8_t bAction);

int32_t EstimateShotDamage(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOpponent,
                           uint8_t ubChanceToHit);
int32_t EstimateStabDamage(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOpponent,
                           uint8_t ubChanceToHit, BOOLEAN fBladeAttack);
int32_t EstimateThrowDamage(struct SOLDIERTYPE *pSoldier, uint8_t ubItemPos,
                            struct SOLDIERTYPE *pOpponent, int16_t sGridno);
int16_t EstimatePathCostToLocation(struct SOLDIERTYPE *pSoldier, int16_t sDestGridNo,
                                   int8_t bDestLevel, BOOLEAN fAddCostAfterClimbingUp,
                                   BOOLEAN *pfClimbingNecessary, int16_t *psClimbGridNo);

BOOLEAN FindBetterSpotForItem(struct SOLDIERTYPE *pSoldier, int8_t bSlot);
int16_t FindClosestClimbPointAvailableToAI(struct SOLDIERTYPE *pSoldier, int16_t sStartGridNo,
                                           int16_t sDesiredGridNo, BOOLEAN fClimbUp);
int16_t FindRouteBackOntoMap(struct SOLDIERTYPE *pSoldier, int16_t sDestGridNo);
int16_t FindClosestBoxingRingSpot(struct SOLDIERTYPE *pSoldier, BOOLEAN fInRing);
int16_t GetInterveningClimbingLocation(struct SOLDIERTYPE *pSoldier, int16_t sDestGridNo,
                                       int8_t bDestLevel, BOOLEAN *pfClimbingNecessary);
uint8_t GetTraversalQuoteActionID(int8_t bDirection);
int16_t GoAsFarAsPossibleTowards(struct SOLDIERTYPE *pSoldier, int16_t sDesGrid, int8_t bAction);

int8_t HeadForTheStairCase(struct SOLDIERTYPE *pSoldier);

BOOLEAN InGas(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);
BOOLEAN InGasOrSmoke(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);
BOOLEAN InWaterGasOrSmoke(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);

void InitAttackType(ATTACKTYPE *pAttack);

int16_t InternalGoAsFarAsPossibleTowards(struct SOLDIERTYPE *pSoldier, int16_t sDesGrid,
                                         int8_t bReserveAPs, int8_t bAction, int8_t fFlags);

int LegalNPCDestination(struct SOLDIERTYPE *pSoldier, int16_t sGridno, uint8_t ubPathMode,
                        uint8_t ubWaterOK, uint8_t fFlags);
void LoadWeaponIfNeeded(struct SOLDIERTYPE *pSoldier);
int16_t MostImportantNoiseHeard(struct SOLDIERTYPE *pSoldier, int32_t *piRetValue,
                                BOOLEAN *pfClimbingNecessary, BOOLEAN *pfReachable);
int16_t NPCConsiderInitiatingConv(struct SOLDIERTYPE *pNPC, uint8_t *pubDesiredMerc);
void NPCDoesAct(struct SOLDIERTYPE *pSoldier);
void NPCDoesNothing(struct SOLDIERTYPE *pSoldier);
int8_t OKToAttack(struct SOLDIERTYPE *ptr, int target);
BOOLEAN NeedToRadioAboutPanicTrigger(void);
int8_t PointPatrolAI(struct SOLDIERTYPE *pSoldier);
void PossiblyMakeThisEnemyChosenOne(struct SOLDIERTYPE *pSoldier);
int8_t RandomPointPatrolAI(struct SOLDIERTYPE *pSoldier);
int32_t RangeChangeDesire(struct SOLDIERTYPE *pSoldier);
uint16_t RealtimeDelay(struct SOLDIERTYPE *pSoldier);
void RearrangePocket(struct SOLDIERTYPE *pSoldier, int8_t bPocket1, int8_t bPocket2,
                     uint8_t bPermanent);
void RTHandleAI(struct SOLDIERTYPE *pSoldier);
uint16_t RunAway(struct SOLDIERTYPE *pSoldier);
int8_t SearchForItems(struct SOLDIERTYPE *pSoldier, int8_t bReason, uint16_t usItem);
uint8_t ShootingStanceChange(struct SOLDIERTYPE *pSoldier, ATTACKTYPE *pAttack,
                             int8_t bDesiredDirection);
uint8_t StanceChange(struct SOLDIERTYPE *pSoldier, uint8_t ubAttackAPCost);
int16_t TrackScent(struct SOLDIERTYPE *pSoldier);
void RefreshAI(struct SOLDIERTYPE *pSoldier);
BOOLEAN InLightAtNight(int16_t sGridNo, int8_t bLevel);
int16_t FindNearbyDarkerSpot(struct SOLDIERTYPE *pSoldier);

BOOLEAN ArmySeesOpponents(void);
