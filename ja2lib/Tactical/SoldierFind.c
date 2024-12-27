#include "Tactical/SoldierFind.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Editor/EditorMercs.h"
#include "GameSettings.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/FOV.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Items.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierTile.h"
#include "Tactical/UICursors.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/WorldMan.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/TimerControl.h"

BOOLEAN IsGridNoInScreenRect(int16_t sGridNo, SGPRect *pRect);
BOOLEAN IsPointInScreenRect(int16_t sXPos, int16_t sYPos, SGPRect *pRect);
void GetSoldierScreenRect(struct SOLDIERTYPE *pSoldier, SGPRect *pRect);

// This value is used to keep a small static array of uBID's which are stacked
#define MAX_STACKED_MERCS 10

uint32_t gScrollSlideInertiaDirection[NUM_WORLD_DIRECTIONS] = {
    3, 0, 0, 0, 0, 0, 3, 3,
};

// Struct used for cycling through multiple mercs per mouse position
typedef struct {
  int8_t bNum;
  uint8_t ubIDs[MAX_STACKED_MERCS];
  int8_t bCur;
  BOOLEAN fUseGridNo;
  uint16_t sUseGridNoGridNo;

} SOLDIER_STACK_TYPE;

SOLDIER_STACK_TYPE gSoldierStack;
BOOLEAN gfHandleStack = FALSE;

extern BOOLEAN gUIActionModeChangeDueToMouseOver;
extern uint32_t guiUITargetSoldierId;

