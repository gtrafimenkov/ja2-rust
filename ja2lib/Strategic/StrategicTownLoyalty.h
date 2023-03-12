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

// initialize a specific town's loyalty if it hasn't already been
void StartTownLoyaltyIfFirstTime(TownID bTownId);

// update the loyalty based on current % control of the town
void UpdateLoyaltyBasedOnControl(TownID bTownId);

// handle the death of a civ
void HandleMurderOfCivilian(struct SOLDIERTYPE *pSoldier, BOOLEAN fIntentional);

// handle town loyalty adjustment for recruitment
void HandleTownLoyaltyForNPCRecruitment(struct SOLDIERTYPE *pSoldier);

// handle loyalty adjustment for theft
BOOLEAN HandleLoyaltyAdjustmentForRobbery(struct SOLDIERTYPE *pSoldier);

// handle loyalty adjustments for dmg/destruction of buildings
void HandleLoyaltyForDemolitionOfBuilding(struct SOLDIERTYPE *pSoldier, INT16 sPointsDmg);

// remove random item from this sector
void RemoveRandomItemsInSector(u8 sSectorX, u8 sSectorY, INT16 sSectorZ, UINT8 ubChance);

// get the shortest distance between these two towns via roads
INT32 GetTownDistances(UINT8 ubTown, UINT8 ubTownA);

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
void HandleDelayedTownLoyaltyEvent( UINT32 uiValue );
// build loyalty event value
UINT32 BuildLoyaltyEventValue( TownID bTownId, UINT32 uiValue, BOOLEAN fIncrement );
*/

BOOLEAN LoadStrategicTownLoyaltyFromSavedGameFile(HWFILE hFile);
BOOLEAN SaveStrategicTownLoyaltyToSaveGameFile(HWFILE hFile);

void ReduceLoyaltyForRebelsBetrayed(void);

// how many towns under player control?
INT32 GetNumberOfWholeTownsUnderControl(void);

// is all the sectors of this town under control by the player
INT32 IsTownUnderCompleteControlByPlayer(TownID bTownId);

// used when monsters attack a town sector without going through tactical and they win
void AdjustLoyaltyForCivsEatenByMonsters(u8 sSectorX, u8 sSectorY, UINT8 ubHowMany);

// these are used to handle global loyalty events (ones that effect EVERY town on the map)
void IncrementTownLoyaltyEverywhere(UINT32 uiLoyaltyIncrease);
void DecrementTownLoyaltyEverywhere(UINT32 uiLoyaltyDecrease);
void HandleGlobalLoyaltyEvent(UINT8 ubEventType, u8 sSectorX, u8 sSectorY, INT8 bSectorZ);
void AffectAllTownsLoyaltyByDistanceFrom(INT32 iLoyaltyChange, u8 sSectorX, u8 sSectorY,
                                         INT8 bSectorZ);

// handle a town being liberated for the first time
void CheckIfEntireTownHasBeenLiberated(TownID bTownId, u8 sSectorX, u8 sSectorY);
void CheckIfEntireTownHasBeenLost(TownID bTownId, u8 sSectorX, u8 sSectorY);

void HandleLoyaltyChangeForNPCAction(UINT8 ubNPCProfileId);

BOOLEAN DidFirstBattleTakePlaceInThisTown(TownID bTownId);
void SetTheFirstBattleSector(INT16 sSectorValue);

// gte number of whole towns but exclude this one
INT32 GetNumberOfWholeTownsUnderControlButExcludeCity(INT8 bCityToExclude);

// Function assumes that mercs have retreated already.  Handles two cases, one for general merc
// retreat which slightly demoralizes the mercs, the other handles abandonment of militia forces
// which poses as a serious loyalty penalty.

#define RETREAT_TACTICAL_TRAVERSAL 0
#define RETREAT_PBI 1
#define RETREAT_AUTORESOLVE 2
void HandleLoyaltyImplicationsOfMercRetreat(INT8 bRetreatCode, u8 sSectorX, u8 sSectorY,
                                            INT16 sSectorZ);

void MaximizeLoyaltyForDeidrannaKilled(void);

#endif
