// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/InterfaceControl.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "GameLoop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/HImage.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterface.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleUI.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/SpreadBurst.h"
#include "Tactical/Squads.h"
#include "Tactical/StrategicExitGUI.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/PopUpBox.h"
#include "Utils/SoundControl.h"
#include "Utils/Utilities.h"
#include "rust_colors.h"

#define CLOCK_X 554
#define CLOCK_Y 459
struct GRect gOldClippingRect, gOldDirtyClippingRect;

uint32_t guiTacticalInterfaceFlags;

uint16_t gusUICurIntTileEffectIndex;
int16_t gsUICurIntTileEffectGridNo;
uint8_t gsUICurIntTileOldShade;

BOOLEAN gfRerenderInterfaceFromHelpText = FALSE;

struct MOUSE_REGION gLockPanelOverlayRegion;

extern void RenderTownIDString();
extern BOOLEAN gfUIOverItemPool;
extern int16_t gfUIOverItemPoolGridNo;
extern BOOLEAN gfInMovementMenu;
extern BOOLEAN gfInItemPickupMenu;
extern BOOLEAN gfInOpenDoorMenu;
extern uint32_t guiUIMessageTimeDelay;

BOOLEAN gfPausedTacticalRenderInterfaceFlags = FALSE;
BOOLEAN gfPausedTacticalRenderFlags = FALSE;

// which assignment menu can be shown?
extern void DetermineWhichAssignmentMenusCanBeShown(void);
extern void HandleAnyMercInSquadHasCompatibleStuff(uint8_t ubSquad, struct OBJECTTYPE *pObject,
                                                   BOOLEAN fReset);
extern BOOLEAN RemoveFlashItemSlot(struct ITEM_POOL *pItemPool);

void SetTacticalInterfaceFlags(uint32_t uiFlags) { guiTacticalInterfaceFlags = uiFlags; }

void HandleTacticalPanelSwitch() {
  if (gfSwitchPanel) {
    SetCurrentInterfacePanel(gbNewPanel);
    SetCurrentTacticalPanelCurrentMerc(gubNewPanelParam);
    gfSwitchPanel = FALSE;

    if (!(guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS) &&
        !(guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE)) {
      RenderButtons();
    }
  }
}

void RenderTacticalInterface() {
  // handle paused render of tactical
  HandlePausedTacticalRender();

  if (!(IsMapScreen())) {
    HandleFlashingItems();

    HandleMultiPurposeLocator();
  }

  // Handle degrading new items...
  DegradeNewlyAddedItems();

  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      RenderSMPanel(&fInterfacePanelDirty);
      break;

    case TEAM_PANEL:
      RenderTEAMPanel(fInterfacePanelDirty);
      break;
  }

  // Handle faces
  if (!(guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE)) HandleAutoFaces();
}

void HandlePausedTacticalRender(void) {
  // for a one frame paused render of tactical
  if (gfPausedTacticalRenderFlags) {
    gRenderFlags |= gfPausedTacticalRenderFlags;
    gfPausedTacticalRenderFlags = FALSE;
  }

  if (gfPausedTacticalRenderInterfaceFlags) {
    fInterfacePanelDirty = gfPausedTacticalRenderInterfaceFlags;
    gfPausedTacticalRenderInterfaceFlags = FALSE;
  }

  return;
}

void RenderTacticalInterfaceWhileScrolling() {
  RenderButtons();

  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      RenderSMPanel(&fInterfacePanelDirty);
      break;

    case TEAM_PANEL:
      RenderTEAMPanel(fInterfacePanelDirty);
      break;
  }

  // Handle faces
  HandleAutoFaces();
}

