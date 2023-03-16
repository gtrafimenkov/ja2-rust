#include "TileEngine/SmokeEffects.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/HandleItems.h"
#include "Tactical/OppList.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Weapons.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"
#include "platform.h"

int8_t FromWorldFlagsToSmokeType(uint8_t ubWorldFlags);
uint8_t FromSmokeTypeToWorldFlags(int8_t bType);

#define NUM_SMOKE_EFFECT_SLOTS 25

// GLOBAL FOR SMOKE LISTING
SMOKEEFFECT gSmokeEffectData[NUM_SMOKE_EFFECT_SLOTS];
uint32_t guiNumSmokeEffects = 0;

int32_t GetFreeSmokeEffect(void);
void RecountSmokeEffects(void);

int32_t GetFreeSmokeEffect(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumSmokeEffects; uiCount++) {
    if ((gSmokeEffectData[uiCount].fAllocated == FALSE)) return ((int32_t)uiCount);
  }

  if (guiNumSmokeEffects < NUM_SMOKE_EFFECT_SLOTS) return ((int32_t)guiNumSmokeEffects++);

  return (-1);
}

void RecountSmokeEffects(void) {
  int32_t uiCount;

  for (uiCount = guiNumSmokeEffects - 1; (uiCount >= 0); uiCount--) {
    if ((gSmokeEffectData[uiCount].fAllocated)) {
      guiNumSmokeEffects = (uint32_t)(uiCount + 1);
      break;
    }
  }
}

// Returns NO_SMOKE_EFFECT if none there...
int8_t GetSmokeEffectOnTile(int16_t sGridNo, int8_t bLevel) {
  uint8_t ubExtFlags;

  ubExtFlags = gpWorldLevelData[sGridNo].ubExtFlags[bLevel];

  // Look at worldleveldata to find flags..
  if (ubExtFlags & ANY_SMOKE_EFFECT) {
    // Which smoke am i?
    return (FromWorldFlagsToSmokeType(ubExtFlags));
  }

  return (NO_SMOKE_EFFECT);
}

int8_t FromWorldFlagsToSmokeType(uint8_t ubWorldFlags) {
  if (ubWorldFlags & MAPELEMENT_EXT_SMOKE) {
    return (NORMAL_SMOKE_EFFECT);
  } else if (ubWorldFlags & MAPELEMENT_EXT_TEARGAS) {
    return (TEARGAS_SMOKE_EFFECT);
  } else if (ubWorldFlags & MAPELEMENT_EXT_MUSTARDGAS) {
    return (MUSTARDGAS_SMOKE_EFFECT);
  } else if (ubWorldFlags & MAPELEMENT_EXT_CREATUREGAS) {
    return (CREATURE_SMOKE_EFFECT);
  } else {
    return (NO_SMOKE_EFFECT);
  }
}

uint8_t FromSmokeTypeToWorldFlags(int8_t bType) {
  switch (bType) {
    case NORMAL_SMOKE_EFFECT:

      return (MAPELEMENT_EXT_SMOKE);
      break;

    case TEARGAS_SMOKE_EFFECT:

      return (MAPELEMENT_EXT_TEARGAS);
      break;

    case MUSTARDGAS_SMOKE_EFFECT:

      return (MAPELEMENT_EXT_MUSTARDGAS);
      break;

    case CREATURE_SMOKE_EFFECT:

      return (MAPELEMENT_EXT_CREATUREGAS);
      break;

    default:

      return (0);
  }
}

