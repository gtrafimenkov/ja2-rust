#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "SGP/Random.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Tactical/Boxing.h"
#include "Tactical/Campaign.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Items.h"
#include "Tactical/Morale.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "UI.h"

void HourlyQuestUpdate(void);
void HourlyLarryUpdate(void);

extern int32_t GetCurrentBalance(void);
extern void PayOffSkyriderDebtIfAny();
void HourlyCheckIfSlayAloneSoHeCanLeave();

void UpdateRegenCounters(void);

void HandleMinuteUpdate() {}

// This function gets called every hour, on the hour.
// It spawns the handling of all sorts of stuff:
// Morale changes, assignment progress, town loyalties, etc.

void HandleHourlyUpdate() {
  // if the game hasnt even started yet ( we havent arrived in the sector ) dont process this
  if (DidGameJustStart()) return;

  // hourly update of team assignments
  UpdateAssignments();

  // hourly update of hated/liked mercs
  UpdateBuddyAndHatedCounters();

  // update morale!
  HourlyMoraleUpdate();

  // degrade camouflage
  HourlyCamouflageUpdate();

  // check mines for monster infestation/extermination
  HourlyMinesUpdate();

  // check how well the player is doing right now
  HourlyProgressUpdate();

  // do any quest stuff necessary
  HourlyQuestUpdate();

  HourlyLarryUpdate();

  HourlyCheckIfSlayAloneSoHeCanLeave();

  PayOffSkyriderDebtIfAny();

  if (GetGameClockHour() % 6 == 0)  // 4 times a day
  {
    UpdateRegenCounters();
  }
}

void UpdateRegenCounters(void) {
  uint8_t ubID;

  for (ubID = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       ubID <= gTacticalStatus.Team[gbPlayerNum].bLastID; ubID++) {
    if (MercPtrs[ubID]->bRegenBoostersUsedToday > 0) {
      MercPtrs[ubID]->bRegenBoostersUsedToday--;
    }
  }
}

void HandleQuarterHourUpdate() {
  // if the game hasnt even started yet ( we havent arrived in the sector ) dont process this
  if (DidGameJustStart()) return;

  DecayTacticalMoraleModifiers();
}

void HourlyQuestUpdate(void) {
  uint32_t uiHour = GetGameClockHour();

  // brothel
  if (uiHour == 4) {
    SetFactFalse(FACT_BROTHEL_OPEN);
  } else if (uiHour == 20) {
    SetFactTrue(FACT_BROTHEL_OPEN);
  }

  // bar/nightclub
  if (uiHour == 15) {
    uint8_t ubLoop;

    SetFactTrue(FACT_CLUB_OPEN);
    SetFactFalse(FACT_PAST_CLUB_CLOSING_AND_PLAYER_WARNED);

    // reset boxes fought
    for (ubLoop = 0; ubLoop < NUM_BOXERS; ubLoop++) {
      gfBoxerFought[ubLoop] = FALSE;
    }

    // if # of boxing matches the player has won is a multiple of
    // 3, and the boxers haven't rested, then make them rest
    if (gfBoxersResting) {
      // done resting now!
      gfBoxersResting = FALSE;
      gubBoxersRests++;
    } else if (gubBoxingMatchesWon / 3 > gubBoxersRests) {
      // time for the boxers to rest!
      gfBoxersResting = TRUE;
    }
  } else if (uiHour == 2) {
    SetFactFalse(FACT_CLUB_OPEN);
  }

  // museum
  if (uiHour == 9) {
    SetFactTrue(FACT_MUSEUM_OPEN);
  } else if (uiHour == 18) {
    SetFactFalse(FACT_MUSEUM_OPEN);
  }
}

#define BAR_TEMPTATION 4
#define NUM_LARRY_ITEMS 6
uint16_t LarryItems[NUM_LARRY_ITEMS][3] = {
    // item, temptation, points to use
    {ADRENALINE_BOOSTER, 5, 100},
    {ALCOHOL, BAR_TEMPTATION, 25},
    {MEDICKIT, 4, 10},
    {WINE, 3, 50},
    {REGEN_BOOSTER, 3, 100},
    {BEER, 2, 100},
};

#define LARRY_FALLS_OFF_WAGON 8

