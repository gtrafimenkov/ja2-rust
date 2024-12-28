// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/InteractiveTiles.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "GameSettings.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/HImage.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleDoors.h"
#include "Tactical/HandleItems.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/Overhead.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/StructureWrap.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "rust_images.h"

#ifdef __GCC
// since some of the code is not complied on Linux
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#define MAX_INTTILE_STACK 10

typedef struct {
  int16_t sGridNo;
  uint8_t ubFlags;
  int16_t sTileIndex;
  int16_t sMaxScreenY;
  int16_t sHeighestScreenY;
  BOOLEAN fFound;
  struct LEVELNODE *pFoundNode;
  int16_t sFoundGridNo;
  uint16_t usStructureID;
  BOOLEAN fStructure;

} CUR_INTERACTIVE_TILE;

typedef struct {
  int8_t bNum;
  CUR_INTERACTIVE_TILE bTiles[MAX_INTTILE_STACK];
  int8_t bCur;

} INTERACTIVE_TILE_STACK_TYPE;

INTERACTIVE_TILE_STACK_TYPE gCurIntTileStack;
BOOLEAN gfCycleIntTile = FALSE;

CUR_INTERACTIVE_TILE gCurIntTile;
BOOLEAN gfOverIntTile = FALSE;

// Values to determine if we should check or not
int16_t gsINTOldRenderCenterX = 0;
int16_t gsINTOldRenderCenterY = 0;
uint16_t gusINTOldMousePosX = 0;
uint16_t gusINTOldMousePosY = 0;

BOOLEAN RefinePointCollisionOnStruct(int16_t sGridNo, int16_t sTestX, int16_t sTestY, int16_t sSrcX,
                                     int16_t sSrcY, struct LEVELNODE *pNode);
BOOLEAN CheckVideoObjectScreenCoordinateInData(struct VObject *hSrcVObject, uint16_t usIndex,
                                               int32_t iTextX, int32_t iTestY);
BOOLEAN RefineLogicOnStruct(int16_t sGridNo, struct LEVELNODE *pNode);

BOOLEAN InitInteractiveTileManagement() { return (TRUE); }

void ShutdownInteractiveTileManagement() {}

BOOLEAN AddInteractiveTile(int16_t sGridNo, struct LEVELNODE *pLevelNode, uint32_t uiFlags,
                           uint16_t usType) {
  return (TRUE);
}

BOOLEAN StartInteractiveObject(int16_t sGridNo, uint16_t usStructureID,
                               struct SOLDIERTYPE *pSoldier, uint8_t ubDirection) {
  struct STRUCTURE *pStructure;

  // ATE: Patch fix: Don't allow if alreay in animation
  if (pSoldier->usAnimState == OPEN_STRUCT || pSoldier->usAnimState == OPEN_STRUCT_CROUCHED ||
      pSoldier->usAnimState == BEGIN_OPENSTRUCT ||
      pSoldier->usAnimState == BEGIN_OPENSTRUCT_CROUCHED) {
    return (FALSE);
  }

  pStructure = FindStructureByID(sGridNo, usStructureID);
  if (pStructure == NULL) {
    return (FALSE);
  }
  if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
    // Add soldier event for opening door....
    pSoldier->ubPendingAction = MERC_OPENDOOR;
    pSoldier->uiPendingActionData1 = usStructureID;
    pSoldier->sPendingActionData2 = sGridNo;
    pSoldier->bPendingActionData3 = ubDirection;
    pSoldier->ubPendingActionAnimCount = 0;

  } else {
    // Add soldier event for opening door....
    pSoldier->ubPendingAction = MERC_OPENSTRUCT;
    pSoldier->uiPendingActionData1 = usStructureID;
    pSoldier->sPendingActionData2 = sGridNo;
    pSoldier->bPendingActionData3 = ubDirection;
    pSoldier->ubPendingActionAnimCount = 0;
  }

  return (TRUE);
}