BOOLEAN FindSoldierFromMouse(uint16_t *pusSoldierIndex, uint32_t *pMercFlags) {
  int16_t sMapPos;

  *pMercFlags = 0;

  if (GetMouseMapPos(&sMapPos)) {
    if (FindSoldier(sMapPos, pusSoldierIndex, pMercFlags, FINDSOLDIERSAMELEVEL(gsInterfaceLevel))) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN SelectiveFindSoldierFromMouse(uint16_t *pusSoldierIndex, uint32_t *pMercFlags) {
  int16_t sMapPos;

  *pMercFlags = 0;

  if (GetMouseMapPos(&sMapPos)) {
    if (FindSoldier(sMapPos, pusSoldierIndex, pMercFlags, FINDSOLDIERSAMELEVEL(gsInterfaceLevel))) {
      return (TRUE);
    }
  }

  return (FALSE);
}

uint32_t GetSoldierFindFlags(uint16_t ubID) {
  uint32_t MercFlags = 0;
  struct SOLDIERTYPE *pSoldier;

  // Get pSoldier!
  pSoldier = MercPtrs[ubID];

  // FInd out and set flags
  if (ubID == gusSelectedSoldier) {
    MercFlags |= SELECTED_MERC;
  }
  if (ubID >= gTacticalStatus.Team[gbPlayerNum].bFirstID &&
      ubID <= gTacticalStatus.Team[gbPlayerNum].bLastID) {
    if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !GetNumberInVehicle(pSoldier->bVehicleID)) {
      // Don't do anything!
    } else {
      // It's our own merc
      MercFlags |= OWNED_MERC;

      if (pSoldier->bAssignment < ON_DUTY) {
        MercFlags |= ONDUTY_MERC;
      }
    }
  } else {
    // Check the side, etc
    if (!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum)) {
      // It's an enemy merc
      MercFlags |= ENEMY_MERC;
    } else {
      // It's not an enemy merc
      MercFlags |= NEUTRAL_MERC;
    }
  }

  // Check for a guy who does not have an iterrupt ( when applicable! )
  if (!OK_INTERRUPT_MERC(pSoldier)) {
    MercFlags |= NOINTERRUPT_MERC;
  }

  if (pSoldier->bLife < OKLIFE) {
    MercFlags |= UNCONSCIOUS_MERC;
  }

  if (pSoldier->bLife == 0) {
    MercFlags |= DEAD_MERC;
  }

  if (pSoldier->bVisible != -1 || (gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
    MercFlags |= VISIBLE_MERC;
  }

  return (MercFlags);
}

extern BOOLEAN CheckVideoObjectScreenCoordinateInData(struct VObject *hSrcVObject, uint16_t usIndex,
                                                      int32_t iTextX, int32_t iTestY);

// THIS FUNCTION IS CALLED FAIRLY REGULARLY
BOOLEAN FindSoldier(int16_t sGridNo, uint16_t *pusSoldierIndex, uint32_t *pMercFlags,
                    uint32_t uiFlags) {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  SGPRect aRect;
  BOOLEAN fSoldierFound = FALSE;
  int16_t sXMapPos, sYMapPos, sScreenX, sScreenY;
  int16_t sMaxScreenMercY, sHeighestMercScreenY = -32000;
  BOOLEAN fDoFull;
  uint8_t ubBestMerc = NOBODY;
  uint16_t usAnimSurface;
  int32_t iMercScreenX, iMercScreenY;
  BOOLEAN fInScreenRect = FALSE;
  BOOLEAN fInGridNo = FALSE;

  *pusSoldierIndex = NOBODY;
  *pMercFlags = 0;

  if (_KeyDown(SHIFT)) {
    uiFlags = FIND_SOLDIER_GRIDNO;
  }

  // Set some values
  if (uiFlags & FIND_SOLDIER_BEGINSTACK) {
    gSoldierStack.bNum = 0;
    gSoldierStack.fUseGridNo = FALSE;
  }

  // Loop through all mercs and make go
  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    pSoldier = MercSlots[cnt];
    fInScreenRect = FALSE;
    fInGridNo = FALSE;

    if (pSoldier != NULL) {
      if (IsSolActive(pSoldier) && !(pSoldier->uiStatusFlags & SOLDIER_DEAD) &&
          (pSoldier->bVisible != -1 || (gTacticalStatus.uiFlags & SHOW_ALL_MERCS))) {
        // OK, ignore if we are a passenger...
        if (pSoldier->uiStatusFlags & (SOLDIER_PASSENGER | SOLDIER_DRIVER)) {
          continue;
        }

        // If we want same level, skip if buggy's not on the same level!
        if (uiFlags & FIND_SOLDIER_SAMELEVEL) {
          if (pSoldier->bLevel != (uint8_t)(uiFlags >> 16)) {
            continue;
          }
        }

        // If we are selective.... do our own guys FULL and other with gridno!
        // First look for owned soldiers, by way of the full method
        if (uiFlags & FIND_SOLDIER_GRIDNO) {
          fDoFull = FALSE;
        } else if (uiFlags & FIND_SOLDIER_SELECTIVE) {
          if (pSoldier->ubID >= gTacticalStatus.Team[gbPlayerNum].bFirstID &&
              GetSolID(pSoldier) <= gTacticalStatus.Team[gbPlayerNum].bLastID) {
            fDoFull = TRUE;
          } else {
            fDoFull = FALSE;
          }
        } else {
          fDoFull = TRUE;
        }

        if (fDoFull) {
          // Get Rect contained in the soldier
          GetSoldierScreenRect(pSoldier, &aRect);

          // Get XY From gridno
          ConvertGridNoToXY(sGridNo, &sXMapPos, &sYMapPos);

          // Get screen XY pos from map XY
          // Be carefull to convert to cell cords
          // CellXYToScreenXY( (int16_t)((sXMapPos*CELL_X_SIZE)), (int16_t)((sYMapPos*CELL_Y_SIZE)),
          // &sScreenX, &sScreenY);

          // Set mouse stuff
          sScreenX = gusMouseXPos;
          sScreenY = gusMouseYPos;

          if (IsPointInScreenRect(sScreenX, sScreenY, &aRect)) {
            fInScreenRect = TRUE;
          }

          if (pSoldier->sGridNo == sGridNo) {
            fInGridNo = TRUE;
          }

          // ATE: If we are an enemy....
          if (!gGameSettings.fOptions[TOPTION_SMART_CURSOR]) {
            if (pSoldier->ubID >= gTacticalStatus.Team[gbPlayerNum].bFirstID &&
                GetSolID(pSoldier) <= gTacticalStatus.Team[gbPlayerNum].bLastID) {
              // ATE: NOT if we are in action or comfirm action mode
              if ((gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE) ||
                  gUIActionModeChangeDueToMouseOver) {
                fInScreenRect = FALSE;
              }
            }
          }

          // ATE: Refine this further....
          // Check if this is the selected guy....
          if (pSoldier->ubID == gusSelectedSoldier) {
            // Are we in action mode...
            if (gCurrentUIMode == ACTION_MODE || gCurrentUIMode == CONFIRM_ACTION_MODE) {
              // Are we in medic mode?
              if (GetActionModeCursor(pSoldier) != AIDCURS) {
                fInScreenRect = FALSE;
                fInGridNo = FALSE;
              }
            }
          }

          // Make sure we are always on guy if we are on same gridno
          if (fInScreenRect || fInGridNo) {
            // Check if we are a vehicle and refine if so....
            if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
              usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

              if (usAnimSurface != INVALID_ANIMATION_SURFACE) {
                iMercScreenX = (int32_t)(sScreenX - aRect.iLeft);
                iMercScreenY = (int32_t)(-1 * (sScreenY - aRect.iBottom));

                if (!CheckVideoObjectScreenCoordinateInData(
                        gAnimSurfaceDatabase[usAnimSurface].hVideoObject, pSoldier->usAniFrame,
                        iMercScreenX, iMercScreenY)) {
                  continue;
                }
              }
            }

            // If thgis is from a gridno, use mouse pos!
            if (pSoldier->sGridNo == sGridNo) {
            }

            // Only break here if we're not creating a stack of these fellas
            if (uiFlags & FIND_SOLDIER_BEGINSTACK) {
              gfHandleStack = TRUE;

              // Add this one!
              gSoldierStack.ubIDs[gSoldierStack.bNum] = GetSolID(pSoldier);
              gSoldierStack.bNum++;

              // Determine if it's the current
              if (aRect.iBottom > sHeighestMercScreenY) {
                sMaxScreenMercY = (uint16_t)aRect.iBottom;
                sHeighestMercScreenY = sMaxScreenMercY;

                gSoldierStack.bCur = gSoldierStack.bNum - 1;
              }
            }
            // Are we handling a stack right now?
            else if (gfHandleStack) {
              // Are we the selected stack?
              if (gSoldierStack.fUseGridNo) {
                fSoldierFound = FALSE;
                break;
              } else if (gSoldierStack.ubIDs[gSoldierStack.bCur] == GetSolID(pSoldier)) {
                // Set it!
                ubBestMerc = GetSolID(pSoldier);

                fSoldierFound = TRUE;
                break;
              }
            } else {
              // Determine if it's the best one
              if (aRect.iBottom > sHeighestMercScreenY) {
                sMaxScreenMercY = (uint16_t)aRect.iBottom;
                sHeighestMercScreenY = sMaxScreenMercY;

                // Set it!
                ubBestMerc = GetSolID(pSoldier);
              }

              fSoldierFound = TRUE;
              // Don't break here, find the rest!
            }
          }

        } else {
          // Otherwise, look for a bad guy by way of gridno]
          // Selective means don't give out enemy mercs if they are not visible

          ///&& !NewOKDestination( pSoldier, sGridNo, TRUE, (int8_t)gsInterfaceLevel )
          if (pSoldier->sGridNo == sGridNo &&
              !NewOKDestination(pSoldier, sGridNo, TRUE, (int8_t)gsInterfaceLevel)) {
            // Set it!
            ubBestMerc = GetSolID(pSoldier);

            fSoldierFound = TRUE;
            break;
          }
        }
      }
    }
  }

  if (fSoldierFound && ubBestMerc != NOBODY) {
    *pusSoldierIndex = (uint16_t)ubBestMerc;

    (*pMercFlags) = GetSoldierFindFlags(ubBestMerc);

    return (TRUE);

  } else {
    // If we were handling a stack, and we have not found anybody, end
    if (gfHandleStack && !(uiFlags & (FIND_SOLDIER_BEGINSTACK | FIND_SOLDIER_SELECTIVE))) {
      if (gSoldierStack.fUseGridNo) {
        if (gSoldierStack.sUseGridNoGridNo != sGridNo) {
          gfHandleStack = FALSE;
        }
      } else {
        gfHandleStack = FALSE;
      }
    }
  }
  return (FALSE);
}