int32_t NewSmokeEffect(int16_t sGridNo, uint16_t usItem, int8_t bLevel, uint8_t ubOwner) {
  SMOKEEFFECT *pSmoke;
  int32_t iSmokeIndex;
  int8_t bSmokeEffectType = 0;
  uint8_t ubDuration = 0;
  uint8_t ubStartRadius = 0;

  if ((iSmokeIndex = GetFreeSmokeEffect()) == (-1)) return (-1);

  memset(&gSmokeEffectData[iSmokeIndex], 0, sizeof(SMOKEEFFECT));

  pSmoke = &gSmokeEffectData[iSmokeIndex];

  // Set some values...
  pSmoke->sGridNo = sGridNo;
  pSmoke->usItem = usItem;
  pSmoke->uiTimeOfLastUpdate = GetWorldTotalSeconds();

  // Are we indoors?
  if (GetTerrainType(sGridNo) == FLAT_FLOOR) {
    pSmoke->bFlags |= SMOKE_EFFECT_INDOORS;
  }

  switch (usItem) {
    case MUSTARD_GRENADE:

      bSmokeEffectType = MUSTARDGAS_SMOKE_EFFECT;
      ubDuration = 5;
      ubStartRadius = 1;
      break;

    case TEARGAS_GRENADE:
    case GL_TEARGAS_GRENADE:
      bSmokeEffectType = TEARGAS_SMOKE_EFFECT;
      ubDuration = 5;
      ubStartRadius = 1;
      break;

    case BIG_TEAR_GAS:
      bSmokeEffectType = TEARGAS_SMOKE_EFFECT;
      ubDuration = 5;
      ubStartRadius = 1;
      break;

    case SMOKE_GRENADE:
    case GL_SMOKE_GRENADE:

      bSmokeEffectType = NORMAL_SMOKE_EFFECT;
      ubDuration = 5;
      ubStartRadius = 1;
      break;

    case SMALL_CREATURE_GAS:
      bSmokeEffectType = CREATURE_SMOKE_EFFECT;
      ubDuration = 3;
      ubStartRadius = 1;
      break;

    case LARGE_CREATURE_GAS:
      bSmokeEffectType = CREATURE_SMOKE_EFFECT;
      ubDuration = 3;
      ubStartRadius = Explosive[Item[LARGE_CREATURE_GAS].ubClassIndex].ubRadius;
      break;

    case VERY_SMALL_CREATURE_GAS:

      bSmokeEffectType = CREATURE_SMOKE_EFFECT;
      ubDuration = 2;
      ubStartRadius = 0;
      break;
  }

  pSmoke->ubDuration = ubDuration;
  pSmoke->ubRadius = ubStartRadius;
  pSmoke->bAge = 0;
  pSmoke->fAllocated = TRUE;
  pSmoke->bType = bSmokeEffectType;
  pSmoke->ubOwner = ubOwner;

  if (pSmoke->bFlags & SMOKE_EFFECT_INDOORS) {
    // Duration is increased by 2 turns...indoors
    pSmoke->ubDuration += 3;
  }

  if (bLevel) {
    pSmoke->bFlags |= SMOKE_EFFECT_ON_ROOF;
  }

  // ATE: FALSE into subsequent-- it's the first one!
  SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner, FALSE, bLevel,
               iSmokeIndex);

  return (iSmokeIndex);
}

