#ifndef __TOWN_H
#define __TOWN_H

#include "LeanTypes.h"
#include "Sector.h"

// Sector name identifiers.  Also town names.
typedef enum {
  BLANK_SECTOR = 0,
  OMERTA,
  DRASSEN,
  ALMA,
  GRUMM,
  TIXA,
  CAMBRIA,
  SAN_MONA,
  ESTONI,
  ORTA,
  BALIME,
  MEDUNA,
  CHITZENA,
  NUM_TOWNS
} TownID;

#define FIRST_TOWN OMERTA

int8_t GetTownIdForSector(uint8_t sMapX, uint8_t sMapY);
int8_t GetTownIdForStrategicMapIndex(int32_t index);

// Mapping townID -> sectorID
struct TownSector {
  TownID townID;
  SectorID16 sectorID;
};

// Mapping of all town sectors.  There may be max 40 mappings.
// The mapping is over, when townID is BLANK_SECTOR.
typedef struct TownSector TownSectors[40];

const TownSectors* GetAllTownSectors();

// build list of town sectors
void BuildListOfTownSectors();

// gain pts per real loyalty pt
#define GAIN_PTS_PER_LOYALTY_PT 500

// --- LOYALTY BONUSES ---
// Omerta
#define LOYALTY_BONUS_MIGUEL_READS_LETTER \
  (10 * GAIN_PTS_PER_LOYALTY_PT)  // multiplied by 4.5 due to Omerta's high seniment, so it's 45%
// Drassen
#define LOYALTY_BONUS_CHILDREN_FREED_DOREEN_KILLED \
  (10 * GAIN_PTS_PER_LOYALTY_PT)  // +50% bonus for Drassen
#define LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // +50% bonus for Drassen
// Cambria
#define LOYALTY_BONUS_MARTHA_WHEN_JOEY_RESCUED \
  (15 * GAIN_PTS_PER_LOYALTY_PT)  // -25% for low Cambria sentiment
#define LOYALTY_BONUS_KEITH_WHEN_HILLBILLY_SOLVED \
  (15 * GAIN_PTS_PER_LOYALTY_PT)  // -25% for low Cambria sentiment
// Chitzena
#define LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_LOCAL \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // +75% higher in Chitzena
#define LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_GLOBAL \
  (10 * GAIN_PTS_PER_LOYALTY_PT)  // for ALL towns!
// Alma
#define LOYALTY_BONUS_AUNTIE_WHEN_BLOODCATS_KILLED \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // Alma's increases reduced by half due to low rebel sentiment
#define LOYALTY_BONUS_MATT_WHEN_DYNAMO_FREED \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // Alma's increases reduced by half due to low rebel sentiment
#define LOYALTY_BONUS_FOR_SERGEANT_KROTT \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // Alma's increases reduced by half due to low rebel sentiment
// Everywhere
#define LOYALTY_BONUS_TERRORISTS_DEALT_WITH (5 * GAIN_PTS_PER_LOYALTY_PT)
#define LOYALTY_BONUS_KILL_QUEEN_MONSTER (10 * GAIN_PTS_PER_LOYALTY_PT)
// Anywhere
// loyalty bonus for completing town training
#define LOYALTY_BONUS_FOR_TOWN_TRAINING (2 * GAIN_PTS_PER_LOYALTY_PT)  // 2%

// --- LOYALTY PENALTIES ---
// Cambria
#define LOYALTY_PENALTY_MARTHA_HEART_ATTACK (20 * GAIN_PTS_PER_LOYALTY_PT)
#define LOYALTY_PENALTY_JOEY_KILLED (10 * GAIN_PTS_PER_LOYALTY_PT)
// Balime
#define LOYALTY_PENALTY_ELDIN_KILLED (20 * GAIN_PTS_PER_LOYALTY_PT)  // effect is double that!
// Any mine
#define LOYALTY_PENALTY_HEAD_MINER_ATTACKED \
  (20 * GAIN_PTS_PER_LOYALTY_PT)  // exact impact depends on rebel sentiment in that town
// Loyalty penalty for being inactive, per day after the third
#define LOYALTY_PENALTY_INACTIVE (10 * GAIN_PTS_PER_LOYALTY_PT)

// increment the town loyalty rating (hundredths!)
void IncrementTownLoyalty(TownID bTownId, uint32_t uiLoyaltyIncrease);

// decrement the town loyalty rating (hundredths!)
void DecrementTownLoyalty(TownID bTownId, uint32_t uiLoyaltyDecrease);

#endif
