#ifndef __SECTOR_INFO_H
#define __SECTOR_INFO_H

#include "SGP/Types.h"

typedef struct SectorInfo SECTORINFO;

struct SectorInfo {
  // information pertaining to this sector
  UINT32 uiFlags;              // various special conditions
  UINT8 ubInvestigativeState;  // When the sector is attacked by the player, the state increases by
                               // 1 permanently. This value determines how quickly it is
                               // investigated by the enemy.
  UINT8 ubGarrisonID;  // IF the sector has an ID for this (non 255), then the queen values this
                       // sector and it indexes the garrison group.
  INT8 ubPendingReinforcements;  // when the enemy owns this sector, this value will keep track of
                                 // HIGH priority reinforcements -- not regular.
  BOOLEAN _only_savedgame_fMilitiaTrainingPaid;
  UINT8 ubMilitiaTrainingPercentDone;
  UINT8 ubMilitiaTrainingHundredths;
  // enemy military presence
  BOOLEAN fPlayer[4];  // whether the player THINKS the sector is unde his control or not. array is
                       // for sublevels
  // enemy only info
  UINT8 ubNumTroops;     // the actual number of troops here.
  UINT8 ubNumElites;     // the actual number of elites here.
  UINT8 ubNumAdmins;     // the actual number of admins here.
  UINT8 ubNumCreatures;  // only set when immediately before ground attack made!
  UINT8 ubTroopsInBattle, ubElitesInBattle, ubAdminsInBattle, ubCreaturesInBattle;

  INT8 bLastKnownEnemies;  // -1 means never been there, no idea, otherwise it's what we'd observed
                           // most recently while this is being maintained (partially, surely
                           // buggy), nothing uses it anymore. ARM

  UINT32 ubDayOfLastCreatureAttack;
  UINT32 uiFacilitiesFlags;  // the flags for various facilities

  UINT8 ubTraversability[5];  // determines the traversability ratings to adjacent sectors.
                              // The last index represents the traversability if travelling
                              // throught the sector without entering it.
  INT8 townID;
  INT8 bUSUSED;
  INT8 bBloodCats;
  INT8 bBloodCatPlacements;
  INT8 UNUSEDbSAMCondition;

  UINT8 ubTravelRating;  // Represents how travelled a sector is.  Typically, the higher the travel
                         // rating, the more people go near it.  A travel rating of 0 means there
                         // are never people around.  This value is used for determining how often
                         // items would "vanish" from a sector (nice theory, except it isn't being
                         // used that way.  Stealing is only in towns.  ARM)

  // militia count storage (only for save games)
  // town militia per experience class, 0/1/2 is GREEN/REGULAR/ELITE
  UINT8 _only_savedgame_ubNumberOfCivsAtLevel[3 /*MAX_MILITIA_LEVELS*/];

  UINT16 usUNUSEDMilitiaLevels;             // unused (ARM)
  UINT8 ubUNUSEDNumberOfJoeBlowCivilians;   // unused (ARM)
  UINT32 uiTimeCurrentSectorWasLastLoaded;  // Specifies the last time the player was in the sector
  UINT8 ubUNUSEDNumberOfEnemiesThoughtToBeHere;  // using bLastKnownEnemies instead
  UINT32 uiTimeLastPlayerLiberated;  // in game seconds (used to prevent the queen from attacking
                                     // for awhile)

  BOOLEAN fSurfaceWasEverPlayerControlled;

  UINT8 bFiller1;
  UINT8 bFiller2;
  UINT8 bFiller3;

  UINT32 uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;

  INT8 bPadding[41];
};

#endif
