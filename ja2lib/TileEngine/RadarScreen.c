#include "TileEngine/RadarScreen.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "Globals.h"
#include "JAScreens.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "StrUtils.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterfaceMapInventory.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationData.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/Squads.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "rust_colors.h"

extern INT32 iCurrentMapSectorZ;

void AdjustWorldCenterFromRadarCoords(INT16 sRadarX, INT16 sRadarY);

// render the squad list
void RenderSquadList(void);

// squad list mvt + btn callback
void TacticalSquadListMvtCallback(struct MOUSE_REGION *pRegion, INT32 iReason);
void TacticalSquadListBtnCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);

// the squad list font
#define SQUAD_FONT COMPFONT

#define SQUAD_REGION_HEIGHT 2 * RADAR_WINDOW_HEIGHT
#define SQUAD_WINDOW_TM_Y RADAR_WINDOW_TM_Y + GetFontHeight(SQUAD_FONT)

// subtractor for squad list from size of radar view region height
#define SUBTRACTOR_FOR_SQUAD_LIST 0

INT16 gsRadarX;
INT16 gsRadarY;
UINT32 gusRadarImage;
BOOLEAN fImageLoaded = FALSE;
BOOLEAN fRenderRadarScreen = TRUE;
INT16 sSelectedSquadLine = -1;

BOOLEAN gfRadarCurrentGuyFlash = FALSE;

struct MOUSE_REGION gRadarRegionSquadList[NUMBER_OF_SQUADS];

BOOLEAN InitRadarScreen() {
  // Add region for radar
  MSYS_DefineRegion(&gRadarRegion, RADAR_WINDOW_X, RADAR_WINDOW_TM_Y,
                    RADAR_WINDOW_X + RADAR_WINDOW_WIDTH, RADAR_WINDOW_TM_Y + RADAR_WINDOW_HEIGHT,
                    MSYS_PRIORITY_HIGHEST, 0, RadarRegionMoveCallback, RadarRegionButtonCallback);

  // Add region
  MSYS_AddRegion(&gRadarRegion);

  // disable the radar map
  MSYS_DisableRegion(&gRadarRegion);

  gsRadarX = RADAR_WINDOW_X;
  gsRadarY = RADAR_WINDOW_TM_Y;

  return (TRUE);
}

BOOLEAN LoadRadarScreenBitmap(CHAR8 *aFilename) {
  CHAR8 zFilename[80];
  INT32 cnt;
  struct VObject *hVObject;

  strcopy(zFilename, ARR_SIZE(zFilename), aFilename);

  // If we have loaded, remove old one
  if (fImageLoaded) {
    DeleteVideoObjectFromIndex(gusRadarImage);

    fImageLoaded = FALSE;
  }

  {
    // Remove extension
    for (cnt = strlen(zFilename) - 1; cnt >= 0; cnt--) {
      if (zFilename[cnt] == '.') {
        zFilename[cnt] = '\0';
      }
    }

    // Grab the Map image
    SGPFILENAME ImageFile;
    sprintf(ImageFile, "RADARMAPS\\%s.STI", zFilename);
    if (!AddVObjectFromFile(ImageFile, &gusRadarImage)) {
      return FALSE;
    }

    fImageLoaded = TRUE;

    if (GetVideoObject(&hVObject, gusRadarImage)) {
      // ATE: Add a shade table!
      VObjectUpdateShade(hVObject, 0, 255, 255, 255, FALSE);
      VObjectUpdateShade(hVObject, 1, 100, 100, 100, FALSE);
    }
  }

  // Dirty interface
  fInterfacePanelDirty = TRUE;

  return (TRUE);
}

void ClearOutRadarMapImage(void) {
  // If we have loaded, remove old one
  if (fImageLoaded) {
    DeleteVideoObjectFromIndex(gusRadarImage);
    fImageLoaded = FALSE;
  }
}