BOOLEAN CalcInteractiveObjectAPs(int16_t sGridNo, struct STRUCTURE *pStructure, int16_t *psAPCost,
                                 int16_t *psBPCost) {
  if (pStructure == NULL) {
    return (FALSE);
  }
  if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
    // For doors, if open, we can safely add APs for closing
    // If closed, we do not know what to do yet...
    // if ( pStructure->fFlags & STRUCTURE_OPEN )
    //{
    *psAPCost = AP_OPEN_DOOR;
    *psBPCost = AP_OPEN_DOOR;
    //}
    // else
    //{
    //	*psAPCost = 0;
    //	*psBPCost = 0;
    //}
  } else {
    *psAPCost = AP_OPEN_DOOR;
    *psBPCost = AP_OPEN_DOOR;
  }

  return (TRUE);
}

BOOLEAN InteractWithInteractiveObject(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                      uint8_t ubDirection) {
  BOOLEAN fDoor = FALSE;

  if (pStructure == NULL) {
    return (FALSE);
  }

  if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
    fDoor = TRUE;
  }

  InteractWithOpenableStruct(pSoldier, pStructure, ubDirection, fDoor);

  return (TRUE);
}

BOOLEAN SoldierHandleInteractiveObject(struct SOLDIERTYPE *pSoldier) {
  struct STRUCTURE *pStructure;
  uint16_t usStructureID;
  int16_t sGridNo;

  sGridNo = pSoldier->sPendingActionData2;
  usStructureID = (uint16_t)pSoldier->uiPendingActionData1;

  // HANDLE SOLDIER ACTIONS
  pStructure = FindStructureByID(sGridNo, usStructureID);
  if (pStructure == NULL) {
    // DEBUG MSG!
    return (FALSE);
  }

  return (HandleOpenableStruct(pSoldier, sGridNo, pStructure));
}

void HandleStructChangeFromGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  struct STRUCTURE *pStructure, *pNewStructure;
  int16_t sAPCost = 0, sBPCost = 0;
  struct ITEM_POOL *pItemPool;
  BOOLEAN fDidMissingQuote = FALSE;

  pStructure = FindStructure(sGridNo, STRUCTURE_OPENABLE);

  if (pStructure == NULL) {
#ifdef JA2TESTVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
              L"ERROR: Told to handle struct that does not exist at %d.", sGridNo);