void SetUpInterface() {
  struct SOLDIERTYPE *pSoldier;
  struct LEVELNODE *pIntTile;

  if ((IsMapScreen())) {
    return;
  }

  DrawUICursor();

  SetupPhysicsTrajectoryUI();

  if (giUIMessageOverlay != -1) {
    if ((GetJA2Clock() - guiUIMessageTime) > guiUIMessageTimeDelay) {
      EndUIMessage();
    }
  }

  if (gusSelectedSoldier != NO_SOLDIER) {
    GetSoldier(&pSoldier, gusSelectedSoldier);
  }

  if (gCurrentUIMode == OPENDOOR_MENU_MODE) {
    HandleOpenDoorMenu();
  }

  HandleTalkingMenu();

  if (gCurrentUIMode == EXITSECTORMENU_MODE) {
    HandleSectorExitMenu();
  }

  // FOR THE MOST PART - SHUTDOWN INTERFACE WHEN IT'S THE ENEMY'S TURN
  if (gTacticalStatus.ubCurrentTeam != gbPlayerNum) {
    return;
  }

  HandleInterfaceBackgrounds();

  if (gfUIHandleSelection == NONSELECTED_GUY_SELECTION) {
    if (gsSelectedLevel > 0) {
      AddRoofToHead(gsSelectedGridNo, GOODRING1);
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

    } else {
      AddObjectToHead(gsSelectedGridNo, GOODRING1);
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
    }
  }

  if (gfUIHandleSelection == SELECTED_GUY_SELECTION) {
    if (gsSelectedLevel > 0) {
      // AddRoofToHead( gsSelectedGridNo, SELRING1 );
      AddRoofToHead(gsSelectedGridNo, FIRSTPOINTERS2);
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

    } else {
      // AddObjectToHead( gsSelectedGridNo, SELRING1 );
      AddObjectToHead(gsSelectedGridNo, FIRSTPOINTERS2);
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
    }
  }

  if (gfUIHandleSelection == ENEMY_GUY_SELECTION) {
    if (gsSelectedLevel > 0) {
      AddRoofToHead(gsSelectedGridNo, FIRSTPOINTERS2);
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

    } else {
      AddObjectToHead(gsSelectedGridNo, FIRSTPOINTERS2);
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
    }
  }

  if (gfUIHandleShowMoveGrid) {
    if (gusSelectedSoldier != NOBODY) {
      if (MercPtrs[gusSelectedSoldier]->sGridNo != gsUIHandleShowMoveGridLocation) {
        if (gfUIHandleShowMoveGrid == 2) {
          AddTopmostToHead(gsUIHandleShowMoveGridLocation, GetSnapCursorIndex(FIRSTPOINTERS4));
          gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel =
              DEFAULT_SHADE_LEVEL;
          gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel =
              DEFAULT_SHADE_LEVEL;
        } else {
          if (MercPtrs[gusSelectedSoldier]->bStealthMode) {
            AddTopmostToHead(gsUIHandleShowMoveGridLocation, GetSnapCursorIndex(FIRSTPOINTERS9));
            gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel =
                DEFAULT_SHADE_LEVEL;
            gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel =
                DEFAULT_SHADE_LEVEL;
          } else {
            AddTopmostToHead(gsUIHandleShowMoveGridLocation, GetSnapCursorIndex(FIRSTPOINTERS2));
            gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel =
                DEFAULT_SHADE_LEVEL;
            gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel =
                DEFAULT_SHADE_LEVEL;
          }
        }
      }
    }
  }

  // Check if we are over an interactive tile...
  if (gfUIShowCurIntTile) {
    pIntTile = GetCurInteractiveTileGridNo(&gsUICurIntTileEffectGridNo);

    if (pIntTile != NULL) {
      gusUICurIntTileEffectIndex = pIntTile->usIndex;

      // Shade green
      gsUICurIntTileOldShade = pIntTile->ubShadeLevel;
      pIntTile->ubShadeLevel = 0;
      pIntTile->uiFlags |= LEVELNODE_DYNAMIC;
    }
  }
}

