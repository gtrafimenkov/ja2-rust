#include "Tactical/Bullets.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "GameSettings.h"
#include "SGP/Container.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Tactical/Faces.h"
#include "Tactical/Gap.h"
#include "Tactical/LOS.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Utilities.h"
#include "rust_fileman.h"

// Defines
#define NUM_BULLET_SLOTS 50

// GLOBAL FOR FACES LISTING
BULLET gBullets[NUM_BULLET_SLOTS];
UINT32 guiNumBullets = 0;

INT32 GetFreeBullet(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBullets; uiCount++) {
    if ((gBullets[uiCount].fAllocated == FALSE)) return ((INT32)uiCount);
  }

  if (guiNumBullets < NUM_BULLET_SLOTS) return ((INT32)guiNumBullets++);

  return (-1);
}

void RecountBullets(void) {
  INT32 uiCount;

  for (uiCount = guiNumBullets - 1; (uiCount >= 0); uiCount--) {
    if ((gBullets[uiCount].fAllocated)) {
      guiNumBullets = (UINT32)(uiCount + 1);
      return;
    }
  }
  guiNumBullets = 0;
}

INT32 CreateBullet(UINT8 ubFirerID, BOOLEAN fFake, UINT16 usFlags) {
  INT32 iBulletIndex;
  BULLET *pBullet;

  if ((iBulletIndex = GetFreeBullet()) == (-1)) return (-1);

  memset(&gBullets[iBulletIndex], 0, sizeof(BULLET));

  pBullet = &gBullets[iBulletIndex];

  pBullet->iBullet = iBulletIndex;
  pBullet->fAllocated = TRUE;
  pBullet->fLocated = FALSE;
  pBullet->ubFirerID = ubFirerID;
  pBullet->usFlags = usFlags;
  pBullet->usLastStructureHit = 0;

  if (fFake) {
    pBullet->fReal = FALSE;
  } else {
    pBullet->fReal = TRUE;
  }

  return (iBulletIndex);
}

void HandleBulletSpecialFlags(INT32 iBulletIndex) {
  BULLET *pBullet;
  ANITILE_PARAMS AniParams;
  FLOAT dX, dY;
  UINT8 ubDirection;

  pBullet = &(gBullets[iBulletIndex]);

  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));

  if (pBullet->fReal) {
    // Create ani tile if this is a spit!
    if (pBullet->usFlags & (BULLET_FLAG_KNIFE)) {
      AniParams.sGridNo = (INT16)pBullet->sGridNo;
      AniParams.ubLevelID = ANI_STRUCT_LEVEL;
      AniParams.sDelay = 100;
      AniParams.sStartFrame = 3;
      AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_LOOPING |
                          ANITILE_USE_DIRECTION_FOR_START_FRAME;
      AniParams.sX = FIXEDPT_TO_INT32(pBullet->qCurrX);
      AniParams.sY = FIXEDPT_TO_INT32(pBullet->qCurrY);
      AniParams.sZ = CONVERT_HEIGHTUNITS_TO_PIXELS(FIXEDPT_TO_INT32(pBullet->qCurrZ));

      if (pBullet->usFlags & (BULLET_FLAG_CREATURE_SPIT)) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\SPIT2.STI");
      } else if (pBullet->usFlags & (BULLET_FLAG_KNIFE)) {
        strcpy(AniParams.zCachedFile, "TILECACHE\\KNIFING.STI");
        pBullet->ubItemStatus = pBullet->pFirer->inv[HANDPOS].bStatus[0];
      }

      // Get direction to use for this guy....
      dX = ((FLOAT)(pBullet->qIncrX) / FIXEDPT_FRACTIONAL_RESOLUTION);
      dY = ((FLOAT)(pBullet->qIncrY) / FIXEDPT_FRACTIONAL_RESOLUTION);

      ubDirection = atan8(0, 0, (INT16)(dX * 100), (INT16)(dY * 100));

      AniParams.uiUserData3 = ubDirection;

      pBullet->pAniTile = CreateAnimationTile(&AniParams);

      // IF we are anything that needs a shadow.. set it here....
      if (pBullet->usFlags & (BULLET_FLAG_KNIFE)) {
        AniParams.ubLevelID = ANI_SHADOW_LEVEL;
        AniParams.sZ = 0;
        pBullet->pShadowAniTile = CreateAnimationTile(&AniParams);
      }
    }
  }
}