#endif
    return;
  }

  // Do sound...
  if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
    // Play Opening sound...
    PlayJA2Sample(GetStructureOpenSound(pStructure, FALSE), RATE_11025,
                  SoundVolume(HIGHVOLUME, sGridNo), 1, SoundDir(sGridNo));
  } else {
    // Play Opening sound...
    PlayJA2Sample((GetStructureOpenSound(pStructure, TRUE)), RATE_11025,
                  SoundVolume(HIGHVOLUME, sGridNo), 1, SoundDir(sGridNo));
  }

  // ATE: Don't handle switches!
  if (!(pStructure->fFlags & STRUCTURE_SWITCH)) {
    if (pSoldier->bTeam == gbPlayerNum) {
      if (sGridNo == BOBBYR_SHIPPING_DEST_GRIDNO &&
          gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X &&
          gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y &&
          gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z &&
          CheckFact(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT, 0) &&
          !(CheckFact(FACT_PLAYER_FOUND_ITEMS_MISSING, 0))) {
        SayQuoteFromNearbyMercInSector(BOBBYR_SHIPPING_DEST_GRIDNO, 3, QUOTE_STUFF_MISSING_DRASSEN);
        fDidMissingQuote = TRUE;
      }
    } else if (pSoldier->bTeam == CIV_TEAM) {
      if (GetSolProfile(pSoldier) != NO_PROFILE) {
        TriggerNPCWithGivenApproach(GetSolProfile(pSoldier), APPROACH_DONE_OPEN_STRUCTURE, FALSE);
      }
    }

    // LOOK for item pool here...
    if (GetItemPool((int16_t)sGridNo, &pItemPool, pSoldier->bLevel)) {
      // Update visiblity....
      if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
        BOOLEAN fDoHumm = TRUE;
        BOOLEAN fDoLocators = TRUE;

        if (pSoldier->bTeam != gbPlayerNum) {
          fDoHumm = FALSE;
          fDoLocators = FALSE;
        }

        // Look for ownership here....
        if (gWorldItems[pItemPool->iItemIndex].o.usItem == OWNERSHIP) {
          fDoHumm = FALSE;
          TacticalCharacterDialogueWithSpecialEvent(
              pSoldier, 0, DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND, BATTLE_SOUND_NOTHING, 500);
        }

        // If now open, set visible...
        SetItemPoolVisibilityOn(pItemPool, ANY_VISIBILITY_VALUE, fDoLocators);

        // Display quote!
        // TacticalCharacterDialogue( pSoldier, (uint16_t)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2
        // ) ) );

        // ATE: Check now many things in pool.....
        if (!fDidMissingQuote) {
          if (pItemPool->pNext != NULL) {
            if (pItemPool->pNext->pNext != NULL) {
              fDoHumm = FALSE;

              TacticalCharacterDialogueWithSpecialEvent(
                  pSoldier, 0, DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND, BATTLE_SOUND_COOL1, 500);
            }
          }

          if (fDoHumm) {
            TacticalCharacterDialogueWithSpecialEvent(
                pSoldier, 0, DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND, BATTLE_SOUND_HUMM, 500);
          }
        }
      } else {
        SetItemPoolVisibilityHidden(pItemPool);
      }
    } else {
      if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
        TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND,
                                                  BATTLE_SOUND_NOTHING, 500);
      }
    }
  }

  // Deduct points!
  // CalcInteractiveObjectAPs( sGridNo, pStructure, &sAPCost, &sBPCost );
  // DeductPoints( pSoldier, sAPCost, sBPCost );

  pNewStructure = SwapStructureForPartner(sGridNo, pStructure);
  if (pNewStructure != NULL) {
    RecompileLocalMovementCosts(sGridNo);
    SetRenderFlags(RENDER_FLAG_FULL);
    if (pNewStructure->fFlags & STRUCTURE_SWITCH) {
      // just turned a switch on!
      ActivateSwitchInGridNo(pSoldier->ubID, sGridNo);
    }
  }
}

uint32_t GetInteractiveTileCursor(uint32_t uiOldCursor, BOOLEAN fConfirm) {
  struct LEVELNODE *pIntNode;
  struct STRUCTURE *pStructure;
  int16_t sGridNo;

  // OK, first see if we have an in tile...
  pIntNode = GetCurInteractiveTileGridNoAndStructure(&sGridNo, &pStructure);

  if (pIntNode != NULL && pStructure != NULL) {
    if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
      SetDoorString(sGridNo);

      if (fConfirm) {
        return (OKHANDCURSOR_UICURSOR);
      } else {
        return (NORMALHANDCURSOR_UICURSOR);
      }

    } else {
      if (pStructure->fFlags & STRUCTURE_SWITCH) {
        wcscpy(gzIntTileLocation, gzLateLocalizedString[25]);
        gfUIIntTileLocation = TRUE;
      }

      if (fConfirm) {
        return (OKHANDCURSOR_UICURSOR);
      } else {
        return (NORMALHANDCURSOR_UICURSOR);
      }
    }
  }

  return (uiOldCursor);
}

void SetActionModeDoorCursorText() {
  struct LEVELNODE *pIntNode;
  struct STRUCTURE *pStructure;
  int16_t sGridNo;

  // If we are over a merc, don't
  if (gfUIFullTargetFound) {
    return;
  }

  // OK, first see if we have an in tile...
  pIntNode = GetCurInteractiveTileGridNoAndStructure(&sGridNo, &pStructure);

  if (pIntNode != NULL && pStructure != NULL) {
    if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
      SetDoorString(sGridNo);
    }
  }
}

