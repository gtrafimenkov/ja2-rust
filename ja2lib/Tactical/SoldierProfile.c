// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/SoldierProfile.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameSettings.h"
#include "Laptop/AIM.h"
#include "Laptop/History.h"
#include "Laptop/Personnel.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/StrategicTownReputation.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/MapInformation.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TileEngine/Environment.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/WorldMan.h"
#include "Town.h"
#include "Utils/EventPump.h"
#include "Utils/TimerControl.h"
#include "rust_fileman.h"

#ifdef JA2EDITOR
extern BOOLEAN gfProfileDataLoaded;
#endif

BOOLEAN gfPotentialTeamChangeDuringDeath = FALSE;

#define MIN_BLINK_FREQ 3000
#define MIN_EXPRESSION_FREQ 2000

#define SET_PROFILE_GAINS2 500, 500, 500, 500, 500, 500, 500, 500, 500

MERCPROFILESTRUCT gMercProfiles[NUM_PROFILES];

int8_t gbSkillTraitBonus[NUM_SKILLTRAITS] = {
    0,   // NO_SKILLTRAIT
    25,  // LOCKPICKING
    15,  // HANDTOHAND
    15,  // ELECTRONICS
    15,  // NIGHTOPS
    12,  // THROWING
    15,  // TEACHING
    15,  // HEAVY_WEAPS
    0,   // AUTO_WEAPS
    15,  // STEALTHY
    0,   // AMBIDEXT
    0,   // THIEF				// UNUSED!
    30,  // MARTIALARTS
    30,  // KNIFING
    15,  // ONROOF
    0,   // CAMOUFLAGED
};

uint8_t gubBasicInventoryPositions[] = {HELMETPOS,   VESTPOS,     LEGPOS,      HANDPOS,
                                        BIGPOCK1POS, BIGPOCK2POS, BIGPOCK3POS, BIGPOCK4POS};

#define NUM_TERRORISTS 6

uint8_t gubTerrorists[NUM_TERRORISTS + 1] = {DRUGGIST, SLAY, ANNIE, CHRIS, TIFFANY, T_REX, 0};

uint8_t gubNumTerrorists = 0;

#define NUM_TERRORIST_POSSIBLE_LOCATIONS 5

int16_t gsTerroristSector[NUM_TERRORISTS][NUM_TERRORIST_POSSIBLE_LOCATIONS][2] = {
    // Elgin... preplaced
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    // Slay
    {{9, MAP_ROW_F}, {14, MAP_ROW_I}, {1, MAP_ROW_G}, {2, MAP_ROW_G}, {8, MAP_ROW_G}},
    // Matron
    {{14, MAP_ROW_I}, {6, MAP_ROW_C}, {2, MAP_ROW_B}, {11, MAP_ROW_L}, {8, MAP_ROW_G}},
    // Imposter
    {{1, MAP_ROW_G}, {9, MAP_ROW_F}, {11, MAP_ROW_L}, {8, MAP_ROW_G}, {2, MAP_ROW_G}},
    // Tiffany
    {{14, MAP_ROW_I}, {2, MAP_ROW_G}, {14, MAP_ROW_H}, {6, MAP_ROW_C}, {2, MAP_ROW_B}},
    // Rexall
    {{9, MAP_ROW_F}, {14, MAP_ROW_H}, {2, MAP_ROW_H}, {1, MAP_ROW_G}, {2, MAP_ROW_B}}};

int16_t gsRobotGridNo;

#define NUM_ASSASSINS 6

uint8_t gubAssassins[NUM_ASSASSINS] = {JIM, JACK, OLAF, RAY, OLGA, TYRONE};

#define NUM_ASSASSIN_POSSIBLE_TOWNS 5

int8_t gbAssassinTown[NUM_ASSASSINS][NUM_ASSASSIN_POSSIBLE_TOWNS] = {
    // Jim
    {CAMBRIA, DRASSEN, ALMA, BALIME, GRUMM},
    // Jack
    {CHITZENA, ESTONI, ALMA, BALIME, GRUMM},
    // Olaf
    {DRASSEN, ESTONI, ALMA, CAMBRIA, BALIME},
    // Ray
    {CAMBRIA, OMERTA, BALIME, GRUMM, DRASSEN},
    // Olga
    {CHITZENA, OMERTA, CAMBRIA, ALMA, GRUMM},
    // Tyrone
    {CAMBRIA, BALIME, ALMA, GRUMM, DRASSEN},
};

uint16_t CalcCompetence(MERCPROFILESTRUCT *pProfile);
int16_t CalcMedicalDeposit(MERCPROFILESTRUCT *pProfile);
extern void HandleEndDemoInCreatureLevel();
void DecideActiveTerrorists(void);

extern struct SOLDIERTYPE *gpSMCurrentMerc;
extern BOOLEAN gfRerenderInterfaceFromHelpText;