void RemoveBullet(INT32 iBullet) {
  CHECKV(iBullet < NUM_BULLET_SLOTS);

  // decrease soldier's bullet count

  if (gBullets[iBullet].fReal) {
    // set to be deleted at next update
    gBullets[iBullet].fToDelete = TRUE;

    // decrement reference to bullet in the firer
    gBullets[iBullet].pFirer->bBulletsLeft--;
    DebugMsg(
        TOPIC_JA2, DBG_INFO,
        String("!!!!!!! Ending bullet, bullets left %d", gBullets[iBullet].pFirer->bBulletsLeft));

    if (gBullets[iBullet].usFlags & (BULLET_FLAG_KNIFE)) {
      // Delete ani tile
      if (gBullets[iBullet].pAniTile != NULL) {
        DeleteAniTile(gBullets[iBullet].pAniTile);
        gBullets[iBullet].pAniTile = NULL;
      }

      // Delete shadow
      if (gBullets[iBullet].usFlags & (BULLET_FLAG_KNIFE)) {
        if (gBullets[iBullet].pShadowAniTile != NULL) {
          DeleteAniTile(gBullets[iBullet].pShadowAniTile);
          gBullets[iBullet].pShadowAniTile = NULL;
        }
      }
    }
  } else {
    // delete this fake bullet right away!
    gBullets[iBullet].fAllocated = FALSE;
    RecountBullets();
  }
}

void LocateBullet(INT32 iBulletIndex) {
  if (gGameSettings.fOptions[TOPTION_SHOW_MISSES]) {
    // Check if a bad guy fired!
    if (gBullets[iBulletIndex].ubFirerID != NOBODY) {
      if (MercPtrs[gBullets[iBulletIndex].ubFirerID]->bSide == gbPlayerNum) {
        if (!gBullets[iBulletIndex].fLocated) {
          gBullets[iBulletIndex].fLocated = TRUE;

          // Only if we are in turnbased and noncombat
          if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
            LocateGridNo((INT16)gBullets[iBulletIndex].sGridNo);
          }
        }
      }
    }
  }
}

