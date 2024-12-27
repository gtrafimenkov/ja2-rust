#include "Strategic/StrategicTownLoyalty.h"

#include "GameSettings.h"
#include "Laptop/History.h"
#include "MessageBoxScreen.h"
#include "SGP/FileMan.h"
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

// town loyalty table
TOWN_LOYALTY gTownLoyalty[NUM_TOWNS];

INT32 iTownDistances[NUM_TOWNS][NUM_TOWNS];

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

// on a scale of 1-100, this is a measure of how much each town hates the Queen's opression & is
// willing to stand against it it primarily controls the RATE of loyalty change in each town: the
// loyalty effect of the same events depends on it
UINT8 gubTownRebelSentiment[NUM_TOWNS] = {
    0,   // not a town - blank sector index
    90,  // OMERTA,	- They ARE the rebels!!!
    30,  // DRASSEN,	- Rebel friendly, makes it pretty easy to get first mine's income going at
         // the start
    12,  // ALMA			- Military town, high loyalty to Queen, need quests to get
         // 100%
    15,  // GRUMM,		- Close to Meduna, strong influence
    20,  // TIXA,		- Not a real town
    15,  // CAMBRIA, - Artificially much lower 'cause it's big and central and too easy to get
         // loyalty up there
    20,  // SAN_MONA,- Neutral ground, loyalty doesn't vary
    20,  // ESTONI,	- Not a real town
    20,  // ORTA,		- Not a real town
    12,  // BALIME,	- Rich town, high loyalty to Queen
    10,  // MEDUNA,	- Enemy HQ, for God's sake!
    35,  // CHITZENA, - Artificially high 'cause there's not enough fights near it to get the
         // loyalty up otherwise
};

BOOLEAN gfTownUsesLoyalty[NUM_TOWNS] = {
    FALSE,  // not a town - blank sector index
    TRUE,   // OMERTA
    TRUE,   // DRASSEN
    TRUE,   // ALMA
    TRUE,   // GRUMM
    FALSE,  // TIXA
    TRUE,   // CAMBRIA
    FALSE,  // SAN_MONA
    FALSE,  // ESTONI
    FALSE,  // ORTA
    TRUE,   // BALIME
    TRUE,   // MEDUNA
    TRUE,   // CHITZENA
};

// location of first enocunter with enemy
INT16 sWorldSectorLocationOfFirstBattle = 0;

// number of items in currently loaded sector
extern uint32_t guiNumWorldItems;

// preprocess sector for mercs in it
extern BOOLEAN fSectorsWithSoldiers[MAP_WORLD_X * MAP_WORLD_X][4];

extern STR16 pTownNames[];

// update the loyalty rating of the passed town id
void UpdateTownLoyaltyRating(TownID bTownId);

// update town loyalty based on number of friendlies in this town
void UpdateTownLoyaltyBasedOnFriendliesInTown(TownID bTownId);

// update town loyalty based on number of bad guys in this town
void UpdateTownLoyaltyBasedOnBadGuysInTown(TownID bTownId);

/* ARM: Civilian theft of items was removed
// handle theft by civi in a town sector
void HandleTheftByCiviliansInSector( INT16 sX, INT16 sY, INT32 iLoyalty );

// handle theft in all towns
void HandleTownTheft( void );
*/

extern void MapScreenDefaultOkBoxCallback(UINT8 bExitValue);

void InitTownLoyalty(void) {
  UINT8 ubTown = 0;

  // set up town loyalty table
  for (ubTown = FIRST_TOWN; ubTown < NUM_TOWNS; ubTown++) {
    gTownLoyalty[ubTown].ubRating = 0;
    gTownLoyalty[ubTown].sChange = 0;
    gTownLoyalty[ubTown].fStarted = FALSE;
    //		gTownLoyalty[ ubTown ].ubRebelSentiment = gubTownRebelSentiment[ ubTown ];
    gTownLoyalty[ubTown].fLiberatedAlready = FALSE;
  }

  return;
}

