#include "Strategic/MapScreenHelicopter.h"

#include <string.h>

#include "Laptop/Finances.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Scheduling.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "rust_laptop.h"
#include "rust_sam_sites.h"

// the amounts of time to wait for hover stuff
#define TIME_DELAY_FOR_HOVER_WAIT 10           // minutes
#define TIME_DELAY_FOR_HOVER_WAIT_TOO_LONG 20  // mintues

#define MIN_DAYS_BETWEEN_SKYRIDER_MONOLOGUES 1

// refuel delay
#define REFUEL_HELICOPTER_DELAY 30  // minutes

// total number of sectors one can go
// #define MAX_HELICOPTER_DISTANCE 25

// maximum chance out of a hundred per unsafe sector that a SAM site in decent working condition
// will hit Skyrider
#define MAX_SAM_SITE_ACCURACY 33

extern FACETYPE *gpCurrentTalkingFace;
extern uint8_t gubCurrentTalkingID;

// current temp path for dest char
extern struct path *pTempHelicopterPath;

// the seating capacities
extern int32_t iSeatingCapacities[];

// the static NPC dialogue faces
extern uint32_t uiExternalStaticNPCFaces[];

// the squad mvt groups
extern int8_t SquadMovementGroups[];

// whether helicopted variables have been set up
BOOLEAN fSkyRiderSetUp = FALSE;

// plotting for a helicopter
BOOLEAN fPlotForHelicopter = FALSE;

// is the helicopter available to player?
BOOLEAN fHelicopterAvailable = FALSE;

// helicopter vehicle id
int32_t iHelicopterVehicleId = -1;

// helicopter icon
uint32_t guiHelicopterIcon;

// total distance travelled
// int32_t iTotalHeliDistanceSinceRefuel = 0;

// total owed to player
int32_t iTotalAccumulatedCostByPlayer = 0;

// whether or not skyrider is alive and well? and on our side yet?
BOOLEAN fSkyRiderAvailable = FALSE;

#ifdef JA2TESTVERSION
BOOLEAN fSAMSitesDisabledFromAttackingPlayer = FALSE;
#endif

// helicopter destroyed
BOOLEAN fHelicopterDestroyed = FALSE;

// list of sector locations where SkyRider can be refueled
uint8_t ubRefuelList[NUMBER_OF_REFUEL_SITES][2] = {
    {13, 2},  // Drassen airport
    {6, 9},   // Estoni
};

int16_t sRefuelStartGridNo[NUMBER_OF_REFUEL_SITES] = {
    9001,   // drassen
    13068,  // estoni
};

// whether or not helicopter can refuel at this site
BOOLEAN fRefuelingSiteAvailable[NUMBER_OF_REFUEL_SITES] = {FALSE, FALSE};

// is the heli in the air?
BOOLEAN fHelicopterIsAirBorne = FALSE;

// is the pilot returning straight to base?
BOOLEAN fHeliReturnStraightToBase = FALSE;

// heli hovering
BOOLEAN fHoveringHelicopter = FALSE;

// time started hovering
uint32_t uiStartHoverTime = 0;

// what state are skyrider's monologues in in?
uint32_t guiHelicopterSkyriderTalkState = 0;

// the flags for skyrider events
BOOLEAN fShowEstoniRefuelHighLight = FALSE;
BOOLEAN fShowOtherSAMHighLight = FALSE;
BOOLEAN fShowDrassenSAMHighLight = FALSE;
BOOLEAN fShowCambriaHospitalHighLight = FALSE;

uint32_t guiTimeOfLastSkyriderMonologue = 0;

uint8_t gubHelicopterHitsTaken = 0;

BOOLEAN gfSkyriderSaidCongratsOnTakingSAM = FALSE;
uint8_t gubPlayerProgressSkyriderLastCommentedOn = 0;

// skyrider placeholder
struct SOLDIERTYPE SoldierSkyRider;

struct SOLDIERTYPE *pSkyRider;

// helicopter char dialogue
BOOLEAN HeliCharacterDialogue(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNum);

// does skyrider notice bad guys in sector?
BOOLEAN DoesSkyriderNoticeEnemiesInSector(uint8_t ubNumEnemies);

// are we at the end of the path for the helicopter
BOOLEAN EndOfHelicoptersPath(void);

// find the location sector of closest refuel point for heli..and the criteria if the sector must be
// under the players control
int32_t FindLocationOfClosestRefuelSite(BOOLEAN fMustBeAvailable);

// add the tactical heli graphic
void AddHelicopterToMaps(BOOLEAN fAdd, uint8_t ubSite);

void PaySkyriderBill(void);
void MakeHeliReturnToBase(void);

void HandleSkyRiderMonologueAboutDrassenSAMSite(uint32_t uiSpecialCode);
void HandleSkyRiderMonologueAboutCambriaHospital(uint32_t uiSpecialCode);
void HandleSkyRiderMonologueAboutOtherSAMSites(uint32_t uiSpecialCode);
void HandleSkyRiderMonologueAboutEstoniRefuel(uint32_t uiSpecialCode);

void InitializeHelicopter(void) {
  // must be called whenever a new game starts up!
  fHelicopterAvailable = FALSE;
  iHelicopterVehicleId = -1;

  fSkyRiderAvailable = FALSE;
  fSkyRiderSetUp = FALSE;
  pSkyRider = NULL;
  memset(&SoldierSkyRider, 0, sizeof(SoldierSkyRider));

  fHelicopterIsAirBorne = FALSE;
  fHeliReturnStraightToBase = FALSE;

  fHoveringHelicopter = FALSE;
  uiStartHoverTime = 0;

  fPlotForHelicopter = FALSE;
  pTempHelicopterPath = NULL;

  //	iTotalHeliDistanceSinceRefuel = 0;
  iTotalAccumulatedCostByPlayer = 0;

  fHelicopterDestroyed = FALSE;

  guiHelicopterSkyriderTalkState = 0;
  guiTimeOfLastSkyriderMonologue = 0;

  fShowEstoniRefuelHighLight = FALSE;
  fShowOtherSAMHighLight = FALSE;
  fShowDrassenSAMHighLight = FALSE;
  fShowCambriaHospitalHighLight = FALSE;

  gfSkyriderEmptyHelpGiven = FALSE;

  gubHelicopterHitsTaken = 0;

  gfSkyriderSaidCongratsOnTakingSAM = FALSE;
  gubPlayerProgressSkyriderLastCommentedOn = 0;
}

BOOLEAN AddSoldierToHelicopter(struct SOLDIERTYPE *pSoldier) {
  // attempt to add soldier to helicopter
  if (iHelicopterVehicleId == -1) {
    // no heli yet
    return (FALSE);
  }

  // check if heli is in motion or if on the ground
  if ((fHelicopterIsAirBorne == TRUE) && (fHoveringHelicopter == FALSE)) {
    return (FALSE);
  }

  // is the heli returning to base?..he ain't waiting if so
  if (fHeliReturnStraightToBase == TRUE) {
    return (FALSE);
  }

  // attempt to add to vehicle
  return (PutSoldierInVehicle(pSoldier, (int8_t)iHelicopterVehicleId));
}

BOOLEAN RemoveSoldierFromHelicopter(struct SOLDIERTYPE *pSoldier) {
  // attempt to add soldier to helicopter
  if (iHelicopterVehicleId == -1) {
    // no heli yet
    return (FALSE);
  }

  // check if heli is in motion or if on the ground
  if ((fHelicopterIsAirBorne == TRUE) && (fHoveringHelicopter == FALSE)) {
    return (FALSE);
  }

  // is the heli returning to base?..he ain't waiting if so
  if (fHeliReturnStraightToBase == TRUE) {
    return (FALSE);
  }

  pSoldier->sSectorX = pVehicleList[iHelicopterVehicleId].sSectorX;
  pSoldier->sSectorY = pVehicleList[iHelicopterVehicleId].sSectorY;
  pSoldier->bSectorZ = 0;

  // reset between sectors
  pSoldier->fBetweenSectors = FALSE;

  // remove from the vehicle
  return (TakeSoldierOutOfVehicle(pSoldier));
}

