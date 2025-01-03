// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/StrategicTownLoyalty.h"

#include "GameSettings.h"
#include "Laptop/History.h"
#include "MessageBoxScreen.h"
#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/Random.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/AnimationData.h"
#include "Tactical/HandleItems.h"
#include "Tactical/LOS.h"
#include "Tactical/Menptr.h"
#include "Tactical/Morale.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "Tactical/WorldItems.h"
#include "Town.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "rust_civ_groups.h"
#include "rust_fileman.h"

// the max loyalty rating for any given town
#define MAX_LOYALTY_VALUE 100

// loyalty Omerta drops to and maxes out at if the player betrays the rebels
#define HOSTILE_OMERTA_LOYALTY_RATING 10

// the loyalty threshold below which theft of unsupervised items in a town sector can occur
#define THRESHOLD_FOR_TOWN_THEFT 50

#define LOYALTY_EVENT_DISTANCE_THRESHOLD 3  // in sectors

// effect of unintentional killing /100 vs intentional murder
#define REDUCTION_FOR_UNINTENTIONAL_KILLING 50
// the effect of a murder by rebel / 100 vs player murdering a civ
#define REDUCTION_FOR_MURDER_BY_REBEL 70
// reduction % for being falsely blamed for a murder we didn't do, out of 100
#define REDUCTION_FOR_MURDER_NOT_OUR_FAULT 50
// reduction % if enemies kills civs in our sector / 100
#define REDUCTION_FOR_MURDER_OF_INNOCENT_BY_ENEMY_IN_OUR_SECTOR 25
// how much worse loyalty hit is for trauma of someone killed by a monster
#define MULTIPLIER_FOR_MURDER_BY_MONSTER 2

// gain for hiring an NPC from a particular town (represents max. at 100% town attachment)
#define MULTIPLIER_LOCAL_RPC_HIRED 25  // 5%

// multiplier for causing pts of damage directly done to a building
#define MULTIPLIER_FOR_DAMAGING_A_BUILDING 10  // 50 pts = 1%
// multiplier for not preventing pts of damage to a building
#define MULTIPLIER_FOR_NOT_PREVENTING_BUILDING_DAMAGE 3  // 167 pts = 1%

// divisor for dmg to a building by allied rebel
#define DIVISOR_FOR_REBEL_BUILDING_DMG 2

int32_t iTownDistances[NUM_TOWNS][NUM_TOWNS];

#define BASIC_COST_FOR_CIV_MURDER (10 * GAIN_PTS_PER_LOYALTY_PT)

uint32_t uiPercentLoyaltyDecreaseForCivMurder[] = {
    // These get multiplied by GAIN_PTS_PER_LOYALTY_PT so they're in % of loyalty decrease (for an
    // average town)
    (5 * GAIN_PTS_PER_LOYALTY_PT),   // fat civ
    (7 * GAIN_PTS_PER_LOYALTY_PT),   // man civ
    (8 * GAIN_PTS_PER_LOYALTY_PT),   // min civ
    (10 * GAIN_PTS_PER_LOYALTY_PT),  // dress (woman)
    (20 * GAIN_PTS_PER_LOYALTY_PT),  // hat kid
    (20 * GAIN_PTS_PER_LOYALTY_PT),  // kid
    (20 * GAIN_PTS_PER_LOYALTY_PT),  // cripple
    (1 * GAIN_PTS_PER_LOYALTY_PT),   // cow
};

// location of first enocunter with enemy
struct SectorPoint locationOfFirstBattle = {.x = 0, .y = 0};

// number of items in currently loaded sector
extern uint32_t guiNumWorldItems;

// preprocess sector for mercs in it
extern BOOLEAN fSectorsWithSoldiers[MAP_WORLD_X * MAP_WORLD_X][4];

extern wchar_t *pTownNames[];

// update town loyalty based on number of friendlies in this town
void UpdateTownLoyaltyBasedOnFriendliesInTown(TownID bTownId);

// update town loyalty based on number of bad guys in this town
void UpdateTownLoyaltyBasedOnBadGuysInTown(TownID bTownId);

extern void MapScreenDefaultOkBoxCallback(uint8_t bExitValue);

void StartTownLoyaltyIfFirstTime(TownID bTownId) {
  StartTownLoyaltyFirstTime(bTownId, CheckFact(FACT_MIGUEL_READ_LETTER, 0),
                            CheckFact(FACT_REBELS_HATE_PLAYER, 0));
}