BOOLEAN CycleSoldierFindStack(uint16_t usMapPos) {
  uint16_t usSoldierIndex;
  uint32_t uiMercFlags;

  // Have we initalized for this yet?
  if (!gfHandleStack) {
    if (FindSoldier(usMapPos, &usSoldierIndex, &uiMercFlags,
                    FINDSOLDIERSAMELEVEL(gsInterfaceLevel) | FIND_SOLDIER_BEGINSTACK)) {
      gfHandleStack = TRUE;
    }
  }

  if (gfHandleStack) {
    // we are cycling now?
    if (!gSoldierStack.fUseGridNo) {
      gSoldierStack.bCur++;
    }

    gfUIForceReExamineCursorData = TRUE;

    if (gSoldierStack.bCur == gSoldierStack.bNum) {
      if (!gSoldierStack.fUseGridNo) {
        gSoldierStack.fUseGridNo = TRUE;
        gUIActionModeChangeDueToMouseOver = FALSE;
        gSoldierStack.sUseGridNoGridNo = usMapPos;
      } else {
        gSoldierStack.bCur = 0;
        gSoldierStack.fUseGridNo = FALSE;
      }
    }

    if (!gSoldierStack.fUseGridNo) {
      gusUIFullTargetID = gSoldierStack.ubIDs[gSoldierStack.bCur];
      guiUIFullTargetFlags = GetSoldierFindFlags(gusUIFullTargetID);
      guiUITargetSoldierId = gusUIFullTargetID;
      gfUIFullTargetFound = TRUE;
    } else {
      gfUIFullTargetFound = FALSE;
    }
  }

  // Return if we are in the cycle mode now...
  return (gfHandleStack);
}

