#include "Strategic/TownMilitia.h"

#include <string.h>

#include "CharList.h"
#include "Laptop/Finances.h"
#include "Militia.h"
#include "Money.h"
#include "SGP/Random.h"
#include "ScreenIDs.h"
#include "Sector.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Tactical.h"
#include "Tactical/DialogueControl.h"
#include "Team.h"
#include "Town.h"
#include "UI.h"
#include "Utils/Text.h"

// how many new green militia civilians are trained at a time
#define MILITIA_TRAINING_SQUAD_SIZE 10  // was 6

// cost of starting a new militia training assignment
#define MILITIA_TRAINING_COST 750

#define SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST 50

struct sectorSearch {
  UINT8 townID;
  INT16 skipX;
  INT16 skipY;
  UINT8 townSectorsIndex;
};

struct militiaState {
  BOOLEAN promptForContinue;  // whether we're starting new training, or continuing
  INT32 totalCostOfTraining;

  // the completed list of sector soldiers for training militia
  INT32 soldiersCompletedMilitiaTraining[SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST];
  struct SOLDIERTYPE *trainer;

  // note that these sector values are STRATEGIC INDEXES, not 0-255!
  SectorID16 unpaidSectors[MAX_CHARACTER_COUNT];

  i8 gbGreenToElitePromotions;
  i8 gbGreenToRegPromotions;
  i8 gbRegToElitePromotions;
  i8 gbMilitiaPromotions;

  struct sectorSearch sectorSearch;

  struct MilitiaCount sectorForce[256];
  bool trainingPaid[256];
};

static struct militiaState _st;

static void promoteMilitia(u8 mapX, u8 mapY, UINT8 ubCurrentRank, UINT8 ubHowMany);

// handle completion of assignment byt his soldier too and inform the player
static void handleTrainingComplete(struct SOLDIERTYPE *pTrainer);

static void PayMilitiaTrainingYesNoBoxCallback(UINT8 bExitValue);
static void CantTrainMilitiaOkBoxCallback(UINT8 bExitValue);
static void MilitiaTrainingRejected(void);
static void initNextSectorSearch(UINT8 ubTownId, INT16 sSkipSectorX, INT16 sSkipSectorY);
static BOOLEAN getNextSectorInTown(u8 *sNeighbourX, u8 *sNeighbourY);
static INT32 GetNumberOfUnpaidTrainableSectors(void);
static void ContinueTrainingInThisSector();
static void StartTrainingInAllUnpaidTrainableSectors();
static void PayForTrainingInSector(SectorID8 ubSector);
static void ResetDoneFlagForAllMilitiaTrainersInSector(SectorID8 ubSector);

void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, u8 mapX, u8 mapY) {
  UINT8 ubMilitiaTrained = 0;
  BOOLEAN fFoundOne;
  u8 sNeighbourX, sNeighbourY;
  UINT8 ubTownId;

  // get town index
  ubTownId = GetTownIdForSector(mapX, mapY);

  if (ubTownId == BLANK_SECTOR) {
    Assert(IsThisSectorASAMSector(mapX, mapY, 0));
  }

  // force tactical to update militia status
  TacticalMilitiaRefreshRequired();

  // ok, so what do we do with all this training?  Well, in order of decreasing priority:
  // 1) If there's room in training sector, create new GREEN militia guys there
  // 2) If not enough room there, create new GREEN militia guys in friendly sectors of the same town
  // 3) If not enough room anywhere in town, promote a number of GREENs in this sector into regulars
  // 4) If not enough GREENS there to promote, promote GREENs in other sectors.
  // 5) If all friendly sectors of this town are completely filled with REGULAR militia, then
  // training effect is wasted

  while (ubMilitiaTrained < MILITIA_TRAINING_SQUAD_SIZE) {
    // is there room for another militia in the training sector itself?
    if (CountAllMilitiaInSector(mapX, mapY) < MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
      // great! Create a new GREEN militia guy in the training sector
      IncMilitiaOfRankInSector(mapX, mapY, GREEN_MILITIA, 1);
      MarkForRedrawalStrategicMap();
    } else {
      fFoundOne = FALSE;

      if (ubTownId != BLANK_SECTOR) {
        initNextSectorSearch(ubTownId, mapX, mapY);

        // check other eligible sectors in this town for room for another militia
        while (getNextSectorInTown(&sNeighbourX, &sNeighbourY)) {
          // is there room for another militia in this neighbouring sector ?
          if (CountAllMilitiaInSector(sNeighbourX, sNeighbourY) <
              MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
            // great! Create a new GREEN militia guy in the neighbouring sector
            IncMilitiaOfRankInSector(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);
            MarkForRedrawalStrategicMap();
            fFoundOne = TRUE;
            break;
          }
        }
      }

      // if we still haven't been able to train anyone
      if (!fFoundOne) {
        // alrighty, then.  We'll have to *promote* guys instead.

        // are there any GREEN militia men in the training sector itself?
        if (GetMilitiaInSector(mapX, mapY).green > 0) {
          // great! Promote a GREEN militia guy in the training sector to a REGULAR
          promoteMilitia(mapX, mapY, GREEN_MILITIA, 1);
        } else {
          if (ubTownId != BLANK_SECTOR) {
            // dammit! Last chance - try to find other eligible sectors in the same town with a
            // Green guy to be promoted
            initNextSectorSearch(ubTownId, mapX, mapY);

            // check other eligible sectors in this town for room for another militia
            while (getNextSectorInTown(&sNeighbourX, &sNeighbourY)) {
              // are there any GREEN militia men in the neighbouring sector ?
              if (GetMilitiaInSector(sNeighbourX, sNeighbourY).green > 0) {
                // great! Promote a GREEN militia guy in the neighbouring sector to a REGULAR
                promoteMilitia(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);

                fFoundOne = TRUE;
                break;
              }
            }
          }

          // if we still haven't been able to train anyone
          if (!fFoundOne) {
            // Well, that's it.  All eligible sectors of this town are full of REGULARs or ELITEs.
            // The training goes to waste in this situation.
            break;  // the main while loop
          }
        }
      }
    }

    // next, please!
    ubMilitiaTrained++;
  }

  // if anyone actually got trained
  if (ubMilitiaTrained > 0) {
    // update the screen display
    MarkForRedrawalStrategicMap();

    if (ubTownId != BLANK_SECTOR) {
      // loyalty in this town increases a bit because we obviously care about them...
      IncrementTownLoyalty(ubTownId, LOYALTY_BONUS_FOR_TOWN_TRAINING);
    }
  }

  // the trainer announces to player that he's finished his assignment.  Make his sector flash!
  AssignmentDone(pTrainer, TRUE, FALSE);

  // handle completion of town by training group
  handleTrainingComplete(pTrainer);
}