void HandleMurderOfCivilian(struct SOLDIERTYPE *pSoldier, BOOLEAN fIntentional) {
  // handle the impact on loyalty of the murder of a civilian
  TownID bTownId = 0;
  int32_t iLoyaltyChange = 0;
  int8_t bSeenState = 0;
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pCivSoldier = NULL;
  uint32_t uiChanceFalseAccusal = 0;
  int8_t bKillerTeam = 0;
  BOOLEAN fIncrement = FALSE;

  // ubAttacker CAN be NOBODY...  Don't treat is as murder if NOBODY killed us...
  if (pSoldier->ubAttackerID == NOBODY) {
    return;
  }

  // ignore murder of non-civilians!
  if ((pSoldier->bTeam != CIV_TEAM) || (pSoldier->ubBodyType == CROW)) {
    return;
  }

  // if this is a profiled civilian NPC
  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    // ignore murder of NPCs if they're not loyal to the rebel cause - they're really just enemies
    // in civilian clothing
    if (gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags3 &
        PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH) {
      return;
    }
  }

  // if civilian belongs to a civilian group
  if (pSoldier->ubCivilianGroup != NON_CIV_GROUP) {
    // and it's one that is hostile to the player's cause
    switch (pSoldier->ubCivilianGroup) {
      case KINGPIN_CIV_GROUP:
      case ALMA_MILITARY_CIV_GROUP:
      case HICKS_CIV_GROUP:
      case WARDEN_CIV_GROUP:
        return;
    }
  }

  // set killer team
  bKillerTeam = Menptr[pSoldier->ubAttackerID].bTeam;

  // if the player did the killing
  if (bKillerTeam == OUR_TEAM) {
    struct SOLDIERTYPE *pKiller = MercPtrs[pSoldier->ubAttackerID];

    // apply morale penalty for killing a civilian!
    HandleMoraleEvent(pKiller, MORALE_KILLED_CIVILIAN, (uint8_t)pKiller->sSectorX,
                      (uint8_t)pKiller->sSectorY, pKiller->bSectorZ);
  }

  // get town id
  bTownId = GetSolTown(pSoldier);

  // if civilian is NOT in a town
  if (bTownId == BLANK_SECTOR) {
    return;
  }

  // check if this town does use loyalty (to skip a lot of unnecessary computation)
  if (!DoesTownUseLoyalty(bTownId)) {
    return;
  }

  if ((pSoldier->ubBodyType >= FATCIV) && (pSoldier->ubBodyType <= COW)) {
    // adjust value for killer and type
    iLoyaltyChange = uiPercentLoyaltyDecreaseForCivMurder[pSoldier->ubBodyType - FATCIV];
  } else {
    iLoyaltyChange = BASIC_COST_FOR_CIV_MURDER;
  }

  if (!fIntentional) {
    // accidental killing, reduce value
    iLoyaltyChange *= REDUCTION_FOR_UNINTENTIONAL_KILLING;
    iLoyaltyChange /= 100;
  }

  // check if LOS between any civ, killer and killed
  // if so, then do not adjust

  for (iCounter = gTacticalStatus.Team[CIV_TEAM].bFirstID;
       iCounter <= gTacticalStatus.Team[CIV_TEAM].bLastID; iCounter++) {
    // set current civ soldier
    pCivSoldier = MercPtrs[iCounter];

    if (pCivSoldier == pSoldier) {
      continue;
    }

    // killer seen by civ?
    if (SoldierToSoldierLineOfSightTest(pCivSoldier, MercPtrs[pSoldier->ubAttackerID],
                                        STRAIGHT_RANGE, TRUE) != 0) {
      bSeenState |= 1;
    }

    // victim seen by civ?
    if (SoldierToSoldierLineOfSightTest(pCivSoldier, pSoldier, STRAIGHT_RANGE, TRUE) != 0) {
      bSeenState |= 2;
    }
  }

  // if player didn't do it
  if (bKillerTeam != OUR_TEAM) {
    // If the murder is not fully witnessed, there's a chance of player being blamed for it even if
    // it's not his fault
    switch (bSeenState) {
      case 0:
        // nobody saw anything.  When body is found, chance player is blamed depends on the town's
        // loyalty at this time
        uiChanceFalseAccusal = MAX_LOYALTY_VALUE - GetTownLoyaltyRating(bTownId);
        break;
      case 1:
        // civilians saw the killer, but not the victim. 10 % chance of blaming player whether or
        // not he did it
        uiChanceFalseAccusal = 10;
        break;
      case 2:
        // civilians only saw the victim.  50/50 chance...
        uiChanceFalseAccusal = 50;
        break;
      case 3:
        // civilians have seen it all, and in this case, they're always honest
        uiChanceFalseAccusal = 0;
        break;
      default:
        Assert(FALSE);
        return;
    }

    // check whether player is falsely accused
    if (Random(100) < uiChanceFalseAccusal) {
      // blame player whether or not he did it - set killer team as our team
      bKillerTeam = OUR_TEAM;

      // not really player's fault, reduce penalty, because some will believe it to be a lie
      iLoyaltyChange *= REDUCTION_FOR_MURDER_NOT_OUR_FAULT;
      iLoyaltyChange /= 100;

      // debug message
      ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"You're being blamed for a death you didn't cause!");
    }
  }

  // check who town thinks killed the civ
  switch (bKillerTeam) {
    case OUR_TEAM:
      // town thinks player committed the murder, bad bad bad
      fIncrement = FALSE;

      // debug message
      ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Civilian killed by friendly forces.");
      break;

    case ENEMY_TEAM:
      // check whose sector this is
      if (IsSectorEnemyControlled(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))) {
        // enemy soldiers... in enemy controlled sector.  Gain loyalty
        fIncrement = TRUE;

        // debug message
        ScreenMsg(MSG_FONT_RED, MSG_DEBUG,
                  L"Enemy soldiers murdered a civilian. Town loyalty increases");
      } else {
        // reduce, we're expected to provide some protection, but not miracles
        iLoyaltyChange *= REDUCTION_FOR_MURDER_OF_INNOCENT_BY_ENEMY_IN_OUR_SECTOR;
        iLoyaltyChange /= 100;

        // lose loyalty
        fIncrement = FALSE;

        // debug message
        ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Town holds you responsible for murder by enemy.");
      }
      break;

    case MILITIA_TEAM:
      // the rebels did it... check if are they on our side
      if (CheckFact(FACT_REBELS_HATE_PLAYER, 0) == FALSE) {
        // on our side, penalty
        iLoyaltyChange *= REDUCTION_FOR_MURDER_BY_REBEL;
        iLoyaltyChange /= 100;

        // lose loyalty
        fIncrement = FALSE;

        // debug message
        ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Town holds you responsible for murder by rebels.");
      }
      break;

    case CREATURE_TEAM:
      // killed by a monster - make sure it was one
      if ((Menptr[pSoldier->ubAttackerID].ubBodyType >= ADULTFEMALEMONSTER) &&
          (Menptr[pSoldier->ubAttackerID].ubBodyType <= QUEENMONSTER)) {
        // increase for the extreme horror of being killed by a monster
        iLoyaltyChange *= MULTIPLIER_FOR_MURDER_BY_MONSTER;

        // check whose sector this is
        if (IsSectorEnemyControlled(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))) {
          // enemy controlled sector - gain loyalty
          fIncrement = TRUE;
        } else {
          // our sector - lose loyalty
          fIncrement = FALSE;
        }
      }
      break;

    default:
      Assert(FALSE);
      return;
  }

  // if it's a decrement, negate the value
  if (!fIncrement) {
    iLoyaltyChange *= -1;
  }

  // this is a hack: to avoid having to adjust the values, divide by 1.75 to compensate for the
  // distance 0
  iLoyaltyChange *= 100;
  iLoyaltyChange /= (100 + (25 * LOYALTY_EVENT_DISTANCE_THRESHOLD));

  AffectAllTownsLoyaltyByDistanceFrom(iLoyaltyChange, GetSolSectorX(pSoldier),
                                      GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
}

