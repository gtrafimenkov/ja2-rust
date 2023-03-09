#include "Strategic/CreatureSpreading.h"

#include "GameSettings.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "SGP/FileMan.h"
#include "SGP/HImage.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/AnimationData.h"
#include "Tactical/MapInformation.h"
#include "Tactical/OppList.h"
#include "Tactical/SoldierInitList.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/MapEdgepoints.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"

#ifdef JA2BETAVERSION
BOOLEAN gfClearCreatureQuest = FALSE;
extern UINT32 uiMeanWhileFlags;
#endif

// GAME BALANCING DEFINITIONS FOR CREATURE SPREADING
// Hopefully, adjusting these following definitions will ease the balancing of the
// creature spreading.
// The one note here is that for any definitions that have a XXX_BONUS at the end of a definition,
// it gets added on to it's counterpart via:
//								XXX_VALUE + Random( 1 + XXX_BONUS )

// The start day random bonus that the queen begins
// #define EASY_QUEEN_START_DAY								10
////easy start day 10-13 #define EASY_QUEEN_START_BONUS 3 #define NORMAL_QUEEN_START_DAY 8
// //normal start day 8-10 #define NORMAL_QUEEN_START_BONUS
// 2 #define HARD_QUEEN_START_DAY								7
////hard start day 7-8 #define HARD_QUEEN_START_BONUS 1

// This is how often the creatures spread, once the quest begins.  The smaller the gap,
// the faster the creatures will advance.  This is also directly related to the reproduction
// rates which are applied each time the creatures spread.
#define EASY_SPREAD_TIME_IN_MINUTES 510    // easy spreads every 8.5 hours
#define NORMAL_SPREAD_TIME_IN_MINUTES 450  // normal spreads every 7.5 hours
#define HARD_SPREAD_TIME_IN_MINUTES 390    // hard spreads every 6.5 hours

// Once the queen is added to the game, we can instantly let her spread x number of times
// to give her a head start.  This can also be a useful tool for having slow reproduction rates
// but quicker head start to compensate to make the creatures less aggressive overall.
#define EASY_QUEEN_INIT_BONUS_SPREADS 1
#define NORMAL_QUEEN_INIT_BONUS_SPREADS 2
#define HARD_QUEEN_INIT_BONUS_SPREADS 3

// This value modifies the chance to populate a given sector.  This is different from the previous
// definition. This value gets applied to a potentially complicated formula, using the creature
// habitat to modify chance to populate, along with factoring in the relative distance to the hive
// range (to promote deeper lair population increases), etc.  I would recommend not tweaking the
// value too much in either direction from zero due to the fact that this can greatly effect spread
// times and maximum populations.  Basically, if the creatures are spreading too quickly, increase
// the value, otherwise decrease it to a negative value
#define EASY_POPULATION_MODIFIER 0
#define NORMAL_POPULATION_MODIFIER 0
#define HARD_POPULATION_MODIFIER 0

// Augments the chance that the creatures will attack a town.  The conditions for attacking a town
// are based strictly on the occupation of the creatures in each of the four mine exits.  For each
// creature there is a base chance of 10% that the creatures will feed sometime during the night.
#define EASY_CREATURE_TOWN_AGGRESSIVENESS -10
#define NORMAL_CREATURE_TOWN_AGGRESSIVENESS 0
#define HARD_CREATURE_TOWN_AGGRESSIVENESS 10

// This is how many creatures the queen produces for each cycle of spreading.  The higher
// the numbers the faster the creatures will advance.
#define EASY_QUEEN_REPRODUCTION_BASE 6  // 6-7
#define EASY_QUEEN_REPRODUCTION_BONUS 1
#define NORMAL_QUEEN_REPRODUCTION_BASE 7  // 7-9
#define NORMAL_QUEEN_REPRODUCTION_BONUS 2
#define HARD_QUEEN_REPRODUCTION_BASE 9  // 9-12
#define HARD_QUEEN_REPRODUCTION_BONUS 3

// When either in a cave level with blue lights or there is a creature presence, then
// we override the normal music with the creature music.  The conditions are maintained
// inside the function PrepareCreaturesForBattle() in this module.
BOOLEAN gfUseCreatureMusic = FALSE;
BOOLEAN gfCreatureMeanwhileScenePlayed = FALSE;
enum {
  QUEEN_LAIR,       // where the queen lives.  Highly protected
  LAIR,             // part of the queen's lair -- lots of babies and defending mothers
  LAIR_ENTRANCE,    // where the creatures access the mine.
  INNER_MINE,       // parts of the mines that aren't close to the outside world
  OUTER_MINE,       // area's where miners work, close to towns, creatures love to eat :)
  FEEDING_GROUNDS,  // creatures love to populate these sectors :)
  MINE_EXIT,        // the area that creatures can initiate town attacks if lots of monsters.
};

typedef struct CREATURE_DIRECTIVE {
  struct CREATURE_DIRECTIVE *next;
  UNDERGROUND_SECTORINFO *pLevel;
} CREATURE_DIRECTIVE;

CREATURE_DIRECTIVE *lair;
INT32 giHabitatedDistance = 0;
INT32 giPopulationModifier = 0;
INT32 giLairID = 0;
INT32 giDestroyedLairID = 0;

// various information required for keeping track of the battle sector involved for
// prebattle interface, autoresolve, etc.
INT16 gsCreatureInsertionCode = 0;
INT16 gsCreatureInsertionGridNo = 0;
UINT8 gubNumCreaturesAttackingTown = 0;
UINT8 gubYoungMalesAttackingTown = 0;
UINT8 gubYoungFemalesAttackingTown = 0;
UINT8 gubAdultMalesAttackingTown = 0;
UINT8 gubAdultFemalesAttackingTown = 0;
UINT8 gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
UINT8 gubSectorIDOfCreatureAttack = 0;

extern UNDERGROUND_SECTORINFO *FindUnderGroundSector(u8 sMapX, u8 sMapY, UINT8 bMapZ);
extern UNDERGROUND_SECTORINFO *NewUndergroundNode(UINT8 ubSectorX, UINT8 ubSectorY,
                                                  UINT8 ubSectorZ);
extern void BuildUndergroundSectorInfoList();
void DeleteCreatureDirectives();

extern MINE_STATUS_TYPE gMineStatus[MAX_NUMBER_OF_MINES];

CREATURE_DIRECTIVE *NewDirective(UINT8 ubSectorID, UINT8 ubSectorZ, UINT8 ubCreatureHabitat) {
  CREATURE_DIRECTIVE *curr;
  UINT8 ubSectorX, ubSectorY;
  curr = (CREATURE_DIRECTIVE *)MemAlloc(sizeof(CREATURE_DIRECTIVE));
  Assert(curr);
  ubSectorX = (UINT8)((ubSectorID % 16) + 1);
  ubSectorY = (UINT8)((ubSectorID / 16) + 1);
  curr->pLevel = FindUnderGroundSector(ubSectorX, ubSectorY, ubSectorZ);
  if (!curr->pLevel) {
    AssertMsg(0, String("Could not find underground sector node (%c%db_%d) that should exist.",
                        ubSectorY + 'A' - 1, ubSectorX, ubSectorZ));
    return 0;
  }

  curr->pLevel->ubCreatureHabitat = ubCreatureHabitat;
  Assert(curr->pLevel);
  curr->next = NULL;
  return curr;
}

