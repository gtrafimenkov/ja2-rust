#ifndef _OPPLIST_H
#define _OPPLIST_H

#include "SGP/Types.h"
#include "Tactical/OverheadTypes.h"

struct SOLDIERTYPE;

#define EVERYBODY MAXMERCS

#define MAX_MISC_NOISE_DURATION 12  // max dur for VERY loud NOBODY noises

#define DOOR_NOISE_VOLUME 2
#define WINDOW_CRACK_VOLUME 4
#define WINDOW_SMASH_VOLUME 8
#define MACHETE_VOLUME 9
#define TRIMMER_VOLUME 18
#define CHAINSAW_VOLUME 30
#define SMASHING_DOOR_VOLUME 6
#define CROWBAR_DOOR_VOLUME 4
#define ITEM_THROWN_VOLUME 2

#define TIME_BETWEEN_RT_OPPLIST_DECAYS 20

// this is a fake "level" value (0 on ground, 1 on roof) for
// HearNoise to ignore the effects of lighting(?)
#define LIGHT_IRRELEVANT 127

#define AUTOMATIC_INTERRUPT 100
#define NO_INTERRUPT 127

#define MOVEINTERRUPT 0
#define SIGHTINTERRUPT 1
#define NOISEINTERRUPT 2

// noise type constants
enum {
  NOISE_UNKNOWN = 0,
  NOISE_MOVEMENT,
  NOISE_CREAKING,
  NOISE_SPLASHING,
  NOISE_BULLET_IMPACT,
  NOISE_GUNFIRE,
  NOISE_EXPLOSION,
  NOISE_SCREAM,
  NOISE_ROCK_IMPACT,
  NOISE_GRENADE_IMPACT,
  NOISE_WINDOW_SMASHING,
  NOISE_DOOR_SMASHING,
  NOISE_SILENT_ALARM,  // only heard by enemies
  MAX_NOISES
};

enum {
  EXPECTED_NOSEND,  // other nodes expecting noise & have all info
  EXPECTED_SEND,    // other nodes expecting noise, but need info
  UNEXPECTED        // other nodes are NOT expecting this noise
};

#define NUM_WATCHED_LOCS 3