BOOLEAN HandleHeliEnteringSector(uint8_t sX, uint8_t sY) {
  uint8_t ubNumEnemies;

  // check for SAM attack upon the chopper.  If it's destroyed by the attack, do nothing else here
  if (HandleSAMSiteAttackOfHelicopterInSector(sX, sY) == TRUE) {
    // destroyed
    return (TRUE);
  }

  // count how many enemies are camped there or passing through
  ubNumEnemies = NumEnemiesInSector(sX, sY);

  // any baddies?
  if (ubNumEnemies > 0) {
    // if the player didn't know about these prior to the chopper's arrival
    if (WhatPlayerKnowsAboutEnemiesInSector(sX, sY) == KNOWS_NOTHING) {
      // but Skyrider notices them
      if (DoesSkyriderNoticeEnemiesInSector(ubNumEnemies) == TRUE) {
        // if just passing through (different quotes are used below if it's his final destination)
        if (!EndOfHelicoptersPath()) {
          // stop time compression and inform player that there are enemies in the sector below
          StopTimeCompression();

          if (Random(2)) {
            HeliCharacterDialogue(pSkyRider, ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_A);
          } else {
            HeliCharacterDialogue(pSkyRider, ENEMIES_SPOTTED_EN_ROUTE_IN_FRIENDLY_SECTOR_B);
          }
        }

        // make their presence appear on the map while Skyrider remains in the sector
        SectorInfo[GetSectorID8(sX, sY)].uiFlags |= SF_SKYRIDER_NOTICED_ENEMIES_HERE;
      }
    }
  }

  // player pays for travel if Skyrider is NOT returning to base (even if empty while scouting/going
  // for pickup)
  if (fHeliReturnStraightToBase == FALSE) {
    // charge cost for flying another sector
    iTotalAccumulatedCostByPlayer += GetCostOfPassageForHelicopter(sX, sY);
  }

  // accumulate distance travelled
  //	AddSectorToHelicopterDistanceTravelled( );

  // check if heli has any real path left
  if (EndOfHelicoptersPath()) {
    // start hovering
    StartHoverTime();

    // if sector is safe, or Skyrider MUST land anyway (returning to base)
    if ((ubNumEnemies == 0) || fHeliReturnStraightToBase) {
      // if he has passengers, or he's not going straight to base, tell player he's arrived
      // (i.e. don't say anything nor stop time compression if he's empty and just returning to
      // base)
      if ((GetNumberOfPassengersInHelicopter() > 0) || !fHeliReturnStraightToBase) {
        // arrived at destination
        HeliCharacterDialogue(pSkyRider, ARRIVED_IN_NON_HOSTILE_SECTOR);
        StopTimeCompression();
      }

      // destination reached, payment due.  If player can't pay, mercs get kicked off and heli flies
      // to base!
      PaySkyriderBill();
    } else {
      // Say quote: "Gonna have to abort.  Enemies below"
      HeliCharacterDialogue(pSkyRider, ARRIVED_IN_HOSTILE_SECTOR);
      StopTimeCompression();
    }

    if (CheckForArrivalAtRefuelPoint()) {
      ReFuelHelicopter();
    }
  }

  return (FALSE);
}

int32_t LocationOfNearestRefuelPoint(BOOLEAN fNotifyPlayerIfNoSafeLZ) {
  int32_t iClosestLocation = -1;

  // try to find one, any one under the players control
  iClosestLocation = FindLocationOfClosestRefuelSite(TRUE);

  // no go?...then find
  if (iClosestLocation == -1) {
    if (fNotifyPlayerIfNoSafeLZ) {
      // no refueling sites available, might wanna warn player about this
      ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pHelicopterEtaStrings[5]);
    }

    // find the closest location regardless
    iClosestLocation = FindLocationOfClosestRefuelSite(FALSE);
  }

  // always returns a valid refuel point, picking a hostile one if unavoidable
  Assert(iClosestLocation != -1);

  return (iClosestLocation);
}