void ResetInterface() {
  struct LEVELNODE *pNode;

  if ((IsMapScreen())) {
    return;
  }

  // find out if we need to show any menus
  DetermineWhichAssignmentMenusCanBeShown();
  CreateDestroyAssignmentPopUpBoxes();

  HideUICursor();

  ResetPhysicsTrajectoryUI();

  if (gfUIHandleSelection) {
    if (gsSelectedLevel > 0) {
      RemoveRoof(gsSelectedGridNo, GOODRING1);
      RemoveRoof(gsSelectedGridNo, FIRSTPOINTERS2);
    } else {
      RemoveObject(gsSelectedGridNo, FIRSTPOINTERS2);
      RemoveObject(gsSelectedGridNo, GOODRING1);
    }
  }

  if (gfUIHandleShowMoveGrid) {
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS4);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS9);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS2);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS13);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS15);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS19);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS20);
  }

  if (fInterfacePanelDirty) {
    fInterfacePanelDirty = FALSE;
  }

  // Reset int tile cursor stuff
  if (gfUIShowCurIntTile) {
    if (gsUICurIntTileEffectGridNo != NOWHERE) {
      // Find our tile!
      pNode = gpWorldLevelData[gsUICurIntTileEffectGridNo].pStructHead;

      while (pNode != NULL) {
        if (pNode->usIndex == gusUICurIntTileEffectIndex) {
          pNode->ubShadeLevel = gsUICurIntTileOldShade;
          pNode->uiFlags &= (~LEVELNODE_DYNAMIC);
          break;
        }

        pNode = pNode->pNext;
      }
    }
  }
}

extern BOOLEAN AnyItemsVisibleOnLevel(struct ITEM_POOL *pItemPool, int8_t bZLevel);

uint32_t guiColors[12] = {FROMRGB(198, 163, 0), FROMRGB(185, 150, 0), FROMRGB(172, 136, 0),
                        FROMRGB(159, 123, 0), FROMRGB(146, 110, 0), FROMRGB(133, 96, 0),
                        FROMRGB(120, 83, 0),  FROMRGB(133, 96, 0),  FROMRGB(146, 110, 0),
                        FROMRGB(159, 123, 0), FROMRGB(172, 136, 0), FROMRGB(185, 150, 0)};

void RenderRubberBanding() {
  uint16_t usLineColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  int16_t iLeft, iRight, iTop, iBottom;
  int32_t iBack = -1;
  static int32_t iFlashColor = 0;
  static int32_t uiTimeOfLastUpdate = 0;

  if (!gRubberBandActive) return;

  iLeft = (int16_t)gRubberBandRect.iLeft;
  iRight = (int16_t)gRubberBandRect.iRight;
  iTop = (int16_t)gRubberBandRect.iTop;
  iBottom = (int16_t)gRubberBandRect.iBottom;

  if (iLeft == iRight && iTop == iBottom) {
    return;
  }

  if ((GetJA2Clock() - uiTimeOfLastUpdate) > 60) {
    uiTimeOfLastUpdate = GetJA2Clock();
    iFlashColor++;

    if (iFlashColor == 12) {
      iFlashColor = 0;
    }
  }

  // Draw rectangle.....
  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y);

  usLineColor = rgb32_to_rgb565(guiColors[iFlashColor]);

  if ((iRight - iLeft) > 0) {
    LineDraw(TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf);
    iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iLeft, iTop, (int16_t)(iRight + 1),
                                   (int16_t)(iTop + 1));
  } else if ((iRight - iLeft) < 0) {
    LineDraw(TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf);
    iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iRight, iTop, (int16_t)(iLeft + 1),
                                   (int16_t)(iTop + 1));
  }

  if (iBack != -1) {
    SetBackgroundRectFilled(iBack);
  }

  iBack = -1;

  if ((iRight - iLeft) > 0) {
    LineDraw(TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf);
    iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iLeft, iBottom, (int16_t)(iRight + 1),
                                   (int16_t)(iBottom + 1));
  } else if ((iRight - iLeft) < 0) {
    LineDraw(TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf);
    iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iRight, iBottom, (int16_t)(iLeft + 1),
                                   (int16_t)(iBottom + 1));
  }

  if (iBack != -1) {
    SetBackgroundRectFilled(iBack);
  }

  iBack = -1;

  if ((iBottom - iTop) > 0) {
    LineDraw(TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf);
    iBack =
        RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iLeft, iTop, (int16_t)(iLeft + 1), iBottom);
  } else if ((iBottom - iTop) < 0) {
    LineDraw(TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf);
    iBack =
        RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iLeft, iBottom, (int16_t)(iLeft + 1), iTop);
  }

  if (iBack != -1) {
    SetBackgroundRectFilled(iBack);
  }

  iBack = -1;

  if ((iBottom - iTop) > 0) {
    LineDraw(TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf);
    iBack =
        RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iRight, iTop, (int16_t)(iRight + 1), iBottom);
  } else if ((iBottom - iTop) < 0) {
    LineDraw(TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf);
    iBack =
        RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, iRight, iBottom, (int16_t)(iRight + 1), iTop);
  }

  if (iBack != -1) {
    SetBackgroundRectFilled(iBack);
  }

  VSurfaceUnlock(vsFB);
}