static void promoteMilitia(u8 mapX, u8 mapY, UINT8 ubCurrentRank, UINT8 ubHowMany) {
  u8 currentCount = GetMilitiaOfRankInSector(mapX, mapY, ubCurrentRank);

  // damn well better have that many around to promote!
  Assert(currentCount >= ubHowMany);

  // KM : July 21, 1999 patch fix
  if (currentCount < ubHowMany) {
    return;
  }

  SetMilitiaOfRankInSector(mapX, mapY, ubCurrentRank, currentCount - ubHowMany);
  IncMilitiaOfRankInSector(mapX, mapY, ubCurrentRank + 1, ubHowMany);

  // update the screen display
  MarkForRedrawalStrategicMap();
}

void StrategicRemoveMilitiaFromSector(u8 mapX, u8 mapY, UINT8 ubRank, UINT8 ubHowMany) {
  u8 currentCount = GetMilitiaOfRankInSector(mapX, mapY, ubRank);

  // damn well better have that many around to remove!
  Assert(currentCount >= ubHowMany);

  // KM : July 21, 1999 patch fix
  if (currentCount < ubHowMany) {
    return;
  }

  SetMilitiaOfRankInSector(mapX, mapY, ubRank, currentCount - ubHowMany);

  // update the screen display
  MarkForRedrawalStrategicMap();
}

// kill pts are (2 * kills) + assists
UINT8 CheckOneMilitiaForPromotion(u8 mapX, u8 mapY, UINT8 ubCurrentRank, UINT8 ubRecentKillPts) {
  uint32_t uiChanceToLevel = 0;

  switch (ubCurrentRank) {
    case GREEN_MILITIA:
      // 2 kill pts minimum
      if (ubRecentKillPts >= 2) {
        // 25% chance per kill pt
        uiChanceToLevel = 25 * ubRecentKillPts;
      }
      break;
    case REGULAR_MILITIA:
      // 5 kill pts minimum
      if (ubRecentKillPts >= 5) {
        // 10% chance per kill pt.
        uiChanceToLevel = 10 * ubRecentKillPts;
      }
      break;
    case ELITE_MILITIA:
      return 0;
      break;
  }
  // roll the bones, and see if he makes it
  if (Random(100) < uiChanceToLevel) {
    promoteMilitia(mapX, mapY, ubCurrentRank, 1);
    if (ubCurrentRank == GREEN_MILITIA) {  // Attempt yet another level up if sufficient points
      if (ubRecentKillPts > 2) {
        if (CheckOneMilitiaForPromotion(
                mapX, mapY, REGULAR_MILITIA,
                (UINT8)(ubRecentKillPts - 2))) {  // success, this militia was promoted twice
          return 2;
        }
      }
    }
    return 1;
  }
  return 0;
}

