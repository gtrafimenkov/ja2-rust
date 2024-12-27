#include "Strategic/PlayerCommand.h"

#include "GameSettings.h"
#include "Laptop/Email.h"
#include "Laptop/LaptopSave.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/Morale.h"
#include "Tactical/Overhead.h"
#include "Tactical/TacticalSave.h"
#include "Utils/Text.h"

extern BOOLEAN fMapScreenBottomDirty;

void GetSectorFacilitiesFlags(int16_t sMapX, int16_t sMapY, wchar_t *sFacilitiesString,
                              size_t bufSize) {
  // will build a string stating current facilities present in sector

  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags == 0) {
    // none
    swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[0]);
    return;
  }

  // hospital
  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags & SFCF_HOSPITAL) {
    swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[1]);
  }

  // industry
  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags & SFCF_INDUSTRY) {
    if (wcslen(sFacilitiesString) == 0) {
      swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[2]);
    } else {
      wcsncat(sFacilitiesString, L",", bufSize);
      wcsncat(sFacilitiesString, sFacilitiesStrings[2], bufSize);
    }
  }

  // prison
  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags & SFCF_PRISON) {
    if (wcslen(sFacilitiesString) == 0) {
      swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[3]);
    } else {
      wcsncat(sFacilitiesString, L",", bufSize);
      wcsncat(sFacilitiesString, sFacilitiesStrings[3], bufSize);
    }
  }

  // airport
  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags & SFCF_AIRPORT) {
    if (wcslen(sFacilitiesString) == 0) {
      swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[5]);
    } else {
      wcsncat(sFacilitiesString, L",", bufSize);
      wcsncat(sFacilitiesString, sFacilitiesStrings[5], bufSize);
    }
  }

  // gun range
  if (SectorInfo[GetSectorID8(sMapX, sMapY)].uiFacilitiesFlags & SFCF_GUN_RANGE) {
    if (wcslen(sFacilitiesString) == 0) {
      swprintf(sFacilitiesString, bufSize, L"%s", sFacilitiesStrings[6]);
    } else {
      wcsncat(sFacilitiesString, L",", bufSize);
      wcsncat(sFacilitiesString, sFacilitiesStrings[6], bufSize);
    }
  }

  sFacilitiesString[wcslen(sFacilitiesString)] = 0;

  return;
}