void RenderTopmostTacticalInterface() {
  struct SOLDIERTYPE *pSoldier;
  uint32_t cnt;
  static uint32_t uiBogTarget = 0;
  int16_t sX, sY;
  int16_t sOffsetX, sOffsetY, sTempY_S, sTempX_S;
  int16_t usMapPos;
  struct ITEM_POOL *pItemPool;

  if (gfRerenderInterfaceFromHelpText == TRUE) {
    fInterfacePanelDirty = DIRTYLEVEL2;

    switch (gsCurInterfacePanel) {
      case SM_PANEL:
        RenderSMPanel(&fInterfacePanelDirty);
        break;

      case TEAM_PANEL:
        RenderTEAMPanel(fInterfacePanelDirty);
        break;
    }
    gfRerenderInterfaceFromHelpText = FALSE;
  }

  if ((IsMapScreen())) {
    if (!(guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS)) {
      // If we want to rederaw whole screen, dirty all buttons!
      if (fInterfacePanelDirty == DIRTYLEVEL2) {
        MarkButtonsDirty();
      }

      RenderButtons();
    }

    return;
  }

  if (InItemStackPopup()) {
    if (fInterfacePanelDirty == DIRTYLEVEL2) {
      RenderItemStackPopup(TRUE);
    } else {
      RenderItemStackPopup(FALSE);
    }
  }

  if ((InKeyRingPopup()) && (!InItemDescriptionBox())) {
    RenderKeyRingPopup((BOOLEAN)(fInterfacePanelDirty == DIRTYLEVEL2));
  }

  if (gfInMovementMenu) {
    RenderMovementMenu();
  }

  // if IN PLAN MODE AND WE HAVE TARGETS, draw black targets!
  if (InUIPlanMode()) {
    // Zero out any planned soldiers
    for (cnt = MAX_NUM_SOLDIERS; cnt < TOTAL_SOLDIERS; cnt++) {
      if (MercPtrs[cnt]->bActive) {
        if (MercPtrs[cnt]->sPlannedTargetX != -1) {
          // Blit bogus target
          if (uiBogTarget == 0) {
            // Loadup cursor!
            AddVObjectFromFile("CURSORS\\targblak.sti", &uiBogTarget);
          }

          if (GridNoOnScreen(
                  (int16_t)MAPROWCOLTOPOS((MercPtrs[cnt]->sPlannedTargetY / CELL_Y_SIZE),
                                        (MercPtrs[cnt]->sPlannedTargetX / CELL_X_SIZE)))) {
            // GET SCREEN COORDINATES
            sOffsetX = (MercPtrs[cnt]->sPlannedTargetX - gsRenderCenterX);
            sOffsetY = (MercPtrs[cnt]->sPlannedTargetY - gsRenderCenterY);

            FromCellToScreenCoordinates(sOffsetX, sOffsetY, &sTempX_S, &sTempY_S);

            sX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + sTempX_S;
            sY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + sTempY_S;

            // Adjust for offset position on screen
            sX -= gsRenderWorldOffsetX;
            sY -= gsRenderWorldOffsetY;

            sX -= 10;
            sY -= 10;

            BltVObjectFromIndex(vsFB, uiBogTarget, 0, sX, sY);
            InvalidateRegion(sX, sY, sX + 20, sY + 20);
          }
        }
      }
    }
  }

  if (gfUIInDeadlock) {
    SetFont(LARGEFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_WHITE);
    gprintfdirty(0, 300, L"OPPONENT %d DEADLOCKED - 'Q' TO DEBUG, <ALT><ENTER> END OPP TURN",
                 gUIDeadlockedSoldier);
    mprintf(0, 300, L"OPPONENT %d DEADLOCKED - 'Q' TO DEBUG, <ALT><ENTER> END OPP TURN",
            gUIDeadlockedSoldier);
  }

  // Setup system for video overlay ( text and blitting ) Sets clipping rects, etc
  StartViewportOverlays();

  RenderTopmostFlashingItems();

  RenderTopmostMultiPurposeLocator();

  RenderAccumulatedBurstLocations();

  // Loop through all mercs and make go
  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    pSoldier = MercSlots[cnt];

    if (pSoldier != NULL) {
      if (pSoldier->ubID == gsSelectedGuy && gfUIHandleSelectionAboveGuy) {
      } else {
        DrawSelectedUIAboveGuy((uint16_t)pSoldier->ubID);
      }

      if (pSoldier->fDisplayDamage) {
        // Display damage

        // Use world coordinates!
        int16_t sMercScreenX, sMercScreenY, sOffsetX, sOffsetY, sDamageX, sDamageY;

        if (pSoldier->sGridNo != NOWHERE && pSoldier->bVisible != -1) {
          GetSoldierScreenPos(pSoldier, &sMercScreenX, &sMercScreenY);
          GetSoldierAnimOffsets(pSoldier, &sOffsetX, &sOffsetY);

          if (pSoldier->ubBodyType == QUEENMONSTER) {
            sDamageX = sMercScreenX + pSoldier->sDamageX - pSoldier->sBoundingBoxOffsetX;
            sDamageY = sMercScreenY + pSoldier->sDamageY - pSoldier->sBoundingBoxOffsetY;

            sDamageX += 25;
            sDamageY += 10;
          } else {
            sDamageX = pSoldier->sDamageX + (int16_t)(sMercScreenX + (2 * 30 / 3));
            sDamageY = pSoldier->sDamageY + (int16_t)(sMercScreenY - 5);

            sDamageX -= sOffsetX;
            sDamageY -= sOffsetY;

            if (sDamageY < gsVIEWPORT_WINDOW_START_Y) {
              sDamageY = (sMercScreenY - sOffsetY);
            }
          }

          SetFont(TINYFONT1);
          SetFontBackground(FONT_MCOLOR_BLACK);
          SetFontForeground(FONT_MCOLOR_WHITE);

          gprintfdirty(sDamageX, sDamageY, L"-%d", pSoldier->sDamage);
          mprintf(sDamageX, sDamageY, L"-%d", pSoldier->sDamage);
        }
      }
    }
  }

  if (gusSelectedSoldier != NOBODY) {
    DrawSelectedUIAboveGuy(gusSelectedSoldier);
  }

  if (gfUIHandleSelectionAboveGuy && gsSelectedGuy != NOBODY) {
    DrawSelectedUIAboveGuy((uint16_t)gsSelectedGuy);
  }

  // FOR THE MOST PART, DISABLE INTERFACE STUFF WHEN IT'S ENEMY'S TURN
  if (gTacticalStatus.ubCurrentTeam == gbPlayerNum) {
    RenderArrows();
  }

  RenderAimCubeUI();

  EndViewportOverlays();

  RenderRubberBanding();

  if (!gfInItemPickupMenu && gpItemPointer == NULL) {
    HandleAnyMercInSquadHasCompatibleStuff((int8_t)CurrentSquad(), NULL, TRUE);
  }

  // CHECK IF OUR CURSOR IS OVER AN INV POOL
  if (GetMouseMapPos(&usMapPos)) {
    if (gfUIOverItemPool) {
      if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
        // Check if we are over an item pool
        if (GetItemPool(gfUIOverItemPoolGridNo, &pItemPool, pSoldier->bLevel)) {
          struct STRUCTURE *pStructure = NULL;
          int16_t sIntTileGridNo;
          int8_t bZLevel = 0;
          int16_t sActionGridNo = usMapPos;

          // Get interactive tile...
          if (ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure,
                                                                 FALSE)) {
            sActionGridNo = sIntTileGridNo;
          }

          bZLevel = GetZLevelOfItemPoolGivenStructure(sActionGridNo, pSoldier->bLevel, pStructure);

          if (AnyItemsVisibleOnLevel(pItemPool, bZLevel)) {
            DrawItemPoolList(pItemPool, gfUIOverItemPoolGridNo, ITEMLIST_DISPLAY, bZLevel,
                             gusMouseXPos, gusMouseYPos);

            // ATE: If over items, remove locator....
            RemoveFlashItemSlot(pItemPool);
          }
        } else {
          int8_t bCheckLevel;

          // ATE: Allow to see list if a different level....
          if (pSoldier->bLevel == 0) {
            bCheckLevel = 1;
          } else {
            bCheckLevel = 0;
          }

          // Check if we are over an item pool
          if (GetItemPool(gfUIOverItemPoolGridNo, &pItemPool, bCheckLevel)) {
            struct STRUCTURE *pStructure = NULL;
            int16_t sIntTileGridNo;
            int8_t bZLevel = 0;
            int16_t sActionGridNo = usMapPos;

            // Get interactive tile...
            if (ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure,
                                                                   FALSE)) {
              sActionGridNo = sIntTileGridNo;
            }

            bZLevel = GetZLevelOfItemPoolGivenStructure(sActionGridNo, bCheckLevel, pStructure);

            if (AnyItemsVisibleOnLevel(pItemPool, bZLevel)) {
              DrawItemPoolList(pItemPool, gfUIOverItemPoolGridNo, ITEMLIST_DISPLAY, bZLevel,
                               gusMouseXPos, gusMouseYPos);

              // ATE: If over items, remove locator....
              RemoveFlashItemSlot(pItemPool);
            }
          }
        }
      }
    }
  }

  // Check if we should render item selection window
  if (gCurrentUIMode == GETTINGITEM_MODE) {
    SetItemPickupMenuDirty(DIRTYLEVEL2);
    // Handle item pickup will return true if it's been closed
    RenderItemPickupMenu();
  }

  // Check if we should render item selection window
  if (gCurrentUIMode == OPENDOOR_MENU_MODE) {
    RenderOpenDoorMenu();
  }

  if (gfInTalkPanel) {
    SetTalkingMenuDirty(DIRTYLEVEL2);
    // Handle item pickup will return true if it's been closed
    RenderTalkingMenu();
  }

  if (gfInSectorExitMenu) {
    RenderSectorExitMenu();
  }

  if (fRenderRadarScreen == TRUE) {
    // Render clock
    RenderClock(CLOCK_X, CLOCK_Y);
    RenderTownIDString();
    CreateMouseRegionForPauseOfClock(CLOCK_REGION_START_X, CLOCK_REGION_START_Y);
  } else {
    RemoveMouseRegionForPauseOfClock();
  }

  if (!(guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS)) {
    // If we want to rederaw whole screen, dirty all buttons!
    if (fInterfacePanelDirty == DIRTYLEVEL2) {
      MarkButtonsDirty();
    }

    RenderButtons();
    RenderPausedGameBox();
  }

  // mark all pop ups as dirty
  MarkAllBoxesAsAltered();

  HandleShowingOfTacticalInterfaceFastHelpText();
  HandleShadingOfLinesForAssignmentMenus();
  DetermineBoxPositions();
  DisplayBoxes(FRAME_BUFFER);
}