void GetLevelNodeScreenRect(struct LEVELNODE *pNode, struct GRect *pRect, int16_t sXPos,
                            int16_t sYPos, int16_t sGridNo) {
  int16_t sScreenX, sScreenY;
  int16_t sOffsetX, sOffsetY;
  int16_t sTempX_S, sTempY_S;
  struct Subimage *pTrav;
  uint32_t usHeight, usWidth;
  TILE_ELEMENT *TileElem;

  // Get 'TRUE' merc position
  sOffsetX = sXPos - gsRenderCenterX;
  sOffsetY = sYPos - gsRenderCenterY;

  FromCellToScreenCoordinates(sOffsetX, sOffsetY, &sTempX_S, &sTempY_S);

  if (pNode->uiFlags & LEVELNODE_CACHEDANITILE) {
    pTrav = &(gpTileCache[pNode->pAniTile->sCachedTileID]
                  .pImagery->vo->subimages[pNode->pAniTile->sCurrentFrame]);
  } else {
    TileElem = &(gTileDatabase[pNode->usIndex]);

    // Adjust for current frames and animations....
    if (TileElem->uiFlags & ANIMATED_TILE) {
      Assert(TileElem->pAnimData != NULL);
      TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[TileElem->pAnimData->bCurrentFrame]];
    } else if ((pNode->uiFlags & LEVELNODE_ANIMATION)) {
      if (pNode->sCurrentFrame != -1) {
        Assert(TileElem->pAnimData != NULL);
        TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
      }
    }

    pTrav = &(TileElem->hTileSurface->subimages[TileElem->usRegionIndex]);
  }

  sScreenX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)sTempX_S;
  sScreenY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)sTempY_S;

  // Adjust for offset position on screen
  sScreenX -= gsRenderWorldOffsetX;
  sScreenY -= gsRenderWorldOffsetY;
  sScreenY -= gpWorldLevelData[sGridNo].sHeight;

  // Adjust based on interface level
  if (gsInterfaceLevel > 0) {
    sScreenY += ROOF_LEVEL_HEIGHT;
  }

  // Adjust for render height
  sScreenY += gsRenderHeight;

  usHeight = (uint32_t)pTrav->height;
  usWidth = (uint32_t)pTrav->width;

  // Add to start position of dest buffer
  sScreenX += (pTrav->x_offset - (WORLD_TILE_X / 2));
  sScreenY += (pTrav->y_offset - (WORLD_TILE_Y / 2));

  // Adjust y offset!
  sScreenY += (WORLD_TILE_Y / 2);

  pRect->iLeft = sScreenX;
  pRect->iTop = sScreenY;
  pRect->iBottom = sScreenY + usHeight;
  pRect->iRight = sScreenX + usWidth;
}

void CompileInteractiveTiles() {}

void LogMouseOverInteractiveTile(int16_t sGridNo) {
  struct GRect aRect;
  int16_t sXMapPos, sYMapPos, sScreenX, sScreenY;
  struct LEVELNODE *pNode;

  // OK, for now, don't allow any interactive tiles on higher interface level!
  if (gsInterfaceLevel > 0) {
    return;
  }

  // Also, don't allow for mercs who are on upper level...
  if (gusSelectedSoldier != NOBODY && MercPtrs[gusSelectedSoldier]->bLevel == 1) {
    return;
  }

  // Get World XY From gridno
  ConvertGridNoToCellXY(sGridNo, &sXMapPos, &sYMapPos);

  // Set mouse stuff
  sScreenX = gusMouseXPos;
  sScreenY = gusMouseYPos;

  pNode = gpWorldLevelData[sGridNo].pStructHead;

  while (pNode != NULL) {
    {
      GetLevelNodeScreenRect(pNode, &aRect, sXMapPos, sYMapPos, sGridNo);

      // Make sure we are always on guy if we are on same gridno
      if (IsPointInScreenRect(sScreenX, sScreenY, &aRect)) {
        // OK refine it!
        if (RefinePointCollisionOnStruct(sGridNo, sScreenX, sScreenY, (int16_t)aRect.iLeft,
                                         (int16_t)aRect.iBottom, pNode)) {
          // Do some additional checks here!
          if (RefineLogicOnStruct(sGridNo, pNode)) {
            gCurIntTile.fFound = TRUE;

            // Only if we are not currently cycling....
            if (!gfCycleIntTile) {
              // Accumulate them!
              gCurIntTileStack.bTiles[gCurIntTileStack.bNum].pFoundNode = pNode;
              gCurIntTileStack.bTiles[gCurIntTileStack.bNum].sFoundGridNo = sGridNo;
              gCurIntTileStack.bNum++;

              // Determine if it's the best one
              if (aRect.iBottom > gCurIntTile.sHeighestScreenY) {
                gCurIntTile.sMaxScreenY = (uint16_t)aRect.iBottom;
                gCurIntTile.sHeighestScreenY = gCurIntTile.sMaxScreenY;

                // Set it!
                gCurIntTile.pFoundNode = pNode;
                gCurIntTile.sFoundGridNo = sGridNo;

                // Set stack current one...
                gCurIntTileStack.bCur = gCurIntTileStack.bNum - 1;
              }
            }
          }
        }
      }

      pNode = pNode->pNext;
    }
  }
}