// ALL changes of control to player must be funneled through here!
BOOLEAN SetThisSectorAsPlayerControlled(int16_t sMapX, int16_t sMapY, int8_t bMapZ,
                                        BOOLEAN fContested) {
  // NOTE: MapSector must be 16-bit, cause MAX_WORLD_X is actually 18, so the sector numbers exceed
  // 256 although we use only 16x16
  uint16_t usMapSector = 0;
  BOOLEAN fWasEnemyControlled = FALSE;
  TownID bTownId = 0;
  uint8_t ubSectorID;

  if (AreInMeanwhile()) {
    return FALSE;
  }

  if (bMapZ == 0) {
    usMapSector = GetSectorID16(sMapX, (sMapY));

    /*
                    // if enemies formerly controlled this sector
                    if (StrategicMap[ usMapSector ].fEnemyControlled)
                    {
                            // remember that the enemies have lost it
                            StrategicMap[ usMapSector ].fLostControlAtSomeTime = TRUE;
                    }
    */
    if (NumHostilesInSector(sMapX, sMapY, bMapZ)) {  // too premature:  enemies still in sector.
      return FALSE;
    }

    // check if we ever grabbed drassen airport, if so, set fact we can go to BR's
    if ((sMapX == BOBBYR_SHIPPING_DEST_SECTOR_X) && (sMapY == BOBBYR_SHIPPING_DEST_SECTOR_Y)) {
      LaptopSaveInfo.fBobbyRSiteCanBeAccessed = TRUE;

      // If the player has been to Bobbyr when it was down, and we havent already sent email, send
      // him an email
      if (LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction ==
              BOBBYR_BEEN_TO_SITE_ONCE &&
          LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction !=
              BOBBYR_ALREADY_SENT_EMAIL) {
        AddEmail(BOBBYR_NOW_OPEN, BOBBYR_NOW_OPEN_LENGTH, BOBBY_R, GetWorldTotalMin());
        LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction =
            BOBBYR_ALREADY_SENT_EMAIL;
      }
    }

    fWasEnemyControlled = StrategicMap[usMapSector].fEnemyControlled;

    StrategicMap[usMapSector].fEnemyControlled = FALSE;
    SectorInfo[GetSectorID8(sMapX, sMapY)].fPlayer[bMapZ] = TRUE;

    bTownId = StrategicMap[usMapSector].townID;

    // check if there's a town in the sector
    if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS)) {
      // yes, start tracking (& displaying) this town's loyalty if not already doing so
      StartTownLoyaltyIfFirstTime(bTownId);
    }

    // if player took control away from enemy
    if (fWasEnemyControlled && fContested) {
      // and it's a town
      if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS)) {
        // don't do these for takeovers of Omerta sectors at the beginning of the game
        if ((bTownId != OMERTA) || (GetWorldDay() != 1)) {
          ubSectorID = (uint8_t)GetSectorID8(sMapX, sMapY);
          if (!bMapZ && ubSectorID != SEC_J9 && ubSectorID != SEC_K4) {
            HandleMoraleEvent(NULL, MORALE_TOWN_LIBERATED, sMapX, sMapY, bMapZ);
            HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_TOWN_SECTOR, sMapX, sMapY, bMapZ);

            // liberation by definition requires that the place was enemy controlled in the first
            // place
            CheckIfEntireTownHasBeenLiberated(bTownId, sMapX, sMapY);
          }
        }
      }

      // if it's a mine that's still worth something
      if (IsThereAMineInThisSector(sMapX, sMapY)) {
        if (GetTotalLeftInMine(GetMineIndexForSector(sMapX, sMapY)) > 0) {
          HandleMoraleEvent(NULL, MORALE_MINE_LIBERATED, sMapX, sMapY, bMapZ);
          HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_MINE, sMapX, sMapY, bMapZ);
        }
      }

      // if it's a SAM site sector
      if (IsThisSectorASAMSector(sMapX, sMapY, bMapZ)) {
        if (1 /*!GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_SECTOR_HAS_BEEN_LIBERATED_ONCE ) */) {
          // SAM site liberated for first time, schedule meanwhile
          HandleMeanWhileEventPostingForSAMLiberation(GetSAMIdFromSector(sMapX, sMapY, bMapZ));
        }

        HandleMoraleEvent(NULL, MORALE_SAM_SITE_LIBERATED, sMapX, sMapY, bMapZ);
        HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_GAIN_SAM, sMapX, sMapY, bMapZ);

        // if Skyrider has been delivered to chopper, and already mentioned Drassen SAM site, but
        // not used this quote yet
        if (IsHelicopterPilotAvailable() && (guiHelicopterSkyriderTalkState >= 1) &&
            (!gfSkyriderSaidCongratsOnTakingSAM)) {
          SkyRiderTalk(SAM_SITE_TAKEN);
          gfSkyriderSaidCongratsOnTakingSAM = TRUE;
        }

        if (!SectorInfo[GetSectorID8(sMapX, sMapY)].fSurfaceWasEverPlayerControlled) {
          // grant grace period
          if (gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD) {
            UpdateLastDayOfPlayerActivity((uint16_t)(GetWorldDay() + 2));
          } else {
            UpdateLastDayOfPlayerActivity((uint16_t)(GetWorldDay() + 1));
          }
        }
      }

      // if it's a helicopter refueling site sector
      if (IsRefuelSiteInSector(sMapX, sMapY)) {
        UpdateRefuelSiteAvailability();
      }

      //			SetSectorFlag( sMapX, sMapY, bMapZ,
      // SF_SECTOR_HAS_BEEN_LIBERATED_ONCE );
      if (bMapZ == 0 && ((sMapY == MAP_ROW_M && (sMapX >= 2 && sMapX <= 6)) ||
                         (sMapY == MAP_ROW_N && sMapX == 6))) {
        HandleOutskirtsOfMedunaMeanwhileScene();
      }
    }

    if (fContested) {
      StrategicHandleQueenLosingControlOfSector((uint8_t)sMapX, (uint8_t)sMapY, (uint8_t)bMapZ);
    }
  } else {
    if (sMapX == 3 && sMapY == 16 && bMapZ == 1) {  // Basement sector (P3_b1)
      gfUseAlternateQueenPosition = TRUE;
    }
  }

  // also set fact the player knows they own it
  SectorInfo[GetSectorID8(sMapX, sMapY)].fPlayer[bMapZ] = TRUE;

  if (bMapZ == 0) {
    SectorInfo[GetSectorID8(sMapX, sMapY)].fSurfaceWasEverPlayerControlled = TRUE;
  }

  // KM : Aug 11, 1999 -- Patch fix:  Relocated this check so it gets called everytime a sector
  // changes hands,
  //     even if the sector isn't a SAM site.  There is a bug _somewhere_ that fails to update the
  //     airspace, even though the player controls it.
  UpdateAirspaceControl();

  // redraw map/income if in mapscreen
  MarkForRedrawalStrategicMap();
  fMapScreenBottomDirty = TRUE;

  return fWasEnemyControlled;
}