void StartViewportOverlays() {
  // Set Clipping Rect to be the viewscreen
  // Save old one
  memcpy(&gOldClippingRect, &ClippingRect, sizeof(gOldClippingRect));

  // Save old dirty clipping rect
  memcpy(&gOldDirtyClippingRect, &ClippingRect, sizeof(gOldDirtyClippingRect));

  // Set bottom clipping value for blitter clipping rect
  ClippingRect.iLeft = INTERFACE_START_X;
  ClippingRect.iTop = gsVIEWPORT_WINDOW_START_Y;
  ClippingRect.iRight = 640;
  ClippingRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

  // Set values for dirty rect clipping rect
  gDirtyClipRect.iLeft = INTERFACE_START_X;
  gDirtyClipRect.iTop = gsVIEWPORT_WINDOW_START_Y;
  gDirtyClipRect.iRight = 640;
  gDirtyClipRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

  SaveFontSettings();
  SetFontDest(vsFB, 0, gsVIEWPORT_WINDOW_START_Y, 640, gsVIEWPORT_WINDOW_END_Y, FALSE);
}

void EndViewportOverlays() {
  // Reset clipping rect
  memcpy(&ClippingRect, &gOldClippingRect, sizeof(gOldClippingRect));
  memcpy(&gDirtyClipRect, &gOldDirtyClippingRect, sizeof(gOldDirtyClippingRect));
  RestoreFontSettings();
}