void InitLairDrassen() {
  CREATURE_DIRECTIVE *curr;
  giLairID = 1;
  // initialize the linked list of lairs
  lair = NewDirective(SEC_F13, 3, QUEEN_LAIR);
  curr = lair;
  if (!curr->pLevel->ubNumCreatures) {
    curr->pLevel->ubNumCreatures = 1;  // for the queen.
  }
  curr->next = NewDirective(SEC_G13, 3, LAIR);
  curr = curr->next;
  curr->next = NewDirective(SEC_G13, 2, LAIR_ENTRANCE);
  curr = curr->next;
  curr->next = NewDirective(SEC_F13, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_E13, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_E13, 1, OUTER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_D13, 1, MINE_EXIT);
}

void InitLairCambria() {
  CREATURE_DIRECTIVE *curr;
  giLairID = 2;
  // initialize the linked list of lairs
  lair = NewDirective(SEC_J8, 3, QUEEN_LAIR);
  curr = lair;
  if (!curr->pLevel->ubNumCreatures) {
    curr->pLevel->ubNumCreatures = 1;  // for the queen.
  }
  curr->next = NewDirective(SEC_I8, 3, LAIR);
  curr = curr->next;
  curr->next = NewDirective(SEC_H8, 3, LAIR);
  curr = curr->next;
  curr->next = NewDirective(SEC_H8, 2, LAIR_ENTRANCE);
  curr = curr->next;
  curr->next = NewDirective(SEC_H9, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_H9, 1, OUTER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_H8, 1, MINE_EXIT);
}

void InitLairAlma() {
  CREATURE_DIRECTIVE *curr;
  giLairID = 3;
  // initialize the linked list of lairs
  lair = NewDirective(SEC_K13, 3, QUEEN_LAIR);
  curr = lair;
  if (!curr->pLevel->ubNumCreatures) {
    curr->pLevel->ubNumCreatures = 1;  // for the queen.
  }
  curr->next = NewDirective(SEC_J13, 3, LAIR);
  curr = curr->next;
  curr->next = NewDirective(SEC_J13, 2, LAIR_ENTRANCE);
  curr = curr->next;
  curr->next = NewDirective(SEC_J14, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_J14, 1, OUTER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_I14, 1, MINE_EXIT);
}

void InitLairGrumm() {
  CREATURE_DIRECTIVE *curr;
  giLairID = 4;
  // initialize the linked list of lairs
  lair = NewDirective(SEC_G4, 3, QUEEN_LAIR);
  curr = lair;
  if (!curr->pLevel->ubNumCreatures) {
    curr->pLevel->ubNumCreatures = 1;  // for the queen.
  }
  curr->next = NewDirective(SEC_H4, 3, LAIR);
  curr = curr->next;
  curr->next = NewDirective(SEC_H4, 2, LAIR_ENTRANCE);
  curr = curr->next;
  curr->next = NewDirective(SEC_H3, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_I3, 2, INNER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_I3, 1, OUTER_MINE);
  curr = curr->next;
  curr->next = NewDirective(SEC_H3, 1, MINE_EXIT);
}

#ifdef JA2BETAVERSION
extern BOOLEAN gfExitViewer;
#endif

void InitCreatureQuest() {
  UNDERGROUND_SECTORINFO *curr;
  BOOLEAN fPlayMeanwhile = FALSE;
  INT32 i = -1;
  INT32 iChosenMine;
  INT32 iRandom;
  INT32 iNumMinesInfectible;
#ifdef JA2BETAVERSION
  INT32 iOrigRandom;
#endif
  BOOLEAN fMineInfectible[4];

  if (giLairID) {
    return;  // already active!
  }

#ifdef JA2BETAVERSION
  if (guiCurrentScreen != AIVIEWER_SCREEN) {
    fPlayMeanwhile = TRUE;
  }
#else
  fPlayMeanwhile = TRUE;
#endif

  if (fPlayMeanwhile && !gfCreatureMeanwhileScenePlayed) {
    // Start the meanwhile scene for the queen ordering the release of the creatures.
    HandleCreatureRelease();
    gfCreatureMeanwhileScenePlayed = TRUE;
  }

  giHabitatedDistance = 0;
  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      giPopulationModifier = EASY_POPULATION_MODIFIER;
      break;
    case DIF_LEVEL_MEDIUM:
      giPopulationModifier = NORMAL_POPULATION_MODIFIER;
      break;
    case DIF_LEVEL_HARD:
      giPopulationModifier = HARD_POPULATION_MODIFIER;
      break;
  }

  // Determine which of the four maps are infectible by creatures.  Infectible mines
  // are those that are player controlled and unlimited.  We don't want the creatures to
  // infect the mine that runs out.

  // Default them all to infectible
  memset(fMineInfectible, 1, sizeof(BOOLEAN) * 4);

  if (gMineStatus[DRASSEN_MINE].fAttackedHeadMiner ||
      gMineStatus[DRASSEN_MINE].uiOreRunningOutPoint ||
      IsSectorEnemyControlled(SectorID8_X(SEC_D13), SectorID8_Y(SEC_D13))) {
    // If head miner was attacked, ore will/has run out, or enemy
    // controlled
    fMineInfectible[0] = FALSE;
  }
  if (gMineStatus[CAMBRIA_MINE].fAttackedHeadMiner ||
      gMineStatus[CAMBRIA_MINE].uiOreRunningOutPoint ||
      IsSectorEnemyControlled(SectorID8_X(SEC_H8), SectorID8_Y(SEC_H8))) {
    // If head miner was attacked, ore will/has run out, or enemy
    // controlled
    fMineInfectible[1] = FALSE;
  }
  if (gMineStatus[ALMA_MINE].fAttackedHeadMiner || gMineStatus[ALMA_MINE].uiOreRunningOutPoint ||
      IsSectorEnemyControlled(SectorID8_X(SEC_I14), SectorID8_Y(SEC_I14))) {
    // If head miner was attacked, ore will/has run out, or enemy
    // controlled
    fMineInfectible[2] = FALSE;
  }
  if (gMineStatus[GRUMM_MINE].fAttackedHeadMiner || gMineStatus[GRUMM_MINE].uiOreRunningOutPoint ||
      IsSectorEnemyControlled(SectorID8_X(SEC_H3), SectorID8_Y(SEC_H3))) {
    // If head miner was attacked, ore will/has run out, or enemy
    // controlled
    fMineInfectible[3] = FALSE;
  }

#ifdef JA2BETAVERSION
  if (guiCurrentScreen == AIVIEWER_SCREEN) {  // If in the AIViewer, allow any mine to get infected
    memset(fMineInfectible, 1, sizeof(BOOLEAN) * 4);
  }
#endif

  iNumMinesInfectible =
      fMineInfectible[0] + fMineInfectible[1] + fMineInfectible[2] + fMineInfectible[3];

  if (!iNumMinesInfectible) {
    return;
  }

  // Choose one of the infectible mines randomly
  iRandom = Random(iNumMinesInfectible) + 1;

#ifdef JA2BETAVERSION
  iOrigRandom = iRandom;