// check town and raise loyalty value for hiring a merc from a town...not a lot of a gain, but some
void HandleTownLoyaltyForNPCRecruitment(struct SOLDIERTYPE *pSoldier) {
  TownID bTownId = 0;
  uint32_t uiLoyaltyValue = 0;

  // get town id civilian
  bTownId = GetSolTown(pSoldier);

  // is the merc currently in their home town?
  if (bTownId == gMercProfiles[GetSolProfile(pSoldier)].bTown) {
    // yep, value of loyalty bonus depends on his importance to this to town
    uiLoyaltyValue =
        MULTIPLIER_LOCAL_RPC_HIRED * gMercProfiles[GetSolProfile(pSoldier)].bTownAttachment;

    // increment town loyalty gain
    IncrementTownLoyalty(bTownId, uiLoyaltyValue);

    // DESIGN QUESTION: How easy is it to abuse this bonus by repeatedly hiring the guy over & over
    // (at worst daily? even more often if terminated & rehired?)  (ARM)
  }

  return;
}

void RemoveRandomItemsInSector(uint8_t sSectorX, uint8_t sSectorY, int16_t sSectorZ,
                               uint8_t ubChance) {
  // remove random items in sector
  uint32_t uiNumberOfItems = 0, iCounter = 0;
  WORLDITEM *pItemList;
  uint32_t uiNewTotal = 0;
  wchar_t wSectorName[128];

  // stealing should fail anyway 'cause there shouldn't be a temp file for unvisited sectors, but
  // let's check anyway
  Assert(GetSectorFlagStatus(sSectorX, sSectorY, (uint8_t)sSectorZ, SF_ALREADY_VISITED) == TRUE);

  // get sector name string
  GetSectorIDString(sSectorX, sSectorY, (int8_t)sSectorZ, wSectorName, ARR_SIZE(wSectorName), TRUE);

  // go through list of items in sector and randomly remove them

  // if unloaded sector
  if (gWorldSectorX != sSectorX || gWorldSectorY != sSectorY || gbWorldSectorZ != sSectorZ) {
    // if the player has never been there, there's no temp file, and 0 items will get returned,
    // preventing any stealing
    GetNumberOfWorldItemsFromTempItemFile(sSectorX, sSectorY, (uint8_t)sSectorZ, &uiNumberOfItems,
                                          FALSE);

    if (uiNumberOfItems == 0) {
      return;
    }

    pItemList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * uiNumberOfItems);

    // now load items
    LoadWorldItemsFromTempItemFile(sSectorX, sSectorY, (uint8_t)sSectorZ, pItemList);
    uiNewTotal = uiNumberOfItems;

    // set up item list ptrs
    for (iCounter = 0; iCounter < uiNumberOfItems; iCounter++) {
      // if the item exists, and is visible and reachable, see if it should be stolen
      if (pItemList[iCounter].fExists && pItemList[iCounter].bVisible == TRUE &&
          pItemList[iCounter].usFlags & WORLD_ITEM_REACHABLE) {
        if (Random(100) < ubChance) {
          // remove
          uiNewTotal--;
          pItemList[iCounter].fExists = FALSE;

          // debug message
          ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%s stolen in %s!",
                    ItemNames[pItemList[iCounter].o.usItem], wSectorName);
        }
      }
    }

    // only save if something was stolen
    if (uiNewTotal < uiNumberOfItems) {
      AddWorldItemsToUnLoadedSector(sSectorX, sSectorY, (uint8_t)sSectorZ, 0, uiNumberOfItems,
                                    pItemList, TRUE);
    }

    // mem free
    MemFree(pItemList);
  } else  // handle a loaded sector
  {
    for (iCounter = 0; iCounter < guiNumWorldItems; iCounter++) {
      // note, can't do reachable test here because we'd have to do a path call...
      if (gWorldItems[iCounter].fExists && gWorldItems[iCounter].bVisible == TRUE) {
        if (Random(100) < ubChance) {
          RemoveItemFromPool(gWorldItems[iCounter].sGridNo, iCounter,
                             gWorldItems[iCounter].ubLevel);
          // debug message
          ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%s stolen in %s!",
                    ItemNames[gWorldItems[iCounter].o.usItem], wSectorName);
        }
      }
    }
  }
}