// call this if the player attacks his own militia
void HandleMilitiaDefections(u8 mapX, u8 mapY) {
  UINT8 ubRank;
  UINT8 ubMilitiaCnt;
  UINT8 ubCount;
  uint32_t uiChanceToDefect;

  for (ubRank = 0; ubRank < MAX_MILITIA_LEVELS; ubRank++) {
    ubMilitiaCnt = GetMilitiaOfRankInSector(mapX, mapY, ubRank);

    // check each guy at each rank to see if he defects
    for (ubCount = 0; ubCount < ubMilitiaCnt; ubCount++) {
      switch (ubRank) {
        case GREEN_MILITIA:
          uiChanceToDefect = 50;
          break;
        case REGULAR_MILITIA:
          uiChanceToDefect = 75;
          break;
        case ELITE_MILITIA:
          uiChanceToDefect = 90;
          break;
        default:
          Assert(0);
          return;
      }

      // roll the bones; should I stay or should I go now?  (for you music fans out there)
      if (Random(100) < uiChanceToDefect) {
        // B'bye!  (for you SNL fans out there)
        StrategicRemoveMilitiaFromSector(mapX, mapY, ubRank, 1);
      }
    }
  }
}

UINT8 CountAllMilitiaInSector(u8 mapX, u8 mapY) {
  struct MilitiaCount milCount = GetMilitiaInSector(mapX, mapY);
  return milCount.green + milCount.regular + milCount.elite;
}

UINT8 CountAllMilitiaInSectorID8(SectorID8 sectorID) {
  struct MilitiaCount milCount = GetMilitiaInSectorID8(sectorID);
  return milCount.green + milCount.regular + milCount.elite;
}

INT32 GetNumberOfMilitiaInSector(u8 sSectorX, u8 sSectorY, INT8 bSectorZ) {
  if (!bSectorZ) {
    return CountAllMilitiaInSector(sSectorX, sSectorY);
  }
  return 0;
}

struct MilitiaCount GetMilitiaInSector(u8 mapX, u8 mapY) {
  return GetMilitiaInSectorID8(GetSectorID8(mapX, mapY));
}

struct MilitiaCount GetMilitiaInSectorID8(SectorID8 sectorID) {
  struct MilitiaCount res = {
      _st.sectorForce[sectorID].green,
      _st.sectorForce[sectorID].regular,
      _st.sectorForce[sectorID].elite,
  };
  return res;
}

void SetMilitiaInSectorID8(SectorID8 sectorID, struct MilitiaCount newCount) {
  _st.sectorForce[sectorID] = newCount;
}

void SetMilitiaInSector(u8 mapX, u8 mapY, struct MilitiaCount newCount) {
  SetMilitiaInSectorID8(GetSectorID8(mapX, mapY), newCount);
}

void SetMilitiaOfRankInSector(u8 mapX, u8 mapY, UINT8 ubRank, u8 count) {
  SectorID8 sectorID = GetSectorID8(mapX, mapY);
  switch (ubRank) {
    case GREEN_MILITIA:
      _st.sectorForce[sectorID].green = count;
      break;
    case REGULAR_MILITIA:
      _st.sectorForce[sectorID].regular = count;
      break;
    case ELITE_MILITIA:
      _st.sectorForce[sectorID].elite = count;
      break;
  }
}

void IncMilitiaOfRankInSector(u8 mapX, u8 mapY, u8 ubRank, u8 increase) {
  SectorID8 sectorID = GetSectorID8(mapX, mapY);
  switch (ubRank) {
    case GREEN_MILITIA:
      _st.sectorForce[sectorID].green += increase;
      break;
    case REGULAR_MILITIA:
      _st.sectorForce[sectorID].regular += increase;
      break;
    case ELITE_MILITIA:
      _st.sectorForce[sectorID].elite += increase;
      break;
  }
}

UINT8 GetMilitiaOfRankInSector(u8 mapX, u8 mapY, UINT8 ubRank) {
  struct MilitiaCount count = GetMilitiaInSector(mapX, mapY);
  switch (ubRank) {
    case GREEN_MILITIA:
      return count.green;
      break;
    case REGULAR_MILITIA:
      return count.regular;
      break;
    case ELITE_MILITIA:
      return count.elite;
      break;
    default:
      return 0;
  }
}

bool IsMilitiaTrainingPayedForSector(u8 mapX, u8 mapY) {
  return IsMilitiaTrainingPayedForSectorID8(GetSectorID8(mapX, mapY));
}

bool IsMilitiaTrainingPayedForSectorID8(SectorID8 sectorID) { return _st.trainingPaid[sectorID]; }

void SetMilitiaTrainingPayedForSectorID8(SectorID8 sectorID, bool value) {
  _st.trainingPaid[sectorID] = value;
}

static void initNextSectorSearch(UINT8 ubTownId, INT16 sSkipSectorX, INT16 sSkipSectorY) {
  _st.sectorSearch.townID = ubTownId;
  _st.sectorSearch.skipX = sSkipSectorX;
  _st.sectorSearch.skipY = sSkipSectorY;
  _st.sectorSearch.townSectorsIndex = 0;
}

