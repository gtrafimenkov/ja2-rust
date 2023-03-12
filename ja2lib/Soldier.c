#include "Soldier.h"

#include "Tactical/Menptr.h"
#include "Town.h"
#include "rust_militia.h"

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index) { return &Menptr[index]; }

uint8_t GetSolID(const struct SOLDIERTYPE *s) { return s->ubID; }
uint8_t GetSolProfile(const struct SOLDIERTYPE *s) { return s->ubProfile; }

// Get town where the soldier is located
TownID GetSolTown(const struct SOLDIERTYPE *s) {
  return GetTownIdForSector(GetSolSectorX(s), GetSolSectorY(s));
}

uint8_t GetSolSectorX(const struct SOLDIERTYPE *s) { return (uint8_t)s->sSectorX; }
uint8_t GetSolSectorY(const struct SOLDIERTYPE *s) { return (uint8_t)s->sSectorY; }
int8_t GetSolSectorZ(const struct SOLDIERTYPE *s) { return s->bSectorZ; }
SectorID8 GetSolSectorID8(const struct SOLDIERTYPE *s) {
  return GetSectorID8((uint8_t)s->sSectorX, (uint8_t)s->sSectorY);
}
int8_t GetSolAssignment(const struct SOLDIERTYPE *s) { return s->bAssignment; }
uint8_t GetSolClass(const struct SOLDIERTYPE *s) { return s->ubSoldierClass; }

bool IsSolActive(const struct SOLDIERTYPE *s) { return s->bActive; }
bool IsSolAlive(const struct SOLDIERTYPE *s) { return s->bLife > 0; }
bool IsSolInSector(const struct SOLDIERTYPE *s) { return s->bInSector; }

uint8_t GetSolMilitiaKills(const struct SOLDIERTYPE *s) { return s->ubMilitiaKills; }
void SetSolMilitiaKills(struct SOLDIERTYPE *s, uint8_t kills) { s->ubMilitiaKills = kills; };

void SetSolAssignmentDone(struct SOLDIERTYPE *s) {
  s->fDoneAssignmentAndNothingToDoFlag = FALSE;
  s->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
}

// feed this a SOLDIER_CLASS_, it will return you a _MITILIA rank, or -1 if the guy's not militia
int8_t SoldierClassToMilitiaRank(uint8_t ubSoldierClass) {
  int8_t bRank = -1;

  switch (ubSoldierClass) {
    case SOLDIER_CLASS_GREEN_MILITIA:
      bRank = GREEN_MILITIA;
      break;
    case SOLDIER_CLASS_REG_MILITIA:
      bRank = REGULAR_MILITIA;
      break;
    case SOLDIER_CLASS_ELITE_MILITIA:
      bRank = ELITE_MILITIA;
      break;
  }

  return (bRank);
}

// feed this a _MITILIA rank, it will return you a SOLDIER_CLASS_, or -1 if the guy's not militia
int8_t MilitiaRankToSoldierClass(uint8_t ubRank) {
  int8_t bSoldierClass = -1;

  switch (ubRank) {
    case GREEN_MILITIA:
      bSoldierClass = SOLDIER_CLASS_GREEN_MILITIA;
      break;
    case REGULAR_MILITIA:
      bSoldierClass = SOLDIER_CLASS_REG_MILITIA;
      break;
    case ELITE_MILITIA:
      bSoldierClass = SOLDIER_CLASS_ELITE_MILITIA;
      break;
  }

  return (bSoldierClass);
}