void LockTacticalInterface() {
  // OK, check and see if we are not locked, if so
  // 1) create a mouse region over the entrie interface panel
  // 2) set flag for use in tactical to indicate we are locked
  if (!(guiTacticalInterfaceFlags & INTERFACE_LOCKEDLEVEL1)) {
    MSYS_DefineRegion(&gLockPanelOverlayRegion, 0, gsVIEWPORT_WINDOW_END_Y, 640, 480,
                      MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
    // Add region
    MSYS_AddRegion(&gLockPanelOverlayRegion);

    guiTacticalInterfaceFlags |= INTERFACE_LOCKEDLEVEL1;
  }
}

void UnLockTacticalInterface() {
  if ((guiTacticalInterfaceFlags & INTERFACE_LOCKEDLEVEL1)) {
    // Remove region
    MSYS_RemoveRegion(&gLockPanelOverlayRegion);

    guiTacticalInterfaceFlags &= (~INTERFACE_LOCKEDLEVEL1);
  }
}

void EraseInterfaceMenus(BOOLEAN fIgnoreUIUnLock) {
  // ATE: If we are currently talking, setup this flag so that the
  // automatic handler in handledialogue doesn't adjust the UI setting
  if ((gTacticalStatus.uiFlags & ENGAGED_IN_CONV) && fIgnoreUIUnLock) {
    gTacticalStatus.uiFlags |= IGNORE_ENGAGED_IN_CONV_UI_UNLOCK;
  }

  // Remove item pointer if one active
  CancelItemPointer();

  ShutDownQuoteBoxIfActive();
  PopDownMovementMenu();
  PopDownOpenDoorMenu();
  DeleteTalkingMenu();
}

BOOLEAN AreWeInAUIMenu() {
  if (gfInMovementMenu || gfInOpenDoorMenu || gfInItemPickupMenu || gfInSectorExitMenu ||
      gfInTalkPanel) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void ResetInterfaceAndUI() {
  // Erase menus
  EraseInterfaceMenus(FALSE);

  EraseRenderArrows();

  EndRubberBanding();

  // ResetMultiSelection( );

  if (giUIMessageOverlay != -1) {
    RemoveVideoOverlay(giUIMessageOverlay);
    giUIMessageOverlay = -1;
  }

  // Set UI back to movement...
  guiPendingOverrideEvent = M_ON_TERRAIN;
  HandleTacticalUI();
}

BOOLEAN InterfaceOKForMeanwhilePopup() {
  if (gfSwitchPanel) {
    return (FALSE);
  }

  return (TRUE);
}