#endif

  iChosenMine = 0;

  for (i = 0; i < 4; i++) {
    if (iRandom) {
      iChosenMine++;
      if (fMineInfectible[i]) {
        iRandom--;
      }
    }
  }

  // Now, choose a start location for the queen.
  switch (iChosenMine) {
    case 1:  // Drassen
      InitLairDrassen();
      curr = FindUnderGroundSector(13, 5, 1);
      curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
      break;
    case 2:  // Cambria
      InitLairCambria();
      curr = FindUnderGroundSector(9, 8, 1);
      curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;  // entrance
      break;
    case 3:  // Alma's mine
      InitLairAlma();
      curr = FindUnderGroundSector(14, 10, 1);
      curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
      break;
    case 4:  // Grumm's mine
      InitLairGrumm();
      curr = FindUnderGroundSector(4, 8, 2);
      curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
      break;
    default:
#ifdef JA2BETAVERSION
    {
      CHAR16 str[512];
      swprintf(str, ARR_SIZE(str),
               L"Creature quest never chose a lair and won't infect any mines.  Infectible mines = "
               L"%d, iRandom = %d.  "
               L"This isn't a bug if you are not receiving income from any mines.",
               iNumMinesInfectible, iOrigRandom);
      DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
    }
#endif
      return;
  }

  // Now determine how often we will spread the creatures.
  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      i = EASY_QUEEN_INIT_BONUS_SPREADS;
      AddPeriodStrategicEvent(EVENT_CREATURE_SPREAD, EASY_SPREAD_TIME_IN_MINUTES, 0);
      break;
    case DIF_LEVEL_MEDIUM:
      i = NORMAL_QUEEN_INIT_BONUS_SPREADS;
      AddPeriodStrategicEvent(EVENT_CREATURE_SPREAD, NORMAL_SPREAD_TIME_IN_MINUTES, 0);
      break;
    case DIF_LEVEL_HARD:
      i = HARD_QUEEN_INIT_BONUS_SPREADS;
      AddPeriodStrategicEvent(EVENT_CREATURE_SPREAD, HARD_SPREAD_TIME_IN_MINUTES, 0);
      break;
  }

  // Set things up so that the creatures can plan attacks on helpless miners and civilians while
  // they are sleeping.  They do their planning at 10PM every day, and decide to attack sometime
  // during the night.
  AddEveryDayStrategicEvent(EVENT_CREATURE_NIGHT_PLANNING, 1320, 0);

  // Got to give the queen some early protection, so do some creature spreading.
  while (i--) {  // # times spread is based on difficulty, and the values in the defines.
    SpreadCreatures();
  }
}

void AddCreatureToNode(CREATURE_DIRECTIVE *node) {
  node->pLevel->ubNumCreatures++;

  if (node->pLevel->uiFlags &
      SF_PENDING_ALTERNATE_MAP) {  // there is an alternate map meaning that there is a dynamic
                                   // opening.  From now on
    // we substitute this map.
    node->pLevel->uiFlags &= ~SF_PENDING_ALTERNATE_MAP;
    node->pLevel->uiFlags |= SF_USE_ALTERNATE_MAP;
  }
}

BOOLEAN PlaceNewCreature(CREATURE_DIRECTIVE *node, INT32 iDistance) {
  if (!node) return FALSE;
  // check to see if the creatures are permitted to spread into certain areas.  There are 4 mines
  // (human perspective), and creatures won't spread to them until the player controls them.
  // Additionally, if the player has recently cleared the mine, then temporarily prevent the
  // spreading of creatures.

  if (giHabitatedDistance == iDistance) {  // FRONT-LINE CONDITIONS -- consider expansion or
                                           // frontline fortification.  The formulae used
    // in this sector are geared towards outer expansion.
    // we have reached the distance limitation for the spreading.  We will determine if
    // the area is populated enough to spread further.  The minimum population must be 4 before
    // spreading is even considered.
    if (node->pLevel->ubNumCreatures * 10 - 10 <= (INT32)Random(60)) {  // x<=1   100%
      // x==2		 83%
      // x==3		 67%
      // x==4		 50%
      // x==5		 33%
      // x==6		 17%
      // x>=7		  0%
      AddCreatureToNode(node);
      return TRUE;
    }
  } else if (giHabitatedDistance > iDistance) {  // we are within the "safe" habitated area of the
                                                 // creature's area of influence.  The chance of
    // increasing the population inside this sector depends on how deep we are within the sector.
    if (node->pLevel->ubNumCreatures < MAX_STRATEGIC_TEAM_SIZE ||
        (node->pLevel->ubNumCreatures < 32 && node->pLevel->ubCreatureHabitat == QUEEN_LAIR)) {
      // there is ALWAYS a chance to habitate an interior sector, though
      // the chances are slim for
      // highly occupied sectors.  This chance is modified by the type of area we are in.
      INT32 iAbsoluteMaxPopulation;
      INT32 iMaxPopulation = -1;
      INT32 iChanceToPopulate;
      switch (node->pLevel->ubCreatureHabitat) {
        case QUEEN_LAIR:  // Defend the queen bonus
          iAbsoluteMaxPopulation = 32;
          break;
        case LAIR:  // Smaller defend the queen bonus
          iAbsoluteMaxPopulation = 18;
          break;
        case LAIR_ENTRANCE:  // Smallest defend the queen bonus
          iAbsoluteMaxPopulation = 15;
          break;
        case INNER_MINE:  // neg bonus -- actually promotes expansion over population, and decrease
                          // max pop here.
          iAbsoluteMaxPopulation = 12;
          break;
        case OUTER_MINE:  // neg bonus -- actually promotes expansion over population, and decrease
                          // max pop here.
          iAbsoluteMaxPopulation = 10;
          break;
        case FEEDING_GROUNDS:  // get free food bonus!  yummy humans :)
          iAbsoluteMaxPopulation = 15;
          break;
        case MINE_EXIT:  // close access to humans (don't want to overwhelm them)
          iAbsoluteMaxPopulation = 10;
          break;
        default:
          Assert(0);
          return FALSE;
      }

      switch (gGameOptions.ubDifficultyLevel) {
        case DIF_LEVEL_EASY:            // 50%
          iAbsoluteMaxPopulation /= 2;  // Half
          break;
        case DIF_LEVEL_MEDIUM:  // 80%
          iAbsoluteMaxPopulation = iAbsoluteMaxPopulation * 4 / 5;
          break;
        case DIF_LEVEL_HARD:  // 100%
          break;
      }

      // Calculate the desired max population percentage based purely on current distant to creature
      // range. The closer we are to the lair, the closer this value will be to 100.
      iMaxPopulation = 100 - iDistance * 100 / giHabitatedDistance;
      iMaxPopulation = max(iMaxPopulation, 25);
      // Now, convert the previous value into a numeric population.
      iMaxPopulation = iAbsoluteMaxPopulation * iMaxPopulation / 100;
      iMaxPopulation = max(iMaxPopulation, 4);

      // The chance to populate a sector is higher for lower populations.  This is calculated on
      // the ratio of current population to the max population.
      iChanceToPopulate = 100 - node->pLevel->ubNumCreatures * 100 / iMaxPopulation;

      if (!node->pLevel->ubNumCreatures || (iChanceToPopulate > (INT32)Random(100) &&
                                            iMaxPopulation > node->pLevel->ubNumCreatures)) {
        AddCreatureToNode(node);
        return TRUE;
      }
    }
  } else {  // we are in a new area, so we will populate it
    AddCreatureToNode(node);
    giHabitatedDistance++;
    return TRUE;
  }
  if (PlaceNewCreature(node->next, iDistance + 1)) return TRUE;
  return FALSE;
}

