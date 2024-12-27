#include "Tactical/MercHiring.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JAScreens.h"
#include "Laptop/History.h"
#include "Laptop/LaptopSave.h"
#include "Laptop/Personnel.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MercContract.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMercHandler.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/Items.h"
#include "Tactical/Overhead.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "TacticalAI/AI.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#define MIN_FLIGHT_PREP_TIME 6

#ifdef JA2TESTVERSION
BOOLEAN gForceHireMerc = FALSE;
void SetFlagToForceHireMerc(BOOLEAN fForceHire);
#endif

extern BOOLEAN gfTacticalDoHeliRun;
extern BOOLEAN gfFirstHeliRun;

// ATE: Globals that dictate where the mercs will land once being hired
// Default to Omerta
// Saved in general saved game structure
INT16 gsMercArriveSectorX = 9;
INT16 gsMercArriveSectorY = 1;

void CheckForValidArrivalSector();

INT8 HireMerc(MERC_HIRE_STRUCT *pHireMerc) {
  struct SOLDIERTYPE *pSoldier;
  uint8_t iNewIndex;
  uint8_t ubCurrentSoldier = pHireMerc->ubProfileID;
  MERCPROFILESTRUCT *pMerc;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  BOOLEAN fReturn = FALSE;
  pMerc = &gMercProfiles[ubCurrentSoldier];

// If we are to disregard the ststus of the merc
#ifdef JA2TESTVERSION
  if (!gForceHireMerc)
#endif
    // If the merc is away, Dont hire him, or if the merc is only slightly annoyed at the player
    if ((pMerc->bMercStatus != 0) && (pMerc->bMercStatus != MERC_ANNOYED_BUT_CAN_STILL_CONTACT) &&
        (pMerc->bMercStatus != MERC_HIRED_BUT_NOT_ARRIVED_YET))
      return (MERC_HIRE_FAILED);

  if (NumberOfMercsOnPlayerTeam() >= 18) return (MERC_HIRE_OVER_20_MERCS_HIRED);

  // ATE: if we are to use landing zone, update to latest value
  // they will be updated again just before arrival...
  if (pHireMerc->fUseLandingZoneForArrival) {
    pHireMerc->sSectorX = gsMercArriveSectorX;
    pHireMerc->sSectorY = gsMercArriveSectorY;
    pHireMerc->bSectorZ = 0;
  }

  // BUILD STRUCTURES
  memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
  MercCreateStruct.ubProfile = ubCurrentSoldier;
  MercCreateStruct.fPlayerMerc = TRUE;
  MercCreateStruct.sSectorX = pHireMerc->sSectorX;
  MercCreateStruct.sSectorY = pHireMerc->sSectorY;
  MercCreateStruct.bSectorZ = pHireMerc->bSectorZ;
  MercCreateStruct.bTeam = SOLDIER_CREATE_AUTO_TEAM;
  MercCreateStruct.fCopyProfileItemsOver = pHireMerc->fCopyProfileItemsOver;

  if (!TacticalCreateSoldier(&MercCreateStruct, &iNewIndex)) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "TacticalCreateSoldier in HireMerc():  Failed to Add Merc");
    return (MERC_HIRE_FAILED);
  }

  if (DidGameJustStart()) {
    // OK, CHECK FOR FIRST GUY, GIVE HIM SPECIAL ITEM!
    if (iNewIndex == 0) {
      // OK, give this item to our merc!
      struct OBJECTTYPE Object;

      // make an objecttype
      memset(&Object, 0, sizeof(struct OBJECTTYPE));
      Object.usItem = LETTER;
      Object.ubNumberOfObjects = 1;
      Object.bStatus[0] = 100;
      // Give it
      fReturn = AutoPlaceObject(MercPtrs[iNewIndex], &Object, FALSE);
      Assert(fReturn);
    }

    // Set insertion for first time in chopper

    // ATE: Insert for demo , not using the heli sequence....
    pHireMerc->ubInsertionCode = INSERTION_CODE_CHOPPER;
  }

  // record how long the merc will be gone for
  pMerc->bMercStatus = (uint8_t)pHireMerc->iTotalContractLength;

  pSoldier = GetSoldierByID(iNewIndex);

  // Copy over insertion data....
  pSoldier->ubStrategicInsertionCode = pHireMerc->ubInsertionCode;
  pSoldier->usStrategicInsertionData = pHireMerc->usInsertionData;
  // ATE: Copy over value for using alnding zone to soldier type
  pSoldier->fUseLandingZoneForArrival = pHireMerc->fUseLandingZoneForArrival;

  // Set assignment
  // ATE: If first time, make ON_DUTY, otherwise GUARD
  if ((pSoldier->bAssignment != IN_TRANSIT)) {
    SetTimeOfAssignmentChangeForMerc(pSoldier);
  }
  ChangeSoldiersAssignment(pSoldier, IN_TRANSIT);

  // set the contract length
  pSoldier->iTotalContractLength = pHireMerc->iTotalContractLength;

  // reset the insurance values
  pSoldier->iStartOfInsuranceContract = 0;
  pSoldier->iTotalLengthOfInsuranceContract = 0;

  // Init the contract charge
  //	pSoldier->iTotalContractCharge = 0;

  // store arrival time in soldier structure so map screen can display it
  pSoldier->uiTimeSoldierWillArrive = pHireMerc->uiTimeTillMercArrives;

  // Set the type of merc

  if (DidGameJustStart()) {
    // Set time of initial merc arrival in minutes
    pHireMerc->uiTimeTillMercArrives = (STARTING_TIME + FIRST_ARRIVAL_DELAY) / NUM_SEC_IN_MIN;

    // Set insertion for first time in chopper
    pHireMerc->ubInsertionCode = INSERTION_CODE_CHOPPER;

    // set when the merc's contract is finished
    pSoldier->iEndofContractTime = GetMidnightOfFutureDayInMinutes(pSoldier->iTotalContractLength) +
                                   (GetHourWhenContractDone(pSoldier) * 60);
  } else {
    // set when the merc's contract is finished ( + 1 cause it takes a day for the merc to arrive )
    pSoldier->iEndofContractTime =
        GetMidnightOfFutureDayInMinutes(1 + pSoldier->iTotalContractLength) +
        (GetHourWhenContractDone(pSoldier) * 60);
  }

  // Set the time and ID of the last hired merc will arrive
  LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = pHireMerc->ubProfileID;
  LaptopSaveInfo.sLastHiredMerc.uiArrivalTime = pHireMerc->uiTimeTillMercArrives;

  // if we are trying to hire a merc that should arrive later, put the merc in the queue
  if (pHireMerc->uiTimeTillMercArrives != 0) {
    AddStrategicEvent(EVENT_DELAYED_HIRING_OF_MERC, pHireMerc->uiTimeTillMercArrives,
                      GetSolID(pSoldier));

    // specify that the merc is hired but hasnt arrived yet
    pMerc->bMercStatus = MERC_HIRED_BUT_NOT_ARRIVED_YET;
  }

  // if the merc is an AIM merc
  if (ubCurrentSoldier < 40) {
    pSoldier->ubWhatKindOfMercAmI = MERC_TYPE__AIM_MERC;
    // determine how much the contract is, and remember what type of contract he got
    if (pHireMerc->iTotalContractLength == 1) {
      // pSoldier->iTotalContractCharge = gMercProfiles[ GetSolProfile(pSoldier) ].sSalary;
      pSoldier->bTypeOfLastContract = CONTRACT_EXTEND_1_DAY;
      pSoldier->iTimeCanSignElsewhere = GetWorldTotalMin();
    } else if (pHireMerc->iTotalContractLength == 7) {
      // pSoldier->iTotalContractCharge = gMercProfiles[ GetSolProfile(pSoldier) ].uiWeeklySalary;
      pSoldier->bTypeOfLastContract = CONTRACT_EXTEND_1_WEEK;
      pSoldier->iTimeCanSignElsewhere = GetWorldTotalMin();
    } else if (pHireMerc->iTotalContractLength == 14) {
      // pSoldier->iTotalContractCharge = gMercProfiles[ GetSolProfile(pSoldier) ].uiBiWeeklySalary;
      pSoldier->bTypeOfLastContract = CONTRACT_EXTEND_2_WEEK;
      // These luck fellows need to stay the whole duration!
      pSoldier->iTimeCanSignElsewhere = pSoldier->iEndofContractTime;
    }

    // remember the medical deposit we PAID.  The one in his profile can increase when he levels!
    pSoldier->usMedicalDeposit = gMercProfiles[GetSolProfile(pSoldier)].sMedicalDepositAmount;
  }
  // if the merc is from M.E.R.C.
  else if ((ubCurrentSoldier >= 40) && (ubCurrentSoldier <= 50)) {
    pSoldier->ubWhatKindOfMercAmI = MERC_TYPE__MERC;
    // pSoldier->iTotalContractCharge = -1;

    gMercProfiles[GetSolProfile(pSoldier)].iMercMercContractLength = 1;

    // Set starting conditions for the merc
    pSoldier->iStartContractTime = GetWorldDay();

    AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_MERC, ubCurrentSoldier, GetWorldTotalMin(), -1,
                           -1);
  }
  // If the merc is from IMP, (ie a player character)
  else if ((ubCurrentSoldier >= 51) && (ubCurrentSoldier < 57)) {
    pSoldier->ubWhatKindOfMercAmI = MERC_TYPE__PLAYER_CHARACTER;
    // pSoldier->iTotalContractCharge = -1;
  }
  // else its a NPC merc
  else {
    pSoldier->ubWhatKindOfMercAmI = MERC_TYPE__NPC;
    // pSoldier->iTotalContractCharge = -1;
  }

  // remove the merc from the Personnel screens departed list ( if they have never been hired
  // before, its ok to call it )
  RemoveNewlyHiredMercFromPersonnelDepartedList(GetSolProfile(pSoldier));

  gfAtLeastOneMercWasHired = TRUE;
  return (MERC_HIRE_OK);
}

