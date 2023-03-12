#include "Strategic/TownMilitia.h"

#include <string.h>

#include "CharList.h"
#include "Laptop/Finances.h"
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
  struct SectorPoint unpaidSectors[MAX_CHARACTER_COUNT];

  i8 gbGreenToElitePromotions;
  i8 gbGreenToRegPromotions;
  i8 gbRegToElitePromotions;
  i8 gbMilitiaPromotions;
};

static struct militiaState _st;

// handle completion of assignment byt his soldier too and inform the player
static void handleTrainingComplete(struct SOLDIERTYPE *pTrainer);

static void PayMilitiaTrainingYesNoBoxCallback(UINT8 bExitValue);
static void CantTrainMilitiaOkBoxCallback(UINT8 bExitValue);
static void MilitiaTrainingRejected(void);
static struct sectorSearch initNextSectorSearch(UINT8 ubTownId, INT16 sSkipSectorX,
                                                INT16 sSkipSectorY);
static BOOLEAN getNextSectorInTown(struct sectorSearch *search, u8 *sNeighbourX, u8 *sNeighbourY);
static INT32 GetNumberOfUnpaidTrainableSectors(void);
static void ContinueTrainingInThisSector();
static void StartTrainingInAllUnpaidTrainableSectors();
static void PayForTrainingInSector(u8 x, u8 y);
static void ResetDoneFlagForAllMilitiaTrainersInSector(u8 x, u8 y);

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
    if (CountMilitiaInSector(mapX, mapY) < MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
      // great! Create a new GREEN militia guy in the training sector
      IncMilitiaOfRankInSector(mapX, mapY, GREEN_MILITIA, 1);
      SetMapPanelDirty(true);
    } else {
      fFoundOne = FALSE;

      if (ubTownId != BLANK_SECTOR) {
        struct sectorSearch search = initNextSectorSearch(ubTownId, mapX, mapY);

        // check other eligible sectors in this town for room for another militia
        while (getNextSectorInTown(&search, &sNeighbourX, &sNeighbourY)) {
          // is there room for another militia in this neighbouring sector ?
          if (CountMilitiaInSector(sNeighbourX, sNeighbourY) < MAX_ALLOWABLE_MILITIA_PER_SECTOR) {
            // great! Create a new GREEN militia guy in the neighbouring sector
            IncMilitiaOfRankInSector(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);
            SetMapPanelDirty(true);
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
          PromoteMilitia(mapX, mapY, GREEN_MILITIA, 1);
        } else {
          if (ubTownId != BLANK_SECTOR) {
            // dammit! Last chance - try to find other eligible sectors in the same town with a
            // Green guy to be promoted
            struct sectorSearch search = initNextSectorSearch(ubTownId, mapX, mapY);

            // check other eligible sectors in this town for room for another militia
            while (getNextSectorInTown(&search, &sNeighbourX, &sNeighbourY)) {
              // are there any GREEN militia men in the neighbouring sector ?
              if (GetMilitiaInSector(sNeighbourX, sNeighbourY).green > 0) {
                // great! Promote a GREEN militia guy in the neighbouring sector to a REGULAR
                PromoteMilitia(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);

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
    SetMapPanelDirty(true);

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

// kill pts are (2 * kills) + assists
UINT8 CheckOneMilitiaForPromotion(u8 mapX, u8 mapY, UINT8 ubCurrentRank, UINT8 ubRecentKillPts) {
  UINT32 uiChanceToLevel = 0;

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
    PromoteMilitia(mapX, mapY, ubCurrentRank, 1);
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
  UINT32 uiChanceToDefect;

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
        RemoveMilitiaFromSector(mapX, mapY, ubRank, 1);
      }
    }
  }
}

static struct sectorSearch initNextSectorSearch(UINT8 ubTownId, INT16 sSkipSectorX,
                                                INT16 sSkipSectorY) {
  struct sectorSearch search;
  search.townID = ubTownId;
  search.skipX = sSkipSectorX;
  search.skipY = sSkipSectorY;
  search.townSectorsIndex = 0;
  return search;
}

// this feeds the X,Y of the next town sector on the town list for the town specified at
// initialization it will skip an entry that matches the skip X/Y value if one was specified at
// initialization MUST CALL initNextSectorSearch() before using!!!
static BOOLEAN getNextSectorInTown(struct sectorSearch *search, u8 *sNeighbourX, u8 *sNeighbourY) {
  BOOLEAN fStopLooking = FALSE;

  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);

  do {
    // have we reached the end of the town list?
    if (townSectors.sectors[search->townSectorsIndex].townID == BLANK_SECTOR) {
      // end of list reached
      return (FALSE);
    }

    u8 mapX = townSectors.sectors[search->townSectorsIndex].x;
    u8 mapY = townSectors.sectors[search->townSectorsIndex].y;

    // if this sector is in the town we're looking for
    if (GetTownIdForSector(mapX, mapY) == search->townID) {
      // A sector in the specified town.  Calculate its X & Y sector compotents

      // Make sure we're not supposed to skip it
      if ((mapX != search->skipX) || (mapY != search->skipY)) {
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
    search->townSectorsIndex++;

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
      if (!IsMilitiaTrainingPayedForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))) {
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

      if (CountMilitiaInSector(sCounterA, sCounterB) > 0) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN IsTownFullMilitia(TownID bTownId) {
  INT32 iNumberOfMilitia = 0;
  INT32 iMaxNumber = 0;

  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);

  for (int iCounter = 0; iCounter < townSectors.count; iCounter++) {
    if (townSectors.sectors[iCounter].townID == bTownId) {
      // get the sector x and y
      u8 mapX = townSectors.sectors[iCounter].x;
      u8 mapY = townSectors.sectors[iCounter].y;

      // if sector is ours get number of militia here
      if (SectorOursAndPeaceful(mapX, mapY, 0)) {
        // don't count GREEN militia, they can be trained into regulars first
        struct MilitiaCount milCount = GetMilitiaInSector(mapX, mapY);
        iNumberOfMilitia += milCount.regular;
        iNumberOfMilitia += milCount.elite;
        iMaxNumber += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
      }
    }
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

  memset(_st.unpaidSectors, 0, sizeof(struct SectorPoint) * MAX_CHARACTER_COUNT);

  if (IsMapScreen()) {
    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      // valid character?
      if (IsCharListEntryValid(iCounter)) {
        // selected?
        if (IsCharSelected(iCounter) || iCounter == GetCharForAssignmentIndex()) {
          struct SOLDIERTYPE *sol = GetMercFromCharacterList(iCounter);
          if (CanCharacterTrainMilitia(sol) == TRUE) {
            if (!IsMilitiaTrainingPayedForSector(GetSolSectorX(sol), GetSolSectorY(sol))) {
              // check to see if this sector is a town and needs equipment
              _st.unpaidSectors[iCounter].x = GetSolSectorX(sol);
              _st.unpaidSectors[iCounter].y = GetSolSectorY(sol);
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
      if (!IsMilitiaTrainingPayedForSector(GetSolSectorX(sol), GetSolSectorY(sol))) {
        // check to see if this sector is a town and needs equipment
        _st.unpaidSectors[iCounter].x = GetSolSectorX(sol);
        _st.unpaidSectors[iCounter].y = GetSolSectorY(sol);
      }
    }
  }

  // now clean out repeated sectors
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT - 1; iCounter++) {
    if (_st.unpaidSectors[iCounter].x > 0 && _st.unpaidSectors[iCounter].y > 0) {
      for (iCounterB = iCounter + 1; iCounterB < MAX_CHARACTER_COUNT; iCounterB++) {
        if ((_st.unpaidSectors[iCounterB].x == _st.unpaidSectors[iCounter].x) &&
            (_st.unpaidSectors[iCounterB].y == _st.unpaidSectors[iCounter].y)) {
          _st.unpaidSectors[iCounterB].x = 0;
          _st.unpaidSectors[iCounterB].y = 0;
        }
      }
    }
  }
}

static INT32 GetNumberOfUnpaidTrainableSectors(void) {
  INT32 iNumberOfSectors = 0;

  BuildListOfUnpaidTrainableSectors();

  // now count up the results
  for (int i = 0; i < MAX_CHARACTER_COUNT; i++) {
    if (_st.unpaidSectors[i].x > 0 && _st.unpaidSectors[i].y > 0) {
      iNumberOfSectors++;
    }
  }

  // return the result
  return (iNumberOfSectors);
}

static void StartTrainingInAllUnpaidTrainableSectors() {
  SetAssignmentForList(TRAIN_TOWN, 0);
  BuildListOfUnpaidTrainableSectors();
  for (int i = 0; i < MAX_CHARACTER_COUNT; i++) {
    if (_st.unpaidSectors[i].x > 0 && _st.unpaidSectors[i].y > 0) {
      PayForTrainingInSector(_st.unpaidSectors[i].x, _st.unpaidSectors[i].y);
    }
  }
}

static void ContinueTrainingInThisSector() {
  Assert(_st.trainer);
  // pay up in the sector where trainer is
  PayForTrainingInSector(GetSolSectorX(_st.trainer), GetSolSectorY(_st.trainer));
}

static void PayForTrainingInSector(u8 x, u8 y) {
  Assert(!IsMilitiaTrainingPayedForSector(x, y));

  // spend the money
  AddTransactionToPlayersBook(TRAIN_TOWN_MILITIA, GetSectorID8(x, y), -(MILITIA_TRAINING_COST));

  // mark this sector sectors as being paid up
  SetMilitiaTrainingPayedForSector(x, y, true);

  // reset done flags
  ResetDoneFlagForAllMilitiaTrainersInSector(x, y);
}

static void ResetDoneFlagForAllMilitiaTrainersInSector(u8 x, u8 y) {
  struct SoldierList sols;
  GetTeamSoldiers_Active(OUR_TEAM, &sols);
  for (int i = 0; i < sols.num; i++) {
    struct SOLDIERTYPE *sol = sols.soldiers[i];
    if (GetSolAssignment(sol) == TRAIN_TOWN) {
      if (GetSolSectorX(sol) == x && GetSolSectorY(sol) == y && GetSolSectorZ(sol) == 0) {
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
  u8 sX, sY;

  // run through list of towns that might have militia..if any return TRUE..else return FALSE
  for (sX = 1; sX < MAP_WORLD_X - 1; sX++) {
    for (sY = 1; sY < MAP_WORLD_Y - 1; sY++) {
      if (CountMilitiaInSector(sX, sY) > 0) {
        // found at least one
        return (TRUE);
      }
    }
  }

  // no one found
  return (FALSE);
}
