// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

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
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, u8 mapX, u8 mapY);

// this will check for promotions and handle them for you
uint8_t CheckOneMilitiaForPromotion(u8 mapX, u8 mapY, uint8_t ubCurrentRank, uint8_t ubRecentKillPts);

void BuildMilitiaPromotionsString(wchar_t *str, size_t bufSize);

// tell player how much it will cost
void HandleInterfaceMessageForCostOfTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// continue training?
void HandleInterfaceMessageForContinuingTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// is there a town with militia here or nearby?
BOOLEAN CanNearbyMilitiaScoutThisSector(u8 mapX, u8 mapY);

// is the town militia full?
BOOLEAN IsTownFullMilitia(TownID bTownId);
// is the SAM site full of militia?
BOOLEAN IsSAMSiteFullOfMilitia(u8 mapX, u8 mapY);

// now that town training is complete, handle the continue boxes
void HandleContinueOfTownTraining(void);

// clear the list of training completed sectors
void ClearSectorListForCompletedTrainingOfMilitia(void);

BOOLEAN MilitiaTrainingAllowedInSector(u8 mapX, u8 mapY, int8_t bSectorZ);

void HandleMilitiaPromotions(u8 mapX, u8 mapY);
void HandleMilitiaDefections(u8 mapX, u8 mapY);

void PrepMilitiaPromotion();
void HandleSingleMilitiaPromotion(u8 mapX, u8 mapY, u8 soldierClass, u8 kills);
bool HasNewMilitiaPromotions();

BOOLEAN DoesPlayerHaveAnyMilitia();

#endif