void SpreadCreatures() {
  UINT16 usNewCreatures = 0;

  if (giLairID == -1) {
    DecayCreatures();
    return;
  }

  // queen just produced a litter of creature larvae.  Let's do some spreading now.
  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      usNewCreatures =
          (UINT16)(EASY_QUEEN_REPRODUCTION_BASE + Random(1 + EASY_QUEEN_REPRODUCTION_BONUS));
      break;
    case DIF_LEVEL_MEDIUM:
      usNewCreatures =
          (UINT16)(NORMAL_QUEEN_REPRODUCTION_BASE + Random(1 + NORMAL_QUEEN_REPRODUCTION_BONUS));
      break;
    case DIF_LEVEL_HARD:
      usNewCreatures =
          (UINT16)(HARD_QUEEN_REPRODUCTION_BASE + Random(1 + HARD_QUEEN_REPRODUCTION_BONUS));
      break;
  }

  while (usNewCreatures--) {
    // Note, this function can and will fail if the population gets dense.  This is a necessary
    // feature.  Otherwise, the queen would fill all the cave levels with MAX_STRATEGIC_TEAM_SIZE
    // monsters, and that would be bad.
    PlaceNewCreature(lair, 0);
  }
}

void DecayCreatures() {  // when the queen dies, we need to kill off the creatures over a period of
                         // time.
}

void AddCreaturesToBattle(UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales, UINT8 ubNumAdultMales,
                          UINT8 ubNumAdultFemales) {
  INT32 iRandom;
  struct SOLDIERTYPE *pSoldier;
  MAPEDGEPOINTINFO MapEdgepointInfo;
  UINT8 bDesiredDirection = 0;
  UINT8 ubCurrSlot = 0;

  switch (gsCreatureInsertionCode) {
    case INSERTION_CODE_NORTH:
      bDesiredDirection = SOUTHEAST;
      break;
    case INSERTION_CODE_EAST:
      bDesiredDirection = SOUTHWEST;
      break;
    case INSERTION_CODE_SOUTH:
      bDesiredDirection = NORTHWEST;
      break;
    case INSERTION_CODE_WEST:
      bDesiredDirection = NORTHEAST;
      break;
    case INSERTION_CODE_GRIDNO:
      break;
    default:
      AssertMsg(0, "Illegal direction passed to AddCreaturesToBattle()");
      break;
  }

#ifdef JA2TESTVERSION
  ScreenMsg(FONT_RED, MSG_INTERFACE, L"Creature attackers have arrived!");
#endif

  if (gsCreatureInsertionCode != INSERTION_CODE_GRIDNO) {
    ChooseMapEdgepoints(
        &MapEdgepointInfo, (UINT8)gsCreatureInsertionCode,
        (UINT8)(ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales + ubNumAdultFemales));
    ubCurrSlot = 0;
  }
  while (ubNumYoungMales || ubNumYoungFemales || ubNumAdultMales || ubNumAdultFemales) {
    iRandom =
        (INT32)Random(ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales + ubNumAdultFemales);
    if (ubNumYoungMales && iRandom < (INT32)ubNumYoungMales) {
      ubNumYoungMales--;
      pSoldier = TacticalCreateCreature(YAM_MONSTER);
    } else if (ubNumYoungFemales && iRandom < (INT32)(ubNumYoungMales + ubNumYoungFemales)) {
      ubNumYoungFemales--;
      pSoldier = TacticalCreateCreature(YAF_MONSTER);
    } else if (ubNumAdultMales &&
               iRandom < (INT32)(ubNumYoungMales + ubNumYoungFemales + ubNumAdultMales)) {
      ubNumAdultMales--;
      pSoldier = TacticalCreateCreature(AM_MONSTER);
    } else if (ubNumAdultFemales && iRandom < (INT32)(ubNumYoungMales + ubNumYoungFemales +
                                                      ubNumAdultMales + ubNumAdultFemales)) {
      ubNumAdultFemales--;
      pSoldier = TacticalCreateCreature(ADULTFEMALEMONSTER);
    } else {
      gsCreatureInsertionCode = 0;
      gsCreatureInsertionGridNo = 0;
      gubNumCreaturesAttackingTown = 0;
      gubYoungMalesAttackingTown = 0;
      gubYoungFemalesAttackingTown = 0;
      gubAdultMalesAttackingTown = 0;
      gubAdultFemalesAttackingTown = 0;
      gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
      gubSectorIDOfCreatureAttack = 0;
      AllTeamsLookForAll(FALSE);

      Assert(0);
      return;
    }
    pSoldier->ubInsertionDirection = bDesiredDirection;
    // Setup the position
    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
    pSoldier->bHunting = TRUE;
    if (gsCreatureInsertionCode != INSERTION_CODE_GRIDNO) {
      if (ubCurrSlot < MapEdgepointInfo.ubNumPoints) {  // using an edgepoint
        pSoldier->usStrategicInsertionData = MapEdgepointInfo.sGridNo[ubCurrSlot++];
      } else {  // no edgepoints left, so put him at the entrypoint.
        pSoldier->ubStrategicInsertionCode = (UINT8)gsCreatureInsertionCode;
      }
    } else {
      pSoldier->usStrategicInsertionData = gsCreatureInsertionGridNo;
    }
    UpdateMercInSector(pSoldier, (u8)gWorldSectorX, (u8)gWorldSectorY, 0);
  }
  gsCreatureInsertionCode = 0;
  gsCreatureInsertionGridNo = 0;
  gubNumCreaturesAttackingTown = 0;
  gubYoungMalesAttackingTown = 0;
  gubYoungFemalesAttackingTown = 0;
  gubAdultMalesAttackingTown = 0;
  gubAdultFemalesAttackingTown = 0;
  gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
  gubSectorIDOfCreatureAttack = 0;
  AllTeamsLookForAll(FALSE);
}

void ChooseTownSectorToAttack(UINT8 ubSectorID, BOOLEAN fOverrideTest) {
  INT32 iRandom;

  if (!fOverrideTest) {
    iRandom = PreRandom(100);
    switch (ubSectorID) {
      case SEC_D13:  // DRASSEN
        if (iRandom < 45)
          ubSectorID = SEC_D13;
        else if (iRandom < 70)
          ubSectorID = SEC_C13;
        else
          ubSectorID = SEC_B13;
        break;
      case SEC_H3:  // GRUMM
        if (iRandom < 35)
          ubSectorID = SEC_H3;
        else if (iRandom < 55)
          ubSectorID = SEC_H2;
        else if (iRandom < 70)
          ubSectorID = SEC_G2;
        else if (iRandom < 85)
          ubSectorID = SEC_H1;
        else
          ubSectorID = SEC_G1;
        break;
      case SEC_H8:  // CAMBRIA
        if (iRandom < 35)
          ubSectorID = SEC_H8;
        else if (iRandom < 55)
          ubSectorID = SEC_G8;
        else if (iRandom < 70)
          ubSectorID = SEC_F8;
        else if (iRandom < 85)
          ubSectorID = SEC_G9;
        else
          ubSectorID = SEC_F9;
        break;
      case SEC_I14:  // ALMA
        if (iRandom < 45)
          ubSectorID = SEC_I14;
        else if (iRandom < 65)
          ubSectorID = SEC_I13;
        else if (iRandom < 85)
          ubSectorID = SEC_H14;
        else
          ubSectorID = SEC_H13;
        break;
      default:
        Assert(0);
        return;
    }
  }
  switch (ubSectorID) {
    case SEC_D13:  // DRASSEN
      gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
      gsCreatureInsertionGridNo = 20703;
      break;
    case SEC_C13:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_B13:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_H3:  // GRUMM
      gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
      gsCreatureInsertionGridNo = 10303;
      break;
    case SEC_H2:
      gsCreatureInsertionCode = INSERTION_CODE_EAST;
      break;
    case SEC_G2:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_H1:
      gsCreatureInsertionCode = INSERTION_CODE_EAST;
      break;
    case SEC_G1:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_H8:  // CAMBRIA
      gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
      gsCreatureInsertionGridNo = 13005;
      break;
    case SEC_G8:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_F8:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_G9:
      gsCreatureInsertionCode = INSERTION_CODE_WEST;
      break;
    case SEC_F9:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_I14:  // ALMA
      gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
      gsCreatureInsertionGridNo = 9726;
      break;
    case SEC_I13:
      gsCreatureInsertionCode = INSERTION_CODE_EAST;
      break;
    case SEC_H14:
      gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SEC_H13:
      gsCreatureInsertionCode = INSERTION_CODE_EAST;
      break;
    default:
      return;
  }
  gubSectorIDOfCreatureAttack = ubSectorID;
}

