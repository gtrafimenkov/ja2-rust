#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

// header for town militia strategic control module

#include "SGP/Types.h"
#include "Town.h"

struct SOLDIERTYPE;

// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20

struct MilitiaCount {
  u8 green;
  u8 regular;
  u8 elite;
};

// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, u8 mapX, u8 mapY);

void StrategicRemoveMilitiaFromSector(u8 mapX, u8 mapY, uint8_t ubRank, uint8_t ubHowMany);

// this will check for promotions and handle them for you
uint8_t CheckOneMilitiaForPromotion(u8 mapX, u8 mapY, uint8_t ubCurrentRank,
                                    uint8_t ubRecentKillPts);

void BuildMilitiaPromotionsString(wchar_t *str, size_t bufSize);

uint8_t CountAllMilitiaInSector(u8 mapX, u8 mapY);
uint8_t CountAllMilitiaInSectorID8(SectorID8 sectorID);
int32_t GetNumberOfMilitiaInSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ);
struct MilitiaCount GetMilitiaInSector(u8 mapX, u8 mapY);
struct MilitiaCount GetMilitiaInSectorID8(SectorID8 sectorID);
u8 GetMilitiaOfRankInSector(u8 mapX, u8 mapY, u8 ubRank);
void SetMilitiaOfRankInSector(u8 mapX, u8 mapY, u8 ubRank, u8 count);
void IncMilitiaOfRankInSector(u8 mapX, u8 mapY, u8 ubRank, u8 increase);
void SetMilitiaInSector(u8 mapX, u8 mapY, struct MilitiaCount newCount);
void SetMilitiaInSectorID8(SectorID8 sectorID, struct MilitiaCount newCount);

bool IsMilitiaTrainingPayedForSector(u8 mapX, u8 mapY);
bool IsMilitiaTrainingPayedForSectorID8(SectorID8 sectorID);
void SetMilitiaTrainingPayedForSectorID8(SectorID8 sectorID, bool value);

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
BOOLEAN MilitiaTrainingAllowedInTown(TownID bTownId);

void HandleMilitiaPromotions(u8 mapX, u8 mapY);
void HandleMilitiaDefections(u8 mapX, u8 mapY);

void PrepMilitiaPromotion();
void HandleSingleMilitiaPromotion(u8 mapX, u8 mapY, u8 soldierClass, u8 kills);
bool HasNewMilitiaPromotions();

BOOLEAN DoesPlayerHaveAnyMilitia();

#endif