void MoveRadarScreen() {
  // check if we are allowed to do anything?
  if (fRenderRadarScreen == FALSE) {
    return;
  }

  // Remove old region
  MSYS_RemoveRegion(&gRadarRegion);

  // Add new one

  // Move based on inventory panel
  if (gsCurInterfacePanel == SM_PANEL) {
    gsRadarY = RADAR_WINDOW_TM_Y;
  } else {
    gsRadarY = RADAR_WINDOW_TM_Y;
  }

  // Add region for radar
  MSYS_DefineRegion(&gRadarRegion, RADAR_WINDOW_X, (UINT16)(gsRadarY),
                    RADAR_WINDOW_X + RADAR_WINDOW_WIDTH, (UINT16)(gsRadarY + RADAR_WINDOW_HEIGHT),
                    MSYS_PRIORITY_HIGHEST, 0, RadarRegionMoveCallback, RadarRegionButtonCallback);

  // Add region
  MSYS_AddRegion(&gRadarRegion);
}

void RadarRegionMoveCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
  INT16 sRadarX, sRadarY;

  // check if we are allowed to do anything?
  if (fRenderRadarScreen == FALSE) {
    return;
  }

  if (iReason == MSYS_CALLBACK_REASON_MOVE) {
    if (pRegion->ButtonState & MSYS_LEFT_BUTTON) {
      // Use relative coordinates to set center of viewport
      sRadarX = pRegion->RelativeXPos - (RADAR_WINDOW_WIDTH / 2);
      sRadarY = pRegion->RelativeYPos - (RADAR_WINDOW_HEIGHT / 2);

      AdjustWorldCenterFromRadarCoords(sRadarX, sRadarY);

      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
}

void RadarRegionButtonCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
  INT16 sRadarX, sRadarY;

  // check if we are allowed to do anything?
  if (fRenderRadarScreen == FALSE) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!InOverheadMap()) {
      // Use relative coordinates to set center of viewport
      sRadarX = pRegion->RelativeXPos - (RADAR_WINDOW_WIDTH / 2);
      sRadarY = pRegion->RelativeYPos - (RADAR_WINDOW_HEIGHT / 2);

      AdjustWorldCenterFromRadarCoords(sRadarX, sRadarY);
    } else {
      KillOverheadMap();
    }
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (!InOverheadMap()) {
      GoIntoOverheadMap();
    } else {
      KillOverheadMap();
    }
  }
}