#ifdef JA2TESTVERSION
void CalcDistancesBetweenTowns(void) {
  // run though each town sector and compare it to the next in terms of distance
  uint8_t ubTownA, ubTownB;
  uint8_t ubTempGroupId = 0;
  int32_t iDistance = 0;

  // this is a little bit tricky, since towns can have multiple sectors, we have to measure all
  // possible combinations

  // preset all distances between towns to high values to prepare for search for the minimum
  // distance
  for (ubTownA = FIRST_TOWN; ubTownA < NUM_TOWNS; ubTownA++) {
    for (ubTownB = FIRST_TOWN; ubTownB < NUM_TOWNS; ubTownB++) {
      iTownDistances[ubTownA][ubTownB] = 999999;
    }
  }

  // create a temporary group (needed to plot strategic paths)
  ubTempGroupId = CreateNewPlayerGroupDepartingFromSector(1, 1);

  // now, measure distance from every town sector to every other town sector!
  // The minimum distances between towns get stored.
  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);
  for (int uiCounterA = 0; uiCounterA < townSectors.count; uiCounterA++) {
    for (int uiCounterB = uiCounterA; uiCounterB < townSectors.count; uiCounterB++) {
      ubTownA = (uint8_t)townSectors.sectors[uiCounterA].townID;
      ubTownB = (uint8_t)townSectors.sectors[uiCounterB].townID;

      // if they're not in the same town
      if (ubTownA != ubTownB) {
        // calculate fastest distance between them (in sectors) - not necessarily shortest distance,
        // roads are faster!
        iDistance =
            FindStratPath(townSectors.sectors[uiCounterA].x, townSectors.sectors[uiCounterA].y,
                          townSectors.sectors[uiCounterB].x, townSectors.sectors[uiCounterB].y,
                          ubTempGroupId, FALSE);
      } else {
        // same town, distance is 0 by definition
        iDistance = 0;
      }

      // if it's the fastest route so far, store its length
      if (iDistance < iTownDistances[ubTownA][ubTownB]) {
        // store it going both ways!  The inner while loop is optimized to search each pair only
        // once.
        iTownDistances[ubTownA][ubTownB] = iDistance;
        iTownDistances[ubTownB][ubTownA] = iDistance;
      }
    }
  }

  RemoveGroup(ubTempGroupId);
}

void WriteOutDistancesBetweenTowns(void) {
  FileID hFileHandle = FILE_ID_ERR;

  hFileHandle = File_OpenForWriting("BinaryData\\TownDistances.dat");

  File_Write(hFileHandle, &(iTownDistances), (sizeof(int32_t) * NUM_TOWNS * NUM_TOWNS), NULL);

  // close file
  File_Close(hFileHandle);

  return;
}

