// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __SECTOR_INFO_H
#define __SECTOR_INFO_H

#include "SGP/Types.h"

typedef struct SectorInfo SECTORINFO;

struct SectorInfo {
  // information pertaining to this sector
  uint32_t uiFlags;              // various special conditions
  uint8_t ubInvestigativeState;  // When the sector is attacked by the player, the state increases
                                 // by 1 permanently. This value determines how quickly it is
                                 // investigated by the enemy.
  uint8_t ubGarrisonID;  // IF the sector has an ID for this (non 255), then the queen values this
                         // sector and it indexes the garrison group.
  int8_t ubPendingReinforcements;  // when the enemy owns this sector, this value will keep track of
                                   // HIGH priority reinforcements -- not regular.
  BOOLEAN _only_savedgame_fMilitiaTrainingPaid;
  uint8_t ubMilitiaTrainingPercentDone;
  uint8_t ubMilitiaTrainingHundredths;
  // enemy military presence
  BOOLEAN fPlayer[4];  // whether the player THINKS the sector is unde his control or not. array is
                       // for sublevels
  // enemy only info
  uint8_t ubNumTroops;     // the actual number of troops here.
  uint8_t ubNumElites;     // the actual number of elites here.
  uint8_t ubNumAdmins;     // the actual number of admins here.
  uint8_t ubNumCreatures;  // only set when immediately before ground attack made!
  uint8_t ubTroopsInBattle, ubElitesInBattle, ubAdminsInBattle, ubCreaturesInBattle;

  int8_t bLastKnownEnemies;  // -1 means never been there, no idea, otherwise it's what we'd
                             // observed most recently while this is being maintained (partially,
                             // surely buggy), nothing uses it anymore. ARM

  uint32_t ubDayOfLastCreatureAttack;
  uint32_t uiFacilitiesFlags;  // the flags for various facilities

  uint8_t ubTraversability[5];  // determines the traversability ratings to adjacent sectors.
                                // The last index represents the traversability if travelling
                                // throught the sector without entering it.
  int8_t townID;
  int8_t bUSUSED;
  int8_t bBloodCats;
  int8_t bBloodCatPlacements;
  int8_t UNUSEDbSAMCondition;

  uint8_t ubTravelRating;  // Represents how travelled a sector is.  Typically, the higher the
                           // travel rating, the more people go near it.  A travel rating of 0 means
                           // there are never people around.  This value is used for determining how
                           // often items would "vanish" from a sector (nice theory, except it isn't
                           // being used that way.  Stealing is only in towns.  ARM)

  // militia count storage (only for save games)
  // town militia per experience class, 0/1/2 is GREEN/REGULAR/ELITE
  uint8_t _only_savedgame_ubNumberOfCivsAtLevel[3 /*MAX_MILITIA_LEVELS*/];

  uint16_t usUNUSEDMilitiaLevels;            // unused (ARM)
  uint8_t ubUNUSEDNumberOfJoeBlowCivilians;  // unused (ARM)
  uint32_t
      uiTimeCurrentSectorWasLastLoaded;  // Specifies the last time the player was in the sector
  uint8_t ubUNUSEDNumberOfEnemiesThoughtToBeHere;  // using bLastKnownEnemies instead
  uint32_t uiTimeLastPlayerLiberated;  // in game seconds (used to prevent the queen from attacking
                                       // for awhile)

  BOOLEAN fSurfaceWasEverPlayerControlled;

  uint8_t bFiller1;
  uint8_t bFiller2;
  uint8_t bFiller3;

  uint32_t uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;

  int8_t bPadding[41];
};

#endif