void StartTownLoyaltyIfFirstTime(TownID bTownId) {
  Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

  // if loyalty tracking hasn't yet been started for this town, and the town does use loyalty
  if (!gTownLoyalty[bTownId].fStarted && gfTownUsesLoyalty[bTownId]) {
    // set starting town loyalty now, equally to that town's current rebel sentiment - not all towns
    // begin equally loyal
    gTownLoyalty[bTownId].ubRating = gubTownRebelSentiment[bTownId];

    // if player hasn't made contact with Miguel yet, or the rebels hate the player
    if (!CheckFact(FACT_MIGUEL_READ_LETTER, 0) || CheckFact(FACT_REBELS_HATE_PLAYER, 0)) {
      // if town is Omerta
      if (bTownId == OMERTA) {
        // start loyalty there at 0, since rebels distrust the player until Miguel receives the
        // letter
        gTownLoyalty[bTownId].ubRating = 0;
      } else {
        // starting loyalty is halved - locals not sure what to make of the player's presence
        gTownLoyalty[bTownId].ubRating /= 2;
      }
    }

    gTownLoyalty[bTownId].sChange = 0;

    // remember we've started
    gTownLoyalty[bTownId].fStarted = TRUE;
  }
}

// set a specified town's loyalty rating (ignores previous loyalty value - probably NOT what you
// want)
void SetTownLoyalty(TownID bTownId, UINT8 ubNewLoyaltyRating) {
  Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

  // if the town does use loyalty
  if (gfTownUsesLoyalty[bTownId]) {
    gTownLoyalty[bTownId].ubRating = ubNewLoyaltyRating;
    gTownLoyalty[bTownId].sChange = 0;

    // this is just like starting the loyalty if it happens first
    gTownLoyalty[bTownId].fStarted = TRUE;
  }

  return;
}

// increments the town's loyalty rating by that many HUNDREDTHS of loyalty pts
void IncrementTownLoyalty(TownID bTownId, uint32_t uiLoyaltyIncrease) {
  uint32_t uiRemainingIncrement;
  INT16 sThisIncrement;

  Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

  // doesn't affect towns where player hasn't established a "presence" yet
  if (!gTownLoyalty[bTownId].fStarted) {
    return;
  }

  // modify loyalty change by town's individual attitude toward rebelling (20 is typical)
  uiLoyaltyIncrease *= (5 * gubTownRebelSentiment[bTownId]);
  uiLoyaltyIncrease /= 100;

  // this whole thing is a hack to avoid rolling over the -32 to 32k range on the sChange value
  // only do a maximum of 10000 pts at a time...
  uiRemainingIncrement = uiLoyaltyIncrease;
  while (uiRemainingIncrement) {
    sThisIncrement = (INT16)min(uiRemainingIncrement, 10000);

    // up the gain value
    gTownLoyalty[bTownId].sChange += (INT16)sThisIncrement;
    // update town value now
    UpdateTownLoyaltyRating(bTownId);

    uiRemainingIncrement -= sThisIncrement;
  }

  return;
}

// decrements the town's loyalty rating by that many HUNDREDTHS of loyalty pts
// NOTE: This function expects a POSITIVE number for a decrease!!!
void DecrementTownLoyalty(TownID bTownId, uint32_t uiLoyaltyDecrease) {
  uint32_t uiRemainingDecrement;
  INT16 sThisDecrement;

  Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

  // doesn't affect towns where player hasn't established a "presence" yet
  if (!gTownLoyalty[bTownId].fStarted) {
    return;
  }

  // modify loyalty change by town's individual attitude toward rebelling (20 is typical)
  uiLoyaltyDecrease *= 100;
  uiLoyaltyDecrease /= (5 * gubTownRebelSentiment[bTownId]);

  // this whole thing is a hack to avoid rolling over the -32 to 32k range on the sChange value
  // only do a maximum of 10000 pts at a time...
  uiRemainingDecrement = uiLoyaltyDecrease;
  while (uiRemainingDecrement) {
    sThisDecrement = (INT16)min(uiRemainingDecrement, 10000);

    // down the gain value
    gTownLoyalty[bTownId].sChange -= sThisDecrement;
    // update town value now
    UpdateTownLoyaltyRating(bTownId);

    uiRemainingDecrement -= sThisDecrement;
  }

  return;
}