// Add smoke to gridno
// ( Replacement algorithm uses distance away )
void AddSmokeEffectToTile(int32_t iSmokeEffectID, int8_t bType, int16_t sGridNo, int8_t bLevel) {
  ANITILE_PARAMS AniParams;
  SMOKEEFFECT *pSmoke;
  BOOLEAN fDissipating = FALSE;

  pSmoke = &gSmokeEffectData[iSmokeEffectID];

  if ((pSmoke->ubDuration - pSmoke->bAge) < 2) {
    fDissipating = TRUE;
    // Remove old one...
    RemoveSmokeEffectFromTile(sGridNo, bLevel);
  }

  // If smoke effect exists already.... stop
  if (gpWorldLevelData[sGridNo].ubExtFlags[bLevel] & ANY_SMOKE_EFFECT) {
    return;
  }

  // OK,  Create anitile....
  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
  AniParams.sGridNo = sGridNo;

  if (bLevel == 0) {
    AniParams.ubLevelID = ANI_STRUCT_LEVEL;
  } else {
    AniParams.ubLevelID = ANI_ONROOF_LEVEL;
  }

  AniParams.sDelay = (int16_t)(300 + Random(300));

  if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
    AniParams.sStartFrame = (int16_t)0;
  } else {
    AniParams.sStartFrame = (int16_t)Random(5);
  }

  // Bare bones flags are...
  //	AniParams.uiFlags							= ANITILE_CACHEDTILE
  //| ANITILE_FORWARD | ANITILE_OPTIMIZEFORSMOKEEFFECT | ANITILE_SMOKE_EFFECT | ANITILE_LOOPING;
  // AniParams.uiFlags
  // = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_SMOKE_EFFECT | ANITILE_LOOPING;

  if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
    AniParams.uiFlags = ANITILE_PAUSED | ANITILE_CACHEDTILE | ANITILE_FORWARD |
                        ANITILE_SMOKE_EFFECT | ANITILE_LOOPING;
  } else {
    AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_SMOKE_EFFECT |
                        ANITILE_LOOPING | ANITILE_ALWAYS_TRANSLUCENT;
  }

  AniParams.sX = CenterX(sGridNo);
  AniParams.sY = CenterY(sGridNo);
  AniParams.sZ = (int16_t)0;

  // Use the right graphic based on type..
  switch (bType) {
    case NORMAL_SMOKE_EFFECT:

      if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\smkechze.STI");
      } else {
        if (fDissipating) {
          strcpy(AniParams.zCachedFile, "TILECACHE\\smalsmke.STI");
        } else {
          strcpy(AniParams.zCachedFile, "TILECACHE\\SMOKE.STI");
        }
      }
      break;

    case TEARGAS_SMOKE_EFFECT:

      if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\tearchze.STI");
      } else {
        if (fDissipating) {
          strcpy(AniParams.zCachedFile, "TILECACHE\\smaltear.STI");
        } else {
          strcpy(AniParams.zCachedFile, "TILECACHE\\TEARGAS.STI");
        }
      }
      break;

    case MUSTARDGAS_SMOKE_EFFECT:

      if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\mustchze.STI");
      } else {
        if (fDissipating) {
          strcpy(AniParams.zCachedFile, "TILECACHE\\smalmust.STI");
        } else {
          strcpy(AniParams.zCachedFile, "TILECACHE\\MUSTARD2.STI");
        }
      }
      break;

    case CREATURE_SMOKE_EFFECT:

      if (!(gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE])) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\spit_gas.STI");
      } else {
        if (fDissipating) {
          strcpy(AniParams.zCachedFile, "TILECACHE\\spit_gas.STI");
        } else {
          strcpy(AniParams.zCachedFile, "TILECACHE\\spit_gas.STI");
        }
      }
      break;
  }

  // Create tile...
  CreateAnimationTile(&AniParams);

  // Set world flags
  gpWorldLevelData[sGridNo].ubExtFlags[bLevel] |= FromSmokeTypeToWorldFlags(bType);

  // All done...

  // Re-draw..... :(
  SetRenderFlags(RENDER_FLAG_FULL);
}

void RemoveSmokeEffectFromTile(int16_t sGridNo, int8_t bLevel) {
  ANITILE *pAniTile;
  uint8_t ubLevelID;

  // Get ANI tile...
  if (bLevel == 0) {
    ubLevelID = ANI_STRUCT_LEVEL;
  } else {
    ubLevelID = ANI_ONROOF_LEVEL;
  }

  pAniTile = GetCachedAniTileOfType(sGridNo, ubLevelID, ANITILE_SMOKE_EFFECT);

  if (pAniTile != NULL) {
    DeleteAniTile(pAniTile);

    SetRenderFlags(RENDER_FLAG_FULL);
  }

  // Unset flags in world....
  // ( // check to see if we are the last one....
  if (GetCachedAniTileOfType(sGridNo, ubLevelID, ANITILE_SMOKE_EFFECT) == NULL) {
    gpWorldLevelData[sGridNo].ubExtFlags[bLevel] &= (~ANY_SMOKE_EFFECT);
  }
}