struct LEVELNODE *InternalGetCurInteractiveTile(BOOLEAN fRejectItemsOnTop) {
  struct LEVELNODE *pNode = NULL;
  struct STRUCTURE *pStructure = NULL;

  // OK, Look for our tile!

  // Check for shift down!
  if (_KeyDown(SHIFT)) {
    return (NULL);
  }

  if (gfOverIntTile) {
    pNode = gpWorldLevelData[gCurIntTile.sGridNo].pStructHead;

    while (pNode != NULL) {
      if (pNode->usIndex == gCurIntTile.sTileIndex) {
        if (fRejectItemsOnTop) {
          // get strucuture here...
          if (gCurIntTile.fStructure) {
            pStructure = FindStructureByID(gCurIntTile.sGridNo, gCurIntTile.usStructureID);
            if (pStructure != NULL) {
              if (pStructure->fFlags & STRUCTURE_HASITEMONTOP) {
                return (NULL);
              }
            } else {
              return (NULL);
            }
          }
        }

        return (pNode);
      }

      pNode = pNode->pNext;
    }
  }

  return (NULL);
}

struct LEVELNODE *GetCurInteractiveTile() { return (InternalGetCurInteractiveTile(TRUE)); }

struct LEVELNODE *GetCurInteractiveTileGridNo(int16_t *psGridNo) {
  struct LEVELNODE *pNode;

  pNode = GetCurInteractiveTile();

  if (pNode != NULL) {
    *psGridNo = gCurIntTile.sGridNo;
  } else {
    *psGridNo = NOWHERE;
  }

  return (pNode);
}

struct LEVELNODE *ConditionalGetCurInteractiveTileGridNoAndStructure(int16_t *psGridNo,
                                                                     struct STRUCTURE **ppStructure,
                                                                     BOOLEAN fRejectOnTopItems) {
  struct LEVELNODE *pNode;
  struct STRUCTURE *pStructure;

  *ppStructure = NULL;

  pNode = InternalGetCurInteractiveTile(fRejectOnTopItems);

  if (pNode != NULL) {
    *psGridNo = gCurIntTile.sGridNo;
  } else {
    *psGridNo = NOWHERE;
  }

  if (pNode != NULL) {
    if (gCurIntTile.fStructure) {
      pStructure = FindStructureByID(gCurIntTile.sGridNo, gCurIntTile.usStructureID);
      if (pStructure == NULL) {
        *ppStructure = NULL;
        return (NULL);
      } else {
        *ppStructure = pStructure;
      }
    }
  }

  return (pNode);
}

struct LEVELNODE *GetCurInteractiveTileGridNoAndStructure(int16_t *psGridNo,
                                                          struct STRUCTURE **ppStructure) {
  return (ConditionalGetCurInteractiveTileGridNoAndStructure(psGridNo, ppStructure, TRUE));
}

void BeginCurInteractiveTileCheck(uint8_t bCheckFlags) {
  gfOverIntTile = FALSE;

  // OK, release our stack, stuff could be different!
  gfCycleIntTile = FALSE;

  // Reset some highest values
  gCurIntTile.sHeighestScreenY = 0;
  gCurIntTile.fFound = FALSE;
  gCurIntTile.ubFlags = bCheckFlags;

  // Reset stack values
  gCurIntTileStack.bNum = 0;
}