// update town loyalty rating based on gain values
void UpdateTownLoyaltyRating(TownID bTownId) {
  // check gain value and update loyaty
  UINT8 ubOldLoyaltyRating = 0;
  INT16 sRatingChange = 0;
  UINT8 ubMaxLoyalty = 0;

  Assert((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS));

  // remember previous loyalty value
  ubOldLoyaltyRating = gTownLoyalty[bTownId].ubRating;

  sRatingChange = gTownLoyalty[bTownId].sChange / GAIN_PTS_PER_LOYALTY_PT;

  // if loyalty is ready to increase
  if (sRatingChange > 0) {
    // if the town is Omerta, and the rebels are/will become hostile
    if ((bTownId == OMERTA) &&
        (gTacticalStatus.fCivGroupHostile[REBEL_CIV_GROUP] != CIV_GROUP_NEUTRAL)) {
      // maximum loyalty is much less than normal
      ubMaxLoyalty = HOSTILE_OMERTA_LOYALTY_RATING;
    } else {
      ubMaxLoyalty = MAX_LOYALTY_VALUE;
    }

    // check if we'd be going over the max
    if ((gTownLoyalty[bTownId].ubRating + sRatingChange) >= ubMaxLoyalty) {
      // set to max and null out gain pts
      gTownLoyalty[bTownId].ubRating = ubMaxLoyalty;
      gTownLoyalty[bTownId].sChange = 0;
    } else {
      // increment loyalty rating, reduce sChange
      gTownLoyalty[bTownId].ubRating += sRatingChange;
      gTownLoyalty[bTownId].sChange %= GAIN_PTS_PER_LOYALTY_PT;
    }
  } else
    // if loyalty is ready to decrease
    if (sRatingChange < 0) {
      // check if we'd be going below zero
      if ((gTownLoyalty[bTownId].ubRating + sRatingChange) < 0) {
        // set to zero and null out gain pts
        gTownLoyalty[bTownId].ubRating = 0;
        gTownLoyalty[bTownId].sChange = 0;
      } else {
        // decrement loyalty rating, reduce sChange
        gTownLoyalty[bTownId].ubRating += sRatingChange;
        gTownLoyalty[bTownId].sChange %= GAIN_PTS_PER_LOYALTY_PT;
      }
    }

  // check old aginst new, if diff, dirty map panel
  if (ubOldLoyaltyRating != gTownLoyalty[bTownId].ubRating) {
    MarkForRedrawalStrategicMap();
  }

  return;
}

// strategic handler, goes through and handles all strategic events for town loyalty
// updates...player controlled, monsters
void HandleTownLoyalty(void) { return; }

