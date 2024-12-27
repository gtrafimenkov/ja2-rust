#ifndef __STRATEGIC_TOWN_LOYALTY_H
#define __STRATEGIC_TOWN_LOYALTY_H

#include "SGP/Types.h"
#include "Strategic/MapScreen.h"
#include "Town.h"

struct SOLDIERTYPE;

typedef enum {
  // There are only for distance-adjusted global loyalty effects.  Others go into list above
  // instead!
  GLOBAL_LOYALTY_BATTLE_WON,
  GLOBAL_LOYALTY_BATTLE_LOST,
  GLOBAL_LOYALTY_ENEMY_KILLED,
  GLOBAL_LOYALTY_NATIVE_KILLED,
  GLOBAL_LOYALTY_GAIN_TOWN_SECTOR,
  GLOBAL_LOYALTY_LOSE_TOWN_SECTOR,
  GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN,  // awarded only the first time it happens
  GLOBAL_LOYALTY_ABANDON_MILITIA,
  GLOBAL_LOYALTY_GAIN_MINE,
  GLOBAL_LOYALTY_LOSE_MINE,
  GLOBAL_LOYALTY_GAIN_SAM,
  GLOBAL_LOYALTY_LOSE_SAM,
  GLOBAL_LOYALTY_QUEEN_BATTLE_WON,

} GlobalLoyaltyEventTypes;

typedef struct TOWN_LOYALTY {
  uint8_t ubRating;
  int16_t sChange;
  BOOLEAN fStarted;  // starting loyalty of each town is initialized only when player first enters
                     // that town
  uint8_t UNUSEDubRebelSentiment;  // current rebel sentiment.  Events could change the starting
                                   // value...
  BOOLEAN fLiberatedAlready;
  uint8_t filler[19];  // reserved for expansion

} TOWN_LOYALTY;

// the loyalty variables for each town
extern TOWN_LOYALTY gTownLoyalty[NUM_TOWNS];

// whether town maintains/displays loyalty or not
extern BOOLEAN gfTownUsesLoyalty[NUM_TOWNS];

// initialize a specific town's loyalty if it hasn't already been
void StartTownLoyaltyIfFirstTime(TownID bTownId);

// set a speciafied town's loyalty rating
void SetTownLoyalty(TownID bTownId, uint8_t ubLoyaltyValue);

// update the loyalty based on current % control of the town
void UpdateLoyaltyBasedOnControl(TownID bTownId);

// strategic handler, goes through and handles all strategic events for town loyalty
// updates...player controlled, monsters
void HandleTownLoyalty(void);

// init town loyalty lists
void InitTownLoyalty(void);

// handle the death of a civ
void HandleMurderOfCivilian(struct SOLDIERTYPE *pSoldier, BOOLEAN fIntentional);

// handle town loyalty adjustment for recruitment
void HandleTownLoyaltyForNPCRecruitment(struct SOLDIERTYPE *pSoldier);

// handle loyalty adjustment for theft
BOOLEAN HandleLoyaltyAdjustmentForRobbery(struct SOLDIERTYPE *pSoldier);

// handle loyalty adjustments for dmg/destruction of buildings
void HandleLoyaltyForDemolitionOfBuilding(struct SOLDIERTYPE *pSoldier, int16_t sPointsDmg);

// remove random item from this sector
void RemoveRandomItemsInSector(uint8_t sSectorX, uint8_t sSectorY, int16_t sSectorZ,
                               uint8_t ubChance);

// get the shortest distance between these two towns via roads
int32_t GetTownDistances(uint8_t ubTown, uint8_t ubTownA);

#ifdef JA2TESTVERSION

// calculate shortest distances between towns
void CalcDistancesBetweenTowns(void);

// write out distances between towns to a data file
void WriteOutDistancesBetweenTowns(void);

// dumps the calc'ed distances into a text file table for easy verification
void DumpDistancesBetweenTowns(void);

#endif  // JA2TESTVERSION

// read in distances between towns
void ReadInDistancesBetweenTowns(void);

/* Delayed loyalty effects elimininated.  Sep.12/98.  ARM
// delayed town loyalty event
void HandleDelayedTownLoyaltyEvent( uint32_t uiValue );
// build loyalty event value
uint32_t BuildLoyaltyEventValue( TownID bTownId, uint32_t uiValue, BOOLEAN fIncrement );
*/

BOOLEAN LoadStrategicTownLoyaltyFromSavedGameFile(HWFILE hFile);
BOOLEAN SaveStrategicTownLoyaltyToSaveGameFile(HWFILE hFile);

void ReduceLoyaltyForRebelsBetrayed(void);

// how many towns under player control?
int32_t GetNumberOfWholeTownsUnderControl(void);

// is all the sectors of this town under control by the player
int32_t IsTownUnderCompleteControlByPlayer(TownID bTownId);

// used when monsters attack a town sector without going through tactical and they win
void AdjustLoyaltyForCivsEatenByMonsters(uint8_t sSectorX, uint8_t sSectorY, uint8_t ubHowMany);

// these are used to handle global loyalty events (ones that effect EVERY town on the map)
void IncrementTownLoyaltyEverywhere(uint32_t uiLoyaltyIncrease);
void DecrementTownLoyaltyEverywhere(uint32_t uiLoyaltyDecrease);
void HandleGlobalLoyaltyEvent(uint8_t ubEventType, uint8_t sSectorX, uint8_t sSectorY,
                              int8_t bSectorZ);
void AffectAllTownsLoyaltyByDistanceFrom(int32_t iLoyaltyChange, uint8_t sSectorX, uint8_t sSectorY,
                                         int8_t bSectorZ);

// handle a town being liberated for the first time
void CheckIfEntireTownHasBeenLiberated(TownID bTownId, uint8_t sSectorX, uint8_t sSectorY);
void CheckIfEntireTownHasBeenLost(TownID bTownId, uint8_t sSectorX, uint8_t sSectorY);

void HandleLoyaltyChangeForNPCAction(uint8_t ubNPCProfileId);

BOOLEAN DidFirstBattleTakePlaceInThisTown(TownID bTownId);
void SetTheFirstBattleSector(int16_t sSectorValue);

// gte number of whole towns but exclude this one
int32_t GetNumberOfWholeTownsUnderControlButExcludeCity(int8_t bCityToExclude);

// Function assumes that mercs have retreated already.  Handles two cases, one for general merc
// retreat which slightly demoralizes the mercs, the other handles abandonment of militia forces
// which poses as a serious loyalty penalty.

#define RETREAT_TACTICAL_TRAVERSAL 0
#define RETREAT_PBI 1
#define RETREAT_AUTORESOLVE 2
void HandleLoyaltyImplicationsOfMercRetreat(int8_t bRetreatCode, uint8_t sSectorX, uint8_t sSectorY,
                                            int16_t sSectorZ);

void MaximizeLoyaltyForDeidrannaKilled(void);

#endif
