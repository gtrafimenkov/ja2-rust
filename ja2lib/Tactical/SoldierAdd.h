#ifndef _SOLDIER_ADD_H
#define _SOLDIER_ADD_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// Finds a gridno given a sweet spot
// Returns a good direction too!
uint16_t FindGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo, INT8 ubRadius,
                                 uint8_t *pubDirection);

// Ensures a good path.....
uint16_t FindGridNoFromSweetSpotThroughPeople(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                              INT8 ubRadius, uint8_t *pubDirection);

// Returns a good sweetspot but not the swetspot!
uint16_t FindGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                                   INT8 ubRadius, uint8_t *pubDirection);

uint16_t FindGridNoFromSweetSpotExcludingSweetSpotInQuardent(struct SOLDIERTYPE *pSoldier,
                                                             INT16 sSweetGridNo, INT8 ubRadius,
                                                             uint8_t *pubDirection,
                                                             INT8 ubQuardentDir);

// Finds a gridno near a sweetspot but a random one!
uint16_t FindRandomGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                       INT8 ubRadius, uint8_t *pubDirection);

// Finds a sweetspot but excluding this one!
uint16_t FindRandomGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier,
                                                         INT16 sSweetGridNo, INT8 ubRadius,
                                                         uint8_t *pubDirection);

// Adds a soldier ( already created in mercptrs[] array )!
// Finds a good placement based on data in the loaded sector and if they are enemy's or not, etc...
BOOLEAN AddSoldierToSector(uint8_t ubID);

BOOLEAN AddSoldierToSectorNoCalculateDirection(uint8_t ubID);

BOOLEAN AddSoldierToSectorNoCalculateDirectionUseAnimation(uint8_t ubID, uint16_t usAnimState,
                                                           uint16_t usAnimCode);

// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(uint8_t ubMercID);
// requires non-intransit assignment, too
BOOLEAN IsMercOnTeamAndInOmertaAlready(uint8_t ubMercID);
// ATE: Added for contract renewals
BOOLEAN IsMercOnTeamAndAlive(uint8_t ubMercID);
// ATE: Added for contract renewals
BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(uint8_t ubMercID);

// GetSoldierIDFromMercID() Gets the Soldier ID from the Merc Profile ID, else returns -1
INT16 GetSoldierIDFromMercID(uint8_t ubMercID);

uint16_t FindGridNoFromSweetSpotWithStructData(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                               INT16 sSweetGridNo, INT8 ubRadius,
                                               uint8_t *pubDirection, BOOLEAN fClosestToMerc);

/*
void SoldierInSectorSleep( struct SOLDIERTYPE *pSoldier, INT16 sGridNo );
*/

uint16_t FindGridNoFromSweetSpotWithStructDataFromSoldier(struct SOLDIERTYPE *pSoldier,
                                                          uint16_t usAnimState, INT8 ubRadius,
                                                          uint8_t *pubDirection,
                                                          BOOLEAN fClosestToMerc,
                                                          struct SOLDIERTYPE *pSrcSoldier);

void SoldierInSectorPatient(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
void SoldierInSectorDoctor(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
void SoldierInSectorRepair(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

BOOLEAN CanSoldierReachGridNoInGivenTileLimit(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                              INT16 sMaxTiles, INT8 bLevel);

#endif