void RenderRadarScreen() {
  INT16 sRadarTLX, sRadarTLY;
  INT16 sRadarBRX, sRadarBRY;
  INT16 sRadarCX, sRadarCY;
  INT32 iItemNumber = 0;

  INT16 sX_S, sY_S;
  INT16 sScreenCenterX, sScreenCenterY;
  INT16 sDistToCenterY, sDistToCenterX;
  INT16 sTopLeftWorldX, sTopLeftWorldY;
  INT16 sBottomRightWorldX, sBottomRightWorldY;

  struct SOLDIERTYPE *pSoldier;

  INT16 sXSoldPos, sYSoldPos, sXSoldScreen, sYSoldScreen, sXSoldRadar, sYSoldRadar;

  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;
  UINT16 usLineColor;
  UINT32 cnt;
  INT16 sHeight, sWidth, sX;
  INT32 iCounter = 0;

  // create / destroy squad list regions as nessacary
  CreateDestroyMouseRegionsForSquadList();

  // check if we are allowed to do anything?
  if (fRenderRadarScreen == FALSE) {
    RenderSquadList();
    return;
  }

  if (AreInMeanwhile() == TRUE) {
    // in a meanwhile, don't render any map
    fImageLoaded = FALSE;
  }

  if (fInterfacePanelDirty == DIRTYLEVEL2 && fImageLoaded) {
    // Set to default
    SetObjectHandleShade(gusRadarImage, 0);

    // If night time and on surface, darken the radarmap.
    if (IsNightTime()) {
      if ((IsMapScreen_2() && !iCurrentMapSectorZ) || (IsTacticalMode() && !gbWorldSectorZ)) {
        SetObjectHandleShade(gusRadarImage, 1);
      }
    }

    BltVObjectFromIndex(vsSB, gusRadarImage, 0, RADAR_WINDOW_X, gsRadarY);
  }

  // FIRST DELETE WHAT'S THERE
  RestoreExternBackgroundRect(RADAR_WINDOW_X, gsRadarY, RADAR_WINDOW_WIDTH + 1,
                              RADAR_WINDOW_HEIGHT + 1);

  // Determine scale factors

  // Find the diustance from render center to true world center
  sDistToCenterX = gsRenderCenterX - gCenterWorldX;
  sDistToCenterY = gsRenderCenterY - gCenterWorldY;

  // From render center in world coords, convert to render center in "screen" coords
  FromCellToScreenCoordinates(sDistToCenterX, sDistToCenterY, &sScreenCenterX, &sScreenCenterY);

  // Subtract screen center
  sScreenCenterX += gsCX;
  sScreenCenterY += gsCY;

  // Get corners in screen coords
  // TOP LEFT
  sX_S = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);
  sY_S = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2);

  sTopLeftWorldX = sScreenCenterX - sX_S;
  sTopLeftWorldY = sScreenCenterY - sY_S;

  sBottomRightWorldX = sScreenCenterX + sX_S;
  sBottomRightWorldY = sScreenCenterY + sY_S;

  // Determine radar coordinates
  sRadarCX = (INT16)(gsCX * gdScaleX);
  sRadarCY = (INT16)(gsCY * gdScaleY);

  sWidth = (RADAR_WINDOW_WIDTH);
  sHeight = (RADAR_WINDOW_HEIGHT);
  sX = RADAR_WINDOW_X;

  sRadarTLX = (INT16)((sTopLeftWorldX * gdScaleX) - sRadarCX + sX + (sWidth / 2));
  sRadarTLY = (INT16)((sTopLeftWorldY * gdScaleY) - sRadarCY + gsRadarY + (sHeight / 2));
  sRadarBRX = (INT16)((sBottomRightWorldX * gdScaleX) - sRadarCX + sX + (sWidth / 2));
  sRadarBRY = (INT16)((sBottomRightWorldY * gdScaleY) - sRadarCY + gsRadarY + (sHeight / 2));

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);

  SetClippingRegionAndImageWidth(uiDestPitchBYTES, RADAR_WINDOW_X, gsRadarY,
                                 (RADAR_WINDOW_X + RADAR_WINDOW_WIDTH - 1),
                                 (gsRadarY + RADAR_WINDOW_HEIGHT - 1));

  if (!(IsMapScreen())) {
    usLineColor = rgb32_to_rgb565(FROMRGB(0, 255, 0));
    RectangleDraw(TRUE, sRadarTLX, sRadarTLY, sRadarBRX, sRadarBRY - 1, usLineColor, pDestBuf);
  }

  // Cycle fFlash variable
  if (COUNTERDONE(RADAR_MAP_BLINK)) {
    RESETCOUNTER(RADAR_MAP_BLINK);

    gfRadarCurrentGuyFlash = !gfRadarCurrentGuyFlash;
  }

  if ((IsMapScreen()) && (fShowMapInventoryPool == TRUE)) {
    for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
      iItemNumber = iCounter + iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT;
      // stolen item
      if ((pInventoryPoolList[iItemNumber].o.ubNumberOfObjects == 0) ||
          (pInventoryPoolList[iItemNumber].sGridNo == 0)) {
        // yep, continue on
        continue;
      }

      ConvertGridNoToXY(pInventoryPoolList[iItemNumber].sGridNo, &sXSoldPos, &sYSoldPos);
      GetWorldXYAbsoluteScreenXY(sXSoldPos, sYSoldPos, &sXSoldScreen, &sYSoldScreen);

      // get radar x and y postion
      sXSoldRadar = (INT16)(sXSoldScreen * gdScaleX);
      sYSoldRadar = (INT16)(sYSoldScreen * gdScaleY);

      // Add starting relative to interface
      sXSoldRadar += RADAR_WINDOW_X;
      sYSoldRadar += gsRadarY;

      // if we are in 16 bit mode....kind of redundant
      if ((fFlashHighLightInventoryItemOnradarMap)) {
        usLineColor = rgb32_to_rgb565(FROMRGB(0, 255, 0));

      } else {
        // DB Need to add a radar color for 8-bit
        usLineColor = rgb32_to_rgb565(FROMRGB(255, 255, 255));
      }

      if (iCurrentlyHighLightedItem == iCounter) {
        RectangleDraw(TRUE, sXSoldRadar, sYSoldRadar, sXSoldRadar + 1, sYSoldRadar + 1, usLineColor,
                      pDestBuf);
      }
    }
  }

  if (!(IsMapScreen())) {
    // RE-RENDER RADAR
    for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
      pSoldier = MercSlots[cnt];

      if (pSoldier != NULL) {
        // Don't place guys in radar until visible!
        if (pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS) &&
            !(pSoldier->ubMiscSoldierFlags & SOLDIER_MISC_XRAYED)) {
          continue;
        }

        // Don't render guys if they are dead!
        if ((pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
          continue;
        }

        // Don't render crows
        if (pSoldier->ubBodyType == CROW) {
          continue;
        }

        // Get FULL screen coordinate for guy's position
        // Getxy from gridno
        ConvertGridNoToXY(pSoldier->sGridNo, &sXSoldPos, &sYSoldPos);
        GetWorldXYAbsoluteScreenXY(sXSoldPos, sYSoldPos, &sXSoldScreen, &sYSoldScreen);

        sXSoldRadar = (INT16)(sXSoldScreen * gdScaleX);
        sYSoldRadar = (INT16)(sYSoldScreen * gdScaleY);

        if (!SoldierOnVisibleWorldTile(pSoldier)) {
          continue;
        }

        // Add starting relative to interface
        sXSoldRadar += RADAR_WINDOW_X;
        sYSoldRadar += gsRadarY;

        // DB Need to add a radar color for 8-bit

        // Are we a selected guy?
        if (pSoldier->ubID == gusSelectedSoldier) {
          if (gfRadarCurrentGuyFlash) {
            usLineColor = 0;
          } else {
            // If on roof, make darker....
            if (pSoldier->bLevel > 0) {
              usLineColor = rgb32_to_rgb565(FROMRGB(150, 150, 0));
            } else {
              usLineColor = rgb32_to_rgb565(gTacticalStatus.Team[pSoldier->bTeam].RadarColor);
            }
          }
        } else {
          usLineColor = rgb32_to_rgb565(gTacticalStatus.Team[pSoldier->bTeam].RadarColor);

          // Override civ team with red if hostile...
          if (pSoldier->bTeam == CIV_TEAM && !pSoldier->bNeutral &&
              (pSoldier->bSide != gbPlayerNum)) {
            usLineColor = rgb32_to_rgb565(FROMRGB(255, 0, 0));
          }

          // Render different color if an enemy and he's unconscious
          if (pSoldier->bTeam != gbPlayerNum && pSoldier->bLife < OKLIFE) {
            usLineColor = rgb32_to_rgb565(FROMRGB(128, 128, 128));
          }

          // If on roof, make darker....
          if (pSoldier->bTeam == gbPlayerNum && pSoldier->bLevel > 0) {
            usLineColor = rgb32_to_rgb565(FROMRGB(150, 150, 0));
          }
        }

        RectangleDraw(TRUE, sXSoldRadar, sYSoldRadar, sXSoldRadar + 1, sYSoldRadar + 1, usLineColor,
                      pDestBuf);
      }
    }
  }
  VSurfaceUnlock(vsFB);

  if ((IsMapScreen()) && (fShowMapInventoryPool == TRUE)) {
    InvalidateRegion(RADAR_WINDOW_X, gsRadarY, RADAR_WINDOW_X + RADAR_WINDOW_WIDTH,
                     gsRadarY + RADAR_WINDOW_HEIGHT);
  }

  return;
}