void CreatureAttackTown(
    UINT8 ubSectorID,
    BOOLEAN fOverrideTest) {  // This is the launching point of the creature attack.
  UNDERGROUND_SECTORINFO *pSector;
  UINT8 ubSectorX, ubSectorY;

  if (gfWorldLoaded &&
      gTacticalStatus.fEnemyInSector) {  // Battle currently in progress, repost the event
    AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + Random(10), ubSectorID);
    return;
  }

  gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;

  ubSectorX = (UINT8)((ubSectorID % 16) + 1);
  ubSectorY = (UINT8)((ubSectorID / 16) + 1);

  if (!fOverrideTest) {
    // Record the number of creatures in the sector.
    pSector = FindUnderGroundSector(ubSectorX, ubSectorY, 1);
    if (!pSector) {
      CreatureAttackTown(ubSectorID, TRUE);
      return;
    }
    gubNumCreaturesAttackingTown = pSector->ubNumCreatures;
    if (!gubNumCreaturesAttackingTown) {
      CreatureAttackTown(ubSectorID, TRUE);
      return;
    }

    pSector->ubNumCreatures = 0;

    // Choose one of the town sectors to attack.  Sectors closer to
    // the mine entrance have a greater chance of being chosen.
    ChooseTownSectorToAttack(ubSectorID, FALSE);
    ubSectorX = (UINT8)((gubSectorIDOfCreatureAttack % 16) + 1);
    ubSectorY = (UINT8)((gubSectorIDOfCreatureAttack / 16) + 1);
  } else {
    ChooseTownSectorToAttack(ubSectorID, TRUE);
    gubNumCreaturesAttackingTown = 5;
  }

  // Now that the sector has been chosen, attack it!
  if (PlayerGroupsInSector(ubSectorX, ubSectorY, 0)) {  // we have players in the sector
    if (ubSectorX == gWorldSectorX && ubSectorY == gWorldSectorY &&
        !gbWorldSectorZ) {  // This is the currently loaded sector.  All we have to do is change the
                            // music and insert
      // the creatures tactically.
      if (IsTacticalMode()) {
        gubCreatureBattleCode = CREATURE_BATTLE_CODE_TACTICALLYADD;
      } else {
        gubCreatureBattleCode = CREATURE_BATTLE_CODE_PREBATTLEINTERFACE;
      }
    } else {
      gubCreatureBattleCode = CREATURE_BATTLE_CODE_PREBATTLEINTERFACE;
    }
  } else if (CountAllMilitiaInSector(ubSectorX, ubSectorY)) {  // we have militia in the sector
    gubCreatureBattleCode = CREATURE_BATTLE_CODE_AUTORESOLVE;
  } else if (!IsSectorEnemyControlled(ubSectorX, ubSectorY)) {
    // player controlled sector -- eat some civilians
    AdjustLoyaltyForCivsEatenByMonsters(ubSectorX, ubSectorY, gubNumCreaturesAttackingTown);
    SectorInfo[ubSectorID].ubDayOfLastCreatureAttack = (UINT8)GetWorldDay();
    return;
  } else {  // enemy controlled sectors don't get attacked.
    return;
  }

  SectorInfo[ubSectorID].ubDayOfLastCreatureAttack = (UINT8)GetWorldDay();
  switch (gubCreatureBattleCode) {
    case CREATURE_BATTLE_CODE_PREBATTLEINTERFACE:
      InitPreBattleInterface(NULL, TRUE);
      break;
    case CREATURE_BATTLE_CODE_AUTORESOLVE:
      gfAutomaticallyStartAutoResolve = TRUE;
      InitPreBattleInterface(NULL, TRUE);
      break;
    case CREATURE_BATTLE_CODE_TACTICALLYADD:
      PrepareCreaturesForBattle();
      break;
  }
  InterruptTime();
  PauseGame();
  LockPauseState(2);
}

// Called by campaign init.
void ChooseCreatureQuestStartDay() {
  //	INT32 iRandom, iDay;
  if (!gGameOptions.fSciFi) return;  // only available in science fiction mode.
  // Post the event.  Once it becomes due, it will setup the queen monster's location, and
  // begin spreading and attacking towns from there.
  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      // AddPeriodStrategicEvent( EVENT_BEGIN_CREATURE_QUEST, (EASY_QUEEN_START_DAY + Random( 1 +
      // EASY_QUEEN_START_BONUS )) * 1440 , 0 );
      break;
    case DIF_LEVEL_MEDIUM:
      // AddPeriodStrategicEvent( EVENT_BEGIN_CREATURE_QUEST, (NORMAL_QUEEN_START_DAY + Random( 1 +
      // NORMAL_QUEEN_START_BONUS )) * 1440, 0 );
      break;
    case DIF_LEVEL_HARD:
      // AddPeriodStrategicEvent( EVENT_BEGIN_CREATURE_QUEST, (HARD_QUEEN_START_DAY + Random( 1 +
      // HARD_QUEEN_START_BONUS )) * 1440, 0 );
      break;
  }
}

void DeleteDirectiveNode(CREATURE_DIRECTIVE **node) {
  if ((*node)->next) DeleteDirectiveNode(&((*node)->next));
  MemFree(*node);
  *node = NULL;
}

// Recursively delete all nodes (from the top down).
void DeleteCreatureDirectives() {
  if (lair) DeleteDirectiveNode(&lair);
  giLairID = 0;
}

void ClearCreatureQuest() {
  // This will remove all of the underground sector information and reinitialize it.
  // The only part that doesn't get added are the queen's lair.
  BuildUndergroundSectorInfoList();
  DeleteAllStrategicEventsOfType(EVENT_BEGIN_CREATURE_QUEST);
  DeleteCreatureDirectives();
}