// ALL changes of control to enemy must be funneled through here!
BOOLEAN SetThisSectorAsEnemyControlled(int16_t sMapX, int16_t sMapY, int8_t bMapZ,
                                       BOOLEAN fContested) {
  uint16_t usMapSector = 0;
  BOOLEAN fWasPlayerControlled = FALSE;
  TownID bTownId = 0;
  uint8_t ubTheftChance;
  uint8_t ubSectorID;

  // KM : August 6, 1999 Patch fix
  //     This check was added because this function gets called when player mercs retreat from an
  //     unresolved battle between militia and enemies.  It will get called again AFTER autoresolve
  //     is finished.
  if (gfAutomaticallyStartAutoResolve) {
    return (FALSE);
  }

  if (bMapZ == 0) {
    usMapSector = GetSectorID16(sMapX, (sMapY));

    fWasPlayerControlled = !StrategicMap[usMapSector].fEnemyControlled;

    StrategicMap[usMapSector].fEnemyControlled = TRUE;

    // if player lost control to the enemy
    if (fWasPlayerControlled) {
      if (PlayerMercsInSector((uint8_t)sMapX, (uint8_t)sMapY,
                              (uint8_t)bMapZ)) {  // too premature:  Player mercs still in sector.
        return FALSE;
      }

      // check if there's a town in the sector
      bTownId = StrategicMap[usMapSector].townID;

      SectorInfo[GetSectorID8(sMapX, sMapY)].fPlayer[bMapZ] = FALSE;

      // and it's a town
      if ((bTownId >= FIRST_TOWN) && (bTownId < NUM_TOWNS)) {
        ubSectorID = (uint8_t)GetSectorID8(sMapX, sMapY);
        if (!bMapZ && ubSectorID != SEC_J9 && ubSectorID != SEC_K4) {
          HandleMoraleEvent(NULL, MORALE_TOWN_LOST, sMapX, sMapY, bMapZ);
          HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_TOWN_SECTOR, sMapX, sMapY, bMapZ);

          CheckIfEntireTownHasBeenLost(bTownId, sMapX, sMapY);
        }
      }

      // if the sector has a mine which is still worth something
      if (IsThereAMineInThisSector(sMapX, sMapY)) {
        // if it isn't empty
        if (GetTotalLeftInMine(GetMineIndexForSector(sMapX, sMapY)) > 0) {
          QueenHasRegainedMineSector(GetMineIndexForSector(sMapX, sMapY));
          HandleMoraleEvent(NULL, MORALE_MINE_LOST, sMapX, sMapY, bMapZ);
          HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_MINE, sMapX, sMapY, bMapZ);
        }
      }

      // if it's a SAM site sector
      if (IsThisSectorASAMSector(sMapX, sMapY, bMapZ)) {
        HandleMoraleEvent(NULL, MORALE_SAM_SITE_LOST, sMapX, sMapY, bMapZ);
        HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_LOSE_SAM, sMapX, sMapY, bMapZ);
      }

      // if it's a helicopter refueling site sector
      if (IsRefuelSiteInSector(sMapX, sMapY)) {
        UpdateRefuelSiteAvailability();
      }

      // ARM: this must be AFTER all resulting loyalty effects are resolved, or reduced mine income
      // shown won't be accurate
      NotifyPlayerWhenEnemyTakesControlOfImportantSector(sMapX, sMapY, 0, fContested);
    }

    // NOTE: Stealing is intentionally OUTSIDE the fWasPlayerControlled branch.  This function gets
    // called if new enemy reinforcements arrive, and they deserve another crack at stealing what
    // the first group missed! :-)

    // stealing should fail anyway 'cause there shouldn't be a temp file for unvisited sectors, but
    // let's check anyway
    if (GetSectorFlagStatus(sMapX, sMapY, (uint8_t)bMapZ, SF_ALREADY_VISITED) == TRUE) {
      // enemies can steal items left lying about (random chance).  The more there are, the more
      // they take!
      ubTheftChance = 5 * NumEnemiesInAnySector(sMapX, sMapY, bMapZ);
      // max 90%, some stuff may just simply not get found
      if (ubTheftChance > 90) {
        ubTheftChance = 90;
      }
      RemoveRandomItemsInSector(sMapX, sMapY, bMapZ, ubTheftChance);
    }

    // don't touch fPlayer flag for a surface sector lost to the enemies!
    // just because player has lost the sector doesn't mean he realizes it - that's up to our caller
    // to decide!
  } else {
    // underground sector control is always up to date, because we don't track control down there
    SectorInfo[GetSectorID8(sMapX, sMapY)].fPlayer[bMapZ] = FALSE;
  }

  // KM : Aug 11, 1999 -- Patch fix:  Relocated this check so it gets called everytime a sector
  // changes hands,
  //     even if the sector isn't a SAM site.  There is a bug _somewhere_ that fails to update the
  //     airspace, even though the player controls it.
  UpdateAirspaceControl();

  // redraw map/income if in mapscreen
  MarkForRedrawalStrategicMap();
  fMapScreenBottomDirty = TRUE;

  return fWasPlayerControlled;
}