void DecaySmokeEffects(uint32_t uiTime) {
  SMOKEEFFECT *pSmoke;
  uint32_t cnt, cnt2;
  BOOLEAN fUpdate = FALSE;
  BOOLEAN fSpreadEffect;
  int8_t bLevel;
  uint16_t usNumUpdates = 1;

  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    if (MercSlots[cnt]) {
      // reset 'hit by gas' flags
      MercSlots[cnt]->fHitByGasFlags = 0;
    }
  }

  // ATE: 1 ) make first pass and delete/mark any smoke effect for update
  // all the deleting has to be done first///

  // age all active tear gas clouds, deactivate those that are just dispersing
  for (cnt = 0; cnt < guiNumSmokeEffects; cnt++) {
    fSpreadEffect = TRUE;

    pSmoke = &gSmokeEffectData[cnt];

    if (pSmoke->fAllocated) {
      if (pSmoke->bFlags & SMOKE_EFFECT_ON_ROOF) {
        bLevel = 1;
      } else {
        bLevel = 0;
      }

      // Do things differently for combat /vs realtime
      // always try to update during combat
      if (gTacticalStatus.uiFlags & INCOMBAT) {
        fUpdate = TRUE;
      } else {
        // ATE: Do this every so ofte, to acheive the effect we want...
        if ((uiTime - pSmoke->uiTimeOfLastUpdate) > 10) {
          fUpdate = TRUE;

          usNumUpdates = (uint16_t)((uiTime - pSmoke->uiTimeOfLastUpdate) / 10);
        }
      }

      if (fUpdate) {
        pSmoke->uiTimeOfLastUpdate = uiTime;

        for (cnt2 = 0; cnt2 < usNumUpdates; cnt2++) {
          pSmoke->bAge++;

          if (pSmoke->bAge == 1) {
            // ATE: At least mark for update!
            pSmoke->bFlags |= SMOKE_EFFECT_MARK_FOR_UPDATE;
            fSpreadEffect = FALSE;
          } else {
            fSpreadEffect = TRUE;
          }

          if (fSpreadEffect) {
            // if this cloud remains effective (duration not reached)
            if (pSmoke->bAge <= pSmoke->ubDuration) {
              // ATE: Only mark now and increse radius - actual drawing is done
              // in another pass cause it could
              // just get erased...
              pSmoke->bFlags |= SMOKE_EFFECT_MARK_FOR_UPDATE;

              // calculate the new cloud radius
              // cloud expands by 1 every turn outdoors, and every other turn indoors

              // ATE: If radius is < maximun, increase radius, otherwise keep at max
              if (pSmoke->ubRadius < Explosive[Item[pSmoke->usItem].ubClassIndex].ubRadius) {
                pSmoke->ubRadius++;
              }
            } else {
              // deactivate tear gas cloud (use last known radius)
              SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner,
                           ERASE_SPREAD_EFFECT, bLevel, cnt);
              pSmoke->fAllocated = FALSE;
              break;
            }
          }
        }
      } else {
        // damage anyone standing in cloud
        SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner,
                     REDO_SPREAD_EFFECT, 0, cnt);
      }
    }
  }

  for (cnt = 0; cnt < guiNumSmokeEffects; cnt++) {
    pSmoke = &gSmokeEffectData[cnt];

    if (pSmoke->fAllocated) {
      if (pSmoke->bFlags & SMOKE_EFFECT_ON_ROOF) {
        bLevel = 1;
      } else {
        bLevel = 0;
      }

      // if this cloud remains effective (duration not reached)
      if (pSmoke->bFlags & SMOKE_EFFECT_MARK_FOR_UPDATE) {
        SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner, TRUE,
                     bLevel, cnt);
        pSmoke->bFlags &= (~SMOKE_EFFECT_MARK_FOR_UPDATE);
      }
    }
  }

  AllTeamsLookForAll(TRUE);
}