void MercArrivesCallback(uint8_t ubSoldierID) {
  MERCPROFILESTRUCT *pMerc;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiTimeOfPost;

  if (!DidGameJustStart() && gsMercArriveSectorX == 9 &&
      gsMercArriveSectorY ==
          1) {  // Mercs arriving in A9.  This sector has been deemed as the always safe sector.
    // Seeing we don't support entry into a hostile sector (except for the beginning),
    // we will nuke any enemies in this sector first.
    if (gWorldSectorX != 9 || gWorldSectorY != 1 || gbWorldSectorZ) {
      EliminateAllEnemies((uint8_t)gsMercArriveSectorX, (uint8_t)gsMercArriveSectorY);
    }
  }

  // This will update ANY soldiers currently schedules to arrive too
  CheckForValidArrivalSector();

  // stop time compression until player restarts it
  StopTimeCompression();

  pSoldier = GetSoldierByID(ubSoldierID);

  pMerc = &gMercProfiles[GetSolProfile(pSoldier)];

  // add the guy to a squad
  AddCharacterToAnySquad(pSoldier);

  // ATE: Make sure we use global.....
  if (pSoldier->fUseLandingZoneForArrival) {
    pSoldier->sSectorX = gsMercArriveSectorX;
    pSoldier->sSectorY = gsMercArriveSectorY;
    pSoldier->bSectorZ = 0;
  }

  // Add merc to sector ( if it's the current one )
  if (gWorldSectorX == GetSolSectorX(pSoldier) && gWorldSectorY == GetSolSectorY(pSoldier) &&
      GetSolSectorZ(pSoldier) == gbWorldSectorZ) {
    // OK, If this sector is currently loaded, and guy does not have CHOPPER insertion code....
    // ( which means we are at beginning of game if so )
    // Setup chopper....
    if (pSoldier->ubStrategicInsertionCode != INSERTION_CODE_CHOPPER &&
        GetSolSectorX(pSoldier) == 9 && GetSolSectorY(pSoldier) == 1) {
      gfTacticalDoHeliRun = TRUE;

      // OK, If we are in mapscreen, get out...
      if (IsMapScreen_2()) {
        // ATE: Make sure the current one is selected!
        ChangeSelectedMapSector(gWorldSectorX, gWorldSectorY, 0);

        RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
      }

      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_CHOPPER;
    }

    UpdateMercInSector(pSoldier, GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                       GetSolSectorZ(pSoldier));
  } else {
    // OK, otherwise, set them in north area, so once we load again, they are here.
    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
  }

  if (pSoldier->ubStrategicInsertionCode != INSERTION_CODE_CHOPPER) {
    ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, TacticalStr[MERC_HAS_ARRIVED_STR], pSoldier->name);

    // ATE: He's going to say something, now that they've arrived...
    if (gTacticalStatus.bMercArrivingQuoteBeingUsed == FALSE && !gfFirstHeliRun) {
      gTacticalStatus.bMercArrivingQuoteBeingUsed = TRUE;

      // Setup the highlight sector value (note this isn't for mines but using same system)
      gsSectorLocatorX = GetSolSectorX(pSoldier);
      gsSectorLocatorY = GetSolSectorY(pSoldier);

      TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT,
                                                2, 0);
      TacticalCharacterDialogue(pSoldier, QUOTE_MERC_REACHED_DESTINATION);
      TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT,
                                                3, 0);
      TacticalCharacterDialogueWithSpecialEventEx(
          pSoldier, 0, DIALOGUE_SPECIAL_EVENT_UNSET_ARRIVES_FLAG, 0, 0, 0);
    }
  }

  // record how long the merc will be gone for
  pMerc->bMercStatus = (uint8_t)pSoldier->iTotalContractLength;

  // remember when excatly he ARRIVED in Arulco, in case he gets fired early
  pSoldier->uiTimeOfLastContractUpdate = GetWorldTotalMin();

  // set when the merc's contract is finished
  pSoldier->iEndofContractTime = GetMidnightOfFutureDayInMinutes(pSoldier->iTotalContractLength) +
                                 (GetHourWhenContractDone(pSoldier) * 60);

  // Do initial check for bad items
  if (pSoldier->bTeam == gbPlayerNum) {
    // ATE: Try to see if our equipment sucks!
    if (SoldierHasWorseEquipmentThanUsedTo(pSoldier)) {
      // Randomly anytime between 9:00, and 10:00
      uiTimeOfPost = 540 + Random(660);

      if (GetWorldMinutesInDay() < uiTimeOfPost) {
        AddSameDayStrategicEvent(EVENT_MERC_COMPLAIN_EQUIPMENT, uiTimeOfPost,
                                 GetSolProfile(pSoldier));
      }
    }
  }

  HandleMercArrivesQuotes(pSoldier);

  fTeamPanelDirty = TRUE;

  // if the currently selected sector has no one in it, select this one instead
  if (!CanGoToTacticalInSector(sSelMapX, sSelMapY, (uint8_t)iCurrentMapSectorZ)) {
    ChangeSelectedMapSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), 0);
  }

  return;
}