struct SOLDIERTYPE *SimpleFindSoldier(int16_t sGridNo, int8_t bLevel) {
  uint8_t ubID;

  ubID = WhoIsThere2(sGridNo, bLevel);
  if (ubID == NOBODY) {
    return (NULL);
  } else {
    return (MercPtrs[ubID]);
  }
}

BOOLEAN IsValidTargetMerc(uint8_t ubSoldierID) {
  struct SOLDIERTYPE *pSoldier = MercPtrs[ubSoldierID];

  // CHECK IF ACTIVE!
  if (!IsSolActive(pSoldier)) {
    return (FALSE);
  }

  // CHECK IF DEAD
  if (pSoldier->bLife == 0) {
    // return( FALSE );
  }

  // IF BAD GUY - CHECK VISIVILITY
  if (pSoldier->bTeam != gbPlayerNum) {
    if (pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN IsGridNoInScreenRect(int16_t sGridNo, SGPRect *pRect) {
  int32_t iXTrav, iYTrav;
  int16_t sMapPos;

  // Start with top left corner
  iXTrav = pRect->iLeft;
  iYTrav = pRect->iTop;

  do {
    do {
      GetScreenXYGridNo((int16_t)iXTrav, (int16_t)iYTrav, &sMapPos);

      if (sMapPos == sGridNo) {
        return (TRUE);
      }

      iXTrav += WORLD_TILE_X;

    } while (iXTrav < pRect->iRight);

    iYTrav += WORLD_TILE_Y;
    iXTrav = pRect->iLeft;

  } while (iYTrav < pRect->iBottom);

  return (FALSE);
}

void GetSoldierScreenRect(struct SOLDIERTYPE *pSoldier, SGPRect *pRect) {
  int16_t sMercScreenX, sMercScreenY;
  uint16_t usAnimSurface;
  //		ETRLEObject *pTrav;
  //		uint32_t usHeight, usWidth;

  GetSoldierScreenPos(pSoldier, &sMercScreenX, &sMercScreenY);

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);
  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    pRect->iLeft = sMercScreenX;
    pRect->iTop = sMercScreenY;
    pRect->iBottom = sMercScreenY + 5;
    pRect->iRight = sMercScreenX + 5;

    return;
  }

  // pTrav = &(gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->pETRLEObject[
  // pSoldier->usAniFrame ] ); usHeight				= (uint32_t)pTrav->usHeight; usWidth
  // = (uint32_t)pTrav->usWidth;

  pRect->iLeft = sMercScreenX;
  pRect->iTop = sMercScreenY;
  pRect->iBottom = sMercScreenY + pSoldier->sBoundingBoxHeight;
  pRect->iRight = sMercScreenX + pSoldier->sBoundingBoxWidth;
}

void GetSoldierAnimDims(struct SOLDIERTYPE *pSoldier, int16_t *psHeight, int16_t *psWidth) {
  uint16_t usAnimSurface;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *psHeight = (int16_t)5;
    *psWidth = (int16_t)5;

    return;
  }

  // OK, noodle here on what we should do... If we take each frame, it will be different slightly
  // depending on the frame and the value returned here will vary thusly. However, for the
  // uses of this function, we should be able to use just the first frame...

  *psHeight = (int16_t)pSoldier->sBoundingBoxHeight;
  *psWidth = (int16_t)pSoldier->sBoundingBoxWidth;
}