// this feeds the X,Y of the next town sector on the town list for the town specified at
// initialization it will skip an entry that matches the skip X/Y value if one was specified at
// initialization MUST CALL initNextSectorSearch() before using!!!
static BOOLEAN getNextSectorInTown(u8 *sNeighbourX, u8 *sNeighbourY) {
  u8 mapX, mapY;
  BOOLEAN fStopLooking = FALSE;

  const TownSectors *townSectors = GetAllTownSectors();

  do {
    // have we reached the end of the town list?
    if ((*townSectors)[_st.sectorSearch.townSectorsIndex].townID == BLANK_SECTOR) {
      // end of list reached
      return (FALSE);
    }

    INT32 iTownSector = (*townSectors)[_st.sectorSearch.townSectorsIndex].sectorID;

    // if this sector is in the town we're looking for
    if (GetTownIdForStrategicMapIndex(iTownSector) == _st.sectorSearch.townID) {
      // A sector in the specified town.  Calculate its X & Y sector compotents
      mapX = SectorID16_X(iTownSector);
      mapY = SectorID16_Y(iTownSector);

      // Make sure we're not supposed to skip it
      if ((mapX != _st.sectorSearch.skipX) || (mapY != _st.sectorSearch.skipY)) {
        // check if it's "friendly" - not enemy controlled, no enemies in it, no combat in progress
        if (SectorOursAndPeaceful(mapX, mapY, 0)) {
          // then that's it!
          *sNeighbourX = mapX;
          *sNeighbourY = mapY;

          fStopLooking = TRUE;
        }
      }
    }

    // advance to next entry in town list
    _st.sectorSearch.townSectorsIndex++;

  } while (!fStopLooking);

  // found & returning a valid sector
  return (TRUE);
}

void HandleInterfaceMessageForCostOfTrainingMilitia(struct SOLDIERTYPE *pSoldier) {
  CHAR16 sString[128];
  INT32 iNumberOfSectors = 0;

  _st.trainer = pSoldier;

  // grab total number of sectors
  iNumberOfSectors = GetNumberOfUnpaidTrainableSectors();
  Assert(iNumberOfSectors > 0);

  // get total cost
  _st.totalCostOfTraining = MILITIA_TRAINING_COST * iNumberOfSectors;
  Assert(_st.totalCostOfTraining > 0);

  _st.promptForContinue = FALSE;

  if (MoneyGetBalance() < _st.totalCostOfTraining) {
    swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[8], _st.totalCostOfTraining);
    DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback);
    return;
  }

  // ok to start training, ask player

  if (iNumberOfSectors > 1) {
    swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[7], iNumberOfSectors,
             _st.totalCostOfTraining, pMilitiaConfirmStrings[1]);
  } else {
    swprintf(sString, ARR_SIZE(sString), L"%s%d. %s", pMilitiaConfirmStrings[0],
             _st.totalCostOfTraining, pMilitiaConfirmStrings[1]);
  }

  // if we are in mapscreen, make a pop up
  if (IsMapScreen_2()) {
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_YESNO,
                    PayMilitiaTrainingYesNoBoxCallback);
  } else {
    DoMessageBox(MSG_BOX_BASIC_STYLE, sString, GAME_SCREEN, MSG_BOX_FLAG_YESNO,
                 PayMilitiaTrainingYesNoBoxCallback, GetMapCenteringRect());
  }

  return;
}

void DoContinueMilitiaTrainingMessageBox(u8 mapX, u8 mapY, CHAR16 *str, UINT16 usFlags,
                                         MSGBOX_CALLBACK ReturnCallback) {
  if (mapX <= 10 && mapY >= 6 && mapY <= 11) {
    DoLowerScreenIndependantMessageBox(str, usFlags, ReturnCallback);
  } else {
    DoScreenIndependantMessageBox(str, usFlags, ReturnCallback);
  }
}