BOOLEAN LoadMercProfiles(void) {
  //	FILE *fptr;
  FileID fptr = FILE_ID_ERR;
  char *pFileName = "BINARYDATA\\Prof.dat";
  uint32_t uiLoop, uiLoop2, uiLoop3;
  uint16_t usItem, usNewGun, usAmmo, usNewAmmo;
  uint32_t uiNumBytesRead;

  fptr = File_OpenForReading(pFileName);
  if (!fptr) {
    DebugMsg(TOPIC_JA2, DBG_INFO, String("FAILED to LoadMercProfiles from file %s", pFileName));
    return (FALSE);
  }

  for (uiLoop = 0; uiLoop < NUM_PROFILES; uiLoop++) {
    if (JA2EncryptedFileRead(fptr, &gMercProfiles[uiLoop], sizeof(MERCPROFILESTRUCT),
                             &uiNumBytesRead) != 1) {
      DebugMsg(TOPIC_JA2, DBG_INFO,
               String("FAILED to Read Merc Profiles from File %d %s", uiLoop, pFileName));
      File_Close(fptr);
      return (FALSE);
    }

    // if the Dialogue exists for the merc, allow the merc to be hired
    if (DialogueDataFileExistsForProfile((uint8_t)uiLoop, 0, FALSE, NULL)) {
      gMercProfiles[uiLoop].bMercStatus = 0;
    } else
      gMercProfiles[uiLoop].bMercStatus = MERC_HAS_NO_TEXT_FILE;

    // if the merc has a medical deposit
    if (gMercProfiles[uiLoop].bMedicalDeposit) {
      gMercProfiles[uiLoop].sMedicalDepositAmount = CalcMedicalDeposit(&gMercProfiles[uiLoop]);
    } else
      gMercProfiles[uiLoop].sMedicalDepositAmount = 0;

    // ATE: New, face display indipendent of ID num now
    // Setup face index value
    // Default is the ubCharNum
    gMercProfiles[uiLoop].ubFaceIndex = (uint8_t)uiLoop;

    if (!gGameOptions.fGunNut) {
      // CJC: replace guns in profile if they aren't available
      for (uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++) {
        usItem = gMercProfiles[uiLoop].inv[uiLoop2];

        if ((Item[usItem].usItemClass & IC_GUN) && ExtendedGunListGun(usItem)) {
          usNewGun = StandardGunListReplacement(usItem);
          if (usNewGun != NOTHING) {
            gMercProfiles[uiLoop].inv[uiLoop2] = usNewGun;

            // must search through inventory and replace ammo accordingly
            for (uiLoop3 = 0; uiLoop3 < NUM_INV_SLOTS; uiLoop3++) {
              usAmmo = gMercProfiles[uiLoop].inv[uiLoop3];
              if ((Item[usAmmo].usItemClass & IC_AMMO)) {
                usNewAmmo = FindReplacementMagazineIfNecessary(usItem, usAmmo, usNewGun);
                if (usNewAmmo != NOTHING) {
                  // found a new magazine, replace...
                  gMercProfiles[uiLoop].inv[uiLoop3] = usNewAmmo;
                }
              }
            }
          }
        }
      }

    }  // end of if not gun nut

    // ATE: Calculate some inital attractiveness values for buddy's inital equipment...
    // Look for gun and armour
    gMercProfiles[uiLoop].bMainGunAttractiveness = -1;
    gMercProfiles[uiLoop].bArmourAttractiveness = -1;

    for (uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++) {
      usItem = gMercProfiles[uiLoop].inv[uiLoop2];

      if (usItem != NOTHING) {
        // Check if it's a gun
        if (Item[usItem].usItemClass & IC_GUN) {
          gMercProfiles[uiLoop].bMainGunAttractiveness = Weapon[usItem].ubDeadliness;
        }

        // If it's armour
        if (Item[usItem].usItemClass & IC_ARMOUR) {
          gMercProfiles[uiLoop].bArmourAttractiveness =
              Armour[Item[usItem].ubClassIndex].ubProtection;
        }
      }
    }

    // OK, if we are a created slot, this will get overriden at some time..

    // add up the items the merc has for the usOptionalGearCost
    gMercProfiles[uiLoop].usOptionalGearCost = 0;
    for (uiLoop2 = 0; uiLoop2 < NUM_INV_SLOTS; uiLoop2++) {
      if (gMercProfiles[uiLoop].inv[uiLoop2] != NOTHING) {
        // get the item
        usItem = gMercProfiles[uiLoop].inv[uiLoop2];

        // add the cost
        gMercProfiles[uiLoop].usOptionalGearCost += Item[usItem].usPrice;
      }
    }

    // These variables to get loaded in
    gMercProfiles[uiLoop].fUseProfileInsertionInfo = FALSE;
    gMercProfiles[uiLoop].sGridNo = 0;

    // ARM: this is also being done inside the profile editor, but put it here too, so this
    // project's code makes sense
    gMercProfiles[uiLoop].bHatedCount[0] = gMercProfiles[uiLoop].bHatedTime[0];
    gMercProfiles[uiLoop].bHatedCount[1] = gMercProfiles[uiLoop].bHatedTime[1];
    gMercProfiles[uiLoop].bLearnToHateCount = gMercProfiles[uiLoop].bLearnToHateTime;
    gMercProfiles[uiLoop].bLearnToLikeCount = gMercProfiles[uiLoop].bLearnToLikeTime;
  }

  // SET SOME DEFAULT LOCATIONS FOR STARTING NPCS

  File_Close(fptr);

  // decide which terrorists are active
  DecideActiveTerrorists();

  // initialize mercs' status
  StartSomeMercsOnAssignment();

  // initial recruitable mercs' reputation in each town
  InitializeProfilesForTownReputation();

#ifdef JA2EDITOR
  gfProfileDataLoaded = TRUE;
#endif

  // no better place..heh?.. will load faces for profiles that are 'extern'.....won't have
  // soldiertype instances
  InitalizeStaticExternalNPCFaces();

  // car portrait values
  LoadCarPortraitValues();

  return (TRUE);
}

#define MAX_ADDITIONAL_TERRORISTS 4

void DecideActiveTerrorists(void) {
  uint8_t ubLoop, ubLoop2;
  uint8_t ubTerrorist;
  uint8_t ubNumAdditionalTerrorists, ubNumTerroristsAdded = 0;
  uint32_t uiChance, uiLocationChoice;
  BOOLEAN fFoundSpot;
  int16_t sTerroristPlacement[MAX_ADDITIONAL_TERRORISTS][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};

  // one terrorist will always be Elgin
  // determine how many more terrorists - 2 to 4 more

  // using this stochastic process(!), the chances for terrorists are:
  // EASY:		3, 9%			4, 42%		5, 49%
  // MEDIUM:	3, 25%		4, 50%		5, 25%
  // HARD:		3, 49%		4, 42%		5, 9%
  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      uiChance = 70;
      break;
    case DIF_LEVEL_HARD:
      uiChance = 30;
      break;
    default:
      uiChance = 50;
      break;
  }
  // add at least 2 more
  ubNumAdditionalTerrorists = 2;
  for (ubLoop = 0; ubLoop < (MAX_ADDITIONAL_TERRORISTS - 2); ubLoop++) {
    if (Random(100) < uiChance) {
      ubNumAdditionalTerrorists++;
    }
  }

// ifdefs added by CJC
#ifdef JA2TESTVERSION
  ubNumAdditionalTerrorists = 4;