void GetSoldierAnimOffsets(struct SOLDIERTYPE *pSoldier, int16_t *sOffsetX, int16_t *sOffsetY) {
  uint16_t usAnimSurface;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *sOffsetX = (int16_t)0;
    *sOffsetY = (int16_t)0;

    return;
  }

  *sOffsetX = (int16_t)pSoldier->sBoundingBoxOffsetX;
  *sOffsetY = (int16_t)pSoldier->sBoundingBoxOffsetY;
}

void GetSoldierScreenPos(struct SOLDIERTYPE *pSoldier, int16_t *psScreenX, int16_t *psScreenY) {
  int16_t sMercScreenX, sMercScreenY;
  FLOAT dOffsetX, dOffsetY;
  FLOAT dTempX_S, dTempY_S;
  uint16_t usAnimSurface;
  //		ETRLEObject *pTrav;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *psScreenX = 0;
    *psScreenY = 0;
    return;
  }

  // Get 'TRUE' merc position
  dOffsetX = pSoldier->dXPos - gsRenderCenterX;
  dOffsetY = pSoldier->dYPos - gsRenderCenterY;

  FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

  // pTrav = &(gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->pETRLEObject[
  // pSoldier->usAniFrame ] );

  sMercScreenX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
  sMercScreenY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S;

  // Adjust starting screen coordinates
  sMercScreenX -= gsRenderWorldOffsetX;
  sMercScreenY -= gsRenderWorldOffsetY;
  sMercScreenY -= gpWorldLevelData[pSoldier->sGridNo].sHeight;

  // Adjust for render height
  sMercScreenY += gsRenderHeight;

  // Add to start position of dest buffer
  // sMercScreenX += pTrav->sOffsetX;
  // sMercScreenY += pTrav->sOffsetY;
  sMercScreenX += pSoldier->sBoundingBoxOffsetX;
  sMercScreenY += pSoldier->sBoundingBoxOffsetY;

  sMercScreenY -= pSoldier->sHeightAdjustment;

  *psScreenX = sMercScreenX;
  *psScreenY = sMercScreenY;
}

// THE TRUE SCREN RECT DOES NOT TAKE THE OFFSETS OF BUDDY INTO ACCOUNT!
void GetSoldierTRUEScreenPos(struct SOLDIERTYPE *pSoldier, int16_t *psScreenX, int16_t *psScreenY) {
  int16_t sMercScreenX, sMercScreenY;
  FLOAT dOffsetX, dOffsetY;
  FLOAT dTempX_S, dTempY_S;
  uint16_t usAnimSurface;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *psScreenX = 0;
    *psScreenY = 0;
    return;
  }

  // Get 'TRUE' merc position
  dOffsetX = pSoldier->dXPos - gsRenderCenterX;
  dOffsetY = pSoldier->dYPos - gsRenderCenterY;

  FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

  sMercScreenX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
  sMercScreenY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S;

  // Adjust starting screen coordinates
  sMercScreenX -= gsRenderWorldOffsetX;
  sMercScreenY -= gsRenderWorldOffsetY;

  // Adjust for render height
  sMercScreenY += gsRenderHeight;
  sMercScreenY -= gpWorldLevelData[pSoldier->sGridNo].sHeight;

  sMercScreenY -= pSoldier->sHeightAdjustment;

  *psScreenX = sMercScreenX;
  *psScreenY = sMercScreenY;
}