void DumpDistancesBetweenTowns(void) {
  char zPrintFileName[60];
  FILE *FDump;
  uint8_t ubTownA, ubTownB;
  wchar_t wHeading[4];

  // open output file
  strcpy(zPrintFileName, "TownDistances.txt");
  FDump = fopen(zPrintFileName, "wt");

  if (FDump == NULL) {
    return;
  }

  // print headings
  fprintf(FDump, "            ");
  for (ubTownB = FIRST_TOWN; ubTownB < NUM_TOWNS; ubTownB++) {
    wcsncpy(wHeading, pTownNames[ubTownB], 3);
    wHeading[3] = '\0';

    fprintf(FDump, " %ls", wHeading);
  }
  fprintf(FDump, "\n");

  fprintf(FDump, "            ");
  for (ubTownB = FIRST_TOWN; ubTownB < NUM_TOWNS; ubTownB++) {
    fprintf(FDump, " ---");
  }
  fprintf(FDump, "\n");

  for (ubTownA = FIRST_TOWN; ubTownA < NUM_TOWNS; ubTownA++) {
    fprintf(FDump, "%12ls", pTownNames[ubTownA]);

    for (ubTownB = FIRST_TOWN; ubTownB < NUM_TOWNS; ubTownB++) {
      fprintf(FDump, " %3d", iTownDistances[ubTownA][ubTownB]);
    }

    fprintf(FDump, "\n");
  }

  fclose(FDump);
}
#endif  // JA2TESTVERSION

void ReadInDistancesBetweenTowns(void) {
  FileID hFileHandle = FILE_ID_ERR;

  hFileHandle = File_OpenForReading("BinaryData\\TownDistances.dat");

  File_Read(hFileHandle, &(iTownDistances), (sizeof(int32_t) * NUM_TOWNS * NUM_TOWNS), NULL);

  // close file
  File_Close(hFileHandle);

  return;
}

int32_t GetTownDistances(uint8_t ubTown, uint8_t ubTownA) {
  return (iTownDistances[ubTown][ubTownA]);
}

BOOLEAN SaveStrategicTownLoyaltyToSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten;

  struct SAVE_LOAD_TOWN_LOYALTY townLoyalty[NUM_TOWNS];
  for (int i = 1; i < NUM_TOWNS; i++) {
    townLoyalty[i] = GetRawTownLoyalty(i);
  }

  File_Write(hFile, townLoyalty, sizeof(townLoyalty), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(townLoyalty)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadStrategicTownLoyaltyFromSavedGameFile(FileID hFile) {
  uint32_t uiNumBytesRead;

  struct SAVE_LOAD_TOWN_LOYALTY townLoyalty[NUM_TOWNS];

  // Restore the Town Loyalty
  File_Read(hFile, townLoyalty, sizeof(townLoyalty), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(townLoyalty)) {
    return (FALSE);
  }

  for (int i = 1; i < NUM_TOWNS; i++) {
    SetRawTownLoyalty(i, &townLoyalty[i]);
  }

  return (TRUE);
}

void ReduceLoyaltyForRebelsBetrayed(void) {
  TownID bTownId;

  // reduce loyalty to player all across Arulco
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (bTownId == OMERTA) {
      // if not already really low
      if (GetTownLoyaltyRating(bTownId) > HOSTILE_OMERTA_LOYALTY_RATING) {
        // loyalty in Omerta tanks big time, and will stay low permanently since this becomes its
        // maximum
        SetTownLoyalty(bTownId, HOSTILE_OMERTA_LOYALTY_RATING);
        // note that rebel sentiment isn't affected - they remain loyal to themselves, after all!
      }
    } else {
      // loyalty in other places is also strongly affected by this falling out with rebels, but this
      // is not permanent
      SetTownLoyalty(bTownId, (uint8_t)(GetTownLoyaltyRating(bTownId) / 3));
    }
  }
}

int32_t GetNumberOfWholeTownsUnderControl(void) {
  int32_t iNumber = 0;
  TownID bTownId = 0;

  // run through the list of towns..if the entire town is under player control, then increment the
  // number of towns under player control

  // make sure that each town is one for which loyalty matters
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (IsTownUnderCompleteControlByPlayer(bTownId) && DoesTownUseLoyalty(bTownId)) {
      iNumber++;
    }
  }

  return (iNumber);
}

int32_t GetNumberOfWholeTownsUnderControlButExcludeCity(int8_t bCityToExclude) {
  int32_t iNumber = 0;
  TownID bTownId = 0;

  // run through the list of towns..if the entire town is under player control, then increment the
  // number of towns under player control
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (IsTownUnderCompleteControlByPlayer(bTownId) && (bCityToExclude != bTownId) &&
        DoesTownUseLoyalty(bTownId)) {
      iNumber++;
    }
  }

  return (iNumber);
}

// is the ENTIRE town under player control?
int32_t IsTownUnderCompleteControlByPlayer(TownID bTownId) {
  if (GetTownSectorSize(bTownId) == GetTownSectorsUnderControl(bTownId)) {
    return (TRUE);
  }

  return (FALSE);
}

// is the ENTIRE town under enemy control?
int32_t IsTownUnderCompleteControlByEnemy(TownID bTownId) {
  if (GetTownSectorsUnderControl(bTownId) == 0) {
    return (TRUE);
  }

  return (FALSE);
}

