#include "Tactical.h"

#include "SGP/Types.h"
#include "SectorInfo.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierInitList.h"
#include "Team.h"

struct TacticalState {
  bool MilitiaRefreshRequired;
};

static struct TacticalState _st;

void TacticalMilitiaRefreshRequired() { _st.MilitiaRefreshRequired = true; }

void RemoveMilitiaFromTactical();

void ReinitMilitiaTactical() {
  if (_st.MilitiaRefreshRequired || gTacticalStatus.uiFlags & LOADING_SAVED_GAME) {
    _st.MilitiaRefreshRequired = false;
    RemoveMilitiaFromTactical();
    PrepareMilitiaForTactical();
  }
}

void RemoveMilitiaFromTactical() {
  SOLDIERINITNODE *curr;
  INT32 i;
  for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
       i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive) {
      TacticalRemoveSoldier(MercPtrs[i]->ubID);
    }
  }
  curr = gSoldierInitHead;
  while (curr) {
    if (curr->pBasicPlacement->bTeam == MILITIA_TEAM) {
      curr->pSoldier = NULL;
    }
    curr = curr->next;
  }
}

void PrepareMilitiaForTactical() {
  if (gbWorldSectorZ > 0) return;

  // Do we have a loaded sector?
  if (gWorldSectorX == 0 && gWorldSectorY == 0) return;

  struct MilitiaCount milCount = GetMilitiaInSector(gWorldSectorX, gWorldSectorY);
  AddSoldierInitListMilitia(milCount.green, milCount.regular, milCount.elite);
}