void HandleInterfaceMessageForContinuingTrainingMilitia(struct SOLDIERTYPE *pSoldier) {
  CHAR16 sString[128];
  u8 mapX = 0, mapY = 0;
  CHAR16 sStringB[128];
  TownID bTownId;

  mapX = GetSolSectorX(pSoldier);
  mapY = GetSolSectorY(pSoldier);

  Assert(!IsMilitiaTrainingPayedForSector(mapX, mapY));

  _st.trainer = pSoldier;

  _st.promptForContinue = TRUE;

  // is there enough loyalty to continue training
  if (DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier) == FALSE) {
    // loyalty too low to continue training
    swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[9],
             pTownNames[GetTownIdForSector(mapX, mapY)], MIN_RATING_TO_TRAIN_TOWN);
    DoContinueMilitiaTrainingMessageBox(mapX, mapY, sString, MSG_BOX_FLAG_OK,
                                        CantTrainMilitiaOkBoxCallback);
    return;
  }

  if (IsMilitiaTrainableFromSoldiersSectorMaxed(pSoldier)) {
    // we're full!!! go home!
    bTownId = GetTownIdForSector(mapX, mapY);
    if (bTownId == BLANK_SECTOR) {
      // wilderness SAM site
      GetSectorIDString(mapX, mapY, 0, sStringB, ARR_SIZE(sStringB), TRUE);
      swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[10], sStringB, GetSectorIDString,
               MIN_RATING_TO_TRAIN_TOWN);
    } else {
      // town
      swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[10], pTownNames[bTownId],
               MIN_RATING_TO_TRAIN_TOWN);
    }
    DoContinueMilitiaTrainingMessageBox(mapX, mapY, sString, MSG_BOX_FLAG_OK,
                                        CantTrainMilitiaOkBoxCallback);
    return;
  }

  // continue training always handles just one sector at a time
  _st.totalCostOfTraining = MILITIA_TRAINING_COST;

  // can player afford to continue training?
  if (MoneyGetBalance() < _st.totalCostOfTraining) {
    // can't afford to continue training
    swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[8], _st.totalCostOfTraining);
    DoContinueMilitiaTrainingMessageBox(mapX, mapY, sString, MSG_BOX_FLAG_OK,
                                        CantTrainMilitiaOkBoxCallback);
    return;
  }

  // ok to continue, ask player

  GetSectorIDString(mapX, mapY, 0, sStringB, ARR_SIZE(sStringB), TRUE);
  swprintf(sString, ARR_SIZE(sString), pMilitiaConfirmStrings[3], sStringB,
           pMilitiaConfirmStrings[4], _st.totalCostOfTraining);

  // ask player whether he'd like to continue training
  // DoContinueMilitiaTrainingMessageBox( mapX, mapY, sString, MSG_BOX_FLAG_YESNO,
  // PayMilitiaTrainingYesNoBoxCallback );
  DoMapMessageBox(MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_YESNO,
                  PayMilitiaTrainingYesNoBoxCallback);
}

// IMPORTANT: This same callback is used both for initial training and for continue training prompt
// use '_st.promptForContinue' flag to tell them apart
static void PayMilitiaTrainingYesNoBoxCallback(UINT8 bExitValue) {
  CHAR16 sString[128];

  Assert(_st.totalCostOfTraining > 0);

  // yes
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // does the player have enough
    if (MoneyGetBalance() >= _st.totalCostOfTraining) {
      if (_st.promptForContinue) {
        ContinueTrainingInThisSector();
      } else {
        StartTrainingInAllUnpaidTrainableSectors();
      }

#ifdef JA2BETAVERSION
      // put this BEFORE training gets handled to avoid detecting an error everytime a sector
      // completes training
      VerifyTownTrainingIsPaidFor();
#endif

      // this completes the training prompt sequence
      _st.trainer = NULL;
    } else  // can't afford it
    {
      StopTimeCompression();

      swprintf(sString, ARR_SIZE(sString), L"%s", pMilitiaConfirmStrings[2]);
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_OK,
                      CantTrainMilitiaOkBoxCallback);
    }
  } else if (bExitValue == MSG_BOX_RETURN_NO) {
    StopTimeCompression();

    MilitiaTrainingRejected();
  }

  return;
}

static void CantTrainMilitiaOkBoxCallback(UINT8 bExitValue) { MilitiaTrainingRejected(); }

// reset assignment for mercs trainign militia in this sector
static void resetTrainersAssignment(u8 mapX, u8 mapY) {
  INT32 iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // valid character?
    if (!IsCharListEntryValid(iCounter)) {
      // nope
      continue;
    }

    pSoldier = GetMercFromCharacterList(iCounter);

    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    if (GetSolAssignment(pSoldier) == TRAIN_TOWN) {
      if ((GetSolSectorX(pSoldier) == mapX) && (GetSolSectorY(pSoldier) == mapY) &&
          (GetSolSectorZ(pSoldier) == 0)) {
        ResumeOldAssignment(pSoldier);
      }
    }
  }
}

// reset assignments for mercs on selected list who have this assignment
static void resetAssignmentsForUnpaidSectors() {
  INT32 iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // valid character?
    if (!IsCharListEntryValid(iCounter)) {
      // nope
      continue;
    }

    pSoldier = GetMercFromCharacterList(iCounter);

    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    if (GetSolAssignment(pSoldier) == TRAIN_TOWN) {
      if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(pSoldier))) {
        ResumeOldAssignment(pSoldier);
      }
    }
  }
}

// IMPORTANT: This same callback is used both for initial training and for continue training prompt
// use '_st.promptForContinue' flag to tell them apart
static void MilitiaTrainingRejected(void) {
  if (_st.promptForContinue) {
    // take all mercs in that sector off militia training
    resetTrainersAssignment(GetSolSectorX(_st.trainer), GetSolSectorY(_st.trainer));
  } else {
    // take all mercs in unpaid sectors EVERYWHERE off militia training
    resetAssignmentsForUnpaidSectors();
  }

#ifdef JA2BETAVERSION
  // put this BEFORE training gets handled to avoid detecting an error everytime a sector completes
  // training
  VerifyTownTrainingIsPaidFor();
#endif

  // this completes the training prompt sequence
  _st.trainer = NULL;
}