void HandleMurderOfCivilian(struct SOLDIERTYPE *pSoldier, BOOLEAN fIntentional) {
  // handle the impact on loyalty of the murder of a civilian
  TownID bTownId = 0;
  INT32 iLoyaltyChange = 0;
  INT8 bSeenState = 0;
  INT32 iCounter = 0;
  struct SOLDIERTYPE *pCivSoldier = NULL;
  uint32_t uiChanceFalseAccusal = 0;
  INT8 bKillerTeam = 0;
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
    HandleMoraleEvent(pKiller, MORALE_KILLED_CIVILIAN, pKiller->sSectorX, pKiller->sSectorY,
                      pKiller->bSectorZ);
  }

  // get town id
  bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

  // if civilian is NOT in a town
  if (bTownId == BLANK_SECTOR) {
    return;
  }

  // check if this town does use loyalty (to skip a lot of unnecessary computation)
  if (!gfTownUsesLoyalty[bTownId]) {
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
        uiChanceFalseAccusal = MAX_LOYALTY_VALUE - gTownLoyalty[bTownId].ubRating;
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
      if (StrategicMap[(GetSolSectorX(pSoldier)) + (MAP_WORLD_X * (GetSolSectorY(pSoldier)))]
              .fEnemyControlled == TRUE) {
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
        if (StrategicMap[(GetSolSectorX(pSoldier)) + (MAP_WORLD_X * (GetSolSectorY(pSoldier)))]
                .fEnemyControlled == TRUE) {
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
  bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

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

BOOLEAN HandleLoyaltyAdjustmentForRobbery(struct SOLDIERTYPE *pSoldier) {
  // not to be implemented at this time
  return (FALSE);
}

// handle loyalty adjustment for dmg inflicted on a building
void HandleLoyaltyForDemolitionOfBuilding(struct SOLDIERTYPE *pSoldier, INT16 sPointsDmg) {
  // find this soldier's team and decrement the loyalty rating for them and for the people who
  // police the sector more penalty for the people who did it, a lesser one for those who should
  // have stopped it
  INT16 sLoyaltyValue = 0;
  INT16 sPolicingLoyalty = 0;
  TownID bTownId = 0;

  // hurt loyalty for damaging the building
  sLoyaltyValue = sPointsDmg * MULTIPLIER_FOR_DAMAGING_A_BUILDING;

  // penalty for not preventing the action
  sPolicingLoyalty = sPointsDmg * MULTIPLIER_FOR_NOT_PREVENTING_BUILDING_DAMAGE;

  // get town id
  bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

  // penalize the side that did it
  if (pSoldier->bTeam == OUR_TEAM) {
    DecrementTownLoyalty(bTownId, sLoyaltyValue);
  } else if (pSoldier->bTeam == ENEMY_TEAM) {
    // enemy damaged sector, it's their fault
    IncrementTownLoyalty(bTownId, sLoyaltyValue);
  } else if (pSoldier->ubCivilianGroup == REBEL_CIV_GROUP) {
    // the rebels did it...are they on our side
    if (CheckFact(FACT_REBELS_HATE_PLAYER, 0) == FALSE) {
      sLoyaltyValue /= DIVISOR_FOR_REBEL_BUILDING_DMG;

      // decrement loyalty value for rebels on our side dmging town
      DecrementTownLoyalty(bTownId, sLoyaltyValue);
    }
  }

  // penalize the side that should have stopped it
  if (StrategicMap[GetSolSectorX(pSoldier) + pSoldier->sSectorY * MAP_WORLD_X].fEnemyControlled ==
      TRUE) {
    // enemy should have prevented it, let them suffer a little
    IncrementTownLoyalty(bTownId, sPolicingLoyalty);
  } else {
    // we should have prevented it, we shall suffer
    DecrementTownLoyalty(bTownId, sPolicingLoyalty);
  }

  return;
}

void RemoveRandomItemsInSector(u8 sSectorX, u8 sSectorY, INT16 sSectorZ, UINT8 ubChance) {
  // remove random items in sector
  uint32_t uiNumberOfItems = 0, iCounter = 0;
  WORLDITEM *pItemList;
  uint32_t uiNewTotal = 0;
  CHAR16 wSectorName[128];

  // stealing should fail anyway 'cause there shouldn't be a temp file for unvisited sectors, but
  // let's check anyway
  Assert(GetSectorFlagStatus(sSectorX, sSectorY, (UINT8)sSectorZ, SF_ALREADY_VISITED) == TRUE);

  // get sector name string
  GetSectorIDString(sSectorX, sSectorY, (INT8)sSectorZ, wSectorName, ARR_SIZE(wSectorName), TRUE);

  // go through list of items in sector and randomly remove them

  // if unloaded sector
  if (gWorldSectorX != sSectorX || gWorldSectorY != sSectorY || gbWorldSectorZ != sSectorZ) {
    // if the player has never been there, there's no temp file, and 0 items will get returned,
    // preventing any stealing
    GetNumberOfWorldItemsFromTempItemFile(sSectorX, sSectorY, (UINT8)sSectorZ, &uiNumberOfItems,
                                          FALSE);

    if (uiNumberOfItems == 0) {
      return;
    }

    pItemList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * uiNumberOfItems);

    // now load items
    LoadWorldItemsFromTempItemFile(sSectorX, sSectorY, (UINT8)sSectorZ, pItemList);
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
      AddWorldItemsToUnLoadedSector(sSectorX, sSectorY, (UINT8)sSectorZ, 0, uiNumberOfItems,
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
  UINT8 ubTownA, ubTownB;
  uint32_t uiCounterA, uiCounterB;
  UINT8 ubTempGroupId = 0;
  INT32 iDistance = 0;

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
  uiCounterA = 0;
  uiCounterB = 0;
  const TownSectors *townSectors = GetAllTownSectors();
  while ((*townSectors)[uiCounterA].townID != 0) {
    // reset second counter
    uiCounterB = uiCounterA;

    while ((*townSectors)[uiCounterB].townID != 0) {
      ubTownA = (UINT8)(*townSectors)[uiCounterA].townID;
      ubTownB = (UINT8)(*townSectors)[uiCounterB].townID;

      // if they're not in the same town
      if (ubTownA != ubTownB) {
        // calculate fastest distance between them (in sectors) - not necessarily shortest distance,
        // roads are faster!
        iDistance = FindStratPath((INT16)(*townSectors)[uiCounterA].sectorID,
                                  (INT16)(*townSectors)[uiCounterB].sectorID, ubTempGroupId, FALSE);
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

      uiCounterB++;
    }

    uiCounterA++;
  }

  RemoveGroup(ubTempGroupId);
}

void WriteOutDistancesBetweenTowns(void) {
  HWFILE hFileHandle;

  hFileHandle =
      FileMan_Open("BinaryData\\TownDistances.dat", FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);

  FileMan_Write(hFileHandle, &(iTownDistances), (sizeof(INT32) * NUM_TOWNS * NUM_TOWNS), NULL);

  // close file
  FileMan_Close(hFileHandle);

  return;
}

void DumpDistancesBetweenTowns(void) {
  CHAR8 zPrintFileName[60];
  FILE *FDump;
  UINT8 ubTownA, ubTownB;
  CHAR16 wHeading[4];

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
  HWFILE hFileHandle;

  hFileHandle = FileMan_Open("BinaryData\\TownDistances.dat", FILE_ACCESS_READ, FALSE);

  FileMan_Read(hFileHandle, &(iTownDistances), (sizeof(INT32) * NUM_TOWNS * NUM_TOWNS), NULL);

  // close file
  FileMan_Close(hFileHandle);

  return;
}

INT32 GetTownDistances(UINT8 ubTown, UINT8 ubTownA) { return (iTownDistances[ubTown][ubTownA]); }

BOOLEAN SaveStrategicTownLoyaltyToSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten;

  // Save the Town Loyalty
  FileMan_Write(hFile, gTownLoyalty, sizeof(TOWN_LOYALTY) * NUM_TOWNS, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(TOWN_LOYALTY) * NUM_TOWNS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadStrategicTownLoyaltyFromSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead;

  // Restore the Town Loyalty
  FileMan_Read(hFile, gTownLoyalty, sizeof(TOWN_LOYALTY) * NUM_TOWNS, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(TOWN_LOYALTY) * NUM_TOWNS) {
    return (FALSE);
  }

  return (TRUE);
}

void ReduceLoyaltyForRebelsBetrayed(void) {
  TownID bTownId;

  // reduce loyalty to player all across Arulco
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (bTownId == OMERTA) {
      // if not already really low
      if (gTownLoyalty[bTownId].ubRating > HOSTILE_OMERTA_LOYALTY_RATING) {
        // loyalty in Omerta tanks big time, and will stay low permanently since this becomes its
        // maximum
        SetTownLoyalty(bTownId, HOSTILE_OMERTA_LOYALTY_RATING);
        // note that rebel sentiment isn't affected - they remain loyal to themselves, after all!
      }
    } else {
      // loyalty in other places is also strongly affected by this falling out with rebels, but this
      // is not permanent
      SetTownLoyalty(bTownId, (UINT8)(gTownLoyalty[bTownId].ubRating / 3));
    }
  }
}

INT32 GetNumberOfWholeTownsUnderControl(void) {
  INT32 iNumber = 0;
  TownID bTownId = 0;

  // run through the list of towns..if the entire town is under player control, then increment the
  // number of towns under player control

  // make sure that each town is one for which loyalty matters
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (IsTownUnderCompleteControlByPlayer(bTownId) && gfTownUsesLoyalty[bTownId]) {
      iNumber++;
    }
  }

  return (iNumber);
}

INT32 GetNumberOfWholeTownsUnderControlButExcludeCity(INT8 bCityToExclude) {
  INT32 iNumber = 0;
  TownID bTownId = 0;

  // run through the list of towns..if the entire town is under player control, then increment the
  // number of towns under player control
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    if (IsTownUnderCompleteControlByPlayer(bTownId) && (bCityToExclude != bTownId) &&
        gfTownUsesLoyalty[bTownId]) {
      iNumber++;
    }
  }

  return (iNumber);
}

// is the ENTIRE town under player control?
INT32 IsTownUnderCompleteControlByPlayer(TownID bTownId) {
  if (GetTownSectorSize(bTownId) == GetTownSectorsUnderControl(bTownId)) {
    return (TRUE);
  }

  return (FALSE);
}

// is the ENTIRE town under enemy control?
INT32 IsTownUnderCompleteControlByEnemy(TownID bTownId) {
  if (GetTownSectorsUnderControl(bTownId) == 0) {
    return (TRUE);
  }

  return (FALSE);
}

void AdjustLoyaltyForCivsEatenByMonsters(u8 sSectorX, u8 sSectorY, UINT8 ubHowMany) {
  TownID bTownId = 0;
  uint32_t uiLoyaltyChange = 0;
  CHAR16 str[256];
  CHAR16 pSectorString[128];

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

// this applies the SAME change to every town equally, regardless of distance from the event
void IncrementTownLoyaltyEverywhere(uint32_t uiLoyaltyIncrease) {
  TownID bTownId;

  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    IncrementTownLoyalty(bTownId, uiLoyaltyIncrease);
  }
}

void DecrementTownLoyaltyEverywhere(uint32_t uiLoyaltyDecrease) {
  TownID bTownId;

  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    DecrementTownLoyalty(bTownId, uiLoyaltyDecrease);
  }
}
// this applies the change to every town differently, depending on the distance from the event
void HandleGlobalLoyaltyEvent(UINT8 ubEventType, u8 sSectorX, u8 sSectorY, INT8 bSectorZ) {
  INT32 iLoyaltyChange;
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

void AffectAllTownsLoyaltyByDistanceFrom(INT32 iLoyaltyChange, u8 sSectorX, u8 sSectorY,
                                         INT8 bSectorZ) {
  INT16 sEventSector;
  UINT8 ubTempGroupId;
  TownID bTownId;
  uint32_t uiIndex;
  INT32 iThisDistance;
  INT32 iShortestDistance[NUM_TOWNS];
  INT32 iPercentAdjustment;
  INT32 iDistanceAdjustedLoyalty;

  // preset shortest distances to high values prior to searching for a minimum
  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    iShortestDistance[bTownId] = 999999;
  }

  sEventSector = sSectorX + (MAP_WORLD_X * sSectorY);

  // need a temporary group create to use for laying down distance paths
  ubTempGroupId = CreateNewPlayerGroupDepartingFromSector((UINT8)sSectorX, (UINT8)sSectorY);

  // calc distance to the event sector from EACH GetSectorID8 of each town, keeping only the
  // shortest one for every town
  uiIndex = 0;
  const TownSectors *townSectors = GetAllTownSectors();
  while ((*townSectors)[uiIndex].townID != 0) {
    bTownId = (UINT8)(*townSectors)[uiIndex].townID;

    // skip path test if distance is already known to be zero to speed this up a bit
    if (iShortestDistance[bTownId] > 0) {
      // calculate across how many sectors the fastest travel path from event to this town sector
      iThisDistance = FindStratPath(sEventSector, (INT16)(*townSectors)[uiIndex].sectorID,
                                    ubTempGroupId, FALSE);

      if (iThisDistance < iShortestDistance[bTownId]) {
        iShortestDistance[bTownId] = iThisDistance;
      }
    }

    uiIndex++;
  }

  // must always remove that temporary group!
  RemoveGroup(ubTempGroupId);

  for (bTownId = FIRST_TOWN; bTownId < NUM_TOWNS; bTownId++) {
    // doesn't affect towns where player hasn't established a "presence" yet
    if (!gTownLoyalty[bTownId].fStarted) {
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
void CheckIfEntireTownHasBeenLiberated(TownID bTownId, u8 sSectorX, u8 sSectorY) {
  // the whole town is under our control, check if we never libed this town before
  if (!gTownLoyalty[bTownId].fLiberatedAlready && IsTownUnderCompleteControlByPlayer(bTownId)) {
    if (MilitiaTrainingAllowedInSector(sSectorX, sSectorY, 0)) {
      // give a loyalty bonus
      HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN, sSectorX, sSectorY, 0);

      // set fact is has been lib'ed and set history event
      AddHistoryToPlayersLog(HISTORY_LIBERATED_TOWN, bTownId, GetWorldTotalMin(), sSectorX,
                             sSectorY);

      HandleMeanWhileEventPostingForTownLiberation(bTownId);
    }

    // even taking over non-trainable "towns" like Orta/Tixa for the first time should count as
    // "player activity"
    if (gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD) {
      UpdateLastDayOfPlayerActivity((UINT16)(GetWorldDay() + 4));
    } else {
      UpdateLastDayOfPlayerActivity((UINT16)(GetWorldDay() + 2));
    }

    // set flag even for towns where you can't train militia, useful for knowing Orta/Tixa were
    // previously controlled
    gTownLoyalty[bTownId].fLiberatedAlready = TRUE;
  }
}

void CheckIfEntireTownHasBeenLost(TownID bTownId, u8 sSectorX, u8 sSectorY) {
  // NOTE:  only towns which allow you to train militia are important enough to get
  // reported here (and they're the only ones you can protect)
  if (MilitiaTrainingAllowedInSector(sSectorX, sSectorY, 0) &&
      IsTownUnderCompleteControlByEnemy(bTownId)) {
    // the whole town is under enemy control, check if we libed this town before
    if (gTownLoyalty[bTownId].fLiberatedAlready) {
      HandleMeanWhileEventPostingForTownLoss(bTownId);
    }
  }
}

void HandleLoyaltyChangeForNPCAction(UINT8 ubNPCProfileId) {
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
void SetTheFirstBattleSector(INT16 sSectorValue) {
  if (sWorldSectorLocationOfFirstBattle == 0) {
    sWorldSectorLocationOfFirstBattle = sSectorValue;
  }

  return;
}

// did first battle take place here
BOOLEAN DidFirstBattleTakePlaceInThisTown(TownID bTownId) {
  INT8 bTownBattleId = 0;

  // get town id for sector
  bTownBattleId = GetTownIdForSector(SectorID16_X(sWorldSectorLocationOfFirstBattle),
                                     SectorID16_Y(sWorldSectorLocationOfFirstBattle));

  return (bTownId == bTownBattleId);
}

uint32_t PlayerStrength(void) {
  UINT8 ubLoop;
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
  UINT8 ubLoop;
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
void HandleLoyaltyImplicationsOfMercRetreat(INT8 bRetreatCode, u8 sSectorX, u8 sSectorY,
                                            INT16 sSectorZ) {
  if (CountAllMilitiaInSector(sSectorX, sSectorY)) {  // Big morale penalty!
    HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ABANDON_MILITIA, sSectorX, sSectorY, (INT8)sSectorZ);
  }

  // Standard retreat penalty
  if (bRetreatCode == RETREAT_TACTICAL_TRAVERSAL) {
    // if not worse than 2:1 odds, then penalize morale
    if (gTacticalStatus.fEnemyInSector && (PlayerStrength() * 2 >= EnemyStrength())) {
      HandleMoraleEvent(NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (INT8)sSectorZ);
    }
  } else {
    HandleMoraleEvent(NULL, MORALE_RAN_AWAY, sSectorX, sSectorY, (INT8)sSectorZ);
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
