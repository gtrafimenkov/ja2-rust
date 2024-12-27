#ifndef _STRATEGIC_TOWN_REPUTATION_H
#define _STRATEGIC_TOWN_REPUTATION_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// header to contain functions to handle strategic town reputation

// initialize profiles for the town reputation system
void InitializeProfilesForTownReputation(void);

// post events to handle spread of town loyalty
void PostEventsForSpreadOfTownOpinion(void);

// get the towns opinion of this merc...indexed by profile type
uint8_t GetTownOpinionOfMerc(uint8_t ubProfileId, uint8_t ubTownId);
uint8_t GetTownOpinionOfMercForSoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubTownId);

// update merc reputation for this town by this amount
void UpdateTownOpinionOfThisMerc(uint8_t ubProfileId, uint8_t ubTownId, int8_t bAmount);
void UpdateTownOpinionOfThisMercForSoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubTownId,
                                           int8_t bAmount);

// global handling of spread
void HandleSpreadOfAllTownsOpinion(void);
// handles the spread of town opinion for one merc
void HandleSpreadOfTownOpinionForMerc(uint8_t ubProfileId);
// handles the opinion spread between these two towns about this merc at the distance between them
void HandleOpinionOfTownsAboutSoldier(int8_t bTownA, int8_t bTownB, INT32 iDistanceBetweenThem,
                                      uint8_t ubProfile);

/*
// handle only for mercs on players team
void HandleSpreadOfTownsOpinionForCurrentMercs( void );
void HandleSpreadOfTownOpinionForMercForSoldier( struct SOLDIERTYPE *pSoldier );
*/

#endif