void UpdateBullets() {
  UINT32 uiCount;
  struct LEVELNODE *pNode;
  BOOLEAN fDeletedSome = FALSE;

  for (uiCount = 0; uiCount < guiNumBullets; uiCount++) {
    if (gBullets[uiCount].fAllocated) {
      if (gBullets[uiCount].fReal && !(gBullets[uiCount].usFlags & BULLET_STOPPED)) {
        // there are duplicate checks for deletion in case the bullet is deleted by shooting
        // someone at point blank range, in the first MoveBullet call in the FireGun code
        if (gBullets[uiCount].fToDelete) {
          // Remove from old position
          gBullets[uiCount].fAllocated = FALSE;
          fDeletedSome = TRUE;
          continue;
        }

        // if ( !( gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ) )
        {
          // ALRIGHTY, CHECK WHAT TYPE OF BULLET WE ARE

          if (gBullets[uiCount].usFlags &
              (BULLET_FLAG_CREATURE_SPIT | BULLET_FLAG_KNIFE | BULLET_FLAG_MISSILE |
               BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON | BULLET_FLAG_FLAME)) {
          } else {
            RemoveStruct(gBullets[uiCount].sGridNo, BULLETTILE1);
            RemoveStruct(gBullets[uiCount].sGridNo, BULLETTILE2);
          }
        }

        MoveBullet(uiCount);
        if (gBullets[uiCount].fToDelete) {
          // Remove from old position
          gBullets[uiCount].fAllocated = FALSE;
          fDeletedSome = TRUE;
          continue;
        }

        if (gBullets[uiCount].usFlags & BULLET_STOPPED) {
          continue;
        }

        // Display bullet
        // if ( !( gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ) )
        {
          if (gBullets[uiCount].usFlags & (BULLET_FLAG_KNIFE)) {
            if (gBullets[uiCount].pAniTile != NULL) {
              gBullets[uiCount].pAniTile->sRelativeX =
                  (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrX);
              gBullets[uiCount].pAniTile->sRelativeY =
                  (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrY);
              gBullets[uiCount].pAniTile->pLevelNode->sRelativeZ =
                  (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS(FIXEDPT_TO_INT32(gBullets[uiCount].qCurrZ));

              if (gBullets[uiCount].usFlags & (BULLET_FLAG_KNIFE)) {
                gBullets[uiCount].pShadowAniTile->sRelativeX =
                    (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrX);
                gBullets[uiCount].pShadowAniTile->sRelativeY =
                    (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrY);
              }
            }
          }
          // Are we a missle?
          else if (gBullets[uiCount].usFlags &
                   (BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON |
                    BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT)) {
          } else {
            pNode = AddStructToTail(gBullets[uiCount].sGridNo, BULLETTILE1);
            pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
            pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
            pNode->uiFlags |= (LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT);
            pNode->sRelativeX = (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrX);
            pNode->sRelativeY = (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrY);
            pNode->sRelativeZ =
                (INT16)CONVERT_HEIGHTUNITS_TO_PIXELS(FIXEDPT_TO_INT32(gBullets[uiCount].qCurrZ));

            // Display shadow
            pNode = AddStructToTail(gBullets[uiCount].sGridNo, BULLETTILE2);
            pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
            pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
            pNode->uiFlags |= (LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT);
            pNode->sRelativeX = (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrX);
            pNode->sRelativeY = (INT16)FIXEDPT_TO_INT32(gBullets[uiCount].qCurrY);
            pNode->sRelativeZ = (INT16)gpWorldLevelData[gBullets[uiCount].sGridNo].sHeight;
          }
        }
      } else {
        if (gBullets[uiCount].fToDelete) {
          gBullets[uiCount].fAllocated = FALSE;
          fDeletedSome = TRUE;
        }
      }
    }
  }

  if (fDeletedSome) {
    RecountBullets();
  }
}

BULLET *GetBulletPtr(INT32 iBullet) {
  BULLET *pBullet;

  CHECKN(iBullet < NUM_BULLET_SLOTS);

  pBullet = &gBullets[iBullet];

  return (pBullet);
}

void AddMissileTrail(BULLET *pBullet, FIXEDPT qCurrX, FIXEDPT qCurrY, FIXEDPT qCurrZ) {
  ANITILE_PARAMS AniParams;

  // If we are a small missle, don't show
  if (pBullet->usFlags &
      (BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT)) {
    if (pBullet->iLoop < 5) {
      return;
    }
  }

  // If we are a small missle, don't show
  if (pBullet->usFlags & (BULLET_FLAG_TANK_CANNON)) {
    // if ( pBullet->iLoop < 40 )
    //{
    return;
    //}
  }

  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
  AniParams.sGridNo = (INT16)pBullet->sGridNo;
  AniParams.ubLevelID = ANI_STRUCT_LEVEL;
  AniParams.sDelay = (INT16)(100 + Random(100));
  AniParams.sStartFrame = 0;
  AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
  AniParams.sX = FIXEDPT_TO_INT32(qCurrX);
  AniParams.sY = FIXEDPT_TO_INT32(qCurrY);
  AniParams.sZ = CONVERT_HEIGHTUNITS_TO_PIXELS(FIXEDPT_TO_INT32(qCurrZ));

  if (pBullet->usFlags & (BULLET_FLAG_MISSILE | BULLET_FLAG_TANK_CANNON)) {
    strcpy(AniParams.zCachedFile, "TILECACHE\\MSLE_SMK.STI");
  } else if (pBullet->usFlags & (BULLET_FLAG_SMALL_MISSILE)) {
    strcpy(AniParams.zCachedFile, "TILECACHE\\MSLE_SMA.STI");
  } else if (pBullet->usFlags & (BULLET_FLAG_CREATURE_SPIT)) {
    strcpy(AniParams.zCachedFile, "TILECACHE\\MSLE_SPT.STI");
  } else if (pBullet->usFlags & (BULLET_FLAG_FLAME)) {
    strcpy(AniParams.zCachedFile, "TILECACHE\\FLMTHR2.STI");
    AniParams.sDelay = (INT16)(100);
  }

  CreateAnimationTile(&AniParams);
}

BOOLEAN SaveBulletStructureToSaveGameFile(FileID hFile) {
  UINT32 uiNumBytesWritten;
  UINT16 usCnt;
  UINT32 uiBulletCount = 0;

  // loop through and count the number of bullets
  for (usCnt = 0; usCnt < NUM_BULLET_SLOTS; usCnt++) {
    // if the bullet is active, save it
    if (gBullets[usCnt].fAllocated) {
      uiBulletCount++;
    }
  }

  // Save the number of Bullets in the array
  File_Write(hFile, &uiBulletCount, sizeof(UINT32), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(UINT32)) {
    return (FALSE);
  }

  if (uiBulletCount != 0) {
    for (usCnt = 0; usCnt < NUM_BULLET_SLOTS; usCnt++) {
      // if the bullet is active, save it
      if (gBullets[usCnt].fAllocated) {
        // Save the the Bullet structure
        File_Write(hFile, &gBullets[usCnt], sizeof(BULLET), &uiNumBytesWritten);
        if (uiNumBytesWritten != sizeof(BULLET)) {
          return (FALSE);
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadBulletStructureFromSavedGameFile(FileID hFile) {
  UINT32 uiNumBytesRead;
  UINT16 usCnt;

  // make sure the bullets are not allocated
  memset(gBullets, 0, NUM_BULLET_SLOTS * sizeof(BULLET));

  // Load the number of Bullets in the array
  File_Read(hFile, &guiNumBullets, sizeof(UINT32), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(UINT32)) {
    return (FALSE);
  }

  for (usCnt = 0; usCnt < guiNumBullets; usCnt++) {
    // Load the the Bullet structure
    File_Read(hFile, &gBullets[usCnt], sizeof(BULLET), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(BULLET)) {
      return (FALSE);
    }

    // Set some parameters
    gBullets[usCnt].uiLastUpdate = 0;
    if (gBullets[usCnt].ubFirerID != NOBODY)
      gBullets[usCnt].pFirer = GetSoldierByID(gBullets[usCnt].ubFirerID);
    else
      gBullets[usCnt].pFirer = NULL;

    gBullets[usCnt].pAniTile = NULL;
    gBullets[usCnt].pShadowAniTile = NULL;
    gBullets[usCnt].iBullet = usCnt;

    HandleBulletSpecialFlags(gBullets[usCnt].iBullet);
  }

  return (TRUE);
}

void StopBullet(INT32 iBullet) {
  gBullets[iBullet].usFlags |= BULLET_STOPPED;

  RemoveStruct(gBullets[iBullet].sGridNo, BULLETTILE1);
  RemoveStruct(gBullets[iBullet].sGridNo, BULLETTILE2);
}

void DeleteAllBullets() {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < guiNumBullets; uiCount++) {
    if (gBullets[uiCount].fAllocated) {
      // Remove from old position
      RemoveBullet(uiCount);
      gBullets[uiCount].fAllocated = FALSE;
    }
  }

  RecountBullets();
}