#endif

  while (ubNumTerroristsAdded < ubNumAdditionalTerrorists) {
    ubLoop = 1;  // start at beginning of array (well, after Elgin)

    // NB terrorist ID of 0 indicates end of array
    while (ubNumTerroristsAdded < ubNumAdditionalTerrorists && gubTerrorists[ubLoop] != 0) {
      ubTerrorist = gubTerrorists[ubLoop];

      // random 40% chance of adding this terrorist if not yet placed
      if ((gMercProfiles[ubTerrorist].sSectorX == 0) && (Random(100) < 40)) {
        fFoundSpot = FALSE;
        // Since there are 5 spots per terrorist and a maximum of 5 terrorists, we
        // are guaranteed to be able to find a spot for each terrorist since there
        // aren't enough other terrorists to use up all the spots for any one
        // terrorist
        do {
          // pick a random spot, see if it's already been used by another terrorist
          uiLocationChoice = Random(NUM_TERRORIST_POSSIBLE_LOCATIONS);
          for (ubLoop2 = 0; ubLoop2 < ubNumTerroristsAdded; ubLoop2++) {
            if (sTerroristPlacement[ubLoop2][0] == gsTerroristSector[ubLoop][uiLocationChoice][0]) {
              if (sTerroristPlacement[ubLoop2][1] ==
                  gsTerroristSector[ubLoop][uiLocationChoice][1]) {
                continue;
              }
            }
          }
          fFoundSpot = TRUE;
        } while (!fFoundSpot);

        // place terrorist!
        gMercProfiles[ubTerrorist].sSectorX = gsTerroristSector[ubLoop][uiLocationChoice][0];
        gMercProfiles[ubTerrorist].sSectorY = gsTerroristSector[ubLoop][uiLocationChoice][1];
        gMercProfiles[ubTerrorist].bSectorZ = 0;
        sTerroristPlacement[ubNumTerroristsAdded][0] = gMercProfiles[ubTerrorist].sSectorX;
        sTerroristPlacement[ubNumTerroristsAdded][1] = gMercProfiles[ubTerrorist].sSectorY;
        ubNumTerroristsAdded++;
      }
      ubLoop++;
    }

    // start over if necessary
  }

  // set total terrorists outstanding in Carmen's info byte
  gMercProfiles[78].bNPCData = 1 + ubNumAdditionalTerrorists;

  // store total terrorists
  gubNumTerrorists = 1 + ubNumAdditionalTerrorists;
}

void MakeRemainingTerroristsTougher(void) {
  uint8_t ubRemainingTerrorists = 0, ubLoop;
  uint16_t usNewItem, usOldItem;
  struct OBJECTTYPE Object;
  uint8_t ubRemainingDifficulty;

  for (ubLoop = 0; ubLoop < NUM_TERRORISTS; ubLoop++) {
    if (gMercProfiles[gubTerrorists[ubLoop]].bMercStatus != MERC_IS_DEAD &&
        gMercProfiles[gubTerrorists[ubLoop]].sSectorX != 0 &&
        gMercProfiles[gubTerrorists[ubLoop]].sSectorY != 0) {
      if (gubTerrorists[ubLoop] == SLAY) {
        if (FindSoldierByProfileID(SLAY, TRUE) != NULL) {
          // Slay on player's team, doesn't count towards remaining terrorists
          continue;
        }
      }
      ubRemainingTerrorists++;
    }
  }

  ubRemainingDifficulty = (60 / gubNumTerrorists) * (gubNumTerrorists - ubRemainingTerrorists);

  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_MEDIUM:
      ubRemainingDifficulty = (ubRemainingDifficulty * 13) / 10;
      break;
    case DIF_LEVEL_HARD:
      ubRemainingDifficulty = (ubRemainingDifficulty * 16) / 10;
      break;
    default:
      break;
  }

  if (ubRemainingDifficulty < 14) {
    // nothing
    return;
  } else if (ubRemainingDifficulty < 28) {
    // mini grenade
    usOldItem = NOTHING;
    usNewItem = MINI_GRENADE;
  } else if (ubRemainingDifficulty < 42) {
    // hand grenade
    usOldItem = MINI_GRENADE;
    usNewItem = HAND_GRENADE;
  } else if (ubRemainingDifficulty < 56) {
    // mustard
    usOldItem = HAND_GRENADE;
    usNewItem = MUSTARD_GRENADE;
  } else if (ubRemainingDifficulty < 70) {
    // LAW
    usOldItem = MUSTARD_GRENADE;
    usNewItem = ROCKET_LAUNCHER;
  } else {
    // LAW and hand grenade
    usOldItem = NOTHING;
    usNewItem = HAND_GRENADE;
  }

  DeleteObj(&Object);
  Object.usItem = usNewItem;
  Object.bStatus[0] = 100;

  for (ubLoop = 0; ubLoop < NUM_TERRORISTS; ubLoop++) {
    if (gMercProfiles[gubTerrorists[ubLoop]].bMercStatus != MERC_IS_DEAD &&
        gMercProfiles[gubTerrorists[ubLoop]].sSectorX != 0 &&
        gMercProfiles[gubTerrorists[ubLoop]].sSectorY != 0) {
      if (gubTerrorists[ubLoop] == SLAY) {
        if (FindSoldierByProfileID(SLAY, TRUE) != NULL) {
          // Slay on player's team, doesn't count towards remaining terrorists
          continue;
        }
      }

      if (usOldItem != NOTHING) {
        RemoveObjectFromSoldierProfile(gubTerrorists[ubLoop], usOldItem);
      }
      PlaceObjectInSoldierProfile(gubTerrorists[ubLoop], &Object);
    }
  }
}

void DecideOnAssassin(void) {
  uint8_t ubAssassinPossibility[NUM_ASSASSINS] = {NO_PROFILE, NO_PROFILE, NO_PROFILE,
                                                  NO_PROFILE, NO_PROFILE, NO_PROFILE};
  uint8_t ubAssassinsPossible = 0;
  uint8_t ubLoop, ubLoop2;
  uint8_t ubTown;

  ubTown = GetTownIdForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);

  for (ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++) {
    // make sure alive and not placed already
    if (gMercProfiles[gubAssassins[ubLoop]].bMercStatus != MERC_IS_DEAD &&
        gMercProfiles[gubAssassins[ubLoop]].sSectorX == 0 &&
        gMercProfiles[gubAssassins[ubLoop]].sSectorY == 0) {
      // check this merc to see if the town is a possibility
      for (ubLoop2 = 0; ubLoop2 < NUM_ASSASSIN_POSSIBLE_TOWNS; ubLoop2++) {
        if (gbAssassinTown[ubLoop][ubLoop2] == ubTown) {
          ubAssassinPossibility[ubAssassinsPossible] = gubAssassins[ubLoop];
          ubAssassinsPossible++;
        }
      }
    }
  }

  if (ubAssassinsPossible != 0) {
    ubLoop = ubAssassinPossibility[Random(ubAssassinsPossible)];
    gMercProfiles[ubLoop].sSectorX = gWorldSectorX;
    gMercProfiles[ubLoop].sSectorY = gWorldSectorY;
    AddStrategicEvent(EVENT_REMOVE_ASSASSIN, GetGameTimeInMin() + 60 * (3 + Random(3)), ubLoop);
  }
}