void EndCreatureQuest() {
  CREATURE_DIRECTIVE *curr;
  UNDERGROUND_SECTORINFO *pSector;
  INT32 i;

  // By setting the lairID to -1, when it comes time to spread creatures,
  // They will get subtracted instead.
  giDestroyedLairID = giLairID;
  giLairID = -1;

  // Also nuke all of the creatures in all of the other mine sectors.  This
  // is keyed on the fact that the queen monster is killed.
  curr = lair;
  if (curr) {  // skip first node (there could be other creatures around.
    curr = curr->next;
  }
  while (curr) {
    curr->pLevel->ubNumCreatures = 0;
    curr = curr->next;
  }

  // Remove the creatures that are trapped underneath Tixa
  pSector = FindUnderGroundSector(9, 10, 2);
  if (pSector) {
    pSector->ubNumCreatures = 0;
  }

  // Also find and nuke all creatures on any surface levels!!!
  // KM: Sept 3, 1999 patch
  for (i = 0; i < 255; i++) {
    SectorInfo[i].ubNumCreatures = 0;
    SectorInfo[i].ubCreaturesInBattle = 0;
  }
}

UINT8 CreaturesInUndergroundSector(UINT8 ubSectorID, UINT8 ubSectorZ) {
  UNDERGROUND_SECTORINFO *pSector;
  UINT8 ubSectorX, ubSectorY;
  ubSectorX = SectorID8_X(ubSectorID);
  ubSectorY = SectorID8_Y(ubSectorID);
  pSector = FindUnderGroundSector(ubSectorX, ubSectorY, ubSectorZ);
  if (pSector) return pSector->ubNumCreatures;
  return 0;
}

BOOLEAN MineClearOfMonsters(UINT8 ubMineIndex) {
  Assert((ubMineIndex >= 0) && (ubMineIndex < MAX_NUMBER_OF_MINES));

  if (!gMineStatus[ubMineIndex].fPrevInvadedByMonsters) {
    switch (ubMineIndex) {
      case MINE_GRUMM:
        if (CreaturesInUndergroundSector(SEC_H3, 1)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_I3, 1)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_I3, 2)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_H3, 2)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_H4, 2)) return FALSE;
        break;
      case MINE_CAMBRIA:
        if (CreaturesInUndergroundSector(SEC_H8, 1)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_H9, 1)) return FALSE;
        break;
      case MINE_ALMA:
        if (CreaturesInUndergroundSector(SEC_I14, 1)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_J14, 1)) return FALSE;
        break;
      case MINE_DRASSEN:
        if (CreaturesInUndergroundSector(SEC_D13, 1)) return FALSE;
        if (CreaturesInUndergroundSector(SEC_E13, 1)) return FALSE;
        break;
      case MINE_CHITZENA:
      case MINE_SAN_MONA:
        // these are never attacked
        break;

      default:
#ifdef JA2BETAVERSION
        ScreenMsg(FONT_RED, MSG_ERROR,
                  L"Attempting to check if mine is clear but mine index is invalid (%d).",
                  ubMineIndex);
#endif
        break;
    }
  } else {  // mine was previously invaded by creatures.  Don't allow mine production until queen is
            // dead.
    if (giLairID != -1) {
      return FALSE;
    }
  }
  return TRUE;
}

void DetermineCreatureTownComposition(UINT8 ubNumCreatures, UINT8 *pubNumYoungMales,
                                      UINT8 *pubNumYoungFemales, UINT8 *pubNumAdultMales,
                                      UINT8 *pubNumAdultFemales) {
  INT32 i, iRandom;
  UINT8 ubYoungMalePercentage = 10;
  UINT8 ubYoungFemalePercentage = 65;
  UINT8 ubAdultMalePercentage = 5;
  UINT8 ubAdultFemalePercentage = 20;

  // First step is to convert the percentages into the numbers we will use.
  ubYoungFemalePercentage += ubYoungMalePercentage;
  ubAdultMalePercentage += ubYoungFemalePercentage;
  ubAdultFemalePercentage += ubAdultMalePercentage;
  if (ubAdultFemalePercentage != 100) {
    AssertMsg(0, "Percentage for adding creatures don't add up to 100.");
  }
  // Second step is to determine the breakdown of the creatures randomly.
  i = ubNumCreatures;
  while (i--) {
    iRandom = Random(100);
    if (iRandom < ubYoungMalePercentage)
      (*pubNumYoungMales)++;
    else if (iRandom < ubYoungFemalePercentage)
      (*pubNumYoungFemales)++;
    else if (iRandom < ubAdultMalePercentage)
      (*pubNumAdultMales)++;
    else
      (*pubNumAdultFemales)++;
  }
}

void DetermineCreatureTownCompositionBasedOnTacticalInformation(UINT8 *pubNumCreatures,
                                                                UINT8 *pubNumYoungMales,
                                                                UINT8 *pubNumYoungFemales,
                                                                UINT8 *pubNumAdultMales,
                                                                UINT8 *pubNumAdultFemales) {
  SECTORINFO *pSector;
  INT32 i;
  struct SOLDIERTYPE *pSoldier;

  pSector = &SectorInfo[GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)];
  *pubNumCreatures = 0;
  pSector->ubNumCreatures = 0;
  pSector->ubCreaturesInBattle = 0;
  for (i = gTacticalStatus.Team[CREATURE_TEAM].bFirstID;
       i <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; i++) {
    pSoldier = MercPtrs[i];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      switch (pSoldier->ubBodyType) {
        case ADULTFEMALEMONSTER:
          (*pubNumCreatures)++;
          (*pubNumAdultFemales)++;
          break;
        case AM_MONSTER:
          (*pubNumCreatures)++;
          (*pubNumAdultMales)++;
          break;
        case YAF_MONSTER:
          (*pubNumCreatures)++;
          (*pubNumYoungFemales)++;
          break;
        case YAM_MONSTER:
          (*pubNumCreatures)++;
          (*pubNumYoungMales)++;
          break;
      }
    }
  }
}

