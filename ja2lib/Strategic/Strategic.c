#include "Strategic/Strategic.h"

#include "JAScreens.h"
#include "Laptop/Personnel.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/IsometricUtils.h"
#include "UI.h"
#include "Utils/MusicControl.h"

StrategicMapElement StrategicMap[MAP_WORLD_X * MAP_WORLD_Y];

extern BOOLEAN fReDrawFace;

BOOLEAN HandleStrategicDeath(struct SOLDIERTYPE *pSoldier) {
  // add the guy to the dead list
  // AddCharacterToDeadList( pSoldier );

  // If in a vehicle, remove them!
  if ((GetSolAssignment(pSoldier) == VEHICLE) && (pSoldier->iVehicleId != -1)) {
    // remove from vehicle
    TakeSoldierOutOfVehicle(pSoldier);
  }

  // if not in mapscreen
  if (!(IsMapScreen())) {
    // ATE; At least make them dead!
    if ((pSoldier->bAssignment != ASSIGNMENT_DEAD)) {
      SetTimeOfAssignmentChangeForMerc(pSoldier);
    }

    ChangeSoldiersAssignment(pSoldier, ASSIGNMENT_DEAD);
  } else if ((pSoldier->bLife == 0) && (pSoldier->bAssignment != ASSIGNMENT_DEAD)) {
    // died in mapscreen

    fReDrawFace = TRUE;

    // dead
    if ((pSoldier->bAssignment != ASSIGNMENT_DEAD)) {
      SetTimeOfAssignmentChangeForMerc(pSoldier);
    }

    ChangeSoldiersAssignment(pSoldier, ASSIGNMENT_DEAD);

    // s et breath and breath max to 0
    pSoldier->bBreath = pSoldier->bBreathMax = 0;

    // rebuild list
    ReBuildCharactersList();

    // ste merc as dead
    // pSoldier->fUIdeadMerc = TRUE;

    // attempt o remove character from squad
    RemoveCharacterFromSquads(pSoldier);

    // handle any passign comments by grunts
    HandleSoldierDeadComments(pSoldier);

    // put the dead guys down
    AddDeadSoldierToUnLoadedSector((uint8_t)(GetSolSectorX(pSoldier)),
                                   (uint8_t)(GetSolSectorY(pSoldier)), GetSolSectorZ(pSoldier),
                                   pSoldier, RandomGridNo(), ADD_DEAD_SOLDIER_TO_SWEETSPOT);

    fTeamPanelDirty = TRUE;
    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;

    StopTimeCompression();
  }

  return (TRUE);
}

void HandleSoldierDeadComments(struct SOLDIERTYPE *pSoldier) {
  INT32 cnt = 0;
  struct SOLDIERTYPE *pTeamSoldier;
  INT8 bBuddyIndex;

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;

  // see if this was the friend of a living merc
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive) {
      bBuddyIndex = WhichBuddy(pTeamSoldier->ubProfile, GetSolProfile(pSoldier));
      switch (bBuddyIndex) {
        case 0:
          // buddy #1 died!
          TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_ONE_KILLED);
          break;
        case 1:
          // buddy #2 died!
          TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_TWO_KILLED);
          break;
        case 2:
          // learn to like buddy died!
          TacticalCharacterDialogue(pTeamSoldier, QUOTE_LEARNED_TO_LIKE_MERC_KILLED);
          break;
        default:
          break;
      }
    }
  }

  return;
}