void MakeRemainingAssassinsTougher(void) {
  uint8_t ubRemainingAssassins = 0, ubLoop;
  uint16_t usNewItem, usOldItem;
  struct OBJECTTYPE Object;
  uint8_t ubRemainingDifficulty;

  for (ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++) {
    if (gMercProfiles[gubAssassins[ubLoop]].bMercStatus != MERC_IS_DEAD) {
      ubRemainingAssassins++;
    }
  }

  ubRemainingDifficulty = (60 / NUM_ASSASSINS) * (NUM_ASSASSINS - ubRemainingAssassins);

  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_MEDIUM:
      ubRemainingDifficulty = (ubRemainingDifficulty * 13) / 10;
      break;
    case DIF_LEVEL_HARD:
      ubRemainingDifficulty = (ubRemainingDifficulty * 16) / 10;
      break;
    default:
      break;
  }

  if (ubRemainingDifficulty < 14) {
    // nothing
    return;
  } else if (ubRemainingDifficulty < 28) {
    // mini grenade
    usOldItem = NOTHING;
    usNewItem = MINI_GRENADE;
  } else if (ubRemainingDifficulty < 42) {
    // hand grenade
    usOldItem = MINI_GRENADE;
    usNewItem = HAND_GRENADE;
  } else if (ubRemainingDifficulty < 56) {
    // mustard
    usOldItem = HAND_GRENADE;
    usNewItem = MUSTARD_GRENADE;
  } else if (ubRemainingDifficulty < 70) {
    // LAW
    usOldItem = MUSTARD_GRENADE;
    usNewItem = ROCKET_LAUNCHER;
  } else {
    // LAW and hand grenade
    usOldItem = NOTHING;
    usNewItem = HAND_GRENADE;
  }

  DeleteObj(&Object);
  Object.usItem = usNewItem;
  Object.bStatus[0] = 100;

  for (ubLoop = 0; ubLoop < NUM_ASSASSINS; ubLoop++) {
    if (gMercProfiles[gubAssassins[ubLoop]].bMercStatus != MERC_IS_DEAD) {
      if (usOldItem != NOTHING) {
        RemoveObjectFromSoldierProfile(gubAssassins[ubLoop], usOldItem);
      }
      PlaceObjectInSoldierProfile(gubAssassins[ubLoop], &Object);
    }
  }
}

void StartSomeMercsOnAssignment(void) {
  uint32_t uiCnt;
  MERCPROFILESTRUCT *pProfile;
  uint32_t uiChance;

  // some randomly picked A.I.M. mercs will start off "on assignment" at the beginning of each new
  // game
  for (uiCnt = 0; uiCnt < AIM_AND_MERC_MERCS; uiCnt++) {
    pProfile = &(gMercProfiles[uiCnt]);

    // calc chance to start on assignment
    uiChance = 5 * pProfile->bExpLevel;

    if (Random(100) < uiChance) {
      pProfile->bMercStatus = MERC_WORKING_ELSEWHERE;
      pProfile->uiDayBecomesAvailable =
          1 + Random(6 + (pProfile->bExpLevel / 2));  // 1-(6 to 11) days
    } else {
      pProfile->bMercStatus = MERC_OK;
      pProfile->uiDayBecomesAvailable = 0;
    }

    pProfile->uiPrecedentQuoteSaid = 0;
    pProfile->ubDaysOfMoraleHangover = 0;
  }
}

void SetProfileFaceData(uint8_t ubCharNum, uint8_t ubFaceIndex, uint16_t usEyesX, uint16_t usEyesY,
                        uint16_t usMouthX, uint16_t usMouthY) {
  gMercProfiles[ubCharNum].ubFaceIndex = ubFaceIndex;
  gMercProfiles[ubCharNum].usEyesX = usEyesX;
  gMercProfiles[ubCharNum].usEyesY = usEyesY;
  gMercProfiles[ubCharNum].usMouthX = usMouthX;
  gMercProfiles[ubCharNum].usMouthY = usMouthY;
}

uint16_t CalcCompetence(MERCPROFILESTRUCT *pProfile) {
  uint32_t uiStats, uiSkills, uiActionPoints, uiSpecialSkills;
  uint16_t usCompetence;

  // count life twice 'cause it's also hit points
  // mental skills are halved 'cause they're actually not that important within the game
  uiStats = ((2 * pProfile->bLifeMax) + pProfile->bStrength + pProfile->bAgility +
             pProfile->bDexterity + ((pProfile->bLeadership + pProfile->bWisdom) / 2)) /
            3;

  // marksmanship is very important, count it double
  uiSkills = (uint32_t)((2 * (pow((float)pProfile->bMarksmanship, 3) / 10000)) +
                        1.5 * (pow((float)pProfile->bMedical, 3) / 10000) +
                        (pow((float)pProfile->bMechanical, 3) / 10000) +
                        (pow((float)pProfile->bExplosive, 3) / 10000));

  // action points
  uiActionPoints = 5 + (((10 * pProfile->bExpLevel + 3 * pProfile->bAgility +
                          2 * pProfile->bLifeMax + 2 * pProfile->bDexterity) +
                         20) /
                        40);

  // count how many he has, don't care what they are
  uiSpecialSkills =
      ((pProfile->bSkillTrait != 0) ? 1 : 0) + ((pProfile->bSkillTrait2 != 0) ? 1 : 0);

  usCompetence = (uint16_t)((pow(pProfile->bExpLevel, 0.2) * uiStats * uiSkills *
                             (uiActionPoints - 6) * (1 + (0.05 * (float)uiSpecialSkills))) /
                            1000);

  // this currently varies from about 10 (Flo) to 1200 (Gus)
  return (usCompetence);
}

int16_t CalcMedicalDeposit(MERCPROFILESTRUCT *pProfile) {
  uint16_t usDeposit;

  // this rounds off to the nearest hundred
  usDeposit = (((5 * CalcCompetence(pProfile)) + 50) / 100) * 100;

  return (usDeposit);
}

struct SOLDIERTYPE *FindSoldierByProfileID(uint8_t ubProfileID, BOOLEAN fPlayerMercsOnly) {
  uint8_t ubLoop, ubLoopLimit;
  struct SOLDIERTYPE *pSoldier;

  if (fPlayerMercsOnly) {
    ubLoopLimit = gTacticalStatus.Team[0].bLastID;
  } else {
    ubLoopLimit = MAX_NUM_SOLDIERS;
  }