void AdjustWorldCenterFromRadarCoords(INT16 sRadarX, INT16 sRadarY) {
  INT16 sScreenX, sScreenY;
  INT16 sTempX_W, sTempY_W;
  INT16 sNewCenterWorldX, sNewCenterWorldY;
  INT16 sNumXSteps, sNumYSteps;

  // Use radar scale values to get screen values, then convert ot map values, rounding to nearest
  // middle tile
  sScreenX = (INT16)(sRadarX / gdScaleX);
  sScreenY = (INT16)(sRadarY / gdScaleY);

  // Adjust to viewport start!
  sScreenX -= ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);
  sScreenY -= ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2);

  // Make sure these coordinates are multiples of scroll steps
  sNumXSteps = sScreenX / SCROLL_X_STEP;
  sNumYSteps = sScreenY / SCROLL_Y_STEP;

  sScreenX = (sNumXSteps * SCROLL_X_STEP);
  sScreenY = (sNumYSteps * SCROLL_Y_STEP);

  // Adjust back
  sScreenX += ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);
  sScreenY += ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2);

  // Subtract world center
  // sScreenX += gsCX;
  // sScreenY += gsCY;

  // Convert these into world coordinates
  FromScreenToCellCoordinates(sScreenX, sScreenY, &sTempX_W, &sTempY_W);

  // Adjust these to world center
  sNewCenterWorldX = (INT16)(gCenterWorldX + sTempX_W);
  sNewCenterWorldY = (INT16)(gCenterWorldY + sTempY_W);

  SetRenderCenter(sNewCenterWorldX, sNewCenterWorldY);
}

