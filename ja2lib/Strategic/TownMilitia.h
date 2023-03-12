#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

#include "SGP/Types.h"
#include "Sector.h"
#include "Town.h"
#include "rust_militia.h"

struct SOLDIERTYPE;

// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20

// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, uint8_t mapX, uint8_t mapY);

// this will check for promotions and handle them for you
uint8_t CheckOneMilitiaForPromotion(uint8_t mapX, uint8_t mapY, uint8_t ubCurrentRank,
                                    uint8_t ubRecentKillPts);

void BuildMilitiaPromotionsString(wchar_t *str, size_t bufSize);

// tell player how much it will cost
void HandleInterfaceMessageForCostOfTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// continue training?
void HandleInterfaceMessageForContinuingTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// is there a town with militia here or nearby?
BOOLEAN CanNearbyMilitiaScoutThisSector(uint8_t mapX, uint8_t mapY);

// is the town militia full?
BOOLEAN IsTownFullMilitia(TownID bTownId);
// is the SAM site full of militia?
BOOLEAN IsSAMSiteFullOfMilitia(uint8_t mapX, uint8_t mapY);

// now that town training is complete, handle the continue boxes
void HandleContinueOfTownTraining(void);

// clear the list of training completed sectors
void ClearSectorListForCompletedTrainingOfMilitia(void);

BOOLEAN MilitiaTrainingAllowedInSector(uint8_t mapX, uint8_t mapY, int8_t bSectorZ);

void HandleMilitiaPromotions(uint8_t mapX, uint8_t mapY);
void HandleMilitiaDefections(uint8_t mapX, uint8_t mapY);

void PrepMilitiaPromotion();
void HandleSingleMilitiaPromotion(uint8_t mapX, uint8_t mapY, uint8_t soldierClass, uint8_t kills);
bool HasNewMilitiaPromotions();

BOOLEAN DoesPlayerHaveAnyMilitia();

#endif