  for (ubLoop = 0, pSoldier = MercPtrs[0]; ubLoop < ubLoopLimit; ubLoop++, pSoldier++) {
    if (IsSolActive(pSoldier) && GetSolProfile(pSoldier) == ubProfileID) {
      return (pSoldier);
    }
  }
  return (NULL);
}

struct SOLDIERTYPE *ChangeSoldierTeam(struct SOLDIERTYPE *pSoldier, uint8_t ubTeam) {
  uint8_t ubID;
  struct SOLDIERTYPE *pNewSoldier = NULL;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  uint32_t cnt;
  int16_t sOldGridNo;

  uint32_t uiSlot;
  struct SOLDIERTYPE *pGroupMember;

  if (gfInTalkPanel) {
    DeleteTalkingMenu();
  }

  // Save merc id for this guy...
  ubID = GetSolID(pSoldier);

  sOldGridNo = pSoldier->sGridNo;

  // Remove him from the game!
  InternalTacticalRemoveSoldier(ubID, FALSE);

  // Create a new one!
  memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
  MercCreateStruct.bTeam = ubTeam;
  MercCreateStruct.ubProfile = GetSolProfile(pSoldier);
  MercCreateStruct.bBodyType = pSoldier->ubBodyType;
  MercCreateStruct.sSectorX = GetSolSectorX(pSoldier);
  MercCreateStruct.sSectorY = GetSolSectorY(pSoldier);
  MercCreateStruct.bSectorZ = GetSolSectorZ(pSoldier);
  MercCreateStruct.sInsertionGridNo = pSoldier->sGridNo;
  MercCreateStruct.bDirection = pSoldier->bDirection;

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    MercCreateStruct.ubProfile = NO_PROFILE;
    MercCreateStruct.fUseGivenVehicle = TRUE;
    MercCreateStruct.bUseGivenVehicleID = pSoldier->bVehicleID;
  }

  if (ubTeam == gbPlayerNum) {
    MercCreateStruct.fPlayerMerc = TRUE;
  }

  if (TacticalCreateSoldier(&MercCreateStruct, &ubID)) {
    pNewSoldier = MercPtrs[ubID];

    // Copy vital stats back!
    pNewSoldier->bLife = pSoldier->bLife;
    pNewSoldier->bLifeMax = pSoldier->bLifeMax;
    pNewSoldier->bAgility = pSoldier->bAgility;
    pNewSoldier->bLeadership = pSoldier->bLeadership;
    pNewSoldier->bDexterity = pSoldier->bDexterity;
    pNewSoldier->bStrength = pSoldier->bStrength;
    pNewSoldier->bWisdom = pSoldier->bWisdom;
    pNewSoldier->bExpLevel = pSoldier->bExpLevel;
    pNewSoldier->bMarksmanship = pSoldier->bMarksmanship;
    pNewSoldier->bMedical = pSoldier->bMedical;
    pNewSoldier->bMechanical = pSoldier->bMechanical;
    pNewSoldier->bExplosive = pSoldier->bExplosive;
    pNewSoldier->bScientific = pSoldier->bScientific;
    pNewSoldier->bLastRenderVisibleValue = pSoldier->bLastRenderVisibleValue;
    pNewSoldier->bVisible = pSoldier->bVisible;

    if (ubTeam == gbPlayerNum) {
      pNewSoldier->bVisible = 1;
    }

    // Copy over any items....
    for (cnt = 0; cnt < NUM_INV_SLOTS; cnt++) {
      pNewSoldier->inv[cnt] = pSoldier->inv[cnt];
    }

    // OK, loop through all active merc slots, change
    // Change ANY attacker's target if they were once on this guy.....
    for (uiSlot = 0; uiSlot < guiNumMercSlots; uiSlot++) {
      pGroupMember = MercSlots[uiSlot];

      if (pGroupMember != NULL) {
        if (pGroupMember->ubTargetID == GetSolID(pSoldier)) {
          pGroupMember->ubTargetID = pNewSoldier->ubID;
        }
      }
    }

    // Set insertion gridNo
    pNewSoldier->sInsertionGridNo = sOldGridNo;

    if (gfPotentialTeamChangeDuringDeath) {
      HandleCheckForDeathCommonCode(pSoldier);
    }

    if (gfWorldLoaded && pSoldier->bInSector
        // GetSolSectorX(pSoldier) == gWorldSectorX && GetSolSectorY(pSoldier) == gWorldSectorY &&
        // GetSolSectorZ(pSoldier) == gbWorldSectorZ
    ) {
      AddSoldierToSectorNoCalculateDirectionUseAnimation(ubID, pSoldier->usAnimState,
                                                         pSoldier->usAniCode);
      HandleSight(pNewSoldier, SIGHT_LOOK | SIGHT_RADIO);
    }

    // fix up the event queue...
    //	ChangeSoldierIDInQueuedEvents( ubOldID, uiOldUniqueId, pNewSoldier->ubID,
    // pNewSoldier->uiUniqueSoldierIdValue );

    if (pNewSoldier->ubProfile != NO_PROFILE) {
      if (ubTeam == gbPlayerNum) {
        gMercProfiles[pNewSoldier->ubProfile].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;
      } else {
        gMercProfiles[pNewSoldier->ubProfile].ubMiscFlags &= (~PROFILE_MISC_FLAG_RECRUITED);
      }
    }
  }

  // AT the low level check if this poor guy is in inv panel, else
  // remove....
  if (gsCurInterfacePanel == SM_PANEL && gpSMCurrentMerc == pSoldier) {
    // Switch....
    SetCurrentInterfacePanel(TEAM_PANEL);
  }

  return (pNewSoldier);
}

