// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Soldier.h"

#include "Tactical/Menptr.h"
#include "Town.h"
#include "rust_militia.h"

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index) { return &Menptr[index]; }

u8 GetSolID(const struct SOLDIERTYPE *s) { return s->ubID; }
u8 GetSolProfile(const struct SOLDIERTYPE *s) { return s->ubProfile; }

// Get town where the soldier is located
TownID GetSolTown(const struct SOLDIERTYPE *s) {
  return GetTownIdForSector(GetSolSectorX(s), GetSolSectorY(s));
}

u8 GetSolSectorX(const struct SOLDIERTYPE *s) { return (u8)s->sSectorX; }
u8 GetSolSectorY(const struct SOLDIERTYPE *s) { return (u8)s->sSectorY; }
i8 GetSolSectorZ(const struct SOLDIERTYPE *s) { return s->bSectorZ; }
SectorID8 GetSolSectorID8(const struct SOLDIERTYPE *s) {
  return GetSectorID8((u8)s->sSectorX, (u8)s->sSectorY);
}
i8 GetSolAssignment(const struct SOLDIERTYPE *s) { return s->bAssignment; }
u8 GetSolClass(const struct SOLDIERTYPE *s) { return s->ubSoldierClass; }

bool IsSolActive(const struct SOLDIERTYPE *s) { return s->bActive; }
bool IsSolAlive(const struct SOLDIERTYPE *s) { return s->bLife > 0; }
bool IsSolInSector(const struct SOLDIERTYPE *s) { return s->bInSector; }

u8 GetSolMilitiaKills(const struct SOLDIERTYPE *s) { return s->ubMilitiaKills; }
void SetSolMilitiaKills(struct SOLDIERTYPE *s, u8 kills) { s->ubMilitiaKills = kills; };

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