extern int8_t gbPublicOpplist[MAXTEAMS][TOTAL_SOLDIERS];
extern int8_t gbSeenOpponents[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
extern INT16 gsLastKnownOppLoc[TOTAL_SOLDIERS][TOTAL_SOLDIERS];  // merc vs. merc
extern int8_t gbLastKnownOppLevel[TOTAL_SOLDIERS][TOTAL_SOLDIERS];
extern INT16 gsPublicLastKnownOppLoc[MAXTEAMS][TOTAL_SOLDIERS];  // team vs. merc
extern int8_t gbPublicLastKnownOppLevel[MAXTEAMS][TOTAL_SOLDIERS];
extern uint8_t gubPublicNoiseVolume[MAXTEAMS];
extern INT16 gsPublicNoiseGridno[MAXTEAMS];
extern int8_t gbPublicNoiseLevel[MAXTEAMS];
extern uint8_t gubKnowledgeValue[10][10];
extern int8_t gbLookDistance[8][8];
extern int8_t gfKnowAboutOpponents;

extern BOOLEAN gfPlayerTeamSawJoey;
extern BOOLEAN gfMikeShouldSayHi;

extern INT16 gsWatchedLoc[TOTAL_SOLDIERS][NUM_WATCHED_LOCS];
extern int8_t gbWatchedLocLevel[TOTAL_SOLDIERS][NUM_WATCHED_LOCS];
extern uint8_t gubWatchedLocPoints[TOTAL_SOLDIERS][NUM_WATCHED_LOCS];
extern BOOLEAN gfWatchedLocReset[TOTAL_SOLDIERS][NUM_WATCHED_LOCS];

#define BEST_SIGHTING_ARRAY_SIZE 6
#define BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL 6
#define BEST_SIGHTING_ARRAY_SIZE_NONCOMBAT 3
#define BEST_SIGHTING_ARRAY_SIZE_INCOMBAT 0
extern uint8_t gubBestToMakeSightingSize;

INT16 AdjustMaxSightRangeForEnvEffects(struct SOLDIERTYPE *pSoldier, int8_t bLightLevel,
                                       INT16 sDistVisible);
INT16 ManLooksForMan(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOpponent, uint8_t ubCaller);
void HandleSight(struct SOLDIERTYPE *pSoldier, uint8_t ubSightFlags);
void AllTeamsLookForAll(uint8_t ubAllowInterrupts);
void GloballyDecideWhoSeesWho(void);
uint16_t GetClosestMerc(uint16_t usSoldierIndex);
void ManLooksForOtherTeams(struct SOLDIERTYPE *pSoldier);
void OtherTeamsLookForMan(struct SOLDIERTYPE *pOpponent);
void ManSeesMan(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOpponent, INT16 sOppGridno,
                int8_t bOppLevel, uint8_t ubCaller, uint8_t ubCaller2);
void DecideTrueVisibility(struct SOLDIERTYPE *pSoldier, uint8_t ubLocate);
void AddOneOpponent(struct SOLDIERTYPE *pSoldier);
void RemoveOneOpponent(struct SOLDIERTYPE *pSoldier);
void UpdatePersonal(struct SOLDIERTYPE *pSoldier, uint8_t ubID, int8_t bNewOpplist, INT16 sGridno,
                    int8_t bLevel);
INT16 MaxDistanceVisible(void);
INT16 DistanceVisible(struct SOLDIERTYPE *pSoldier, int8_t bFacingDir, int8_t bSubjectDir,
                      INT16 sSubjectGridNo, int8_t bLevel);
void ResetLastKnownLocs(struct SOLDIERTYPE *ptr);
void RecalculateOppCntsDueToNoLongerNeutral(struct SOLDIERTYPE *pSoldier);

void InitOpponentKnowledgeSystem(void);
void InitSoldierOppList(struct SOLDIERTYPE *pSoldier);
void BetweenTurnsVisibilityAdjustments(void);
void RemoveManAsTarget(struct SOLDIERTYPE *pSoldier);
void UpdatePublic(uint8_t ubTeam, uint8_t ubID, int8_t bNewOpplist, INT16 sGridno, int8_t bLevel);
void RadioSightings(struct SOLDIERTYPE *pSoldier, uint8_t ubAbout, uint8_t ubTeamToRadioTo);
void OurTeamRadiosRandomlyAbout(uint8_t ubAbout);
void DebugSoldierPage1();
void DebugSoldierPage2();
void DebugSoldierPage3();
void DebugSoldierPage4();

uint8_t MovementNoise(struct SOLDIERTYPE *pSoldier);
uint8_t DoorOpeningNoise(struct SOLDIERTYPE *pSoldier);
void MakeNoise(uint8_t ubNoiseMaker, INT16 sGridNo, int8_t bLevel, uint8_t ubTerrType,
               uint8_t ubVolume, uint8_t ubNoiseType);
void OurNoise(uint8_t ubNoiseMaker, INT16 sGridNo, int8_t bLevel, uint8_t ubTerrType,
              uint8_t ubVolume, uint8_t ubNoiseType);

void ResolveInterruptsVs(struct SOLDIERTYPE *pSoldier, uint8_t ubInterruptType);

void VerifyAndDecayOpplist(struct SOLDIERTYPE *pSoldier);
void DecayIndividualOpplist(struct SOLDIERTYPE *pSoldier);
void VerifyPublicOpplistDueToDeath(struct SOLDIERTYPE *pSoldier);
void NoticeUnseenAttacker(struct SOLDIERTYPE *pAttacker, struct SOLDIERTYPE *pDefender,
                          int8_t bReason);

BOOLEAN MercSeesCreature(struct SOLDIERTYPE *pSoldier);

int8_t GetWatchedLocPoints(uint8_t ubID, INT16 sGridNo, int8_t bLevel);
int8_t GetHighestVisibleWatchedLoc(uint8_t ubID);
int8_t GetHighestWatchedLocPoints(uint8_t ubID);

void TurnOffEveryonesMuzzleFlashes(void);
void TurnOffTeamsMuzzleFlashes(uint8_t ubTeam);
void EndMuzzleFlash(struct SOLDIERTYPE *pSoldier);
void NonCombatDecayPublicOpplist(uint32_t uiTime);

void CheckHostileOrSayQuoteList(void);
void InitOpplistForDoorOpening(void);
uint8_t DoorOpeningNoise(struct SOLDIERTYPE *pSoldier);

void AddToShouldBecomeHostileOrSayQuoteList(uint8_t ubID);

extern int8_t gbLightSighting[1][16];

#endif