BOOLEAN IsMercHireable(uint8_t ubMercID) {
  // If the merc has an .EDT file, is not away on assignment, and isnt already hired (but not
  // arrived yet), he is not DEAD and he isnt returning home
  if ((gMercProfiles[ubMercID].bMercStatus == MERC_HAS_NO_TEXT_FILE) ||
      (gMercProfiles[ubMercID].bMercStatus > 0) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_HIRED_BUT_NOT_ARRIVED_YET) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_IS_DEAD) ||
      (gMercProfiles[ubMercID].uiDayBecomesAvailable > 0) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_WORKING_ELSEWHERE) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_FIRED_AS_A_POW) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_RETURNING_HOME))
    return (FALSE);
  else
    return (TRUE);
}

BOOLEAN IsMercDead(uint8_t ubMercID) {
  if (gMercProfiles[ubMercID].bMercStatus == MERC_IS_DEAD)
    return (TRUE);
  else
    return (FALSE);
}

BOOLEAN IsTheSoldierAliveAndConcious(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->bLife >= CONSCIOUSNESS)
    return (TRUE);
  else
    return (FALSE);
}

uint8_t NumberOfMercsOnPlayerTeam() {
  INT8 cnt;
  struct SOLDIERTYPE *pSoldier;
  INT16 bLastTeamID;
  uint8_t ubCount = 0;

  // Set locator to first merc
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

  for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
    // if the is active, and is not a vehicle
    if (IsSolActive(pSoldier) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      ubCount++;
    }
  }

  return (ubCount);
}