BOOLEAN CanNearbyMilitiaScoutThisSector(u8 mapX, u8 mapY) {
  i8 sCounterA = 0, sCounterB = 0;
  UINT8 ubScoutingRange = 1;

  // get the sector value
  for (sCounterA = mapX - ubScoutingRange; sCounterA <= mapX + ubScoutingRange; sCounterA++) {
    for (sCounterB = mapY - ubScoutingRange; sCounterB <= mapY + ubScoutingRange; sCounterB++) {
      // skip out of bounds sectors
      if ((sCounterA < 1) || (sCounterA > 16) || (sCounterB < 1) || (sCounterB > 16)) {
        continue;
      }

      if (CountAllMilitiaInSector(sCounterA, sCounterB) > 0) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN IsTownFullMilitia(TownID bTownId) {
  INT32 iCounter = 0;
  u8 mapX = 0, mapY = 0;
  INT32 iNumberOfMilitia = 0;
  INT32 iMaxNumber = 0;

  const TownSectors *townSectors = GetAllTownSectors();

  while ((*townSectors)[iCounter].townID != 0) {
    if ((*townSectors)[iCounter].townID == bTownId) {
      // get the sector x and y
      mapX = SectorID16_X((*townSectors)[iCounter].sectorID);
      mapY = SectorID16_Y((*townSectors)[iCounter].sectorID);

      // if sector is ours get number of militia here
      if (SectorOursAndPeaceful(mapX, mapY, 0)) {
        // don't count GREEN militia, they can be trained into regulars first
        struct MilitiaCount milCount = GetMilitiaInSector(mapX, mapY);
        iNumberOfMilitia += milCount.regular;
        iNumberOfMilitia += milCount.elite;
        iMaxNumber += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
      }
    }

    iCounter++;
  }

  // now check the number of militia
  if (iMaxNumber > iNumberOfMilitia) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN IsSAMSiteFullOfMilitia(u8 mapX, u8 mapY) {
  BOOLEAN fSamSitePresent = FALSE;
  INT32 iNumberOfMilitia = 0;
  INT32 iMaxNumber = 0;

  // check if SAM site is ours?
  fSamSitePresent = IsThisSectorASAMSector(mapX, mapY, 0);

  if (fSamSitePresent == FALSE) {
    return (FALSE);
  }

  if (SectorOursAndPeaceful(mapX, mapY, 0)) {
    struct MilitiaCount milCount = GetMilitiaInSector(mapX, mapY);
    // don't count GREEN militia, they can be trained into regulars first
    iNumberOfMilitia += milCount.regular;
    iNumberOfMilitia += milCount.elite;
    iMaxNumber += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
  }

  // now check the number of militia
  if (iMaxNumber > iNumberOfMilitia) {
    return (FALSE);
  }

  return (TRUE);
}

static void handleTrainingComplete(struct SOLDIERTYPE *pTrainer) {
  u8 mapX = 0, mapY = 0;
  INT8 bSectorZ = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  INT32 iCounter = 0;

  // get the sector values
  mapX = GetSolSectorX(pTrainer);
  mapY = GetSolSectorY(pTrainer);
  bSectorZ = GetSolSectorZ(pTrainer);

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // valid character?
    if (!IsCharListEntryValid(iCounter)) {
      // nope
      continue;
    }

    pSoldier = GetMercFromCharacterList(iCounter);

    // valid soldier?
    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    if ((GetSolAssignment(pSoldier) == TRAIN_TOWN) && (GetSolSectorX(pSoldier) == mapX) &&
        (GetSolSectorY(pSoldier) == mapY) && (GetSolSectorZ(pSoldier) == bSectorZ)) {
      // done assignment
      AssignmentDone(pSoldier, FALSE, FALSE);
    }
  }

  return;
}

void AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining(struct SOLDIERTYPE *pSoldier) {
  INT32 iCounter = 0;
  INT16 sSector = 0, sCurrentSector = 0;
  struct SOLDIERTYPE *pCurrentSoldier = NULL;

  // get the sector value
  sSector = GetSectorID16(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

  while (_st.soldiersCompletedMilitiaTraining[iCounter] != -1) {
    // get the current soldier
    pCurrentSoldier = GetSoldierByID(_st.soldiersCompletedMilitiaTraining[iCounter]);

    // get the current sector value
    sCurrentSector = GetSectorID16(GetSolSectorX(pCurrentSoldier), GetSolSectorY(pCurrentSoldier));

    // is the merc's sector already in the list?
    if (sCurrentSector == sSector) {
      // already here
      return;
    }

    iCounter++;

    Assert(iCounter < SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST);
  }

  // add merc to the list
  _st.soldiersCompletedMilitiaTraining[iCounter] = GetSolID(pSoldier);

  return;
}

// clear out the list of training sectors...should be done once the list is posted
void ClearSectorListForCompletedTrainingOfMilitia(void) {
  INT32 iCounter = 0;

  for (iCounter = 0; iCounter < SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST; iCounter++) {
    _st.soldiersCompletedMilitiaTraining[iCounter] = -1;
  }

  return;
}

void HandleContinueOfTownTraining(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  INT32 iCounter = 0;
  BOOLEAN fContinueEventPosted = FALSE;

  while (_st.soldiersCompletedMilitiaTraining[iCounter] != -1) {
    // get the soldier
    pSoldier = GetSoldierByID(_st.soldiersCompletedMilitiaTraining[iCounter]);

    if (IsSolActive(pSoldier)) {
      fContinueEventPosted = TRUE;
      SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_CONTINUE_TRAINING_MILITIA,
                                    GetSolProfile(pSoldier), 0, 0, 0, 0);
    }

    // next entry
    iCounter++;
  }

  // now clear the list
  ClearSectorListForCompletedTrainingOfMilitia();

  if (fContinueEventPosted) {
    // ATE: If this event happens in tactical mode we will be switching at some time to mapscreen...
    if (IsTacticalMode()) {
      gfEnteringMapScreen = TRUE;
    }

    // If the militia view isn't currently active, then turn it on when prompting to continue
    // training.
    SwitchMapToMilitiaMode();
  }
}

static void BuildListOfUnpaidTrainableSectors(void) {
  INT32 iCounter = 0, iCounterB = 0;

  memset(_st.unpaidSectors, 0, sizeof(INT16) * MAX_CHARACTER_COUNT);

  if (IsMapScreen()) {
    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      // valid character?
      if (IsCharListEntryValid(iCounter)) {
        // selected?
        if (IsCharSelected(iCounter) || iCounter == GetCharForAssignmentIndex()) {
          struct SOLDIERTYPE *sol = GetMercFromCharacterList(iCounter);
          if (CanCharacterTrainMilitia(sol) == TRUE) {
            if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(sol))) {
              // check to see if this sector is a town and needs equipment
              _st.unpaidSectors[iCounter] = GetSolSectorID16(sol);
            }
          }
        }
      }
    }
  } else {
    // handle for tactical
    struct SOLDIERTYPE *sol = GetTacticalContextMenuMerc();
    iCounter = 0;

    if (CanCharacterTrainMilitia(sol) == TRUE) {
      if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(sol))) {
        // check to see if this sector is a town and needs equipment
        _st.unpaidSectors[iCounter] = GetSolSectorID16(sol);
      }
    }
  }

  // now clean out repeated sectors
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT - 1; iCounter++) {
    if (_st.unpaidSectors[iCounter] > 0) {
      for (iCounterB = iCounter + 1; iCounterB < MAX_CHARACTER_COUNT; iCounterB++) {
        if (_st.unpaidSectors[iCounterB] == _st.unpaidSectors[iCounter]) {
          _st.unpaidSectors[iCounterB] = 0;
        }
      }
    }
  }
}