BOOLEAN RecruitRPC(uint8_t ubCharNum) {
  struct SOLDIERTYPE *pSoldier, *pNewSoldier;

  // Get soldier pointer
  pSoldier = FindSoldierByProfileID(ubCharNum, FALSE);

  if (!pSoldier) {
    return (FALSE);
  }

  // OK, set recruit flag..
  gMercProfiles[ubCharNum].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;

  // Add this guy to our team!
  pNewSoldier = ChangeSoldierTeam(pSoldier, gbPlayerNum);

  // handle set up any RPC's that will leave us in time
  if (ubCharNum == SLAY) {
    // slay will leave in a week
    pNewSoldier->iEndofContractTime = GetGameTimeInMin() + (7 * 24 * 60);

    KickOutWheelchair(pNewSoldier);
  } else if (ubCharNum == DYNAMO && gubQuest[QUEST_FREE_DYNAMO] == QUESTINPROGRESS) {
    EndQuest(QUEST_FREE_DYNAMO, GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));
  }
  // handle town loyalty adjustment
  HandleTownLoyaltyForNPCRecruitment(pNewSoldier);

  // Try putting them into the current squad
  if (AddCharacterToSquad(pNewSoldier, (int8_t)CurrentSquad()) == FALSE) {
    AddCharacterToAnySquad(pNewSoldier);
  }

  ResetDeadSquadMemberList(pNewSoldier->bAssignment);

  DirtyMercPanelInterface(pNewSoldier, DIRTYLEVEL2);

  if (pNewSoldier->inv[HANDPOS].usItem == NOTHING) {
    // empty handed - swap in first available weapon
    int8_t bSlot;

    bSlot = FindObjClass(pNewSoldier, IC_WEAPON);
    if (bSlot != NO_SLOT) {
      if (Item[pNewSoldier->inv[bSlot].usItem].fFlags & ITEM_TWO_HANDED) {
        if (bSlot != SECONDHANDPOS && pNewSoldier->inv[SECONDHANDPOS].usItem != NOTHING) {
          // need to move second hand item out first
          AutoPlaceObject(pNewSoldier, &(pNewSoldier->inv[SECONDHANDPOS]), FALSE);
        }
      }
      // swap item to hand
      SwapObjs(&(pNewSoldier->inv[bSlot]), &(pNewSoldier->inv[HANDPOS]));
    }
  }

  if (ubCharNum == IRA) {
    // trigger 0th PCscript line
    TriggerNPCRecord(IRA, 0);
  }

  // Set whatkind of merc am i
  pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__NPC;

  //
  // add a history log that tells the user that a npc has joined the team
  //
  // ( pass in pNewSoldier->sSectorX cause if its invalid, -1, n/a will appear as the sector in the
  // history log )
  AddHistoryToPlayersLog(HISTORY_RPC_JOINED_TEAM, pNewSoldier->ubProfile, GetGameTimeInMin(),
                         (uint8_t)pNewSoldier->sSectorX, (uint8_t)pNewSoldier->sSectorY);

  // remove the merc from the Personnel screens departed list ( if they have never been hired
  // before, its ok to call it )
  RemoveNewlyHiredMercFromPersonnelDepartedList(GetSolProfile(pSoldier));

  return (TRUE);
}

BOOLEAN RecruitEPC(uint8_t ubCharNum) {
  struct SOLDIERTYPE *pSoldier, *pNewSoldier;

  // Get soldier pointer
  pSoldier = FindSoldierByProfileID(ubCharNum, FALSE);

  if (!pSoldier) {
    return (FALSE);
  }

  // OK, set recruit flag..
  gMercProfiles[ubCharNum].ubMiscFlags |= PROFILE_MISC_FLAG_EPCACTIVE;

  gMercProfiles[ubCharNum].ubMiscFlags3 &= ~PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

  // Add this guy to our team!
  pNewSoldier = ChangeSoldierTeam(pSoldier, gbPlayerNum);
  pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__EPC;

  // Try putting them into the current squad
  if (AddCharacterToSquad(pNewSoldier, (int8_t)CurrentSquad()) == FALSE) {
    AddCharacterToAnySquad(pNewSoldier);
  }

  ResetDeadSquadMemberList(pNewSoldier->bAssignment);

  DirtyMercPanelInterface(pNewSoldier, DIRTYLEVEL2);
  // Make the interface panel dirty..
  // This will dirty the panel next frame...
  gfRerenderInterfaceFromHelpText = TRUE;

  // If we are a robot, look to update controller....
  if (pNewSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    UpdateRobotControllerGivenRobot(pNewSoldier);
  }

  // Set whatkind of merc am i
  pNewSoldier->ubWhatKindOfMercAmI = MERC_TYPE__EPC;

  UpdateTeamPanelAssignments();

  return (TRUE);
}

BOOLEAN UnRecruitEPC(uint8_t ubCharNum) {
  struct SOLDIERTYPE *pSoldier;

  // Get soldier pointer
  pSoldier = FindSoldierByProfileID(ubCharNum, FALSE);

  if (!pSoldier) {
    return (FALSE);
  }

  if (pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__EPC) {
    return (FALSE);
  }

  if (pSoldier->bAssignment < ON_DUTY) {
    ResetDeadSquadMemberList(pSoldier->bAssignment);
  }

  // Rmeove from squad....
  RemoveCharacterFromSquads(pSoldier);

  // O< check if this is the only guy in the sector....
  if (gusSelectedSoldier == GetSolID(pSoldier)) {
    gusSelectedSoldier = NOBODY;
  }

  // OK, UN set recruit flag..
  gMercProfiles[ubCharNum].ubMiscFlags &= (~PROFILE_MISC_FLAG_EPCACTIVE);

  // update sector values to current

  // check to see if this person should disappear from the map after this
  if ((ubCharNum == JOHN || ubCharNum == MARY) && GetSolSectorX(pSoldier) == 13 &&
      GetSolSectorY(pSoldier) == MAP_ROW_B && GetSolSectorZ(pSoldier) == 0) {
    gMercProfiles[ubCharNum].sSectorX = 0;
    gMercProfiles[ubCharNum].sSectorY = 0;
    gMercProfiles[ubCharNum].bSectorZ = 0;
  } else {
    gMercProfiles[ubCharNum].sSectorX = GetSolSectorX(pSoldier);
    gMercProfiles[ubCharNum].sSectorY = GetSolSectorY(pSoldier);
    gMercProfiles[ubCharNum].bSectorZ = GetSolSectorZ(pSoldier);
  }

  // how do we decide whether or not to set this?
  gMercProfiles[ubCharNum].fUseProfileInsertionInfo = TRUE;
  gMercProfiles[ubCharNum].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE;

  // Add this guy to CIV team!
  ChangeSoldierTeam(pSoldier, CIV_TEAM);

  UpdateTeamPanelAssignments();

  return (TRUE);
}

int8_t WhichBuddy(uint8_t ubCharNum, uint8_t ubBuddy) {
  MERCPROFILESTRUCT *pProfile;
  int8_t bLoop;

  pProfile = &(gMercProfiles[ubCharNum]);

  for (bLoop = 0; bLoop < 3; bLoop++) {
    if (pProfile->bBuddy[bLoop] == ubBuddy) {
      return (bLoop);
    }
  }
  return (-1);
}

int8_t WhichHated(uint8_t ubCharNum, uint8_t ubHated) {
  MERCPROFILESTRUCT *pProfile;
  int8_t bLoop;

  pProfile = &(gMercProfiles[ubCharNum]);

  for (bLoop = 0; bLoop < 3; bLoop++) {
    if (pProfile->bHated[bLoop] == ubHated) {
      return (bLoop);
    }
  }
  return (-1);
}