void AdjustLoyaltyForCivsEatenByMonsters(uint8_t sSectorX, uint8_t sSectorY, uint8_t ubHowMany) {
  TownID bTownId = 0;
  uint32_t uiLoyaltyChange = 0;
  wchar_t str[256];
  wchar_t pSectorString[128];

  // get town id
  bTownId = GetTownIdForSector(sSectorX, sSectorY);

  // if NOT in a town
  if (bTownId == BLANK_SECTOR) {
    return;
  }

  // Report this to player
  GetSectorIDString(sSectorX, sSectorY, 0, pSectorString, ARR_SIZE(pSectorString), TRUE);
  swprintf(str, ARR_SIZE(str), gpStrategicString[STR_DIALOG_CREATURES_KILL_CIVILIANS], ubHowMany,
           pSectorString);
  DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);

  // use same formula as if it were a civilian "murder" in tactical!!!
  uiLoyaltyChange = ubHowMany * BASIC_COST_FOR_CIV_MURDER * MULTIPLIER_FOR_MURDER_BY_MONSTER;
  DecrementTownLoyalty(bTownId, uiLoyaltyChange);
}

// TODO: rustlib
// this applies the SAME change to every town equally, regardless of distance from the event
void IncrementTownLoyaltyEverywhere(uint32_t uiLoyaltyIncrease) {
  TownID bTownId;

  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    IncrementTownLoyalty(bTownId, uiLoyaltyIncrease);
  }
}

// TODO: rustlib
void DecrementTownLoyaltyEverywhere(uint32_t uiLoyaltyDecrease) {
  TownID bTownId;

  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    DecrementTownLoyalty(bTownId, uiLoyaltyDecrease);
  }
}
// this applies the change to every town differently, depending on the distance from the event
void HandleGlobalLoyaltyEvent(uint8_t ubEventType, uint8_t sSectorX, uint8_t sSectorY,
                              int8_t bSectorZ) {
  int32_t iLoyaltyChange;
  TownID bTownId = 0;

  if (bSectorZ == 0) {
    // grab town id, if this event occured within one
    bTownId = GetTownIdForSector(sSectorX, sSectorY);
  }

  // should other towns ignore events occuring in this town?
  if (bTownId == SAN_MONA) {
    return;
  }

  // determine what the base loyalty change of this event type is worth
  // these are ->hundredths<- of loyalty points, so choose appropriate values accordingly!
  // the closer a town is to the event, the more pronounced the effect upon that town is
  switch (ubEventType) {
    case GLOBAL_LOYALTY_BATTLE_WON:
      CheckConditionsForTriggeringCreatureQuest(sSectorX, sSectorY, bSectorZ);
      iLoyaltyChange = 500;
      break;
    case GLOBAL_LOYALTY_QUEEN_BATTLE_WON:
      CheckConditionsForTriggeringCreatureQuest(sSectorX, sSectorY, bSectorZ);
      iLoyaltyChange = 1000;
      break;
    case GLOBAL_LOYALTY_BATTLE_LOST:
      iLoyaltyChange = -750;
      break;
    case GLOBAL_LOYALTY_ENEMY_KILLED:
      iLoyaltyChange = 50;
      break;
    case GLOBAL_LOYALTY_NATIVE_KILLED:
      // note that there is special code (much more severe) for murdering civilians in the currently
      // loaded sector. this event is intended more for processing militia casualties, and the like
      iLoyaltyChange = -50;
      break;
    case GLOBAL_LOYALTY_ABANDON_MILITIA:
      // it doesn't matter how many of them are being abandoned
      iLoyaltyChange = -750;
      break;
    case GLOBAL_LOYALTY_GAIN_TOWN_SECTOR:
      iLoyaltyChange = 500;
      break;
    case GLOBAL_LOYALTY_LOSE_TOWN_SECTOR:
      iLoyaltyChange = -1000;
      break;
    case GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN:
      iLoyaltyChange = 1000;
      break;
    case GLOBAL_LOYALTY_GAIN_MINE:
      iLoyaltyChange = 1000;
      break;
    case GLOBAL_LOYALTY_LOSE_MINE:
      iLoyaltyChange = -1500;
      break;
    case GLOBAL_LOYALTY_GAIN_SAM:
      iLoyaltyChange = 250;
      break;
    case GLOBAL_LOYALTY_LOSE_SAM:
      iLoyaltyChange = -250;
      break;

    default:
      Assert(FALSE);
      return;
  }

  AffectAllTownsLoyaltyByDistanceFrom(iLoyaltyChange, sSectorX, sSectorY, bSectorZ);
}

