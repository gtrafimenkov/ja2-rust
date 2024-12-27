#ifndef _SOLDIER_ADD_H
#define _SOLDIER_ADD_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// Finds a gridno given a sweet spot
// Returns a good direction too!
uint16_t FindGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo, INT8 ubRadius,
                                 UINT8 *pubDirection);

// Ensures a good path.....
uint16_t FindGridNoFromSweetSpotThroughPeople(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                              INT8 ubRadius, UINT8 *pubDirection);

// Returns a good sweetspot but not the swetspot!
uint16_t FindGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                                   INT8 ubRadius, UINT8 *pubDirection);

uint16_t FindGridNoFromSweetSpotExcludingSweetSpotInQuardent(struct SOLDIERTYPE *pSoldier,
                                                             INT16 sSweetGridNo, INT8 ubRadius,
                                                             UINT8 *pubDirection,
                                                             INT8 ubQuardentDir);

// Finds a gridno near a sweetspot but a random one!
uint16_t FindRandomGridNoFromSweetSpot(struct SOLDIERTYPE *pSoldier, INT16 sSweetGridNo,
                                       INT8 ubRadius, UINT8 *pubDirection);

// Finds a sweetspot but excluding this one!
uint16_t FindRandomGridNoFromSweetSpotExcludingSweetSpot(struct SOLDIERTYPE *pSoldier,
                                                         INT16 sSweetGridNo, INT8 ubRadius,
                                                         UINT8 *pubDirection);

// Adds a soldier ( already created in mercptrs[] array )!
// Finds a good placement based on data in the loaded sector and if they are enemy's or not, etc...
BOOLEAN AddSoldierToSector(UINT8 ubID);

BOOLEAN AddSoldierToSectorNoCalculateDirection(UINT8 ubID);

BOOLEAN AddSoldierToSectorNoCalculateDirectionUseAnimation(UINT8 ubID, uint16_t usAnimState,
                                                           uint16_t usAnimCode);

// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(UINT8 ubMercID);
// requires non-intransit assignment, too
BOOLEAN IsMercOnTeamAndInOmertaAlready(UINT8 ubMercID);
// ATE: Added for contract renewals
BOOLEAN IsMercOnTeamAndAlive(UINT8 ubMercID);
// ATE: Added for contract renewals
BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(UINT8 ubMercID);

// GetSoldierIDFromMercID() Gets the Soldier ID from the Merc Profile ID, else returns -1
INT16 GetSoldierIDFromMercID(UINT8 ubMercID);

uint16_t FindGridNoFromSweetSpotWithStructData(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                               INT16 sSweetGridNo, INT8 ubRadius,
                                               UINT8 *pubDirection, BOOLEAN fClosestToMerc);

/*
void SoldierInSectorSleep( struct SOLDIERTYPE *pSoldier, INT16 sGridNo );
*/

uint16_t FindGridNoFromSweetSpotWithStructDataFromSoldier(struct SOLDIERTYPE *pSoldier,
                                                          uint16_t usAnimState, INT8 ubRadius,
                                                          UINT8 *pubDirection,
                                                          BOOLEAN fClosestToMerc,
                                                          struct SOLDIERTYPE *pSrcSoldier);

void SoldierInSectorPatient(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
void SoldierInSectorDoctor(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
void SoldierInSectorRepair(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

BOOLEAN CanSoldierReachGridNoInGivenTileLimit(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                              INT16 sMaxTiles, INT8 bLevel);

#endif