void HourlyLarryUpdate(void) {
  struct SOLDIERTYPE *pSoldier;
  int8_t bSlot, bBoozeSlot;
  int8_t bLarryItemLoop;
  uint16_t usTemptation = 0;
  uint16_t usCashAmount;
  BOOLEAN fBar = FALSE;

  pSoldier = FindSoldierByProfileID(LARRY_NORMAL, TRUE);
  if (!pSoldier) {
    pSoldier = FindSoldierByProfileID(LARRY_DRUNK, TRUE);
  }
  if (pSoldier) {
    if (pSoldier->bAssignment >= ON_DUTY) {
      return;
    }
    if (pSoldier->fBetweenSectors) {
      return;
    }
    if (IsSolActive(pSoldier) && pSoldier->bInSector &&
        (gTacticalStatus.fEnemyInSector || IsTacticalMode())) {
      return;
    }

    // look for items in Larry's inventory to maybe use
    for (bLarryItemLoop = 0; bLarryItemLoop < NUM_LARRY_ITEMS; bLarryItemLoop++) {
      bSlot = FindObj(pSoldier, LarryItems[bLarryItemLoop][0]);
      if (bSlot != NO_SLOT) {
        usTemptation = LarryItems[bLarryItemLoop][1];
        break;
      }
    }

    // check to see if we're in a bar sector, if we are, we have access to alcohol
    // which may be better than anything we've got...
    if (usTemptation < BAR_TEMPTATION && GetCurrentBalance() >= Item[ALCOHOL].usPrice) {
      if (GetSolSectorZ(pSoldier) == 0 &&
          ((GetSolSectorX(pSoldier) == 13 && GetSolSectorY(pSoldier) == MAP_ROW_B) ||
           (GetSolSectorX(pSoldier) == 13 && GetSolSectorY(pSoldier) == MAP_ROW_C) ||
           (GetSolSectorX(pSoldier) == 5 && GetSolSectorY(pSoldier) == MAP_ROW_C) ||
           (GetSolSectorX(pSoldier) == 6 && GetSolSectorY(pSoldier) == MAP_ROW_C) ||
           (GetSolSectorX(pSoldier) == 5 && GetSolSectorY(pSoldier) == MAP_ROW_D) ||
           (GetSolSectorX(pSoldier) == 2 && GetSolSectorY(pSoldier) == MAP_ROW_H))) {
        // in a bar!
        fBar = TRUE;
        usTemptation = BAR_TEMPTATION;
      }
    }

    if (usTemptation > 0) {
      if (GetSolProfile(pSoldier) == LARRY_NORMAL) {
        gMercProfiles[LARRY_NORMAL].bNPCData += (int8_t)Random(usTemptation);
        if (gMercProfiles[LARRY_NORMAL].bNPCData >= LARRY_FALLS_OFF_WAGON) {
          if (fBar) {
            // take $ from player's account
            usCashAmount = Item[ALCOHOL].usPrice;
            AddTransactionToPlayersBook(TRANSFER_FUNDS_TO_MERC, GetSolProfile(pSoldier),
                                        -(usCashAmount));
            // give Larry some booze and set slot etc values appropriately
            bBoozeSlot = FindEmptySlotWithin(pSoldier, HANDPOS, SMALLPOCK8POS);
            if (bBoozeSlot != NO_SLOT) {
              // give Larry booze here
              CreateItem(ALCOHOL, 100, &(pSoldier->inv[bBoozeSlot]));
            }
            bSlot = bBoozeSlot;
            bLarryItemLoop = 1;
          }
          // ahhhh!!!
          SwapLarrysProfiles(pSoldier);
          if (bSlot != NO_SLOT) {
            UseKitPoints(&(pSoldier->inv[bSlot]), LarryItems[bLarryItemLoop][2], pSoldier);
          }
        }
      } else {
        // NB store all drunkenness info in LARRY_NORMAL profile (to use same values)
        // so long as he keeps consuming, keep number above level at which he cracked
        gMercProfiles[LARRY_NORMAL].bNPCData =
            max(gMercProfiles[LARRY_NORMAL].bNPCData, LARRY_FALLS_OFF_WAGON);
        gMercProfiles[LARRY_NORMAL].bNPCData += (int8_t)Random(usTemptation);
        // allow value to keep going up to 24 (about 2 days since we subtract Random( 2 ) when he
        // has no access )
        gMercProfiles[LARRY_NORMAL].bNPCData = min(gMercProfiles[LARRY_NORMAL].bNPCData, 24);
        if (fBar) {
          // take $ from player's account
          usCashAmount = Item[ALCOHOL].usPrice;
          AddTransactionToPlayersBook(TRANSFER_FUNDS_TO_MERC, GetSolProfile(pSoldier),
                                      -(usCashAmount));
          // give Larry some booze and set slot etc values appropriately
          bBoozeSlot = FindEmptySlotWithin(pSoldier, HANDPOS, SMALLPOCK8POS);
          if (bBoozeSlot != NO_SLOT) {
            // give Larry booze here
            CreateItem(ALCOHOL, 100, &(pSoldier->inv[bBoozeSlot]));
          }
          bSlot = bBoozeSlot;
          bLarryItemLoop = 1;
        }
        if (bSlot != NO_SLOT) {
          // ahhhh!!!
          UseKitPoints(&(pSoldier->inv[bSlot]), LarryItems[bLarryItemLoop][2], pSoldier);
        }
      }
    } else if (GetSolProfile(pSoldier) == LARRY_DRUNK) {
      gMercProfiles[LARRY_NORMAL].bNPCData -= (int8_t)Random(2);
      if (gMercProfiles[LARRY_NORMAL].bNPCData <= 0) {
        // goes sober!
        SwapLarrysProfiles(pSoldier);
      }
    }
  }
}

void HourlyCheckIfSlayAloneSoHeCanLeave() {
  struct SOLDIERTYPE *pSoldier;
  pSoldier = FindSoldierByProfileID(SLAY, TRUE);
  if (!pSoldier) {
    return;
  }
  if (pSoldier->fBetweenSectors) {
    return;
  }
  if (!IsSolActive(pSoldier) || !pSoldier->bLife) {
    return;
  }
  if (PlayerMercsInSector((uint8_t)GetSolSectorX(pSoldier), (uint8_t)GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier)) == 1) {
    if (Chance(15)) {
      pSoldier->ubLeaveHistoryCode = HISTORY_SLAY_MYSTERIOUSLY_LEFT;
      TacticalCharacterDialogueWithSpecialEvent(
          pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP, 0, 0);
    }
  }
}