void DisableRadarScreenRender(void) {
  fRenderRadarScreen = FALSE;
  return;
}

void EnableRadarScreenRender(void) {
  fRenderRadarScreen = TRUE;
  return;
}

void ToggleRadarScreenRender(void) {
  fRenderRadarScreen = !fRenderRadarScreen;
  return;
}

BOOLEAN CreateDestroyMouseRegionsForSquadList(void) {
  // will check the state of renderradarscreen flag and decide if we need to create mouse regions
  // for
  static BOOLEAN fCreated = FALSE;
  INT16 sCounter = 0;
  struct VObject *hHandle;
  UINT32 uiHandle;

  if ((fRenderRadarScreen == FALSE) && (fCreated == FALSE)) {
    // create regions
    // load graphics
    if (!AddVObjectFromFile("INTERFACE\\squadpanel.sti", &uiHandle)) {
      return FALSE;
    }

    GetVideoObject(&hHandle, uiHandle);
    BltVObject(vsSB, hHandle, 0, 538, 0 + gsVIEWPORT_END_Y);

    RestoreExternBackgroundRect(538, gsVIEWPORT_END_Y, (640 - 538),
                                (INT16)(480 - gsVIEWPORT_END_Y));

    for (sCounter = 0; sCounter < NUMBER_OF_SQUADS; sCounter++) {
      // run through list of squads and place appropriatly
      if (sCounter < NUMBER_OF_SQUADS / 2) {
        // left half of list
        MSYS_DefineRegion(
            &gRadarRegionSquadList[sCounter], RADAR_WINDOW_X,
            (INT16)(SQUAD_WINDOW_TM_Y +
                    (sCounter *
                     ((SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / (NUMBER_OF_SQUADS / 2)))),
            RADAR_WINDOW_X + RADAR_WINDOW_WIDTH / 2 - 1,
            (INT16)(SQUAD_WINDOW_TM_Y +
                    ((sCounter + 1) *
                     ((SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / (NUMBER_OF_SQUADS / 2)))),
            MSYS_PRIORITY_HIGHEST, 0, TacticalSquadListMvtCallback, TacticalSquadListBtnCallBack);
      } else {
        // right half of list
        MSYS_DefineRegion(
            &gRadarRegionSquadList[sCounter], RADAR_WINDOW_X + RADAR_WINDOW_WIDTH / 2,
            (INT16)(SQUAD_WINDOW_TM_Y +
                    ((sCounter - (NUMBER_OF_SQUADS / 2)) *
                     (2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
            RADAR_WINDOW_X + RADAR_WINDOW_WIDTH - 1,
            (INT16)(SQUAD_WINDOW_TM_Y +
                    (((sCounter + 1) - (NUMBER_OF_SQUADS / 2)) *
                     (2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
            MSYS_PRIORITY_HIGHEST, 0, TacticalSquadListMvtCallback, TacticalSquadListBtnCallBack);
      }

      // set user data
      MSYS_SetRegionUserData(&gRadarRegionSquadList[sCounter], 0, sCounter);
    }

    DeleteVideoObjectFromIndex(uiHandle);

    // reset the highlighted line
    sSelectedSquadLine = -1;

    // set fact regions are created
    fCreated = TRUE;
  } else if ((fRenderRadarScreen == TRUE) && (fCreated == TRUE)) {
    // destroy regions

    for (sCounter = 0; sCounter < NUMBER_OF_SQUADS; sCounter++) {
      MSYS_RemoveRegion(&gRadarRegionSquadList[sCounter]);
    }

    // set fact regions are destroyed
    fCreated = FALSE;

    if (IsTacticalMode()) {
      // dirty region
      fInterfacePanelDirty = DIRTYLEVEL2;

      MarkButtonsDirty();

      // re render region
      RenderTacticalInterface();

      RenderButtons();

      // if game is paused, then render paused game text
      RenderPausedGameBox();
    }
  }

  return (TRUE);
}

void RenderSquadList(void) {
  // show list of squads
  INT16 sCounter = 0;
  INT16 sX, sY;

  // clear region
  RestoreExternBackgroundRect(RADAR_WINDOW_X, gsRadarY, RADAR_WINDOW_WIDTH, SQUAD_REGION_HEIGHT);

  // fill area
  VSurfaceColorFill(vsFB, RADAR_WINDOW_X, RADAR_WINDOW_TM_Y, RADAR_WINDOW_X + RADAR_WINDOW_WIDTH,
                    RADAR_WINDOW_TM_Y + SQUAD_REGION_HEIGHT, rgb32_to_rgb565(FROMRGB(0, 0, 0)));

  // set font
  SetFont(SQUAD_FONT);

  for (sCounter = 0; sCounter < NUMBER_OF_SQUADS; sCounter++) {
    // run through list of squads and place appropriatly
    if (sCounter < NUMBER_OF_SQUADS / 2) {
      FindFontCenterCoordinates(
          RADAR_WINDOW_X,
          (INT16)(SQUAD_WINDOW_TM_Y +
                  (sCounter *
                   (2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
          RADAR_WINDOW_WIDTH / 2 - 1,
          (INT16)(((2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
          pSquadMenuStrings[sCounter], SQUAD_FONT, &sX, &sY);
    } else {
      FindFontCenterCoordinates(
          RADAR_WINDOW_X + RADAR_WINDOW_WIDTH / 2,
          (INT16)(SQUAD_WINDOW_TM_Y +
                  ((sCounter - (NUMBER_OF_SQUADS / 2)) *
                   (2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
          RADAR_WINDOW_WIDTH / 2 - 1,
          (INT16)(((2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS))),
          pSquadMenuStrings[sCounter], SQUAD_FONT, &sX, &sY);
    }

    // highlight line?
    if (sSelectedSquadLine == sCounter) {
      SetFontForeground(FONT_WHITE);
    } else {
      if (IsSquadOnCurrentTacticalMap((INT32)sCounter) == TRUE) {
        if (CurrentSquad() == (INT32)sCounter) {
          SetFontForeground(FONT_LTGREEN);
        } else {
          SetFontForeground(FONT_DKGREEN);
        }
      } else {
        SetFontForeground(FONT_BLACK);
      }
    }

    SetFontBackground(FONT_BLACK);

    if (sCounter < NUMBER_OF_SQUADS / 2) {
      sX = RADAR_WINDOW_X + 2;
    } else {
      sX = RADAR_WINDOW_X + (RADAR_WINDOW_WIDTH / 2) - 2;
    }
    mprintf(sX, sY, pSquadMenuStrings[sCounter]);
  }
}

void TacticalSquadListMvtCallback(struct MOUSE_REGION *pRegion, INT32 iReason) {
  INT32 iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (IsSquadOnCurrentTacticalMap(iValue) == TRUE) {
      sSelectedSquadLine = (INT16)iValue;
    }
  }
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    sSelectedSquadLine = -1;
  }

  return;
}

void TacticalSquadListBtnCallBack(struct MOUSE_REGION *pRegion, INT32 iReason) {
  // btn callback handler for team list info region
  INT32 iValue = 0;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // find out if this squad is valid and on this map..if so, set as selected
    if (IsSquadOnCurrentTacticalMap(iValue) == TRUE) {
      // ok, squad is here, set as selected
      SetCurrentSquad(iValue, FALSE);

      // stop showing
      fRenderRadarScreen = TRUE;
    }
  }

  return;
}