BOOLEAN GridNoOnScreen(int16_t sGridNo) {
  int16_t sNewCenterWorldX, sNewCenterWorldY;
  int16_t sWorldX;
  int16_t sWorldY;
  int16_t sAllowance = 20;

  if (gsVIEWPORT_WINDOW_START_Y == 20) {
    sAllowance = 40;
  }

  ConvertGridNoToXY(sGridNo, &sNewCenterWorldX, &sNewCenterWorldY);

  // Get screen coordinates for current position of soldier
  GetWorldXYAbsoluteScreenXY((int16_t)(sNewCenterWorldX), (int16_t)(sNewCenterWorldY), &sWorldX,
                             &sWorldY);

  // ATE: OK, here, adjust the top value so that it's a tile and a bit over, because of our mercs!
  if (sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX &&
      sWorldY >= (gsTopLeftWorldY + sAllowance) && sWorldY <= (gsBottomRightWorldY + 20)) {
    return (TRUE);
  }
  return (FALSE);
}

BOOLEAN SoldierOnScreen(uint16_t usID) {
  struct SOLDIERTYPE *pSoldier;

  // Get pointer of soldier
  pSoldier = MercPtrs[usID];

  return (GridNoOnScreen(pSoldier->sGridNo));
}

BOOLEAN SoldierOnVisibleWorldTile(struct SOLDIERTYPE *pSoldier) {
  return (GridNoOnVisibleWorldTile(pSoldier->sGridNo));
}

BOOLEAN SoldierLocationRelativeToScreen(int16_t sGridNo, uint16_t usReasonID, int8_t *pbDirection,
                                        uint32_t *puiScrollFlags) {
  int16_t sWorldX;
  int16_t sWorldY;
  int16_t sY, sX;
  static BOOLEAN fCountdown = 0;
  int16_t sScreenCenterX, sScreenCenterY;
  int16_t sDistToCenterY, sDistToCenterX;

  *puiScrollFlags = 0;

  sX = CenterX(sGridNo);
  sY = CenterY(sGridNo);

  // Get screen coordinates for current position of soldier
  GetWorldXYAbsoluteScreenXY((int16_t)(sX / CELL_X_SIZE), (int16_t)(sY / CELL_Y_SIZE), &sWorldX,
                             &sWorldY);

  // Find the diustance from render center to true world center
  sDistToCenterX = gsRenderCenterX - gCenterWorldX;
  sDistToCenterY = gsRenderCenterY - gCenterWorldY;

  // From render center in world coords, convert to render center in "screen" coords
  FromCellToScreenCoordinates(sDistToCenterX, sDistToCenterY, &sScreenCenterX, &sScreenCenterY);

  // Subtract screen center
  sScreenCenterX += gsCX;
  sScreenCenterY += gsCY;

  // Adjust for offset origin!
  sScreenCenterX += 0;
  sScreenCenterY += 10;

  // Get direction
  //*pbDirection = atan8( sScreenCenterX, sScreenCenterY, sWorldX, sWorldY );
  *pbDirection = atan8(gsRenderCenterX, gsRenderCenterY, (int16_t)(sX), (int16_t)(sY));

  // Check values!
  if (sWorldX > (sScreenCenterX + 20)) {
    (*puiScrollFlags) |= SCROLL_RIGHT;
  }
  if (sWorldX < (sScreenCenterX - 20)) {
    (*puiScrollFlags) |= SCROLL_LEFT;
  }
  if (sWorldY > (sScreenCenterY + 20)) {
    (*puiScrollFlags) |= SCROLL_DOWN;
  }
  if (sWorldY < (sScreenCenterY - 20)) {
    (*puiScrollFlags) |= SCROLL_UP;
  }

  // If we are on screen, stop
  if (sWorldX >= gsTopLeftWorldX && sWorldX <= gsBottomRightWorldX && sWorldY >= gsTopLeftWorldY &&
      sWorldY <= (gsBottomRightWorldY + 20)) {
    // CHECK IF WE ARE DONE...
    if (fCountdown > gScrollSlideInertiaDirection[*pbDirection]) {
      fCountdown = 0;
      return (FALSE);
    } else {
      fCountdown++;
    }
  }

  return (TRUE);
}