void HandleMercArrivesQuotes(struct SOLDIERTYPE *pSoldier) {
  INT8 cnt, bHated, bLastTeamID;
  struct SOLDIERTYPE *pTeamSoldier;

  // If we are approaching with helicopter, don't say any ( yet )
  if (pSoldier->ubStrategicInsertionCode != INSERTION_CODE_CHOPPER) {
    // Player-generated characters issue a comment about arriving in Omerta.
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER) {
      if (gubQuest[QUEST_DELIVER_LETTER] == QUESTINPROGRESS) {
        TacticalCharacterDialogue(pSoldier, QUOTE_PC_DROPPED_OMERTA);
      }
    }

    // Check to see if anyone hates this merc and will now complain
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;
    // loop though all the mercs
    for (pTeamSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pTeamSoldier++) {
      if (pTeamSoldier->bActive) {
        if (pTeamSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
          bHated = WhichHated(pTeamSoldier->ubProfile, GetSolProfile(pSoldier));
          if (bHated != -1) {
            // hates the merc who has arrived and is going to gripe about it!
            switch (bHated) {
              case 0:
                TacticalCharacterDialogue(pTeamSoldier, QUOTE_HATED_1_ARRIVES);
                break;
              case 1:
                TacticalCharacterDialogue(pTeamSoldier, QUOTE_HATED_2_ARRIVES);
                break;
              default:
                break;
            }
          }
        }
      }
    }
  }
}

#ifdef JA2TESTVERSION
void SetFlagToForceHireMerc(BOOLEAN fForceHire) { gForceHireMerc = fForceHire; }
#endif