void AffectAllTownsLoyaltyByDistanceFrom(int32_t iLoyaltyChange, uint8_t sSectorX, uint8_t sSectorY,
                                         int8_t bSectorZ) {
  uint8_t ubTempGroupId;
  int32_t iThisDistance;
  int32_t iShortestDistance[NUM_TOWNS];
  int32_t iPercentAdjustment;
  int32_t iDistanceAdjustedLoyalty;

  // preset shortest distances to high values prior to searching for a minimum
  for (TownID bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    iShortestDistance[bTownId] = 999999;
  }

  // need a temporary group create to use for laying down distance paths
  ubTempGroupId = CreateNewPlayerGroupDepartingFromSector((uint8_t)sSectorX, (uint8_t)sSectorY);

  // calc distance to the event sector from EACH GetSectorID8 of each town, keeping only the
  // shortest one for every town
  struct TownSectors townSectors;
  GetAllTownSectors(&townSectors);
  for (int i = 0; i < townSectors.count; i++) {
    TownID bTownId = townSectors.sectors[i].townID;

    // skip path test if distance is already known to be zero to speed this up a bit
    if (iShortestDistance[bTownId] > 0) {
      // calculate across how many sectors the fastest travel path from event to this town sector
      iThisDistance = FindStratPath(sSectorX, sSectorY, townSectors.sectors[i].x,
                                    townSectors.sectors[i].y, ubTempGroupId, FALSE);

      if (iThisDistance < iShortestDistance[bTownId]) {
        iShortestDistance[bTownId] = iThisDistance;
      }
    }
  }

  // must always remove that temporary group!
  RemoveGroup(ubTempGroupId);

  for (TownID bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    // doesn't affect towns where player hasn't established a "presence" yet
    if (!IsTownLoyaltyStarted(bTownId)) {
      continue;
    }

    // if event was underground, double effective distance
    if (bSectorZ != 0) {
      iShortestDistance[bTownId] *= 2;
    }

    // This number here controls how many sectors away is the "norm" for the unadjusted loyalty
    // change value
    if (iShortestDistance[bTownId] < LOYALTY_EVENT_DISTANCE_THRESHOLD) {
      // add 25% per sector below the threshold
      iPercentAdjustment = 25 * (LOYALTY_EVENT_DISTANCE_THRESHOLD - iShortestDistance[bTownId]);
    } else if (iShortestDistance[bTownId] > LOYALTY_EVENT_DISTANCE_THRESHOLD) {
      // subtract 10% per sector above the threshold
      iPercentAdjustment = -10 * (iShortestDistance[bTownId] - LOYALTY_EVENT_DISTANCE_THRESHOLD);

      // don't allow it to go below -100, that would change the sign of the loyalty effect!
      if (iPercentAdjustment < -100) {
        iPercentAdjustment = -100;
      }
    } else {
      // no distance adjustment necessary
      iPercentAdjustment = 0;
    }

    // calculate loyalty affects as adjusted for distance to this town
    iDistanceAdjustedLoyalty = (iLoyaltyChange * (100 + iPercentAdjustment)) / 100;

    if (iDistanceAdjustedLoyalty == 0) {
      // no measurable effect, skip this town
      continue;
    }

    if (iDistanceAdjustedLoyalty > 0) {
      IncrementTownLoyalty(bTownId, iDistanceAdjustedLoyalty);
    } else {
      // the decrement amount MUST be positive
      iDistanceAdjustedLoyalty *= -1;
      DecrementTownLoyalty(bTownId, iDistanceAdjustedLoyalty);
    }
  }
}

// to be called whenever player gains control of a sector in any way
void CheckIfEntireTownHasBeenLiberated(TownID bTownId, uint8_t sSectorX, uint8_t sSectorY) {
  // the whole town is under our control, check if we never libed this town before
  if (!IsTownLiberated(bTownId) && IsTownUnderCompleteControlByPlayer(bTownId)) {
    if (MilitiaTrainingAllowedInSector(sSectorX, sSectorY, 0)) {
      // give a loyalty bonus
      HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN, sSectorX, sSectorY, 0);

      // set fact is has been lib'ed and set history event
      AddHistoryToPlayersLog(HISTORY_LIBERATED_TOWN, bTownId, GetGameTimeInMin(), sSectorX,
                             sSectorY);

      HandleMeanWhileEventPostingForTownLiberation(bTownId);
    }

    // even taking over non-trainable "towns" like Orta/Tixa for the first time should count as
    // "player activity"
    if (gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD) {
      UpdateLastDayOfPlayerActivity((uint16_t)(GetGameTimeInDays() + 4));
    } else {
      UpdateLastDayOfPlayerActivity((uint16_t)(GetGameTimeInDays() + 2));
    }

    // set flag even for towns where you can't train militia, useful for knowing Orta/Tixa were
    // previously controlled
    SetTownAsLiberated(bTownId);
  }
}

void CheckIfEntireTownHasBeenLost(TownID bTownId, uint8_t sSectorX, uint8_t sSectorY) {
  // NOTE:  only towns which allow you to train militia are important enough to get
  // reported here (and they're the only ones you can protect)
  if (MilitiaTrainingAllowedInSector(sSectorX, sSectorY, 0) &&
      IsTownUnderCompleteControlByEnemy(bTownId)) {
    // the whole town is under enemy control, check if we libed this town before
    if (IsTownLiberated(bTownId)) {
      HandleMeanWhileEventPostingForTownLoss(bTownId);
    }
  }
}

