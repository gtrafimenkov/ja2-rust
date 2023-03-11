#ifndef __SOLDIER_H
#define __SOLDIER_H

#include "SGP/Types.h"
#include "Sector.h"
#include "Town.h"

struct SOLDIERTYPE;

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index);

uint8_t GetSolID(const struct SOLDIERTYPE *s);
uint8_t GetSolProfile(const struct SOLDIERTYPE *s);

// Get town where the soldier is located
TownID GetSolTown(const struct SOLDIERTYPE *s);

uint8_t GetSolSectorX(const struct SOLDIERTYPE *s);
uint8_t GetSolSectorY(const struct SOLDIERTYPE *s);
int8_t GetSolSectorZ(const struct SOLDIERTYPE *s);
SectorID8 GetSolSectorID8(const struct SOLDIERTYPE *s);
SectorID16 GetSolSectorID16(const struct SOLDIERTYPE *s);

int8_t GetSolAssignment(const struct SOLDIERTYPE *s);
uint8_t GetSolClass(const struct SOLDIERTYPE *s);

bool IsSolActive(const struct SOLDIERTYPE *s);
bool IsSolAlive(const struct SOLDIERTYPE *s);
bool IsSolInSector(const struct SOLDIERTYPE *s);

uint8_t GetSolMilitiaKills(const struct SOLDIERTYPE *s);
void SetSolMilitiaKills(struct SOLDIERTYPE *s, uint8_t kills);

void SetSolAssignmentDone(struct SOLDIERTYPE *s);

// feed this a SOLDIER_CLASS_, it will return you a _MITILIA rank, or -1 if the guy's not militia
int8_t SoldierClassToMilitiaRank(uint8_t ubSoldierClass);
// feed this a _MITILIA rank, it will return you a SOLDIER_CLASS_, or -1 if the guy's not militia
int8_t MilitiaRankToSoldierClass(uint8_t ubRank);

// used for color codes, but also shows the enemy type for debugging purposes
enum {
  SOLDIER_CLASS_NONE,
  SOLDIER_CLASS_ADMINISTRATOR,
  SOLDIER_CLASS_ELITE,
  SOLDIER_CLASS_ARMY,
  SOLDIER_CLASS_GREEN_MILITIA,
  SOLDIER_CLASS_REG_MILITIA,
  SOLDIER_CLASS_ELITE_MILITIA,
  SOLDIER_CLASS_CREATURE,
  SOLDIER_CLASS_MINER,
};

#endif