BOOLEAN PrepareCreaturesForBattle() {
  UNDERGROUND_SECTORINFO *pSector;
  INT32 i, iRandom;
  struct SGPPaletteEntry LColors[3];
  BOOLEAN fQueen;
  UINT8 ubLarvaePercentage;
  UINT8 ubInfantPercentage;
  UINT8 ubYoungMalePercentage;
  UINT8 ubYoungFemalePercentage;
  UINT8 ubAdultMalePercentage;
  UINT8 ubAdultFemalePercentage;
  UINT8 ubCreatureHabitat;
  UINT8 ubNumLarvae = 0;
  UINT8 ubNumInfants = 0;
  UINT8 ubNumYoungMales = 0;
  UINT8 ubNumYoungFemales = 0;
  UINT8 ubNumAdultMales = 0;
  UINT8 ubNumAdultFemales = 0;
  UINT8 ubNumCreatures;

  if (!gubCreatureBattleCode) {
    LightGetColors(LColors);
    // if( ubNumColors != 1 )
    //	ScreenMsg( FONT_RED, MSG_ERROR, L"This map has more than one light color -- KM, LC : 1" );

    // By default, we only play creature music in the cave levels (the creature levels all
    // consistently have blue lights while human occupied mines have red lights.  We always play
    // creature music when creatures are in the level.
    if (LColors->peBlue)
      gfUseCreatureMusic = TRUE;
    else
      gfUseCreatureMusic = FALSE;

    if (!gbWorldSectorZ) return FALSE;  // Creatures don't attack overworld with this battle code.
    pSector = FindUnderGroundSector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
    if (!pSector) {
      return FALSE;
    }
    if (!pSector->ubNumCreatures) {
      return FALSE;
    }
    gfUseCreatureMusic = TRUE;  // creatures are here, so play creature music
    ubCreatureHabitat = pSector->ubCreatureHabitat;
    ubNumCreatures = pSector->ubNumCreatures;
  } else {  // creatures are attacking a town sector
    gfUseCreatureMusic = TRUE;
    SetMusicMode(MUSIC_TACTICAL_NOTHING);
    ubCreatureHabitat = MINE_EXIT;
    ubNumCreatures = gubNumCreaturesAttackingTown;
  }

  switch (ubCreatureHabitat) {
    case QUEEN_LAIR:
      fQueen = TRUE;
      ubLarvaePercentage = 20;
      ubInfantPercentage = 40;
      ubYoungMalePercentage = 0;
      ubYoungFemalePercentage = 0;
      ubAdultMalePercentage = 30;
      ubAdultFemalePercentage = 10;
      break;
    case LAIR:
      fQueen = FALSE;
      ubLarvaePercentage = 15;
      ubInfantPercentage = 35;
      ubYoungMalePercentage = 10;
      ubYoungFemalePercentage = 5;
      ubAdultMalePercentage = 25;
      ubAdultFemalePercentage = 10;
      break;
    case LAIR_ENTRANCE:
      fQueen = FALSE;
      ubLarvaePercentage = 0;
      ubInfantPercentage = 15;
      ubYoungMalePercentage = 30;
      ubYoungFemalePercentage = 10;
      ubAdultMalePercentage = 35;
      ubAdultFemalePercentage = 10;
      break;
    case INNER_MINE:
      fQueen = FALSE;
      ubLarvaePercentage = 0;
      ubInfantPercentage = 0;
      ubYoungMalePercentage = 20;
      ubYoungFemalePercentage = 40;
      ubAdultMalePercentage = 10;
      ubAdultFemalePercentage = 30;
      break;
    case OUTER_MINE:
    case MINE_EXIT:
      fQueen = FALSE;
      ubLarvaePercentage = 0;
      ubInfantPercentage = 0;
      ubYoungMalePercentage = 10;
      ubYoungFemalePercentage = 65;
      ubAdultMalePercentage = 5;
      ubAdultFemalePercentage = 20;
      break;
    default:
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_RED, MSG_ERROR,
                L"Invalid creature habitat ID of %d for PrepareCreaturesForBattle.  Ignoring...",
                ubCreatureHabitat);
#endif
      return FALSE;
  }

  // First step is to convert the percentages into the numbers we will use.
  if (fQueen) {
    ubNumCreatures--;
  }
  ubInfantPercentage += ubLarvaePercentage;
  ubYoungMalePercentage += ubInfantPercentage;
  ubYoungFemalePercentage += ubYoungMalePercentage;
  ubAdultMalePercentage += ubYoungFemalePercentage;
  ubAdultFemalePercentage += ubAdultMalePercentage;
  if (ubAdultFemalePercentage != 100) {
    AssertMsg(0, "Percentage for adding creatures don't add up to 100.");
  }
  // Second step is to determine the breakdown of the creatures randomly.
  i = ubNumCreatures;
  while (i--) {
    iRandom = Random(100);
    if (iRandom < ubLarvaePercentage)
      ubNumLarvae++;
    else if (iRandom < ubInfantPercentage)
      ubNumInfants++;
    else if (iRandom < ubYoungMalePercentage)
      ubNumYoungMales++;
    else if (iRandom < ubYoungFemalePercentage)
      ubNumYoungFemales++;
    else if (iRandom < ubAdultMalePercentage)
      ubNumAdultMales++;
    else
      ubNumAdultFemales++;
  }

  if (gbWorldSectorZ) {
    UNDERGROUND_SECTORINFO *pUndergroundSector;
    pUndergroundSector =
        FindUnderGroundSector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
    if (!pUndergroundSector) {  // No info?!!!!!
      AssertMsg(0,
                "Please report underground sector you are in or going to and send save if "
                "possible.  KM : 0");
      return FALSE;
    }
    pUndergroundSector->ubCreaturesInBattle = pUndergroundSector->ubNumCreatures;
  } else {
    SECTORINFO *pSector;
    pSector = &SectorInfo[GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)];
    pSector->ubNumCreatures = ubNumCreatures;
    pSector->ubCreaturesInBattle = ubNumCreatures;
  }

  switch (gubCreatureBattleCode) {
    case CREATURE_BATTLE_CODE_NONE:  // in the mines
      AddSoldierInitListCreatures(fQueen, ubNumLarvae, ubNumInfants, ubNumYoungMales,
                                  ubNumYoungFemales, ubNumAdultMales, ubNumAdultFemales);
      break;
    case CREATURE_BATTLE_CODE_TACTICALLYADD:  // creature attacking a town sector
    case CREATURE_BATTLE_CODE_PREBATTLEINTERFACE:
      AddCreaturesToBattle(ubNumYoungMales, ubNumYoungFemales, ubNumAdultMales, ubNumAdultFemales);
      break;
    case CREATURE_BATTLE_CODE_AUTORESOLVE:
      return FALSE;
  }
  return TRUE;
}

void CreatureNightPlanning() {  // Check the populations of the mine exits, and factor a chance for
                                // them to attack at night.
  UINT8 ubNumCreatures;
  ubNumCreatures = CreaturesInUndergroundSector(SEC_H3, 1);
  if (ubNumCreatures > 1 &&
      ubNumCreatures * 10 >
          (INT32)PreRandom(100)) {  // 10% chance for each creature to decide it's time to attack.
    AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom(429), SEC_H3);
  }
  ubNumCreatures = CreaturesInUndergroundSector(SEC_D13, 1);
  if (ubNumCreatures > 1 &&
      ubNumCreatures * 10 >
          (INT32)PreRandom(100)) {  // 10% chance for each creature to decide it's time to attack.
    AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom(429), SEC_D13);
  }
  ubNumCreatures = CreaturesInUndergroundSector(SEC_I14, 1);
  if (ubNumCreatures > 1 &&
      ubNumCreatures * 10 >
          (INT32)PreRandom(100)) {  // 10% chance for each creature to decide it's time to attack.
    AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom(429), SEC_I14);
  }
  ubNumCreatures = CreaturesInUndergroundSector(SEC_H8, 1);
  if (ubNumCreatures > 1 &&
      ubNumCreatures * 10 >
          (INT32)PreRandom(100)) {  // 10% chance for each creature to decide it's time to attack.
    AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom(429), SEC_H8);
  }
}

void CheckConditionsForTriggeringCreatureQuest(u8 sSectorX, u8 sSectorY, INT8 bSectorZ) {
  UINT8 ubValidMines = 0;
  if (!gGameOptions.fSciFi) return;  // No scifi, no creatures...
  if (giLairID) return;              // Creature quest already begun

  // Count the number of "infectible mines" the player occupies
  if (!IsSectorEnemyControlled(SectorID8_X(SEC_D13), SectorID8_Y(SEC_D13))) {
    ubValidMines++;
  }
  if (!IsSectorEnemyControlled(SectorID8_X(SEC_H8), SectorID8_Y(SEC_H8))) {
    ubValidMines++;
  }
  if (!IsSectorEnemyControlled(SectorID8_X(SEC_I14), SectorID8_Y(SEC_I14))) {
    ubValidMines++;
  }
  if (!IsSectorEnemyControlled(SectorID8_X(SEC_H3), SectorID8_Y(SEC_H3))) {
    ubValidMines++;
  }

  if (ubValidMines >= 3) {
    InitCreatureQuest();
  }
}

