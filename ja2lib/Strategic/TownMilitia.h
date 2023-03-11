#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

// header for town militia strategic control module

#include "SGP/Types.h"
#include "Sector.h"
#include "Town.h"

struct SOLDIERTYPE;

// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20

struct MilitiaCount {
  uint8_t green;
  uint8_t regular;
  uint8_t elite;
};

// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, uint8_t mapX, uint8_t mapY);

void StrategicRemoveMilitiaFromSector(uint8_t mapX, uint8_t mapY, uint8_t ubRank,
                                      uint8_t ubHowMany);

// this will check for promotions and handle them for you
uint8_t CheckOneMilitiaForPromotion(uint8_t mapX, uint8_t mapY, uint8_t ubCurrentRank,
                                    uint8_t ubRecentKillPts);

void BuildMilitiaPromotionsString(wchar_t *str, size_t bufSize);

uint8_t CountAllMilitiaInSector(uint8_t mapX, uint8_t mapY);
uint8_t CountAllMilitiaInSectorID8(SectorID8 sectorID);
int32_t GetNumberOfMilitiaInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);
struct MilitiaCount GetMilitiaInSector(uint8_t mapX, uint8_t mapY);
struct MilitiaCount GetMilitiaInSectorID8(SectorID8 sectorID);
uint8_t GetMilitiaOfRankInSector(uint8_t mapX, uint8_t mapY, uint8_t ubRank);
void SetMilitiaOfRankInSector(uint8_t mapX, uint8_t mapY, uint8_t ubRank, uint8_t count);
void IncMilitiaOfRankInSector(uint8_t mapX, uint8_t mapY, uint8_t ubRank, uint8_t increase);
void SetMilitiaInSector(uint8_t mapX, uint8_t mapY, struct MilitiaCount newCount);
void SetMilitiaInSectorID8(SectorID8 sectorID, struct MilitiaCount newCount);

bool IsMilitiaTrainingPayedForSector(uint8_t mapX, uint8_t mapY);
bool IsMilitiaTrainingPayedForSectorID8(SectorID8 sectorID);
void SetMilitiaTrainingPayedForSectorID8(SectorID8 sectorID, bool value);

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