BOOLEAN SaveSmokeEffectsToSaveGameFile(HWFILE hFile) {
  /*
          uint32_t	uiNumBytesWritten;
          uint32_t	uiCnt=0;
          uint32_t	uiNumSmokeEffects=0;


          //loop through and count the number of smoke effects
          for( uiCnt=0; uiCnt<guiNumSmokeEffects; uiCnt++)
          {
                  if( gSmokeEffectData[ uiCnt ].fAllocated )
                          uiNumSmokeEffects++;
          }


          //Save the Number of Smoke Effects
          FileMan_Write( hFile, &uiNumSmokeEffects, sizeof( uint32_t ), &uiNumBytesWritten );
          if( uiNumBytesWritten != sizeof( uint32_t ) )
          {
                  return( FALSE );
          }


          if( uiNumSmokeEffects != 0 )
          {
                  //loop through and save the number of smoke effects
                  for( uiCnt=0; uiCnt < guiNumSmokeEffects; uiCnt++)
                  {
                          //if the smoke is active
                          if( gSmokeEffectData[ uiCnt ].fAllocated )
                          {
                                  //Save the Smoke effect Data
                                  FileMan_Write( hFile, &gSmokeEffectData[ uiCnt ], sizeof(
     SMOKEEFFECT
     ), &uiNumBytesWritten ); if( uiNumBytesWritten != sizeof( SMOKEEFFECT ) )
                                  {
                                          return( FALSE );
                                  }
                          }
                  }
          }
  */
  return (TRUE);
}

BOOLEAN LoadSmokeEffectsFromLoadGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead;
  uint32_t uiCount;
  uint32_t uiCnt = 0;
  int8_t bLevel;

  // no longer need to load smoke effects.  They are now in temp files
  if (guiSaveGameVersion < 75) {
    // Clear out the old list
    memset(gSmokeEffectData, 0, sizeof(SMOKEEFFECT) * NUM_SMOKE_EFFECT_SLOTS);

    // Load the Number of Smoke Effects
    FileMan_Read(hFile, &guiNumSmokeEffects, sizeof(uint32_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint32_t)) {
      return (FALSE);
    }

    // This is a TEMP hack to allow us to use the saves
    if (guiSaveGameVersion < 37 && guiNumSmokeEffects == 0) {
      // Load the Smoke effect Data
      FileMan_Read(hFile, gSmokeEffectData, sizeof(SMOKEEFFECT), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(SMOKEEFFECT)) {
        return (FALSE);
      }
    }

    // loop through and load the list
    for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++) {
      // Load the Smoke effect Data
      FileMan_Read(hFile, &gSmokeEffectData[uiCnt], sizeof(SMOKEEFFECT), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(SMOKEEFFECT)) {
        return (FALSE);
      }
      // This is a TEMP hack to allow us to use the saves
      if (guiSaveGameVersion < 37) break;
    }

    // loop through and apply the smoke effects to the map
    for (uiCount = 0; uiCount < guiNumSmokeEffects; uiCount++) {
      // if this slot is allocated
      if (gSmokeEffectData[uiCount].fAllocated) {
        if (gSmokeEffectData[uiCount].bFlags & SMOKE_EFFECT_ON_ROOF) {
          bLevel = 1;
        } else {
          bLevel = 0;
        }

        SpreadEffect(gSmokeEffectData[uiCount].sGridNo, gSmokeEffectData[uiCount].ubRadius,
                     gSmokeEffectData[uiCount].usItem, gSmokeEffectData[uiCount].ubOwner, TRUE,
                     bLevel, uiCount);
      }
    }
  }

  return (TRUE);
}