void HandleLoyaltyChangeForNPCAction(uint8_t ubNPCProfileId) {
  switch (ubNPCProfileId) {
    case MIGUEL:
      // Omerta loyalty increases when Miguel receives letter from Enrico
      IncrementTownLoyalty(OMERTA, LOYALTY_BONUS_MIGUEL_READS_LETTER);
      break;

    case DOREEN:
      // having freed the child labourers... she is releasing them herself!
      IncrementTownLoyalty(DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED);
      break;

    case MARTHA:
      // if Joey is still alive
      if (gMercProfiles[JOEY].bMercStatus != MERC_IS_DEAD) {
        IncrementTownLoyalty(CAMBRIA, LOYALTY_BONUS_MARTHA_WHEN_JOEY_RESCUED);
      }
      break;

    case KEITH:
      // Hillbilly problem solved
      IncrementTownLoyalty(CAMBRIA, LOYALTY_BONUS_KEITH_WHEN_HILLBILLY_SOLVED);
      break;

    case YANNI:
      // Chalice of Chance returned to Chitzena
      IncrementTownLoyalty(CHITZENA, LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_LOCAL);
      // NOTE: This affects Chitzena,too, a second time, so first value is discounted for it
      IncrementTownLoyaltyEverywhere(LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_GLOBAL);
      break;

    case AUNTIE:
      // Bloodcats killed
      IncrementTownLoyalty(ALMA, LOYALTY_BONUS_AUNTIE_WHEN_BLOODCATS_KILLED);
      break;

    case MATT:
      // Brother Dynamo freed
      IncrementTownLoyalty(ALMA, LOYALTY_BONUS_MATT_WHEN_DYNAMO_FREED);
      break;
  }
}

// set the location of the first encounter with enemy
void SetTheFirstBattleSector(int16_t sSectorValue) {
  if (locationOfFirstBattle.x == 0 && locationOfFirstBattle.y == 0) {
    locationOfFirstBattle.x = SectorID16_X(sSectorValue);
    locationOfFirstBattle.y = SectorID16_Y(sSectorValue);
  }
}

// did first battle take place here
BOOLEAN DidFirstBattleTakePlaceInThisTown(TownID bTownId) {
  int8_t bTownBattleId = 0;

  // get town id for sector
  bTownBattleId = GetTownIdForSector(locationOfFirstBattle.x, locationOfFirstBattle.y);

  return (bTownId == bTownBattleId);
}

uint32_t PlayerStrength(void) {
  uint8_t ubLoop;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiStrength, uiTotal = 0;

  for (ubLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       ubLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; ubLoop++) {
    pSoldier = MercPtrs[ubLoop];
    if (IsSolActive(pSoldier)) {
      if (pSoldier->bInSector ||
          (pSoldier->fBetweenSectors && ((pSoldier->ubPrevSectorID % 16) + 1) == gWorldSectorX &&
           ((pSoldier->ubPrevSectorID / 16) + 1) == gWorldSectorY &&
           (GetSolSectorZ(pSoldier) == gbWorldSectorZ))) {
        // count this person's strength (condition), calculated as life reduced up to half according
        // to maxbreath
        uiStrength = pSoldier->bLife * (pSoldier->bBreathMax + 100) / 200;
        uiTotal += uiStrength;
      }
    }
  }
  return (uiTotal);
}

uint32_t EnemyStrength(void) {
  uint8_t ubLoop;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiStrength, uiTotal = 0;

  for (ubLoop = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
       ubLoop <= gTacticalStatus.Team[CIV_TEAM].bLastID; ubLoop++) {
    pSoldier = MercPtrs[ubLoop];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && !pSoldier->bNeutral) {
      // count this person's strength (condition), calculated as life reduced up to half according
      // to maxbreath
      uiStrength = pSoldier->bLife * (pSoldier->bBreathMax + 100) / 200;
      uiTotal += uiStrength;
    }
  }

  return (uiTotal);
}

// Function assumes that mercs have retreated already.  Handles two cases, one for general merc
// retreat which slightly demoralizes the mercs, the other handles abandonment of militia forces
// which poses as a serious loyalty penalty.
void HandleLoyaltyImplicationsOfMercRetreat(int8_t bRetreatCode, uint8_t sSectorX, uint8_t sSectorY,
                                            int16_t sSectorZ) {
  if (CountMilitiaInSector(sSectorX, sSectorY)) {  // Big morale penalty!
    HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ABANDON_MILITIA, sSectorX, sSectorY, (int8_t)sSectorZ);
  }

  // Standard retreat penalty
  if (bRetreatCode == RETREAT_TACTICAL_TRAVERSAL) {
    // if not worse than 2:1 odds, then penalize morale
    if (gTacticalStatus.fEnemyInSector && (PlayerStrength() * 2 >= EnemyStrength())) {
      HandleMoraleEvent(NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (int8_t)sSectorZ);
    }
  } else {
    HandleMoraleEvent(NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (int8_t)sSectorZ);
  }
}

void MaximizeLoyaltyForDeidrannaKilled(void) {
  TownID bTownId;

  // max out loyalty to player all across Arulco
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    // it's possible one of the towns still has creature problems, but it's too much of a pain to
    // worry about it now
    SetTownLoyalty(bTownId, 100);
  }
}