static INT32 GetNumberOfUnpaidTrainableSectors(void) {
  INT32 iCounter = 0;
  INT32 iNumberOfSectors = 0;

  BuildListOfUnpaidTrainableSectors();

  // now count up the results
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (_st.unpaidSectors[iCounter] > 0) {
      iNumberOfSectors++;
    }
  }

  // return the result
  return (iNumberOfSectors);
}

static void StartTrainingInAllUnpaidTrainableSectors() {
  INT32 iCounter = 0;
  SectorID8 ubSector;

  SetAssignmentForList(TRAIN_TOWN, 0);

  BuildListOfUnpaidTrainableSectors();

  // pay up in each sector
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (_st.unpaidSectors[iCounter] > 0) {
      // convert strategic sector to 0-255 system
      ubSector = SectorID16To8(_st.unpaidSectors[iCounter]);
      PayForTrainingInSector(ubSector);
    }
  }
}

static void ContinueTrainingInThisSector() {
  Assert(_st.trainer);
  // pay up in the sector where trainer is
  PayForTrainingInSector(GetSolSectorID8(_st.trainer));
}

static void PayForTrainingInSector(SectorID8 ubSector) {
  Assert(!IsMilitiaTrainingPayedForSectorID8(ubSector));

  // spend the money
  AddTransactionToPlayersBook(TRAIN_TOWN_MILITIA, ubSector, -(MILITIA_TRAINING_COST));

  // mark this sector sectors as being paid up
  SetMilitiaTrainingPayedForSectorID8(ubSector, true);

  // reset done flags
  ResetDoneFlagForAllMilitiaTrainersInSector(ubSector);
}

static void ResetDoneFlagForAllMilitiaTrainersInSector(SectorID8 ubSector) {
  struct SoldierList sols;
  GetTeamSoldiers_Active(OUR_TEAM, &sols);
  for (int i = 0; i < sols.num; i++) {
    struct SOLDIERTYPE *sol = sols.soldiers[i];
    if (GetSolAssignment(sol) == TRAIN_TOWN) {
      if (GetSolSectorID8(sol) == ubSector && GetSolSectorZ(sol) == 0) {
        SetSolAssignmentDone(sol);
      }
    }
  }
}