BOOLEAN IsProfileATerrorist(uint8_t ubProfile) {
  if (ubProfile == 83 || ubProfile == 111 || ubProfile == 64 || ubProfile == 112 ||
      ubProfile == 82 || ubProfile == 110) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN IsProfileAHeadMiner(uint8_t ubProfile) {
  if (ubProfile == 106 || ubProfile == 148 || ubProfile == 156 || ubProfile == 157 ||
      ubProfile == 158) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void UpdateSoldierPointerDataIntoProfile(BOOLEAN fPlayerMercs) {
  uint32_t uiCount;
  struct SOLDIERTYPE *pSoldier = NULL;
  MERCPROFILESTRUCT *pProfile;
  BOOLEAN fDoCopy = FALSE;

  for (uiCount = 0; uiCount < guiNumMercSlots; uiCount++) {
    pSoldier = MercSlots[uiCount];

    if (pSoldier != NULL) {
      if (GetSolProfile(pSoldier) != NO_PROFILE) {
        fDoCopy = FALSE;

        // If we are above player mercs
        if (fPlayerMercs) {
          if (GetSolProfile(pSoldier) < FIRST_RPC) {
            fDoCopy = TRUE;
          }
        } else {
          if (GetSolProfile(pSoldier) >= FIRST_RPC) {
            fDoCopy = TRUE;
          }
        }

        if (fDoCopy) {
          // get profile...
          pProfile = &(gMercProfiles[GetSolProfile(pSoldier)]);

          // Copy....
          pProfile->bLife = pSoldier->bLife;
          pProfile->bLifeMax = pSoldier->bLifeMax;
          pProfile->bAgility = pSoldier->bAgility;
          pProfile->bLeadership = pSoldier->bLeadership;
          pProfile->bDexterity = pSoldier->bDexterity;
          pProfile->bStrength = pSoldier->bStrength;
          pProfile->bWisdom = pSoldier->bWisdom;
          pProfile->bExpLevel = pSoldier->bExpLevel;
          pProfile->bMarksmanship = pSoldier->bMarksmanship;
          pProfile->bMedical = pSoldier->bMedical;
          pProfile->bMechanical = pSoldier->bMechanical;
          pProfile->bExplosive = pSoldier->bExplosive;
          pProfile->bScientific = pSoldier->bScientific;
        }
      }
    }
  }
}

BOOLEAN DoesMercHaveABuddyOnTheTeam(uint8_t ubMercID) {
  uint8_t ubCnt;
  int8_t bBuddyID;

  // loop through the list of people the merc is buddies with
  for (ubCnt = 0; ubCnt < 3; ubCnt++) {
    // see if the merc has a buddy on the team
    bBuddyID = gMercProfiles[ubMercID].bBuddy[ubCnt];

    // If its not a valid 'buddy'
    if (bBuddyID < 0) continue;

    if (IsMercOnTeam(bBuddyID)) {
      if (!IsMercDead(bBuddyID)) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN MercIsHot(struct SOLDIERTYPE *pSoldier) {
  if (GetSolProfile(pSoldier) != NO_PROFILE &&
      gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait == HEAT_INTOLERANT) {
    if (SectorTemperature(GetMinutesSinceDayStart(), GetSolSectorX(pSoldier),
                          GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier)) > 0) {
      return (TRUE);
    }
  }
  return (FALSE);
}

struct SOLDIERTYPE *SwapLarrysProfiles(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubSrcProfile;
  uint8_t ubDestProfile;
  MERCPROFILESTRUCT *pNewProfile;

  ubSrcProfile = GetSolProfile(pSoldier);
  if (ubSrcProfile == LARRY_NORMAL) {
    ubDestProfile = LARRY_DRUNK;
  } else if (ubSrcProfile == LARRY_DRUNK) {
    ubDestProfile = LARRY_NORMAL;
  } else {
    // I don't think so!
    return (pSoldier);
  }

  pNewProfile = &gMercProfiles[ubDestProfile];
  pNewProfile->ubMiscFlags2 = gMercProfiles[ubSrcProfile].ubMiscFlags2;
  pNewProfile->ubMiscFlags = gMercProfiles[ubSrcProfile].ubMiscFlags;
  pNewProfile->sSectorX = gMercProfiles[ubSrcProfile].sSectorX;
  pNewProfile->sSectorY = gMercProfiles[ubSrcProfile].sSectorY;
  pNewProfile->uiDayBecomesAvailable = gMercProfiles[ubSrcProfile].uiDayBecomesAvailable;
  pNewProfile->usKills = gMercProfiles[ubSrcProfile].usKills;
  pNewProfile->usAssists = gMercProfiles[ubSrcProfile].usAssists;
  pNewProfile->usShotsFired = gMercProfiles[ubSrcProfile].usShotsFired;
  pNewProfile->usShotsHit = gMercProfiles[ubSrcProfile].usShotsHit;
  pNewProfile->usBattlesFought = gMercProfiles[ubSrcProfile].usBattlesFought;
  pNewProfile->usTimesWounded = gMercProfiles[ubSrcProfile].usTimesWounded;
  pNewProfile->usTotalDaysServed = gMercProfiles[ubSrcProfile].usTotalDaysServed;
  pNewProfile->bResigned = gMercProfiles[ubSrcProfile].bResigned;
  pNewProfile->bActive = gMercProfiles[ubSrcProfile].bActive;
  pNewProfile->fUseProfileInsertionInfo = gMercProfiles[ubSrcProfile].fUseProfileInsertionInfo;
  pNewProfile->sGridNo = gMercProfiles[ubSrcProfile].sGridNo;
  pNewProfile->ubQuoteActionID = gMercProfiles[ubSrcProfile].ubQuoteActionID;
  pNewProfile->ubLastQuoteSaid = gMercProfiles[ubSrcProfile].ubLastQuoteSaid;
  pNewProfile->ubStrategicInsertionCode = gMercProfiles[ubSrcProfile].ubStrategicInsertionCode;
  pNewProfile->bMercStatus = gMercProfiles[ubSrcProfile].bMercStatus;
  pNewProfile->bSectorZ = gMercProfiles[ubSrcProfile].bSectorZ;
  pNewProfile->usStrategicInsertionData = gMercProfiles[ubSrcProfile].usStrategicInsertionData;
  pNewProfile->sTrueSalary = gMercProfiles[ubSrcProfile].sTrueSalary;
  pNewProfile->ubMiscFlags3 = gMercProfiles[ubSrcProfile].ubMiscFlags3;
  pNewProfile->ubDaysOfMoraleHangover = gMercProfiles[ubSrcProfile].ubDaysOfMoraleHangover;
  pNewProfile->ubNumTimesDrugUseInLifetime =
      gMercProfiles[ubSrcProfile].ubNumTimesDrugUseInLifetime;
  pNewProfile->uiPrecedentQuoteSaid = gMercProfiles[ubSrcProfile].uiPrecedentQuoteSaid;
  pNewProfile->sPreCombatGridNo = gMercProfiles[ubSrcProfile].sPreCombatGridNo;

  // CJC: this is causing problems so just skip the transfer of exp...
  /*
          pNewProfile->sLifeGain = gMercProfiles[ ubSrcProfile ].sLifeGain;
          pNewProfile->sAgilityGain = gMercProfiles[ ubSrcProfile ].sAgilityGain;
          pNewProfile->sDexterityGain = gMercProfiles[ ubSrcProfile ].sDexterityGain;
          pNewProfile->sStrengthGain = gMercProfiles[ ubSrcProfile ].sStrengthGain;
          pNewProfile->sLeadershipGain = gMercProfiles[ ubSrcProfile ].sLeadershipGain;
          pNewProfile->sWisdomGain = gMercProfiles[ ubSrcProfile ].sWisdomGain;
          pNewProfile->sExpLevelGain = gMercProfiles[ ubSrcProfile ].sExpLevelGain;
          pNewProfile->sMarksmanshipGain = gMercProfiles[ ubSrcProfile ].sMarksmanshipGain;
          pNewProfile->sMedicalGain = gMercProfiles[ ubSrcProfile ].sMedicalGain;
          pNewProfile->sMechanicGain = gMercProfiles[ ubSrcProfile ].sMechanicGain;
          pNewProfile->sExplosivesGain = gMercProfiles[ ubSrcProfile ].sExplosivesGain;

          pNewProfile->bLifeDelta = gMercProfiles[ ubSrcProfile ].bLifeDelta;
          pNewProfile->bAgilityDelta = gMercProfiles[ ubSrcProfile ].bAgilityDelta;
          pNewProfile->bDexterityDelta = gMercProfiles[ ubSrcProfile ].bDexterityDelta;
          pNewProfile->bStrengthDelta = gMercProfiles[ ubSrcProfile ].bStrengthDelta;
          pNewProfile->bLeadershipDelta = gMercProfiles[ ubSrcProfile ].bLeadershipDelta;
          pNewProfile->bWisdomDelta = gMercProfiles[ ubSrcProfile ].bWisdomDelta;
          pNewProfile->bExpLevelDelta = gMercProfiles[ ubSrcProfile ].bExpLevelDelta;
          pNewProfile->bMarksmanshipDelta = gMercProfiles[ ubSrcProfile ].bMarksmanshipDelta;
          pNewProfile->bMedicalDelta = gMercProfiles[ ubSrcProfile ].bMedicalDelta;
          pNewProfile->bMechanicDelta = gMercProfiles[ ubSrcProfile ].bMechanicDelta;
          pNewProfile->bExplosivesDelta = gMercProfiles[ ubSrcProfile ].bExplosivesDelta;
          */

  memcpy(pNewProfile->bInvStatus, gMercProfiles[ubSrcProfile].bInvStatus, sizeof(uint8_t) * 19);
  memcpy(pNewProfile->bInvStatus, gMercProfiles[ubSrcProfile].bInvStatus, sizeof(uint8_t) * 19);
  memcpy(pNewProfile->inv, gMercProfiles[ubSrcProfile].inv, sizeof(uint16_t) * 19);
  memcpy(pNewProfile->bMercTownReputation, gMercProfiles[ubSrcProfile].bMercTownReputation,
         sizeof(uint8_t) * 20);

  // remove face
  DeleteSoldierFace(pSoldier);

  pSoldier->ubProfile = ubDestProfile;

  // create new face
  pSoldier->iFaceIndex = InitSoldierFace(pSoldier);

  // replace profile in group
  ReplaceSoldierProfileInPlayerGroup(pSoldier->ubGroupID, ubSrcProfile, ubDestProfile);

  pSoldier->bStrength = pNewProfile->bStrength + pNewProfile->bStrengthDelta;
  pSoldier->bDexterity = pNewProfile->bDexterity + pNewProfile->bDexterityDelta;
  pSoldier->bAgility = pNewProfile->bAgility + pNewProfile->bAgilityDelta;
  pSoldier->bWisdom = pNewProfile->bWisdom + pNewProfile->bWisdomDelta;
  pSoldier->bExpLevel = pNewProfile->bExpLevel + pNewProfile->bExpLevelDelta;
  pSoldier->bLeadership = pNewProfile->bLeadership + pNewProfile->bLeadershipDelta;

  pSoldier->bMarksmanship = pNewProfile->bMarksmanship + pNewProfile->bMarksmanshipDelta;
  pSoldier->bMechanical = pNewProfile->bMechanical + pNewProfile->bMechanicDelta;
  pSoldier->bMedical = pNewProfile->bMedical + pNewProfile->bMedicalDelta;
  pSoldier->bExplosive = pNewProfile->bExplosive + pNewProfile->bExplosivesDelta;

  if (GetSolProfile(pSoldier) == LARRY_DRUNK) {
    SetFactTrue(FACT_LARRY_CHANGED);
  } else {
    SetFactFalse(FACT_LARRY_CHANGED);
  }

  DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

  return (pSoldier);
}

BOOLEAN DoesNPCOwnBuilding(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  uint8_t ubRoomInfo;

  // Get room info
  ubRoomInfo = gubWorldRoomInfo[sGridNo];

  if (ubRoomInfo == NO_ROOM) {
    return (FALSE);
  }

  // Are we an NPC?
  if (pSoldier->bTeam != CIV_TEAM) {
    return (FALSE);
  }

  // OK, check both ranges
  if (ubRoomInfo >= gMercProfiles[GetSolProfile(pSoldier)].ubRoomRangeStart[0] &&
      ubRoomInfo <= gMercProfiles[GetSolProfile(pSoldier)].ubRoomRangeEnd[0]) {
    return (TRUE);
  }

  if (ubRoomInfo >= gMercProfiles[GetSolProfile(pSoldier)].ubRoomRangeStart[1] &&
      ubRoomInfo <= gMercProfiles[GetSolProfile(pSoldier)].ubRoomRangeEnd[1]) {
    return (TRUE);
  }

  return (FALSE);
}