void EndCurInteractiveTileCheck() {
  CUR_INTERACTIVE_TILE *pCurIntTile;

  if (gCurIntTile.fFound) {
    // Set our currently cycled guy.....
    if (gfCycleIntTile) {
      // OK, we're over this cycled node
      pCurIntTile = &(gCurIntTileStack.bTiles[gCurIntTileStack.bCur]);
    } else {
      // OK, we're over this levelnode,
      pCurIntTile = &gCurIntTile;
    }

    gCurIntTile.sGridNo = pCurIntTile->sFoundGridNo;
    gCurIntTile.sTileIndex = pCurIntTile->pFoundNode->usIndex;

    if (pCurIntTile->pFoundNode->pStructureData != NULL) {
      gCurIntTile.usStructureID = pCurIntTile->pFoundNode->pStructureData->usStructureID;
      gCurIntTile.fStructure = TRUE;
    } else {
      gCurIntTile.fStructure = FALSE;
    }

    gfOverIntTile = TRUE;

  } else {
    // If we are in cycle mode, end it
    if (gfCycleIntTile) {
      gfCycleIntTile = FALSE;
    }
  }
}

BOOLEAN RefineLogicOnStruct(int16_t sGridNo, struct LEVELNODE *pNode) {
  TILE_ELEMENT *TileElem;
  struct STRUCTURE *pStructure;

  if (pNode->uiFlags & LEVELNODE_CACHEDANITILE) {
    return (FALSE);
  }

  TileElem = &(gTileDatabase[pNode->usIndex]);

  if (gCurIntTile.ubFlags == INTILE_CHECK_SELECTIVE) {
    // See if we are on an interactable tile!
    // Try and get struct data from levelnode pointer
    pStructure = pNode->pStructureData;

    // If no data, quit
    if (pStructure == NULL) {
      return (FALSE);
    }

    if (!(pStructure->fFlags & (STRUCTURE_OPENABLE | STRUCTURE_HASITEMONTOP))) {
      return (FALSE);
    }

    if (gusSelectedSoldier != NOBODY && MercPtrs[gusSelectedSoldier]->ubBodyType == ROBOTNOWEAPON) {
      return (FALSE);
    }

    // If we are a door, we need a different definition of being visible than other structs
    if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
      if (!IsDoorVisibleAtGridNo(sGridNo)) {
        return (FALSE);
      }

      // OK, For a OPENED door, addition requirements are: need to be in 'HAND CURSOR' mode...
      if (pStructure->fFlags & STRUCTURE_OPEN) {
        // Are we in hand cursor mode?
        if (gCurrentUIMode != HANDCURSOR_MODE && gCurrentUIMode != ACTION_MODE) {
          return (FALSE);
        }
      }

      // If this option is on...
      if (!gGameSettings.fOptions[TOPTION_SNAP_CURSOR_TO_DOOR]) {
        if (gCurrentUIMode != HANDCURSOR_MODE) {
          return (FALSE);
        }
      }
    } else {
      // IF we are a switch, reject in another direction...
      if (pStructure->fFlags & STRUCTURE_SWITCH) {
        // Find a new gridno based on switch's orientation...
        int16_t sNewGridNo = NOWHERE;

        switch (pStructure->pDBStructureRef->pDBStructure->ubWallOrientation) {
          case OUTSIDE_TOP_LEFT:
          case INSIDE_TOP_LEFT:

            // Move south...
            sNewGridNo = NewGridNo(sGridNo, DirectionInc(SOUTH));
            break;

          case OUTSIDE_TOP_RIGHT:
          case INSIDE_TOP_RIGHT:

            // Move east...
            sNewGridNo = NewGridNo(sGridNo, DirectionInc(EAST));
            break;
        }

        if (sNewGridNo != NOWHERE) {
          // If we are hidden by a roof, reject it!
          if (!gfBasement && IsRoofVisible2(sNewGridNo) &&
              !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS)) {
            return (FALSE);
          }
        }
      } else {
        // If we are hidden by a roof, reject it!
        if (!gfBasement && IsRoofVisible(sGridNo) && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS)) {
          return (FALSE);
        }
      }
    }

    // Check if it's a hidden struct and we have not revealed anything!
    if (TileElem->uiFlags & HIDDEN_TILE) {
      if (!IsHiddenStructureVisible(sGridNo, pNode->usIndex)) {
        // Return false
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN RefinePointCollisionOnStruct(int16_t sGridNo, int16_t sTestX, int16_t sTestY, int16_t sSrcX,
                                     int16_t sSrcY, struct LEVELNODE *pNode) {
  TILE_ELEMENT *TileElem;

  if (pNode->uiFlags & LEVELNODE_CACHEDANITILE) {
    // Check it!
    return (CheckVideoObjectScreenCoordinateInData(
        gpTileCache[pNode->pAniTile->sCachedTileID].pImagery->vo, pNode->pAniTile->sCurrentFrame,
        (int32_t)(sTestX - sSrcX), (int32_t)(-1 * (sTestY - sSrcY))));

  } else {
    TileElem = &(gTileDatabase[pNode->usIndex]);

    // Adjust for current frames and animations....
    if (TileElem->uiFlags & ANIMATED_TILE) {
      Assert(TileElem->pAnimData != NULL);
      TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[TileElem->pAnimData->bCurrentFrame]];
    } else if ((pNode->uiFlags & LEVELNODE_ANIMATION)) {
      if (pNode->sCurrentFrame != -1) {
        Assert(TileElem->pAnimData != NULL);
        TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
      }
    }

    // Check it!
    return (CheckVideoObjectScreenCoordinateInData(TileElem->hTileSurface, TileElem->usRegionIndex,
                                                   (int32_t)(sTestX - sSrcX),
                                                   (int32_t)(-1 * (sTestY - sSrcY))));
  }
}

// This function will check the video object at SrcX and SrcY for the lack of transparency
// will return true if data found, else false
BOOLEAN CheckVideoObjectScreenCoordinateInData(struct VObject *hSrcVObject, uint16_t usIndex,
                                               int32_t iTestX, int32_t iTestY) {
  uint32_t uiOffset;
  uint32_t usHeight, usWidth;
  uint8_t *SrcPtr;
  uint32_t LineSkip;
  struct Subimage *pTrav;
  BOOLEAN fDataFound = FALSE;
  int32_t iTestPos, iStartPos;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (uint32_t)pTrav->height;
  usWidth = (uint32_t)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Calculate test position we are looking for!
  // Calculate from 0, 0 at top left!
  iTestPos = ((usHeight - iTestY) * usWidth) + iTestX;
  iStartPos = 0;
  LineSkip = usWidth;

  SrcPtr = (uint8_t *)hSrcVObject->image_data + uiOffset;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, iStartPos
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		inc		esi

            // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1


BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		add		esi, 2

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1


BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		add		esi, 4

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

    // Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

        // Here check if we have passed!
		cmp		edi, iTestPos
		jge		BlitDone

		dec		usHeight
		jz		BlitDone
            //		add		edi, LineSkip
		jmp		BlitDispatch


BlitFound:

		mov		fDataFound, 1

BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (fDataFound);
}

BOOLEAN ShouldCheckForMouseDetections() {
  BOOLEAN fOK = FALSE;

  if (gsINTOldRenderCenterX != gsRenderCenterX || gsINTOldRenderCenterY != gsRenderCenterY ||
      gusINTOldMousePosX != gusMouseXPos || gusINTOldMousePosY != gusMouseYPos) {
    fOK = TRUE;
  }

  // Set old values
  gsINTOldRenderCenterX = gsRenderCenterX;
  gsINTOldRenderCenterY = gsRenderCenterY;

  gusINTOldMousePosX = gusMouseXPos;
  gusINTOldMousePosY = gusMouseYPos;

  return (fOK);
}

void CycleIntTileFindStack(uint16_t usMapPos) {
  gfCycleIntTile = TRUE;

  // Cycle around!
  gCurIntTileStack.bCur++;

  // PLot new movement
  gfPlotNewMovement = TRUE;

  if (gCurIntTileStack.bCur == gCurIntTileStack.bNum) {
    gCurIntTileStack.bCur = 0;
  }
}

#ifdef __GCC
#pragma GCC diagnostic pop
#endif
