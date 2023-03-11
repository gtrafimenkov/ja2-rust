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

void StrategicRemoveMilitiaFromSector(u8 mapX, u8 mapY, UINT8 ubRank, UINT8 ubHowMany);

// this will check for promotions and handle them for you
UINT8 CheckOneMilitiaForPromotion(u8 mapX, u8 mapY, UINT8 ubCurrentRank, UINT8 ubRecentKillPts);

void BuildMilitiaPromotionsString(CHAR16 *str, size_t bufSize);

INT32 GetNumberOfMilitiaInSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

bool IsMilitiaTrainingPayedForSector(u8 mapX, u8 mapY);
void SetMilitiaTrainingPayedForSector(u8 mapX, u8 mapY, bool value);

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

BOOLEAN MilitiaTrainingAllowedInSector(u8 mapX, u8 mapY, INT8 bSectorZ);

void HandleMilitiaPromotions(u8 mapX, u8 mapY);
void HandleMilitiaDefections(u8 mapX, u8 mapY);

void PrepMilitiaPromotion();
void HandleSingleMilitiaPromotion(u8 mapX, u8 mapY, u8 soldierClass, u8 kills);
bool HasNewMilitiaPromotions();

BOOLEAN DoesPlayerHaveAnyMilitia();

#endif