BOOLEAN IsPointInSoldierBoundingBox(struct SOLDIERTYPE *pSoldier, int16_t sX, int16_t sY) {
  SGPRect aRect;

  // Get Rect contained in the soldier
  GetSoldierScreenRect(pSoldier, &aRect);

  if (IsPointInScreenRect(sX, sY, &aRect)) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN FindRelativeSoldierPosition(struct SOLDIERTYPE *pSoldier, uint16_t *usFlags, int16_t sX,
                                    int16_t sY) {
  SGPRect aRect;
  int16_t sRelX, sRelY;
  FLOAT dRelPer;

  // Get Rect contained in the soldier
  GetSoldierScreenRect(pSoldier, &aRect);

  if (IsPointInScreenRectWithRelative(sX, sY, &aRect, &sRelX, &sRelY)) {
    dRelPer = (FLOAT)sRelY / (aRect.iBottom - aRect.iTop);

    // Determine relative positions
    switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
      case ANIM_STAND:

        if (dRelPer < .2) {
          (*usFlags) = TILE_FLAG_HEAD;
          return (TRUE);
        } else if (dRelPer < .6) {
          (*usFlags) = TILE_FLAG_MID;
          return (TRUE);
        } else {
          (*usFlags) = TILE_FLAG_FEET;
          return (TRUE);
        }
        break;

      case ANIM_CROUCH:

        if (dRelPer < .2) {
          (*usFlags) = TILE_FLAG_HEAD;
          return (TRUE);
        } else if (dRelPer < .7) {
          (*usFlags) = TILE_FLAG_MID;
          return (TRUE);
        } else {
          (*usFlags) = TILE_FLAG_FEET;
          return (TRUE);
        }
        break;
    }
  }

  return (FALSE);
}

// VERY quickly finds a soldier at gridno , ( that is visible )
uint8_t QuickFindSoldier(int16_t sGridNo) {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier = NULL;

  // Loop through all mercs and make go
  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    pSoldier = MercSlots[cnt];

    if (pSoldier != NULL) {
      if (pSoldier->sGridNo == sGridNo && pSoldier->bVisible != -1) {
        return ((uint8_t)cnt);
      }
    }
  }

  return (NOBODY);
}

void GetGridNoScreenPos(int16_t sGridNo, uint8_t ubLevel, int16_t *psScreenX, int16_t *psScreenY) {
  int16_t sScreenX, sScreenY;
  FLOAT dOffsetX, dOffsetY;
  FLOAT dTempX_S, dTempY_S;

  // Get 'TRUE' merc position
  dOffsetX = (FLOAT)(CenterX(sGridNo) - gsRenderCenterX);
  dOffsetY = (FLOAT)(CenterY(sGridNo) - gsRenderCenterY);

  // OK, DONT'T ASK... CONVERSION TO PROPER Y NEEDS THIS...
  dOffsetX -= CELL_Y_SIZE;

  FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

  sScreenX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
  sScreenY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S;

  // Adjust starting screen coordinates
  sScreenX -= gsRenderWorldOffsetX;
  sScreenY -= gsRenderWorldOffsetY;

  sScreenY += gsRenderHeight;

  // Adjust for world height
  sScreenY -= gpWorldLevelData[sGridNo].sHeight;

  // Adjust for level height
  if (ubLevel) {
    sScreenY -= ROOF_LEVEL_HEIGHT;
  }

  *psScreenX = sScreenX;
  *psScreenY = sScreenY;
}