#ifdef JA2TESTVERSION
void ClearMapControlledFlags(void) {
  int32_t iCounterA = 0, iCounterB = 0;
  uint16_t usMapSector = 0;

  for (iCounterA = 1; iCounterA < MAP_WORLD_X - 1; iCounterA++) {
    for (iCounterB = 1; iCounterB < MAP_WORLD_Y - 1; iCounterB++) {
      usMapSector = GetSectorID16(iCounterA, (iCounterB));
      StrategicMap[usMapSector].fEnemyControlled = FALSE;
      SectorInfo[GetSectorID8(iCounterA, iCounterB)].fPlayer[0] = TRUE;
    }
  }
}
#endif

/*
BOOLEAN IsTheSectorPerceivedToBeUnderEnemyControl( int16_t sMapX, int16_t sMapY, int8_t bMapZ )
{

        // are we in battle in this sector?
        if( ( sMapX == gWorldSectorX ) && ( sMapY == gWorldSectorY ) && ( bMapZ == gbWorldSectorZ )
&& ( gTacticalStatus.uiFlags & INCOMBAT ) )
        {
                return( TRUE );
        }


        // does the player believe this sector is under enemy control?
        return( !( SectorInfo[ GetSectorID8( sMapX, sMapY ) ].fPlayer[ bMapZ ] ) );
}


void MakePlayerPerceptionOfSectorControlCorrect( int16_t sMapX, int16_t sMapY, int8_t bMapZ )
{
        if (bMapZ == 0)
        {
                SectorInfo[ GetSectorID8( sMapX, sMapY ) ].fPlayer[ bMapZ ] = !( StrategicMap[
GetSectorID16( sMapX, sMapY ) ].fEnemyControlled );
        }
        // else nothing, underground sector control is always up to date, because we don't track
control down there

        MarkForRedrawalStrategicMap();
}
*/

void ReplaceSoldierProfileInPlayerGroup(uint8_t ubGroupID, uint8_t ubOldProfile,
                                        uint8_t ubNewProfile) {
  struct GROUP *pGroup;
  PLAYERGROUP *curr;

  pGroup = GetGroup(ubGroupID);

  if (!pGroup) {
    return;
  }

  curr = pGroup->pPlayerList;

  while (curr) {
    if (curr->ubProfileID == ubOldProfile) {
      // replace and return!
      curr->ubProfileID = ubNewProfile;
      return;
    }
    curr = curr->next;
  }
}
