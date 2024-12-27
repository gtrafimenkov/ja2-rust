// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Team.h"

#include "Tactical/Overhead.h"

uint8_t GetTeamSide(uint8_t teamID) { return gTacticalStatus.Team[teamID].bSide; }
void SetTeamSide(uint8_t teamID, uint8_t side) { gTacticalStatus.Team[teamID].bSide = side; }

void GetTeamSoldiers(TeamID teamID, struct SoldierList* list) {
  const TacticalTeamType* team = &gTacticalStatus.Team[teamID];
  list->num = 0;
  for (int i = team->bFirstID; i <= team->bLastID; i++) {
    list->soldiers[list->num++] = MercPtrs[i];
  }
}

void GetTeamSoldiers_Active(TeamID teamID, struct SoldierList* list) {
  const TacticalTeamType* team = &gTacticalStatus.Team[teamID];
  list->num = 0;
  for (int i = team->bFirstID; i <= team->bLastID; i++) {
    struct SOLDIERTYPE* sol = MercPtrs[i];
    if (IsSolActive(sol)) {
      list->soldiers[list->num++] = sol;
    }
  }
}