BOOLEAN MilitiaTrainingAllowedInSector(u8 mapX, u8 mapY, INT8 bSectorZ) {
  TownID bTownId;
  BOOLEAN fSamSitePresent = FALSE;

  if (bSectorZ != 0) {
    return (FALSE);
  }

  fSamSitePresent = IsThisSectorASAMSector(mapX, mapY, bSectorZ);

  if (fSamSitePresent) {
    // all SAM sites may have militia trained at them
    return (TRUE);
  }

  bTownId = GetTownIdForSector(mapX, mapY);

  return (MilitiaTrainingAllowedInTown(bTownId));
}

BOOLEAN MilitiaTrainingAllowedInTown(TownID bTownId) {
  switch (bTownId) {
    case DRASSEN:
    case ALMA:
    case GRUMM:
    case CAMBRIA:
    case BALIME:
    case MEDUNA:
    case CHITZENA:
      return (TRUE);

    case OMERTA:
    case ESTONI:
    case SAN_MONA:
    case TIXA:
    case ORTA:
      // can't keep militia in these towns
      return (FALSE);

    case BLANK_SECTOR:
    default:
      // not a town sector!
      return (FALSE);
  }
}

void PrepMilitiaPromotion() {
  _st.gbGreenToElitePromotions = 0;
  _st.gbGreenToRegPromotions = 0;
  _st.gbRegToElitePromotions = 0;
  _st.gbMilitiaPromotions = 0;
}

void HandleSingleMilitiaPromotion(u8 mapX, u8 mapY, u8 soldierClass, u8 kills) {
  u8 rank = SoldierClassToMilitiaRank(soldierClass);
  u8 ubPromotions = CheckOneMilitiaForPromotion(mapX, mapY, rank, kills);
  if (ubPromotions) {
    if (ubPromotions == 2) {
      _st.gbGreenToElitePromotions++;
      _st.gbMilitiaPromotions++;
    } else if (soldierClass == SOLDIER_CLASS_GREEN_MILITIA) {
      _st.gbGreenToRegPromotions++;
      _st.gbMilitiaPromotions++;
    } else if (soldierClass == SOLDIER_CLASS_REG_MILITIA) {
      _st.gbRegToElitePromotions++;
      _st.gbMilitiaPromotions++;
    }
  }
}

void HandleMilitiaPromotions(u8 mapX, u8 mapY) {
  PrepMilitiaPromotion();

  struct SoldierList mil;
  GetTeamSoldiers_Active(MILITIA_TEAM, &mil);

  for (int i = 0; i < mil.num; i++) {
    struct SOLDIERTYPE *sol = mil.soldiers[i];
    u8 kills = GetSolMilitiaKills(sol);
    if (IsSolInSector(sol) && IsSolAlive(sol) && kills > 0) {
      HandleSingleMilitiaPromotion(mapX, mapY, GetSolClass(sol), kills);
      SetSolMilitiaKills(sol, 0);
    }
  }
}

void BuildMilitiaPromotionsString(CHAR16 *str, size_t bufSize) {
  CHAR16 pStr[256];
  BOOLEAN fAddSpace = FALSE;
  swprintf(str, bufSize, L"");

  if (!_st.gbMilitiaPromotions) {
    return;
  }
  if (_st.gbGreenToElitePromotions > 1) {
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[22], _st.gbGreenToElitePromotions);
    wcsncat(str, pStr, bufSize);
    fAddSpace = TRUE;
  } else if (_st.gbGreenToElitePromotions == 1) {
    wcsncat(str, gzLateLocalizedString[29], bufSize);
    fAddSpace = TRUE;
  }

  if (_st.gbGreenToRegPromotions > 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[23], _st.gbGreenToRegPromotions);
    wcsncat(str, pStr, bufSize);
    fAddSpace = TRUE;
  } else if (_st.gbGreenToRegPromotions == 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    wcsncat(str, gzLateLocalizedString[30], bufSize);
    fAddSpace = TRUE;
  }

  if (_st.gbRegToElitePromotions > 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[24], _st.gbRegToElitePromotions);
    wcsncat(str, pStr, bufSize);
  } else if (_st.gbRegToElitePromotions == 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    wcsncat(str, gzLateLocalizedString[31], bufSize);
    fAddSpace = TRUE;
  }

  // Clear the fields
  _st.gbGreenToElitePromotions = 0;
  _st.gbGreenToRegPromotions = 0;
  _st.gbRegToElitePromotions = 0;
  _st.gbMilitiaPromotions = 0;
}

bool HasNewMilitiaPromotions() { return _st.gbMilitiaPromotions > 0; }

BOOLEAN DoesPlayerHaveAnyMilitia() {
  INT16 sX, sY;

  // run through list of towns that might have militia..if any return TRUE..else return FALSE
  for (sX = 1; sX < MAP_WORLD_X - 1; sX++) {
    for (sY = 1; sY < MAP_WORLD_Y - 1; sY++) {
      if (CountAllMilitiaInSector(sX, sY) > 0) {
        // found at least one
        return (TRUE);
      }
    }
  }

  // no one found
  return (FALSE);
}