int32_t FindLocationOfClosestRefuelSite(BOOLEAN fMustBeAvailable) {
  int32_t iShortestDistance = 9999;
  int32_t iCounter = 0;
  int32_t iDistance = 9999;
  int32_t iClosestLocation = -1;

  // find shortest distance to refuel site
  for (iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++) {
    // if this refuelling site is available
    if ((fRefuelingSiteAvailable[iCounter]) || (fMustBeAvailable == FALSE)) {
      // find if sector is under control, find distance from heli to it
      iDistance = (int32_t)FindStratPath((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                                         (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY,
                                         ubRefuelList[iCounter][0], ubRefuelList[iCounter][1],
                                         pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE);

      if (iDistance < iShortestDistance) {
        // shorter, copy over
        iShortestDistance = iDistance;
        iClosestLocation = iCounter;
      }
    }
  }

  // return the location
  return (iClosestLocation);
}

int32_t DistanceToNearestRefuelPoint(uint8_t sX, uint8_t sY) {
  int32_t iClosestLocation;
  int32_t iDistance;

  // don't notify player during these checks!
  iClosestLocation = LocationOfNearestRefuelPoint(FALSE);

  iDistance = (int32_t)FindStratPath(sX, sY, ubRefuelList[iClosestLocation][0],
                                     ubRefuelList[iClosestLocation][1],
                                     pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE);
  return (iDistance);
}

void ReFuelHelicopter(void) {
  // land, pay the man, and refuel

  LandHelicopter();
}

int32_t GetCostOfPassageForHelicopter(uint8_t sX, uint8_t sY) {
  // check if sector is air controlled or not, if so, then normal cost, otherwise increase the cost
  int32_t iCost = 0;

  // if they don't control it
  if (!IsSectorEnemyAirControlled(sX, sY)) {
    iCost = COST_AIRSPACE_SAFE;
  } else {
    iCost = COST_AIRSPACE_UNSAFE;
  }

  return (iCost);
}

void SkyriderDestroyed(void) {
  // remove any arrival events for the helicopter's group
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pVehicleList[iHelicopterVehicleId].ubMovementGroup);

  // kill eveyone on board
  KillAllInVehicle(iHelicopterVehicleId);

  // kill skyrider
  fSkyRiderAvailable = FALSE;
  SoldierSkyRider.bLife = 0;
  gMercProfiles[SKYRIDER].bLife = 0;

  // heli no longer available
  fHelicopterAvailable = FALSE;

  // destroy helicopter
  fHelicopterDestroyed = TRUE;

  // zero out balance due
  gMercProfiles[SKYRIDER].iBalance = 0;
  //	iTotalHeliDistanceSinceRefuel = 0;
  iTotalAccumulatedCostByPlayer = 0;

  // remove vehicle and reset
  RemoveVehicleFromList(iHelicopterVehicleId);
  iHelicopterVehicleId = -1;

  return;
}

BOOLEAN CanHelicopterFly(void) {
  // check if heli is available for flight?

  // is the heli available
  if (fHelicopterAvailable == FALSE) {
    return (FALSE);
  }

  if (VehicleIdIsValid(iHelicopterVehicleId) == FALSE) {
    return (FALSE);
  }

  // is the pilot alive, well, and willing to help us?
  if (IsHelicopterPilotAvailable() == FALSE) {
    return (FALSE);
  }

  if (fHeliReturnStraightToBase == TRUE) {
    return (FALSE);
  }

  // grounded by enemies in sector?
  if (CanHelicopterTakeOff() == FALSE) {
    return (FALSE);
  }

  // everything A-OK!
  return (TRUE);
}

BOOLEAN IsHelicopterPilotAvailable(void) {
  // what is state of skyrider?
  if (fSkyRiderAvailable == FALSE) {
    return (FALSE);
  }

  // owe any money to skyrider?
  if (gMercProfiles[SKYRIDER].iBalance < 0) {
    return (FALSE);
  }

  // Drassen too disloyal to wanna help player?
  if (CheckFact(FACT_LOYALTY_LOW, SKYRIDER)) {
    return (FALSE);
  }

  return (TRUE);
}

void LandHelicopter(void) {
  // set the helictoper down, call arrive callback for this mvt group
  fHelicopterIsAirBorne = FALSE;

  // no longer hovering
  fHoveringHelicopter = FALSE;

  // reset fact that we might have returned straight here
  fHeliReturnStraightToBase = FALSE;

  HandleHelicopterOnGroundGraphic();
  HandleHelicopterOnGroundSkyriderProfile();

  // if we'll be unable to take off again (because there are enemies in the sector, or we owe pilot
  // money)
  if (CanHelicopterFly() == FALSE) {
    // kick everyone out!
    MoveAllInHelicopterToFootMovementGroup();
  } else {
    // play meanwhile scene if it hasn't been used yet
    HandleKillChopperMeanwhileScene();
  }
}

void TakeOffHelicopter(void) {
  // heli in the air
  fHelicopterIsAirBorne = TRUE;

  // no longer hovering
  fHoveringHelicopter = FALSE;

  HandleHelicopterOnGroundGraphic();
  HandleHelicopterOnGroundSkyriderProfile();
}

void StartHoverTime(void) {
  // start hover in this sector
  fHoveringHelicopter = TRUE;

  // post event for x mins in future, save start time, if event time - delay = start time, then
  // hover has gone on too long
  uiStartHoverTime = GetWorldTotalMin();

  // post event..to call handle hover
  AddStrategicEvent(EVENT_HELICOPTER_HOVER_TOO_LONG, GetWorldTotalMin() + TIME_DELAY_FOR_HOVER_WAIT,
                    0);

  return;
}

void HandleHeliHoverLong(void) {
  // post message about hovering too long
  if (fHoveringHelicopter) {
    // proper event, post next one
    AddStrategicEvent(EVENT_HELICOPTER_HOVER_WAY_TOO_LONG,
                      uiStartHoverTime + TIME_DELAY_FOR_HOVER_WAIT_TOO_LONG, 0);

    // inform player
    HeliCharacterDialogue(pSkyRider, HOVERING_A_WHILE);

    // stop time compression if it's on
    StopTimeCompression();
  } else {
    // reset
    uiStartHoverTime = 0;
  }
}

void HandleHeliHoverTooLong(void) {
  // reset hover time
  uiStartHoverTime = 0;

  if (fHoveringHelicopter == FALSE) {
    return;
  }

  // hovered too long, inform player heli is returning to base
  HeliCharacterDialogue(pSkyRider, RETURN_TO_BASE);

  // If the sector is safe
  if (NumEnemiesInSector((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                         (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY) == 0) {
    // kick everyone out!
    MoveAllInHelicopterToFootMovementGroup();
  }

  MakeHeliReturnToBase();
}

// check if anyone in the chopper sees any baddies in sector
BOOLEAN DoesSkyriderNoticeEnemiesInSector(uint8_t ubNumEnemies) {
  uint8_t ubChance;

  // is the pilot and heli around?
  if (CanHelicopterFly() == FALSE) {
    return (FALSE);
  }

  // if there aren't any, he obviously won't see them
  if (ubNumEnemies == 0) {
    return (FALSE);
  }

  // figure out what the chance is of seeing them
  // make this relatively accurate most of the time, to encourage helicopter scouting by making it
  // useful
  ubChance = 60 + ubNumEnemies;

  if (PreRandom(100) < ubChance) {
    return (TRUE);
  }

  return (FALSE);
}

// if the heli is on the move, what is the distance it will move..the length of the merc path, less
// the first node
int32_t DistanceOfIntendedHelicopterPath(void) {
  struct path *pNode = NULL;
  int32_t iLength = 0;

  if (CanHelicopterFly() == FALSE) {
    // big number, no go
    return (9999);
  }

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  // any path yet?
  if (pNode != NULL) {
    while (pNode->pNext) {
      iLength++;
      pNode = pNode->pNext;
    }
  }

  pNode = MoveToBeginningOfPathList(pTempHelicopterPath);

  // any path yet?
  if (pNode != NULL) {
    while (pNode->pNext) {
      iLength++;
      pNode = pNode->pNext;
    }
  }

  return (iLength);
}

BOOLEAN CheckForArrivalAtRefuelPoint(void) {
  // check if this is our final destination
  if (GetLengthOfPath(pVehicleList[iHelicopterVehicleId].pMercPath) > 0) {
    return (FALSE);
  }

  // check if we're at a refuel site
  if (DistanceToNearestRefuelPoint((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                                   (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY) > 0) {
    // not at a refuel point
    return (FALSE);
  }

  // we are at a refuel site
  return (TRUE);
}

void SetUpHelicopterForMovement(void) {
  // check if helicopter vehicle has a mvt group, if not, assign one in this sector
  int32_t iCounter = 0;

  // if no group, create one for vehicle
  if (pVehicleList[iHelicopterVehicleId].ubMovementGroup == 0) {
    // get the vehicle a mvt group
    pVehicleList[iHelicopterVehicleId].ubMovementGroup = CreateNewVehicleGroupDepartingFromSector(
        (uint8_t)(pVehicleList[iHelicopterVehicleId].sSectorX),
        (uint8_t)(pVehicleList[iHelicopterVehicleId].sSectorY), iHelicopterVehicleId);

    // add everyone in vehicle to this mvt group
    for (iCounter = 0;
         iCounter < iSeatingCapacities[pVehicleList[iHelicopterVehicleId].ubVehicleType];
         iCounter++) {
      if (pVehicleList[iHelicopterVehicleId].pPassengers[iCounter] != NULL) {
        // add character
        AddPlayerToGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup,
                         pVehicleList[iHelicopterVehicleId].pPassengers[iCounter]);
      }
    }
  }
}

BOOLEAN HeliCharacterDialogue(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNum) {
  // ARM: we could just return, but since various flags are often being set it's safer to honk so it
  // gets fixed right!
  Assert(fSkyRiderAvailable);

  return (CharacterDialogue(SKYRIDER, usQuoteNum, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
                            DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE));
}

int32_t GetNumberOfPassengersInHelicopter(void) {
  int32_t iNumber = 0;

  if (iHelicopterVehicleId != -1) {
    iNumber = GetNumberInVehicle(iHelicopterVehicleId);
  }

  return (iNumber);
}

BOOLEAN IsRefuelSiteInSector(uint8_t sMapX, uint8_t sMapY) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++) {
    if ((ubRefuelList[iCounter][0] == sMapX) && (ubRefuelList[iCounter][1] == sMapY)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void UpdateRefuelSiteAvailability(void) {
  int32_t iCounter = 0;

  // Generally, only Drassen is initially available for refuelling
  // Estoni must first be captured (although player may already have it when he gets Skyrider!)

  for (iCounter = 0; iCounter < NUMBER_OF_REFUEL_SITES; iCounter++) {
    // if enemy controlled sector (ground OR air, don't want to fly into enemy air territory)
    if (IsSectorEnemyControlled(ubRefuelList[iCounter][0], ubRefuelList[iCounter][1]) ||
        IsSectorEnemyAirControlled(ubRefuelList[iCounter][0], ubRefuelList[iCounter][1]) ||
        ((iCounter == ESTONI_REFUELING_SITE) &&
         (CheckFact(FACT_ESTONI_REFUELLING_POSSIBLE, 0) == FALSE))) {
      // mark refueling site as unavailable
      fRefuelingSiteAvailable[iCounter] = FALSE;
    } else {
      // mark refueling site as available
      fRefuelingSiteAvailable[iCounter] = TRUE;

      // reactivate a grounded helicopter, if here
      if (!fHelicopterAvailable && !fHelicopterDestroyed && fSkyRiderAvailable &&
          (iHelicopterVehicleId != -1)) {
        if ((pVehicleList[iHelicopterVehicleId].sSectorX == ubRefuelList[iCounter][0]) &&
            (pVehicleList[iHelicopterVehicleId].sSectorY == ubRefuelList[iCounter][1])) {
          // no longer grounded
          DoScreenIndependantMessageBox(pSkyriderText[5], MSG_BOX_FLAG_OK, NULL);
        }
      }
    }
  }
}

void SetUpHelicopterForPlayer(uint8_t sX, uint8_t sY) {
  if (fSkyRiderSetUp == FALSE) {
    fHelicopterAvailable = TRUE;
    fSkyRiderAvailable = TRUE;

    iHelicopterVehicleId = AddVehicleToList(sX, sY, 0, HELICOPTER);

    Assert(iHelicopterVehicleId != -1);

    memset(&SoldierSkyRider, 0, sizeof(struct SOLDIERTYPE));
    SoldierSkyRider.ubProfile = SKYRIDER;
    SoldierSkyRider.bLife = 80;

    pSkyRider = &(SoldierSkyRider);

    // set up for movement
    SetUpHelicopterForMovement();
    UpdateRefuelSiteAvailability();

    fSkyRiderSetUp = TRUE;

    gMercProfiles[SKYRIDER].fUseProfileInsertionInfo = FALSE;
  }

  return;
}

uint8_t MoveAllInHelicopterToFootMovementGroup(void) {
  // take everyone out of heli and add to movement group
  int32_t iCounter = 0;
  uint8_t ubGroupId = 0;
  struct SOLDIERTYPE *pSoldier;
  int8_t bNewSquad;
  BOOLEAN fAnyoneAboard = FALSE;
  BOOLEAN fSuccess;
  uint8_t ubInsertionCode;
  BOOLEAN fInsertionCodeSet = FALSE;
  uint16_t usInsertionData;

  // put these guys on their own squad (we need to return their group ID, and can only return one,
  // so they need a unique one
  bNewSquad = GetFirstEmptySquad();
  if (bNewSquad == -1) {
    return (0);
  }

  // go through list of everyone in helicopter
  for (iCounter = 0;
       iCounter < iSeatingCapacities[pVehicleList[iHelicopterVehicleId].ubVehicleType];
       iCounter++) {
    // get passenger
    pSoldier = pVehicleList[iHelicopterVehicleId].pPassengers[iCounter];

    if (pSoldier != NULL) {
      // better really be in there!
      Assert(GetSolAssignment(pSoldier) == VEHICLE);
      Assert(pSoldier->iVehicleId == iHelicopterVehicleId);

      fAnyoneAboard = TRUE;

      fSuccess = RemoveSoldierFromHelicopter(pSoldier);
      Assert(fSuccess);

      AddCharacterToSquad(pSoldier, bNewSquad);

      // ATE: OK - the ubStrategicInsertionCode is set 'cause groupArrivesInsector has been
      // called when buddy is added to a squad. However, the insertion code onlt sets set for
      // the first merc, so the rest are going to use whatever they had previously....
      if (!fInsertionCodeSet) {
        ubInsertionCode = pSoldier->ubStrategicInsertionCode;
        usInsertionData = pSoldier->usStrategicInsertionData;
        fInsertionCodeSet = TRUE;
      } else {
        pSoldier->ubStrategicInsertionCode = ubInsertionCode;
        pSoldier->usStrategicInsertionData = usInsertionData;
      }
    }
  }

  if (fAnyoneAboard) {
    ubGroupId = SquadMovementGroups[bNewSquad];
  }

  return (ubGroupId);
}

void SkyRiderTalk(uint16_t usQuoteNum) {
  // have skyrider talk to player
  HeliCharacterDialogue(pSkyRider, usQuoteNum);

  fTeamPanelDirty = TRUE;

  return;
}

void HandleSkyRiderMonologueEvent(uint32_t uiEventCode, uint32_t uiSpecialCode) {
  // will handle the skyrider monologue about where the SAM sites are and what not

  TurnOnAirSpaceMode();

  switch (uiEventCode) {
    case (SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE):
      SetExternMapscreenSpeechPanelXY(DEFAULT_EXTERN_PANEL_X_POS, 117);
      HandleSkyRiderMonologueAboutDrassenSAMSite(uiSpecialCode);
      break;
    case SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL:
      SetExternMapscreenSpeechPanelXY(DEFAULT_EXTERN_PANEL_X_POS, 172);
      HandleSkyRiderMonologueAboutCambriaHospital(uiSpecialCode);
      break;
    case (SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES):
      SetExternMapscreenSpeechPanelXY(335, DEFAULT_EXTERN_PANEL_Y_POS);
      HandleSkyRiderMonologueAboutOtherSAMSites(uiSpecialCode);
      break;
    case (SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL):
      SetExternMapscreenSpeechPanelXY(DEFAULT_EXTERN_PANEL_X_POS, DEFAULT_EXTERN_PANEL_Y_POS);
      HandleSkyRiderMonologueAboutEstoniRefuel(uiSpecialCode);
      break;
  }

  // update time
  guiTimeOfLastSkyriderMonologue = GetWorldTotalMin();
}

void HandleSkyRiderMonologueAboutEstoniRefuel(uint32_t uiSpecialCode) {
  // once estoni is free tell player about refueling

  switch (uiSpecialCode) {
    case (0):
      CharacterDialogueWithSpecialEvent(
          SKYRIDER, SPIEL_ABOUT_ESTONI_AIRSPACE, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 1);
      // if special event data 2 is true, then do dialogue, else this is just a trigger for an event
      CharacterDialogue(SKYRIDER, SPIEL_ABOUT_ESTONI_AIRSPACE,
                        uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI,
                        FALSE, FALSE);

      CharacterDialogueWithSpecialEvent(
          SKYRIDER, SPIEL_ABOUT_ESTONI_AIRSPACE, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 2);
      break;

    case (1):
      // highlight Estoni
      fShowEstoniRefuelHighLight = TRUE;
      break;

    case (2):
      fShowEstoniRefuelHighLight = FALSE;
      break;
  }
  return;
}

void HandleSkyRiderMonologueAboutDrassenSAMSite(uint32_t uiSpecialCode) {
  switch (uiSpecialCode) {
    case (0):
      // if special event data 2 is true, then do dialogue, else this is just a trigger for an event
      CharacterDialogue(SKYRIDER, MENTION_DRASSEN_SAM_SITE,
                        uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI,
                        FALSE, FALSE);
      CharacterDialogueWithSpecialEvent(
          SKYRIDER, MENTION_DRASSEN_SAM_SITE, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, TRUE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 1);

      if (!IsSamUnderPlayerControl(SamSiteDrassen)) {
        CharacterDialogue(SKYRIDER, SECOND_HALF_OF_MENTION_DRASSEN_SAM_SITE,
                          uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
                          DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE);
      } else {
        // Ian says don't use the SAM site quote unless player has tried flying already
        if (CheckFact(FACT_SKYRIDER_USED_IN_MAPSCREEN, SKYRIDER)) {
          CharacterDialogue(SKYRIDER, SAM_SITE_TAKEN,
                            uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
                            DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE);
          gfSkyriderSaidCongratsOnTakingSAM = TRUE;
        }
      }

      CharacterDialogueWithSpecialEvent(
          SKYRIDER, MENTION_DRASSEN_SAM_SITE, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, TRUE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 2);
      break;

    case (1):
      // highlight Drassen SAM site sector
      fShowDrassenSAMHighLight = TRUE;
      SetSAMSiteAsFound(SamSiteDrassen);
      break;

    case (2):
      fShowDrassenSAMHighLight = FALSE;
      break;
  }
  return;
}

void HandleSkyRiderMonologueAboutCambriaHospital(uint32_t uiSpecialCode) {
  switch (uiSpecialCode) {
    case (0):
      // gpCurrentTalkingFace = &gFacesData[ uiExternalStaticNPCFaces[ SKYRIDER_EXTERNAL_FACE ] ];
      // gubCurrentTalkingID = SKYRIDER;

      // if special event data 2 is true, then do dialogue, else this is just a trigger for an event
      CharacterDialogue(SKYRIDER, MENTION_HOSPITAL_IN_CAMBRIA,
                        uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI,
                        FALSE, FALSE);
      CharacterDialogueWithSpecialEvent(
          SKYRIDER, MENTION_HOSPITAL_IN_CAMBRIA, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, TRUE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL, 1);

      // highlight Drassen hospital sector
      fShowCambriaHospitalHighLight = TRUE;
      break;

    case (1):
      fShowCambriaHospitalHighLight = FALSE;
      break;
  }
  return;
}

void HandleSkyRiderMonologueAboutOtherSAMSites(uint32_t uiSpecialCode) {
  // handle skyrider telling player about other sam sites..on fifth hiring or after one near drassen
  // is taken out

  switch (uiSpecialCode) {
    case (0):
      // do quote 21
      gpCurrentTalkingFace = &gFacesData[uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE]];
      gubCurrentTalkingID = SKYRIDER;

      // if special event data 2 is true, then do dialogue, else this is just a trigger for an event
      CharacterDialogue(SKYRIDER, SPIEL_ABOUT_OTHER_SAM_SITES,
                        uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI,
                        FALSE, FALSE);
      CharacterDialogueWithSpecialEvent(
          SKYRIDER, SPIEL_ABOUT_OTHER_SAM_SITES, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 1);

      CharacterDialogue(SKYRIDER, SECOND_HALF_OF_SPIEL_ABOUT_OTHER_SAM_SITES,
                        uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE], DIALOGUE_EXTERNAL_NPC_UI,
                        FALSE, FALSE);
      CharacterDialogueWithSpecialEvent(
          SKYRIDER, SPIEL_ABOUT_OTHER_SAM_SITES, uiExternalStaticNPCFaces[SKYRIDER_EXTERNAL_FACE],
          DIALOGUE_EXTERNAL_NPC_UI, FALSE, FALSE, DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT,
          SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 2);

      break;

    case (1):
      // highlight other SAMs
      fShowOtherSAMHighLight = TRUE;
      // reveal other 3 SAM sites
      SetSAMSiteAsFound(SamSiteChitzena);
      SetSAMSiteAsFound(SamSiteCambria);
      SetSAMSiteAsFound(SamSiteMeduna);
      break;

    case (2):
      fShowOtherSAMHighLight = FALSE;
      break;
  }
  return;
}

void CheckAndHandleSkyriderMonologues(void) {
  // wait at least this many days between Skyrider monologues
  if ((GetWorldTotalMin() - guiTimeOfLastSkyriderMonologue) >=
      (MIN_DAYS_BETWEEN_SKYRIDER_MONOLOGUES * 24 * 60)) {
    if (guiHelicopterSkyriderTalkState == 0) {
      HandleSkyRiderMonologueEvent(SKYRIDER_MONOLOGUE_EVENT_DRASSEN_SAM_SITE, 0);
      guiHelicopterSkyriderTalkState = 1;
    } else if (guiHelicopterSkyriderTalkState == 1) {
      // if enemy still controls the Cambria hospital sector
      if (IsSectorEnemyControlled(HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y)) {
        HandleSkyRiderMonologueEvent(SKYRIDER_MONOLOGUE_EVENT_CAMBRIA_HOSPITAL, 0);
      }
      // advance state even if player already has Cambria's hospital sector!!!
      guiHelicopterSkyriderTalkState = 2;
    } else if (guiHelicopterSkyriderTalkState == 2) {
      // wait until player has taken over a SAM site before saying this and advancing state
      if (gfSkyriderSaidCongratsOnTakingSAM) {
        HandleSkyRiderMonologueEvent(SKYRIDER_MONOLOGUE_EVENT_OTHER_SAM_SITES, 0);
        guiHelicopterSkyriderTalkState = 3;
      }
    } else if (guiHelicopterSkyriderTalkState == 3) {
      // wait until Estoni refuelling site becomes available
      if (fRefuelingSiteAvailable[ESTONI_REFUELING_SITE]) {
        HandleSkyRiderMonologueEvent(SKYRIDER_MONOLOGUE_EVENT_ESTONI_REFUEL, 0);
        guiHelicopterSkyriderTalkState = 4;
      }
    }
  }
}

void HandleAnimationOfSectors(void) {
  BOOLEAN fSkipSpeakersLocator = FALSE;
  // these don't need to be saved, they merely turn off the highlights after they stop flashing
  static BOOLEAN fOldShowDrassenSAMHighLight = FALSE;
  static BOOLEAN fOldShowCambriaHospitalHighLight = FALSE;
  static BOOLEAN fOldShowEstoniRefuelHighLight = FALSE;
  static BOOLEAN fOldShowOtherSAMHighLight = FALSE;

  // find out which mode we are in and animate for that mode

  // Drassen SAM site
  if (fShowDrassenSAMHighLight) {
    fOldShowDrassenSAMHighLight = TRUE;
    HandleBlitOfSectorLocatorIcon(GetSamSiteX(1), GetSamSiteY(1), 0, LOCATOR_COLOR_RED);
    fSkipSpeakersLocator = TRUE;
  } else if (fOldShowDrassenSAMHighLight) {
    fOldShowDrassenSAMHighLight = FALSE;
    SetMapPanelDirty(true);
  }

  // Cambria hospital
  if (fShowCambriaHospitalHighLight) {
    fOldShowCambriaHospitalHighLight = TRUE;
    HandleBlitOfSectorLocatorIcon(HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y, 0, LOCATOR_COLOR_RED);
    fSkipSpeakersLocator = TRUE;
  } else if (fOldShowCambriaHospitalHighLight) {
    fOldShowCambriaHospitalHighLight = FALSE;
    SetMapPanelDirty(true);
  }

  // show other SAM sites
  if (fShowOtherSAMHighLight) {
    fOldShowOtherSAMHighLight = TRUE;
    for (int i = 0; i < GetSamSiteCount(); i++) {
      if (i != 1) {
        HandleBlitOfSectorLocatorIcon(GetSamSiteX(i), GetSamSiteY(i), 0, LOCATOR_COLOR_RED);
      }
    }
    fSkipSpeakersLocator = TRUE;
  } else if (fOldShowOtherSAMHighLight) {
    fOldShowOtherSAMHighLight = FALSE;
    SetMapPanelDirty(true);
  }

  // show Estoni site
  if (fShowEstoniRefuelHighLight) {
    fOldShowEstoniRefuelHighLight = TRUE;
    HandleBlitOfSectorLocatorIcon(ubRefuelList[ESTONI_REFUELING_SITE][0],
                                  ubRefuelList[ESTONI_REFUELING_SITE][1], 0, LOCATOR_COLOR_RED);
    fSkipSpeakersLocator = TRUE;
  } else if (fOldShowEstoniRefuelHighLight) {
    fOldShowEstoniRefuelHighLight = FALSE;
    SetMapPanelDirty(true);
  }

  // don't show sector locator over the speaker's sector if he is talking about another sector -
  // it's confusing
  if (!fSkipSpeakersLocator) {
    switch (gubBlitSectorLocatorCode) {
      case LOCATOR_COLOR_RED:  // normal one used for mines (will now be overriden with yellow)
        HandleBlitOfSectorLocatorIcon(gsSectorLocatorX, gsSectorLocatorY, 0, LOCATOR_COLOR_RED);
        break;
      case LOCATOR_COLOR_YELLOW:  // used for all other dialogues
        HandleBlitOfSectorLocatorIcon(gsSectorLocatorX, gsSectorLocatorY, 0, LOCATOR_COLOR_YELLOW);
        break;
    }
  }

  return;
}

int16_t LastSectorInHelicoptersPath(void) {
  // get the last sector value in the helictoper's path
  struct path *pNode = NULL;
  uint32_t uiLocation = 0;

  // if the heli is on the move, what is the distance it will move..the length of the merc path,
  // less the first node
  if (CanHelicopterFly() == FALSE) {
    // big number, no go
    return (0);
  }

  uiLocation = pVehicleList[iHelicopterVehicleId].sSectorX +
               pVehicleList[iHelicopterVehicleId].sSectorY * MAP_WORLD_X;

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  // any path yet?
  if (pNode != NULL) {
    while (pNode) {
      uiLocation = pNode->uiSectorId;

      pNode = pNode->pNext;
    }
  }

  pNode = MoveToBeginningOfPathList(pTempHelicopterPath);
  // any path yet?
  if (pNode != NULL) {
    while (pNode) {
      uiLocation = pNode->uiSectorId;

      pNode = pNode->pNext;
    }
  }

  return ((int16_t)uiLocation);
}

void HandleHelicopterOnGroundGraphic(void) {
  uint8_t ubSite = 0;
  struct SOLDIERTYPE *pSoldier;

  // no worries if underground
  if (gbWorldSectorZ != 0) {
    return;
  }

  for (ubSite = 0; ubSite < NUMBER_OF_REFUEL_SITES; ubSite++) {
    // is this refueling site sector the loaded sector ?
    if ((ubRefuelList[ubSite][0] == gWorldSectorX) && (ubRefuelList[ubSite][1] == gWorldSectorY)) {
      // YES, so find out if the chopper is landed here
      if (IsHelicopterOnGroundAtRefuelingSite(ubSite)) {
        // Add....
        AddHelicopterToMaps(TRUE, ubSite);
        // ATE: Add skyridder too
        // ATE: only if hired......
        if (fHelicopterAvailable) {
          gMercProfiles[SKYRIDER].sSectorX = gWorldSectorX;
          gMercProfiles[SKYRIDER].sSectorY = gWorldSectorY;
        }
      } else {
        AddHelicopterToMaps(FALSE, ubSite);
        // ATE: Remove skyridder....
        if (fHelicopterAvailable) {
          gMercProfiles[SKYRIDER].sSectorX = 0;
          gMercProfiles[SKYRIDER].sSectorY = 0;

          // see if we can find him and remove him if so....
          pSoldier = FindSoldierByProfileID(SKYRIDER, FALSE);

          // ATE: Don't do this if buddy is on our team!
          if (pSoldier != NULL && pSoldier->bTeam != gbPlayerNum) {
            TacticalRemoveSoldier(pSoldier->ubID);
          }
        }
      }

      // Invalidate rendering
      InvalidateWorldRedundency();

      // can't be 2 places at once!
      break;
    }
  }
}

void HandleHelicopterOnGroundSkyriderProfile(void) {
  uint8_t ubSite = 0;
  struct SOLDIERTYPE *pSoldier;

  // no worries if underground
  if (gbWorldSectorZ != 0) {
    return;
  }

  for (ubSite = 0; ubSite < NUMBER_OF_REFUEL_SITES; ubSite++) {
    // is this refueling site sector the loaded sector ?
    if ((ubRefuelList[ubSite][0] == gWorldSectorX) && (ubRefuelList[ubSite][1] == gWorldSectorY)) {
      // YES, so find out if the chopper is landed here
      if (IsHelicopterOnGroundAtRefuelingSite(ubSite)) {
        // ATE: Add skyridder too
        // ATE: only if hired......
        if (fHelicopterAvailable) {
          gMercProfiles[SKYRIDER].sSectorX = gWorldSectorX;
          gMercProfiles[SKYRIDER].sSectorY = gWorldSectorY;
        }
      } else {
        // ATE: Remove skyridder....
        if (fHelicopterAvailable) {
          gMercProfiles[SKYRIDER].sSectorX = 0;
          gMercProfiles[SKYRIDER].sSectorY = 0;

          // see if we can find him and remove him if so....
          pSoldier = FindSoldierByProfileID(SKYRIDER, FALSE);

          // ATE: Don't do this if buddy is on our team!
          if (pSoldier != NULL && pSoldier->bTeam != gbPlayerNum) {
            TacticalRemoveSoldier(pSoldier->ubID);
          }
        }
      }

      // can't be 2 places at once!
      break;
    }
  }
}

BOOLEAN IsHelicopterOnGroundAtRefuelingSite(uint8_t ubRefuelingSite) {
  if (fHelicopterDestroyed) {
    return (FALSE);
  }

  if (fHelicopterIsAirBorne) {
    return (FALSE);
  }

  // if we haven't even met SkyRider
  if (!fSkyRiderSetUp) {
    // then it's always at Drassen
    if (ubRefuelingSite == DRASSEN_REFUELING_SITE) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  // skyrider is setup, helicopter isn't destroyed, so this ought to be a valid vehicle id
  Assert(iHelicopterVehicleId != -1);

  // on the ground, but is it at this site or at another one?
  if ((ubRefuelList[ubRefuelingSite][0] == pVehicleList[iHelicopterVehicleId].sSectorX) &&
      (ubRefuelList[ubRefuelingSite][1] == pVehicleList[iHelicopterVehicleId].sSectorY)) {
    return (TRUE);
  }

  // not here
  return (FALSE);
}

void HeliCrashSoundStopCallback(void *pData) { SkyriderDestroyed(); }

BOOLEAN HandleSAMSiteAttackOfHelicopterInSector(uint8_t sSectorX, uint8_t sSectorY) {
  uint8_t ubChance;

  // if this sector is in friendly airspace, we're safe
  if (!IsSectorEnemyAirControlled(sSectorX, sSectorY)) {
    // no problem, friendly airspace
    return (FALSE);
  }

  // which SAM controls this sector?
  struct OptionalSamSite samSite = GetSamControllingSector(sSectorX, sSectorY);

  // if none of them
  if (samSite.tag == None) {
    return (FALSE);
  }

  uint8_t bSAMCondition = GetSamCondition(samSite.some);

  // if the SAM site is too damaged to be a threat
  if (bSAMCondition < MIN_CONDITION_FOR_SAM_SITE_TO_WORK) {
    // no problem, SAM site not working
    return (FALSE);
  }

#ifdef JA2TESTVERSION
  if (fSAMSitesDisabledFromAttackingPlayer == TRUE) {
    return (FALSE);
  }
#endif
  // Hostile airspace controlled by a working SAM site, so SAM site fires a SAM at Skyrider!!!

  // calc chance that chopper will be shot down
  ubChance = bSAMCondition;

  // there's a fair chance of a miss even if the SAM site is in perfect working order
  if (ubChance > MAX_SAM_SITE_ACCURACY) {
    ubChance = MAX_SAM_SITE_ACCURACY;
  }

  if (PreRandom(100) < ubChance) {
    // another hit!
    gubHelicopterHitsTaken++;

    // Took a hit!  Pause time so player can reconsider
    StopTimeCompression();

    // first hit?
    if (gubHelicopterHitsTaken == 1) {
      HeliCharacterDialogue(pSkyRider, HELI_TOOK_MINOR_DAMAGE);
    }
    // second hit?
    else if (gubHelicopterHitsTaken == 2) {
      // going back to base (no choice, dialogue says so)
      HeliCharacterDialogue(pSkyRider, HELI_TOOK_MAJOR_DAMAGE);
      MakeHeliReturnToBase();
    }
    // third hit!
    else {
      // Important: Skyrider must still be alive when he talks, so must do this before heli is
      // destroyed!
      HeliCharacterDialogue(pSkyRider, HELI_GOING_DOWN);

      // everyone die die die
      // play sound
      if (PlayJA2StreamingSampleFromFile("stsounds\\blah2.wav", RATE_11025, HIGHVOLUME, 1,
                                         MIDDLEPAN, HeliCrashSoundStopCallback) == SOUND_ERROR) {
        // Destroy here if we cannot play streamed sound sample....
        SkyriderDestroyed();
      } else {
        // otherwise it's handled in the callback
        // remove any arrival events for the helicopter's group
        DeleteStrategicEvent(EVENT_GROUP_ARRIVAL,
                             pVehicleList[iHelicopterVehicleId].ubMovementGroup);
      }

      // special return code indicating heli was destroyed
      return (TRUE);
    }
  }

  // still flying
  return (FALSE);
}

// are we at the end of the path for the heli?
BOOLEAN EndOfHelicoptersPath(void) {
  if (pVehicleList[iHelicopterVehicleId].pMercPath == NULL) {
    return (TRUE);
  }

  if (pVehicleList[iHelicopterVehicleId].pMercPath->pNext == NULL) {
    return (TRUE);
  }

  return (FALSE);
}

// check if helicopter can take off?
BOOLEAN CanHelicopterTakeOff(void) {
  // if it's already in the air
  if (fHelicopterIsAirBorne == TRUE) {
    return (TRUE);
  }

  if (!IsSectorEnemyControlled((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                               (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY)) {
    return (TRUE);
  }

  return (FALSE);
}

void AddHeliPeice(int16_t sGridNo, uint16_t sOStruct) {
  uint16_t usDummy;

  // ATE: Check first if already exists....
  if (!TypeExistsInStructLayer(sGridNo, sOStruct, &usDummy)) {
    // place in the world
    AddStructToTail(sGridNo, sOStruct);
  }
}

void AddHelicopterToMaps(BOOLEAN fAdd, uint8_t ubSite) {
  int16_t sGridNo = sRefuelStartGridNo[ubSite];
  int16_t sOStruct = 0;
  int16_t usGridNo;
  int16_t sGridX, sGridY;
  int16_t sCentreGridX, sCentreGridY;

  // find out what slot it is by which site
  if (ubSite == 0) {
    // drassen
    sOStruct = FIRSTOSTRUCT1;
  } else {
    // estoni
    sOStruct = FOURTHOSTRUCT1;
  }

  // are we adding or taking away
  if (fAdd) {
    AddHeliPeice(sGridNo, sOStruct);
    AddHeliPeice(sGridNo, (uint16_t)(sOStruct + 1));
    AddHeliPeice((int16_t)(sGridNo - 800), (uint16_t)(sOStruct + 2));
    AddHeliPeice(sGridNo, (uint16_t)(sOStruct + 3));
    AddHeliPeice(sGridNo, (uint16_t)(sOStruct + 4));
    AddHeliPeice((int16_t)(sGridNo - 800), (uint16_t)(sOStruct + 5));

    InvalidateWorldRedundency();
    SetRenderFlags(RENDER_FLAG_FULL);

    // ATE: If any mercs here, bump them off!
    ConvertGridNoToXY(sGridNo, &sCentreGridX, &sCentreGridY);

    for (sGridY = sCentreGridY - 5; sGridY < sCentreGridY + 5; sGridY++) {
      for (sGridX = sCentreGridX - 5; sGridX < sCentreGridX + 5; sGridX++) {
        usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);

        BumpAnyExistingMerc(usGridNo);
      }
    }
  } else {
    // remove from the world
    RemoveStruct(sRefuelStartGridNo[ubSite], (uint16_t)(sOStruct));
    RemoveStruct(sRefuelStartGridNo[ubSite], (uint16_t)(sOStruct + 1));
    RemoveStruct(sRefuelStartGridNo[ubSite] - 800, (uint16_t)(sOStruct + 2));
    RemoveStruct(sRefuelStartGridNo[ubSite], (uint16_t)(sOStruct + 3));
    RemoveStruct(sRefuelStartGridNo[ubSite], (uint16_t)(sOStruct + 4));
    RemoveStruct(sRefuelStartGridNo[ubSite] - 800, (uint16_t)(sOStruct + 5));

    InvalidateWorldRedundency();
    SetRenderFlags(RENDER_FLAG_FULL);
  }
}

BOOLEAN IsSkyriderIsFlyingInSector(uint8_t sSectorX, uint8_t sSectorY) {
  struct GROUP *pGroup;

  // up and about?
  if (fHelicopterAvailable && (iHelicopterVehicleId != -1) && CanHelicopterFly() &&
      fHelicopterIsAirBorne) {
    pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);

    // the right sector?
    if ((sSectorX == pGroup->ubSectorX) && (sSectorY == pGroup->ubSectorY)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN IsGroupTheHelicopterGroup(struct GROUP *pGroup) {
  if ((iHelicopterVehicleId != -1) && VehicleIdIsValid(iHelicopterVehicleId) &&
      (pVehicleList[iHelicopterVehicleId].ubMovementGroup != 0) &&
      (pVehicleList[iHelicopterVehicleId].ubMovementGroup == pGroup->ubGroupID)) {
    return (TRUE);
  }

  return (FALSE);
}

int16_t GetNumSafeSectorsInPath(void) {
  // get the last sector value in the helictoper's path
  struct path *pNode = NULL;
  uint32_t uiLocation = 0;
  uint32_t uiCount = 0;
  int32_t iHeliSector = -1;
  struct GROUP *pGroup;

  // if the heli is on the move, what is the distance it will move..the length of the merc path,
  // less the first node
  if (CanHelicopterFly() == FALSE) {
    // big number, no go
    return (0);
  }

  // may need to skip the sector the chopper is currently in
  iHeliSector = GetSectorID16((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                              (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY);

  // get chopper's group ptr
  pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  // any path yet?
  if (pNode != NULL) {
    // first node: skip it if that's the sector the chopper is currently in, AND
    // we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt
    // AFTER plotting is done)
    if (((int32_t)pNode->uiSectorId == iHeliSector) && (pNode->pNext != NULL) &&
        !GroupBetweenSectorsAndSectorXYIsInDifferentDirection(
            pGroup, SectorID16_X(pNode->pNext->uiSectorId),
            SectorID16_Y(pNode->pNext->uiSectorId))) {
      pNode = pNode->pNext;
    }

    while (pNode) {
      uiLocation = pNode->uiSectorId;

      if (!IsSectorEnemyAirControlled(SectorID16_X(uiLocation), SectorID16_Y(uiLocation))) {
        uiCount++;
      }

      pNode = pNode->pNext;
    }
  }

  pNode = MoveToBeginningOfPathList(pTempHelicopterPath);
  // any path yet?
  if (pNode != NULL) {
    // first node: skip it if that's the sector the chopper is currently in, AND
    // we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt
    // AFTER plotting is done) OR if the chopper has a mercpath, in which case this a continuation
    // of it that would count the sector twice
    if ((((int32_t)pNode->uiSectorId == iHeliSector) && (pNode->pNext != NULL) &&
         !GroupBetweenSectorsAndSectorXYIsInDifferentDirection(
             pGroup, SectorID16_X(pNode->pNext->uiSectorId),
             SectorID16_Y(pNode->pNext->uiSectorId))) ||
        (GetLengthOfPath(pVehicleList[iHelicopterVehicleId].pMercPath) > 0)) {
      pNode = pNode->pNext;
    }

    while (pNode) {
      uiLocation = pNode->uiSectorId;

      if (!IsSectorEnemyAirControlled(SectorID16_X(uiLocation), SectorID16_Y(uiLocation))) {
        uiCount++;
      }

      pNode = pNode->pNext;
    }
  }

  return ((int16_t)uiCount);
}

int16_t GetNumUnSafeSectorsInPath(void) {
  // get the last sector value in the helictoper's path
  struct path *pNode = NULL;
  uint32_t uiLocation = 0;
  uint32_t uiCount = 0;
  int32_t iHeliSector = -1;
  struct GROUP *pGroup;

  // if the heli is on the move, what is the distance it will move..the length of the merc path,
  // less the first node
  if (CanHelicopterFly() == FALSE) {
    // big number, no go
    return (0);
  }

  // may need to skip the sector the chopper is currently in
  iHeliSector = GetSectorID16((uint8_t)pVehicleList[iHelicopterVehicleId].sSectorX,
                              (uint8_t)pVehicleList[iHelicopterVehicleId].sSectorY);

  // get chopper's group ptr
  pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);

  pNode = pVehicleList[iHelicopterVehicleId].pMercPath;

  // any path yet?
  if (pNode != NULL) {
    // first node: skip it if that's the sector the chopper is currently in, AND
    // we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt
    // AFTER plotting is done)
    if (((int32_t)pNode->uiSectorId == iHeliSector) && (pNode->pNext != NULL) &&
        !GroupBetweenSectorsAndSectorXYIsInDifferentDirection(
            pGroup, SectorID16_X(pNode->pNext->uiSectorId),
            SectorID16_Y(pNode->pNext->uiSectorId))) {
      pNode = pNode->pNext;
    }

    while (pNode) {
      uiLocation = pNode->uiSectorId;

      if (IsSectorEnemyAirControlled(SectorID16_X(uiLocation), SectorID16_Y(uiLocation))) {
        uiCount++;
      }

      pNode = pNode->pNext;
    }
  }

  pNode = MoveToBeginningOfPathList(pTempHelicopterPath);
  // any path yet?
  if (pNode != NULL) {
    // first node: skip it if that's the sector the chopper is currently in, AND
    // we're NOT gonna be changing directions (not actually performed until waypoints are rebuilt
    // AFTER plotting is done) OR if the chopper has a mercpath, in which case this a continuation
    // of it that would count the sector twice
    if ((((int32_t)pNode->uiSectorId == iHeliSector) && (pNode->pNext != NULL) &&
         !GroupBetweenSectorsAndSectorXYIsInDifferentDirection(
             pGroup, SectorID16_X(pNode->pNext->uiSectorId),
             SectorID16_Y(pNode->pNext->uiSectorId))) ||
        (GetLengthOfPath(pVehicleList[iHelicopterVehicleId].pMercPath) > 0)) {
      pNode = pNode->pNext;
    }

    while (pNode) {
      uiLocation = pNode->uiSectorId;

      if (IsSectorEnemyAirControlled(SectorID16_X(uiLocation), SectorID16_Y(uiLocation))) {
        uiCount++;
      }

      pNode = pNode->pNext;
    }
  }

  return ((int16_t)uiCount);
}

void PaySkyriderBill(void) {
  // if we owe anything for the trip
  if (iTotalAccumulatedCostByPlayer > 0) {
    // if player can afford to pay the Skyrider bill
    if (LaptopMoneyGetBalance() >= iTotalAccumulatedCostByPlayer) {
      // no problem, pay the man
      // add the transaction
      AddTransactionToPlayersBook(PAYMENT_TO_NPC, SKYRIDER, -iTotalAccumulatedCostByPlayer);
      ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[0], iTotalAccumulatedCostByPlayer);
    } else {
      // money owed
      if (LaptopMoneyGetBalance() > 0) {
        ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[0], LaptopMoneyGetBalance());
        gMercProfiles[SKYRIDER].iBalance = LaptopMoneyGetBalance() - iTotalAccumulatedCostByPlayer;
        // add the transaction
        AddTransactionToPlayersBook(PAYMENT_TO_NPC, SKYRIDER, -LaptopMoneyGetBalance());
      } else {
        gMercProfiles[SKYRIDER].iBalance = -iTotalAccumulatedCostByPlayer;
      }

      HeliCharacterDialogue(pSkyRider, OWED_MONEY_TO_SKYRIDER);
      ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[1],
                -gMercProfiles[SKYRIDER].iBalance);

      // kick everyone out! (we know we're in a safe sector if we're paying)
      MoveAllInHelicopterToFootMovementGroup();

      MakeHeliReturnToBase();
    }

    iTotalAccumulatedCostByPlayer = 0;
  }
}

void PayOffSkyriderDebtIfAny() {
  int32_t iAmountOwed;
  int32_t iPayAmount;

  iAmountOwed = -gMercProfiles[SKYRIDER].iBalance;

  // if we owe him anything, and have any money
  if ((iAmountOwed > 0) && (LaptopMoneyGetBalance() > 0)) {
    iPayAmount = min(iAmountOwed, LaptopMoneyGetBalance());

    // pay the man what we can
    gMercProfiles[SKYRIDER].iBalance += iPayAmount;
    // add the transaction
    AddTransactionToPlayersBook(PAYMENT_TO_NPC, SKYRIDER, -iPayAmount);
    // tell player
    ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[0], iPayAmount);

    // now whaddawe owe?
    iAmountOwed = -gMercProfiles[SKYRIDER].iBalance;

    // if it wasn't enough
    if (iAmountOwed > 0) {
      ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pSkyriderText[1], iAmountOwed);
      HeliCharacterDialogue(pSkyRider, OWED_MONEY_TO_SKYRIDER);
    }
  }
}

void MakeHeliReturnToBase(void) {
  int32_t iLocation = 0;

  // if already at a refueling point
  if (CheckForArrivalAtRefuelPoint()) {
    ReFuelHelicopter();
  } else {
    // choose destination (closest refueling sector)
    iLocation = LocationOfNearestRefuelPoint(TRUE);

    // null out path
    pVehicleList[iHelicopterVehicleId].pMercPath =
        ClearStrategicPathList(pVehicleList[iHelicopterVehicleId].pMercPath,
                               pVehicleList[iHelicopterVehicleId].ubMovementGroup);

    // plot path to that sector
    pVehicleList[iHelicopterVehicleId].pMercPath = AppendStrategicPath(
        MoveToBeginningOfPathList(BuildAStrategicPath(
            NULL, GetLastSectorIdInVehiclePath(iHelicopterVehicleId),
            (int16_t)(GetSectorID16(ubRefuelList[iLocation][0], ubRefuelList[iLocation][1])),
            pVehicleList[iHelicopterVehicleId].ubMovementGroup, FALSE /*, FALSE */)),
        pVehicleList[iHelicopterVehicleId].pMercPath);
    pVehicleList[iHelicopterVehicleId].pMercPath =
        MoveToBeginningOfPathList(pVehicleList[iHelicopterVehicleId].pMercPath);

    // rebuild the movement waypoints
    RebuildWayPointsForGroupPath(pVehicleList[iHelicopterVehicleId].pMercPath,
                                 pVehicleList[iHelicopterVehicleId].ubMovementGroup);

    fHeliReturnStraightToBase = TRUE;
    fHoveringHelicopter = FALSE;
  }

  // stop time compression if it's on so player can digest this
  StopTimeCompression();
}

BOOLEAN SoldierAboardAirborneHeli(struct SOLDIERTYPE *pSoldier) {
  Assert(pSoldier);

  // if not in a vehicle, or not aboard the helicopter
  if ((pSoldier->bAssignment != VEHICLE) || (pSoldier->iVehicleId != iHelicopterVehicleId)) {
    return (FALSE);
  }

  // he's in the heli - is it airborne?
  if (!fHelicopterIsAirBorne) {
    // nope, it's currently on the ground
    return (FALSE);
  }

  // yes, airborne
  return (TRUE);
}