BOOLEAN SaveCreatureDirectives(HWFILE hFile) {
  UINT32 uiNumBytesWritten;

  FileMan_Write(hFile, &giHabitatedDistance, 4, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(INT32)) {
    return (FALSE);
  }

  FileMan_Write(hFile, &giPopulationModifier, 4, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(INT32)) {
    return (FALSE);
  }
  FileMan_Write(hFile, &giLairID, 4, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(INT32)) {
    return (FALSE);
  }
  FileMan_Write(hFile, &gfUseCreatureMusic, 1, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) {
    return (FALSE);
  }
  FileMan_Write(hFile, &giDestroyedLairID, 4, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(INT32)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadCreatureDirectives(HWFILE hFile, UINT32 uiSavedGameVersion) {
  UINT32 uiNumBytesRead;
  FileMan_Read(hFile, &giHabitatedDistance, 4, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(INT32)) {
    return (FALSE);
  }

  FileMan_Read(hFile, &giPopulationModifier, 4, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(INT32)) {
    return (FALSE);
  }
  FileMan_Read(hFile, &giLairID, 4, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(INT32)) {
    return (FALSE);
  }

  FileMan_Read(hFile, &gfUseCreatureMusic, 1, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) {
    return (FALSE);
  }

  if (uiSavedGameVersion >= 82) {
    FileMan_Read(hFile, &giDestroyedLairID, 4, &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(INT32)) {
      return (FALSE);
    }
  } else {
    giDestroyedLairID = 0;
  }

#ifdef JA2BETAVERSION
  if (gfClearCreatureQuest && giLairID != -1) {
    giLairID = 0;
    gfCreatureMeanwhileScenePlayed = FALSE;
    uiMeanWhileFlags &= ~(0x00000800);
  }
  gfClearCreatureQuest = FALSE;
#endif

  switch (giLairID) {
    case -1:
      break;  // creature quest finished -- it's okay
    case 0:
      break;  // lair doesn't exist yet -- it's okay
    case 1:
      InitLairDrassen();
      break;
    case 2:
      InitLairCambria();
      break;
    case 3:
      InitLairAlma();
      break;
    case 4:
      InitLairGrumm();
      break;
    default:
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_RED, MSG_ERROR,
                L"Invalid restoration of creature lair ID of %d.  Save game potentially hosed.",
                giLairID);
#endif
      break;
  }

  return (TRUE);
}

void ForceCreaturesToAvoidMineTemporarily(UINT8 ubMineIndex) {
  gMineStatus[MINE_GRUMM].usValidDayCreaturesCanInfest = (UINT16)(GetWorldDay() + 2);
}

BOOLEAN PlayerGroupIsInACreatureInfestedMine() {
  CREATURE_DIRECTIVE *curr;
  struct SOLDIERTYPE *pSoldier;
  INT32 i;
  u8 sSectorX, sSectorY;
  INT8 bSectorZ;

  if (giLairID <= 0) {  // Creature quest inactive
    return FALSE;
  }

  // Lair is active, so look for live soldier in any creature level
  curr = lair;
  while (curr) {
    sSectorX = curr->pLevel->ubSectorX;
    sSectorY = curr->pLevel->ubSectorY;
    bSectorZ = (INT8)curr->pLevel->ubSectorZ;
    // Loop through all the creature directives (mine sectors that are infectible) and
    // see if players are there.
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {
      pSoldier = MercPtrs[i];
      if (IsSolActive(pSoldier) && pSoldier->bLife && GetSolSectorX(pSoldier) == sSectorX &&
          GetSolSectorY(pSoldier) == sSectorY && GetSolSectorZ(pSoldier) == bSectorZ &&
          !pSoldier->fBetweenSectors) {
        return TRUE;
      }
    }
    curr = curr->next;
  }

  // Lair is active, but no mercs are in these sectors
  return FALSE;
}

// Returns TRUE if valid and creature quest over, FALSE if creature quest active or not yet started
BOOLEAN GetWarpOutOfMineCodes(INT16 *psSectorX, INT16 *psSectorY, INT8 *pbSectorZ,
                              INT16 *psInsertionGridNo) {
  INT32 iSwitchValue;

  if (!gfWorldLoaded) {
    return FALSE;
  }

  if (gbWorldSectorZ == 0) {
    return (FALSE);
  }

  iSwitchValue = giLairID;

  if (iSwitchValue == -1) {
    iSwitchValue = giDestroyedLairID;
  }

  if (!iSwitchValue) {
    return FALSE;
  }

  // Now make sure the mercs are in the previously infested mine
  switch (iSwitchValue) {
    case 1:  // Drassen
      if ((gWorldSectorX == 13 && gWorldSectorY == 6 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 13 && gWorldSectorY == 7 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 13 && gWorldSectorY == 7 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 13 && gWorldSectorY == 6 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 13 && gWorldSectorY == 5 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 13 && gWorldSectorY == 5 && gbWorldSectorZ == 1) ||
          (gWorldSectorX == 13 && gWorldSectorY == 4 && gbWorldSectorZ == 1)) {
        *psSectorX = 13;
        *psSectorY = 4;
        *pbSectorZ = 0;
        *psInsertionGridNo = 20700;
        return TRUE;
      }
      break;
    case 3:  // Cambria
      if ((gWorldSectorX == 8 && gWorldSectorY == 9 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 8 && gWorldSectorY == 8 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 8 && gWorldSectorY == 8 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 9 && gWorldSectorY == 8 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 9 && gWorldSectorY == 8 && gbWorldSectorZ == 1) ||
          (gWorldSectorX == 8 && gWorldSectorY == 8 && gbWorldSectorZ == 1)) {
        *psSectorX = 8;
        *psSectorY = 8;
        *pbSectorZ = 0;
        *psInsertionGridNo = 13002;
        return TRUE;
      }
      break;
    case 2:  // Alma
      if ((gWorldSectorX == 13 && gWorldSectorY == 11 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 13 && gWorldSectorY == 10 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 13 && gWorldSectorY == 10 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 14 && gWorldSectorY == 10 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 14 && gWorldSectorY == 10 && gbWorldSectorZ == 1) ||
          (gWorldSectorX == 14 && gWorldSectorY == 9 && gbWorldSectorZ == 1)) {
        *psSectorX = 14;
        *psSectorY = 9;
        *pbSectorZ = 0;
        *psInsertionGridNo = 9085;
        return TRUE;
      }
      break;
    case 4:  // Grumm
      if ((gWorldSectorX == 4 && gWorldSectorY == 7 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 4 && gWorldSectorY == 8 && gbWorldSectorZ == 3) ||
          (gWorldSectorX == 3 && gWorldSectorY == 8 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 3 && gWorldSectorY == 8 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 3 && gWorldSectorY == 9 && gbWorldSectorZ == 2) ||
          (gWorldSectorX == 3 && gWorldSectorY == 9 && gbWorldSectorZ == 1) ||
          (gWorldSectorX == 3 && gWorldSectorY == 8 && gbWorldSectorZ == 1)) {
        *psSectorX = 3;
        *psSectorY = 8;
        *pbSectorZ = 0;
        *psInsertionGridNo = 9822;
        return TRUE;
      }
      break;
  }
  return (FALSE);
}