uint32_t GetMercArrivalTimeOfDay() {
  uint32_t uiCurrHour;
  uint32_t uiMinHour;

  // Pick a time...

  // First get the current time of day.....
  uiCurrHour = GetWorldHour();

  // Subtract the min time for any arrival....
  uiMinHour = uiCurrHour + MIN_FLIGHT_PREP_TIME;

  // OK, first check if we need to advance a whole day's time...
  // See if we have missed the last flight for the day...
  if ((uiCurrHour) > 13)  // ( > 1:00 pm - too bad )
  {
    // 7:30 flight....
    return (GetMidnightOfFutureDayInMinutes(1) + MERC_ARRIVE_TIME_SLOT_1);
  }

  // Well, now we can handle flights all in one day....
  // Find next possible flight
  if (uiMinHour <= 7) {
    return (GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_1);  // 7:30 am
  } else if (uiMinHour <= 13) {
    return (GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_2);  // 1:30 pm
  } else {
    return (GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_3);  // 7:30 pm
  }
}

void UpdateAnyInTransitMercsWithGlobalArrivalSector() {
  INT32 cnt;
  struct SOLDIERTYPE *pSoldier;

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
        if (pSoldier->fUseLandingZoneForArrival) {
          pSoldier->sSectorX = gsMercArriveSectorX;
          pSoldier->sSectorY = gsMercArriveSectorY;
          pSoldier->bSectorZ = 0;
        }
      }
    }
  }
}

INT16 StrategicPythSpacesAway(INT16 sOrigin, INT16 sDest) {
  INT16 sRows, sCols, sResult;

  sRows = abs((SectorID16_Y(sOrigin)) - (SectorID16_Y(sDest)));
  sCols = abs((SectorID16_X(sOrigin)) - (SectorID16_X(sDest)));

  // apply Pythagoras's theorem for right-handed triangle:
  // dist^2 = rows^2 + cols^2, so use the square root to get the distance
  sResult = (INT16)sqrt((float)(sRows * sRows) + (sCols * sCols));

  return (sResult);
}

// ATE: This function will check if the current arrival sector
// is valid
// if there are enemies present, it's invalid
// if so, search around for nearest non-occupied sector.
void CheckForValidArrivalSector() {
  INT16 sTop, sBottom;
  INT16 sLeft, sRight;
  INT16 cnt1, cnt2, sGoodX, sGoodY;
  uint8_t ubRadius = 4;
  INT32 leftmost;
  INT16 sSectorGridNo, sSectorGridNo2;
  INT32 uiRange, uiLowestRange = 999999;
  BOOLEAN fFound = FALSE;
  CHAR16 sString[1024];
  CHAR16 zShortTownIDString1[50];
  CHAR16 zShortTownIDString2[50];

  sSectorGridNo = gsMercArriveSectorX + (MAP_WORLD_X * gsMercArriveSectorY);

  // Check if valid...
  if (!StrategicMap[sSectorGridNo].fEnemyControlled) {
    return;
  }

  GetShortSectorString(gsMercArriveSectorX, gsMercArriveSectorY, zShortTownIDString1,
                       ARR_SIZE(zShortTownIDString1));

  // If here - we need to do a search!
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSectorGridNo + (MAP_WORLD_X * cnt1)) / MAP_WORLD_X) * MAP_WORLD_X;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sSectorGridNo2 = sSectorGridNo + (MAP_WORLD_X * cnt1) + cnt2;

      if (sSectorGridNo2 >= 1 && sSectorGridNo2 < ((MAP_WORLD_X - 1) * (MAP_WORLD_X - 1)) &&
          sSectorGridNo2 >= leftmost && sSectorGridNo2 < (leftmost + MAP_WORLD_X)) {
        if (!StrategicMap[sSectorGridNo2].fEnemyControlled &&
            !StrategicMap[sSectorGridNo2].fEnemyAirControlled) {
          uiRange = StrategicPythSpacesAway(sSectorGridNo2, sSectorGridNo);

          if (uiRange < uiLowestRange) {
            sGoodY = cnt1;
            sGoodX = cnt2;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }

  if (fFound) {
    gsMercArriveSectorX = gsMercArriveSectorX + sGoodX;
    gsMercArriveSectorY = gsMercArriveSectorY + sGoodY;

    UpdateAnyInTransitMercsWithGlobalArrivalSector();

    GetShortSectorString(gsMercArriveSectorX, gsMercArriveSectorY, zShortTownIDString2,
                         ARR_SIZE(zShortTownIDString2));

    swprintf(sString, ARR_SIZE(sString),
             L"Arrival of new recruits is being rerouted to sector %s, as scheduled drop-off point "
             L"of sector %s is enemy occupied.",
             zShortTownIDString2, zShortTownIDString1);

    DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
  }
}