BOOLEAN SaveSmokeEffectsToMapTempFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ) {
  uint32_t uiNumSmokeEffects = 0;
  HWFILE hFile;
  uint32_t uiNumBytesWritten = 0;
  char zMapName[128];
  uint32_t uiCnt;

  // get the name of the map
  GetMapTempFileName(SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ);

  // delete file the file.
  Plat_DeleteFile(zMapName);

  // loop through and count the number of smoke effects
  for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++) {
    if (gSmokeEffectData[uiCnt].fAllocated) uiNumSmokeEffects++;
  }

  // if there are no smoke effects
  if (uiNumSmokeEffects == 0) {
    // set the fact that there are no smoke effects for this sector
    ReSetSectorFlag(sMapX, sMapY, bMapZ, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS);

    return (TRUE);
  }

  // Open the file for writing
  hFile = FileMan_Open(zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);
  if (hFile == 0) {
    // Error opening file
    return (FALSE);
  }

  // Save the Number of Smoke Effects
  FileMan_Write(hFile, &uiNumSmokeEffects, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    // Close the file
    FileMan_Close(hFile);

    return (FALSE);
  }

  // loop through and save the number of smoke effects
  for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++) {
    // if the smoke is active
    if (gSmokeEffectData[uiCnt].fAllocated) {
      // Save the Smoke effect Data
      FileMan_Write(hFile, &gSmokeEffectData[uiCnt], sizeof(SMOKEEFFECT), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(SMOKEEFFECT)) {
        // Close the file
        FileMan_Close(hFile);

        return (FALSE);
      }
    }
  }

  // Close the file
  FileMan_Close(hFile);

  SetSectorFlag(sMapX, sMapY, bMapZ, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS);

  return (TRUE);
}

BOOLEAN LoadSmokeEffectsFromMapTempFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ) {
  uint32_t uiNumBytesRead;
  uint32_t uiCount;
  uint32_t uiCnt = 0;
  HWFILE hFile;
  char zMapName[128];
  int8_t bLevel;

  GetMapTempFileName(SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ);

  // Open the file for reading, Create it if it doesnt exist
  hFile = FileMan_Open(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Clear out the old list
  ResetSmokeEffects();

  // Load the Number of Smoke Effects
  FileMan_Read(hFile, &guiNumSmokeEffects, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // loop through and load the list
  for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++) {
    // Load the Smoke effect Data
    FileMan_Read(hFile, &gSmokeEffectData[uiCnt], sizeof(SMOKEEFFECT), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(SMOKEEFFECT)) {
      FileMan_Close(hFile);
      return (FALSE);
    }
  }

  // loop through and apply the smoke effects to the map
  for (uiCount = 0; uiCount < guiNumSmokeEffects; uiCount++) {
    // if this slot is allocated
    if (gSmokeEffectData[uiCount].fAllocated) {
      if (gSmokeEffectData[uiCount].bFlags & SMOKE_EFFECT_ON_ROOF) {
        bLevel = 1;
      } else {
        bLevel = 0;
      }

      SpreadEffect(gSmokeEffectData[uiCount].sGridNo, gSmokeEffectData[uiCount].ubRadius,
                   gSmokeEffectData[uiCount].usItem, gSmokeEffectData[uiCount].ubOwner, TRUE,
                   bLevel, uiCount);
    }
  }

  FileMan_Close(hFile);

  return (TRUE);
}

void ResetSmokeEffects() {
  // Clear out the old list
  memset(gSmokeEffectData, 0, sizeof(SMOKEEFFECT) * NUM_SMOKE_EFFECT_SLOTS);
  guiNumSmokeEffects = 0;
}

void UpdateSmokeEffectGraphics() {
  uint32_t uiCnt;
  SMOKEEFFECT *pSmoke;
  int8_t bLevel;

  // loop through and save the number of smoke effects
  for (uiCnt = 0; uiCnt < guiNumSmokeEffects; uiCnt++) {
    pSmoke = &gSmokeEffectData[uiCnt];

    // if the smoke is active
    if (gSmokeEffectData[uiCnt].fAllocated) {
      if (gSmokeEffectData[uiCnt].bFlags & SMOKE_EFFECT_ON_ROOF) {
        bLevel = 1;
      } else {
        bLevel = 0;
      }

      SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner,
                   ERASE_SPREAD_EFFECT, bLevel, uiCnt);

      SpreadEffect(pSmoke->sGridNo, pSmoke->ubRadius, pSmoke->usItem, pSmoke->ubOwner, TRUE, bLevel,
                   uiCnt);
    }
  }
}
