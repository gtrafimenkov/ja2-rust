#include "TileEngine/OverheadMap.h"

#include <stdio.h>
#include <stdlib.h>

#include "GameLoop.h"
#include "Globals.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "SysGlobals.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFind.h"
#include "Tactical/Squads.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Environment.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/TileSurface.h"
#include "TileEngine/WorldDat.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Utilities.h"

#ifdef JA2EDITOR
#include "Tactical/SoldierInitList.h"
extern SOLDIERINITNODE *gpSelected;
#endif

// OK, these are values that are calculated in InitRenderParams( ) with normal view settings.
// These would be different if we change ANYTHING about the game worlkd map sizes...
#define NORMAL_MAP_SCREEN_WIDTH 3160
#define NORMAL_MAP_SCREEN_HEIGHT 1540
#define NORMAL_MAP_SCREEN_X 1580
#define NORMAL_MAP_SCREEN_BY 2400
#define NORMAL_MAP_SCREEN_TY 860

#define FASTMAPROWCOLTOPOS(r, c) ((r) * WORLD_COLS + (c))

typedef struct {
  struct VObject *vo;
  uint32_t fType;

} SMALL_TILE_SURF;

typedef struct {
  struct VObject *vo;
  uint16_t usSubIndex;
  uint32_t fType;

} SMALL_TILE_DB;

SMALL_TILE_SURF gSmTileSurf[NUMBEROFTILETYPES];
SMALL_TILE_DB gSmTileDB[NUMBEROFTILES];
uint8_t gubSmTileNum = 0;
BOOLEAN gfSmTileLoaded = FALSE;
BOOLEAN gfInOverheadMap = FALSE;
struct MOUSE_REGION OverheadRegion;
struct MOUSE_REGION OverheadBackgroundRegion;
uint32_t uiOVERMAP;
uint32_t uiPERSONS;
BOOLEAN gfOverheadMapDirty = FALSE;
extern BOOLEAN gfRadarCurrentGuyFlash;
int16_t gsStartRestrictedX, gsStartRestrictedY;
BOOLEAN gfOverItemPool = FALSE;
int16_t gsOveritemPoolGridNo;

void HandleOverheadUI();
void ClickOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason);
void MoveOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason);
void DeleteOverheadDB();
BOOLEAN GetOverheadMouseGridNoForFullSoldiersGridNo(int16_t *psGridNo);

extern BOOLEAN AnyItemsVisibleOnLevel(struct ITEM_POOL *pItemPool, int8_t bZLevel);
extern void HandleAnyMercInSquadHasCompatibleStuff(uint8_t ubSquad, struct OBJECTTYPE *pObject,
                                                   BOOLEAN fReset);

// Isometric utilities (for overhead stuff only)
BOOLEAN GetOverheadMouseGridNo(int16_t *psGridNo);
void GetOverheadScreenXYFromGridNo(int16_t sGridNo, int16_t *psScreenX, int16_t *psScreenY);
void CopyOverheadDBShadetablesFromTileset();

void RenderOverheadOverlays();

void InitNewOverheadDB(uint8_t ubTilesetID) {
  uint32_t uiLoop;
  struct VObject *hVObject;
  char cFileBPP[128];
  char cAdjustedFile[200];
  uint32_t cnt1, cnt2;
  SMALL_TILE_SURF s;
  uint32_t NumRegions;
  uint32_t dbSize = 0;

  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    // Create video object

    // Adjust for BPP
    CopyFilename(gTilesets[ubTilesetID].TileSurfaceFilenames[uiLoop], cFileBPP);

    // Adjust for tileset position
    sprintf(cAdjustedFile, "TILESETS\\%d\\T\\%s", ubTilesetID, cFileBPP);

    hVObject = CreateVObjectFromFile(cAdjustedFile);

    if (hVObject == NULL) {
      // TRY loading from default directory
      CopyFilename(gTilesets[GENERIC_1].TileSurfaceFilenames[uiLoop], cFileBPP);
      // Adjust for tileset position
      sprintf(cAdjustedFile, "TILESETS\\0\\T\\%s", cFileBPP);

      // LOAD ONE WE KNOW ABOUT!
      hVObject = CreateVObjectFromFile(cAdjustedFile);

      if (hVObject == NULL) {
        // LOAD ONE WE KNOW ABOUT!
        hVObject = CreateVObjectFromFile("TILESETS\\0\\T\\grass.sti");
      }
    }

    gSmTileSurf[uiLoop].vo = hVObject;
    gSmTileSurf[uiLoop].fType = uiLoop;
  }

  // NOW LOOP THROUGH AND CREATE DATABASE
  for (cnt1 = 0; cnt1 < NUMBEROFTILETYPES; cnt1++) {
    // Get number of regions
    s = gSmTileSurf[cnt1];

    NumRegions = s.vo->usNumberOfObjects;

    // Check for overflow
    if (NumRegions > gNumTilesPerType[cnt1]) {
      // Cutof
      NumRegions = gNumTilesPerType[cnt1];
    }

    for (cnt2 = 0; cnt2 < NumRegions; cnt2++) {
      gSmTileDB[dbSize].vo = s.vo;
      gSmTileDB[dbSize].usSubIndex = (uint16_t)cnt2;
      gSmTileDB[dbSize].fType = cnt1;

      dbSize++;
    }

    // Check if data matches what should be there
    if (NumRegions < gNumTilesPerType[cnt1]) {
      // Do underflows here
      for (cnt2 = NumRegions; cnt2 < gNumTilesPerType[cnt1]; cnt2++) {
        gSmTileDB[dbSize].vo = s.vo;
        gSmTileDB[dbSize].usSubIndex = 0;
        gSmTileDB[dbSize].fType = cnt1;
        dbSize++;
      }
    }
  }

  gsStartRestrictedX = 0;
  gsStartRestrictedY = 0;

  // Calculate Scale factors because of restricted map scroll regions
  if (gMapInformation.ubRestrictedScrollID != 0) {
    int16_t sX1, sY1, sX2, sY2;

    CalculateRestrictedMapCoords(NORTH, &sX1, &sY1, &sX2, &gsStartRestrictedY, 640, 320);
    CalculateRestrictedMapCoords(WEST, &sX1, &sY1, &gsStartRestrictedX, &sY2, 640, 320);
  }

  // Copy over shade tables from main tileset
  CopyOverheadDBShadetablesFromTileset();
}

void DeleteOverheadDB() {
  int32_t cnt;

  for (cnt = 0; cnt < NUMBEROFTILETYPES; cnt++) {
    DeleteVideoObject(gSmTileSurf[cnt].vo);
  }
}

BOOLEAN GetClosestItemPool(int16_t sSweetGridNo, struct ITEM_POOL **ppReturnedItemPool,
                           uint8_t ubRadius, int8_t bLevel) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct ITEM_POOL *pItemPool;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        // Go on sweet stop
        if (GetItemPool(sGridNo, &pItemPool, bLevel)) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);

          if (uiRange < uiLowestRange) {
            (*ppReturnedItemPool) = pItemPool;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }

  return (fFound);
}

BOOLEAN GetClosestMercInOverheadMap(int16_t sSweetGridNo, struct SOLDIERTYPE **ppReturnedSoldier,
                                    uint8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSweetGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSweetGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        // Go on sweet stop
        if (gpWorldLevelData[sGridNo].pMercHead != NULL &&
            gpWorldLevelData[sGridNo].pMercHead->pSoldier->bVisible != -1) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSweetGridNo, sGridNo);

          if (uiRange < uiLowestRange) {
            (*ppReturnedSoldier) = gpWorldLevelData[sGridNo].pMercHead->pSoldier;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }

  return (fFound);
}

void DisplayMercNameInOverhead(struct SOLDIERTYPE *pSoldier) {
  int16_t sWorldScreenX, sX;
  int16_t sWorldScreenY, sY;

  // Get Screen position of guy.....
  GetWorldXYAbsoluteScreenXY((pSoldier->sX / CELL_X_SIZE), (pSoldier->sY / CELL_Y_SIZE),
                             &sWorldScreenX, &sWorldScreenY);

  sWorldScreenX = gsStartRestrictedX + (sWorldScreenX / 5) + 5;
  sWorldScreenY = gsStartRestrictedY + (sWorldScreenY / 5) + (pSoldier->sHeightAdjustment / 5) +
                  (gpWorldLevelData[pSoldier->sGridNo].sHeight / 5) - 8;

  sWorldScreenY += (gsRenderHeight / 5);

  // Display name
  SetFont(TINYFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_WHITE);

  // Center here....
  FindFontCenterCoordinates(sWorldScreenX, sWorldScreenY, (int16_t)(1), 1, pSoldier->name,
                            TINYFONT1, &sX, &sY);

  // OK, selected guy is here...
  gprintfdirty(sX, sY, pSoldier->name);
  mprintf(sX, sY, pSoldier->name);
}

void HandleOverheadMap() {
  static BOOLEAN fFirst = TRUE;
  struct SOLDIERTYPE *pSoldier;

  if (fFirst) {
    fFirst = FALSE;
  }

  gfInOverheadMap = TRUE;
  gfOverItemPool = FALSE;

  // Check tileset numbers
  if (gubSmTileNum != giCurrentTilesetID) {
    // If loaded, unload!
    if (gfSmTileLoaded) {
      // Unload
      DeleteOverheadDB();

      // Force load
      gfSmTileLoaded = FALSE;
    }
  }

  gubSmTileNum = (uint8_t)giCurrentTilesetID;

  if (gfSmTileLoaded == FALSE) {
    // LOAD LAND
    InitNewOverheadDB(gubSmTileNum);
    gfSmTileLoaded = TRUE;
  }

  // restore background rects
  RestoreBackgroundRects();

  // RENDER!!!!!!!!
  RenderOverheadMap(0, (WORLD_COLS / 2), 0, 0, 640, 320);

  HandleTalkingAutoFaces();

  if (!gfEditMode) {
    // CHECK FOR UI
    if (gfTacticalPlacementGUIActive) {
      TacticalPlacementHandle();
      if (!gfTacticalPlacementGUIActive) {
        return;
      }
    } else {
      HandleOverheadUI();

      if (!gfInOverheadMap) {
        return;
      }
      RenderTacticalInterface();
      RenderRadarScreen();
      RenderClock(CLOCK_X, CLOCK_Y);
      RenderTownIDString();

      HandleAutoFaces();
    }
  }

  if (!gfEditMode && !gfTacticalPlacementGUIActive) {
    int16_t usMapPos;
    struct ITEM_POOL *pItemPool;

    gfUIHandleSelectionAboveGuy = FALSE;

    HandleAnyMercInSquadHasCompatibleStuff((int8_t)CurrentSquad(), NULL, TRUE);

    if (GetOverheadMouseGridNo(&usMapPos)) {
      // ATE: Find the closest item pool within 5 tiles....
      if (GetClosestItemPool(usMapPos, &pItemPool, 1, 0)) {
        struct STRUCTURE *pStructure = NULL;
        int16_t sIntTileGridNo;
        int8_t bZLevel = 0;
        int16_t sActionGridNo = usMapPos;

        // Get interactive tile...
        if (ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure,
                                                               FALSE)) {
          sActionGridNo = sIntTileGridNo;
        }

        bZLevel = GetZLevelOfItemPoolGivenStructure(sActionGridNo, 0, pStructure);

        if (AnyItemsVisibleOnLevel(pItemPool, bZLevel)) {
          DrawItemPoolList(pItemPool, usMapPos, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos,
                           gusMouseYPos);

          gfOverItemPool = TRUE;
          gsOveritemPoolGridNo = pItemPool->sGridNo;
        }
      }

      if (GetClosestItemPool(usMapPos, &pItemPool, 1, 1)) {
        int8_t bZLevel = 0;

        if (AnyItemsVisibleOnLevel(pItemPool, bZLevel)) {
          DrawItemPoolList(pItemPool, usMapPos, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos,
                           (uint16_t)(gusMouseYPos - 5));

          gfOverItemPool = TRUE;
          gsOveritemPoolGridNo = pItemPool->sGridNo;
        }
      }
    }

    if (GetOverheadMouseGridNoForFullSoldiersGridNo(&usMapPos)) {
      if (GetClosestMercInOverheadMap(usMapPos, &pSoldier, 1)) {
        if (pSoldier->bTeam == gbPlayerNum) {
          gfUIHandleSelectionAboveGuy = TRUE;
          gsSelectedGuy = GetSolID(pSoldier);
        }

        DisplayMercNameInOverhead(pSoldier);
      }
    }
  }

  RenderOverheadOverlays();
  if (!gfEditMode && !gfTacticalPlacementGUIActive && gusSelectedSoldier != NOBODY) {
    pSoldier = MercPtrs[gusSelectedSoldier];

    DisplayMercNameInOverhead(pSoldier);
  }

  RenderButtons();
  StartFrameBufferRender();

  // save background rects
  SaveBackgroundRects();

  RenderButtonsFastHelp();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  fInterfacePanelDirty = FALSE;
}

BOOLEAN InOverheadMap() { return (gfInOverheadMap); }

void GoIntoOverheadMap() {
  struct VObject *hVObject;

  gfInOverheadMap = TRUE;

  MSYS_DefineRegion(&OverheadBackgroundRegion, 0, 0, 640, 360, MSYS_PRIORITY_HIGH, CURSOR_NORMAL,
                    MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
  // Add region
  MSYS_AddRegion(&OverheadBackgroundRegion);

  MSYS_DefineRegion(&OverheadRegion, 0, 0, gsVIEWPORT_END_X, 320, MSYS_PRIORITY_HIGH, CURSOR_NORMAL,
                    MoveOverheadRegionCallback, ClickOverheadRegionCallback);
  // Add region
  MSYS_AddRegion(&OverheadRegion);

  // LOAD CLOSE ANIM
  if (!AddVObjectFromFile("INTERFACE\\MAP_BORD.sti", &uiOVERMAP))
    AssertMsg(0, "Missing INTERFACE\\MAP_BORD.sti");

  // LOAD PERSONS
  if (!AddVObjectFromFile("INTERFACE\\PERSONS.sti", &uiPERSONS))
    AssertMsg(0, "Missing INTERFACE\\PERSONS.sti");

  // Add shades to persons....
  GetVideoObject(&hVObject, uiPERSONS);
  VObjectUpdateShade(hVObject, 0, 256, 256, 256, FALSE);
  VObjectUpdateShade(hVObject, 1, 310, 310, 310, FALSE);
  VObjectUpdateShade(hVObject, 2, 0, 0, 0, FALSE);

  gfOverheadMapDirty = TRUE;

  if (!gfEditMode) {
    // Make sure we are in team panel mode...
    gfSwitchPanel = TRUE;
    gbNewPanel = TEAM_PANEL;
    gubNewPanelParam = (uint8_t)gusSelectedSoldier;
    fInterfacePanelDirty = DIRTYLEVEL2;

    // Disable tactical buttons......
    if (!gfEnterTacticalPlacementGUI) {
      // Handle switch of panel....
      HandleTacticalPanelSwitch();
      DisableTacticalTeamPanelButtons(TRUE);
    }

    EmptyBackgroundRects();
  }
}

void HandleOverheadUI() {
  InputAtom InputEvent;
  int16_t sMousePos = 0;
  uint8_t ubID;

  // CHECK FOR MOUSE OVER REGIONS...
  if (GetOverheadMouseGridNo(&sMousePos)) {
    // Look quickly for a soldier....
    ubID = QuickFindSoldier(sMousePos);

    if (ubID != NOBODY) {
      // OK, selected guy is here...
      // gprintfdirty( gusMouseXPos, gusMouseYPos, MercPtrs[ ubID ]->name );
      // mprintf( gusMouseXPos, gusMouseYPos, MercPtrs[ ubID ]->name );
    }
  }

  while (DequeueEvent(&InputEvent) == TRUE) {
    if ((InputEvent.usEvent == KEY_DOWN)) {
      switch (InputEvent.usParam) {
        case (ESC):
        case (INSERT):

          KillOverheadMap();
          break;

        case ('x'):
          if ((InputEvent.usKeyState & ALT_DOWN)) {
            HandleShortCutExitState();
          }
          break;
      }
    }
  }
}

void KillOverheadMap() {
  gfInOverheadMap = FALSE;
  SetRenderFlags(RENDER_FLAG_FULL);
  RenderWorld();

  MSYS_RemoveRegion(&OverheadRegion);
  MSYS_RemoveRegion(&OverheadBackgroundRegion);

  DeleteVideoObjectFromIndex(uiOVERMAP);
  DeleteVideoObjectFromIndex(uiPERSONS);

  HandleTacticalPanelSwitch();
  DisableTacticalTeamPanelButtons(FALSE);
}

int16_t GetOffsetLandHeight(int32_t sGridNo) {
  int16_t sTileHeight;

  sTileHeight = gpWorldLevelData[sGridNo].sHeight;

  return (sTileHeight);
}

int16_t GetModifiedOffsetLandHeight(int32_t sGridNo) {
  int16_t sTileHeight;
  int16_t sModifiedTileHeight;

  sTileHeight = gpWorldLevelData[sGridNo].sHeight;

  sModifiedTileHeight = (((sTileHeight / 80) - 1) * 80);

  if (sModifiedTileHeight < 0) {
    sModifiedTileHeight = 0;
  }

  return (sModifiedTileHeight);
}

void RenderOverheadMap(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                       int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sModifiedHeight = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint32_t usTileIndex;
  int16_t sX, sY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  struct LEVELNODE *pNode;
  SMALL_TILE_DB *pTile;
  int16_t sHeight;
  struct VObject *hVObject;
  int16_t sX1, sX2, sY1, sY2;

  // Get video object for persons...
  GetVideoObject(&hVObject, uiPERSONS);

  if (gfOverheadMapDirty) {
    // Black out.......
    VSurfaceColorFill(vsFB, sStartPointX_S, sStartPointY_S, sEndXS, sEndYS, 0);

    InvalidateScreen();
    gfOverheadMapDirty = FALSE;

    // Begin Render Loop
    sAnchorPosX_M = sStartPointX_M;
    sAnchorPosY_M = sStartPointY_M;
    sAnchorPosX_S = sStartPointX_S;
    sAnchorPosY_S = sStartPointY_S;

    pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);

    do {
      fEndRenderRow = FALSE;
      sTempPosX_M = sAnchorPosX_M;
      sTempPosY_M = sAnchorPosY_M;
      sTempPosX_S = sAnchorPosX_S;
      sTempPosY_S = sAnchorPosY_S;

      if (bXOddFlag > 0) sTempPosX_S += 4;

      do {
        usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

        if (usTileIndex < GRIDSIZE) {
          sHeight = (GetOffsetLandHeight(usTileIndex) / 5);

          pNode = gpWorldLevelData[usTileIndex].pLandStart;
          while (pNode != NULL) {
            pTile = &(gSmTileDB[pNode->usIndex]);

            sX = sTempPosX_S;
            sY = sTempPosY_S - sHeight + (gsRenderHeight / 5);

            pTile->vo->pShadeCurrent = gSmTileSurf[pTile->fType].vo->pShades[pNode->ubShadeLevel];

            Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, pTile->vo,
                                                sX, sY, pTile->usSubIndex);

            pNode = pNode->pPrevNode;
          }
        }

        sTempPosX_S += 8;
        sTempPosX_M++;
        sTempPosY_M--;

        if (sTempPosX_S >= sEndXS) {
          fEndRenderRow = TRUE;
        }

      } while (!fEndRenderRow);

      if (bXOddFlag > 0) {
        sAnchorPosY_M++;
      } else {
        sAnchorPosX_M++;
      }

      bXOddFlag = !bXOddFlag;
      sAnchorPosY_S += 2;

      if (sAnchorPosY_S >= sEndYS) {
        fEndRenderCol = TRUE;
      }

    } while (!fEndRenderCol);

    // Begin Render Loop
    sAnchorPosX_M = sStartPointX_M;
    sAnchorPosY_M = sStartPointY_M;
    sAnchorPosX_S = sStartPointX_S;
    sAnchorPosY_S = sStartPointY_S;
    bXOddFlag = 0;
    fEndRenderRow = FALSE;
    fEndRenderCol = FALSE;

    do {
      fEndRenderRow = FALSE;
      sTempPosX_M = sAnchorPosX_M;
      sTempPosY_M = sAnchorPosY_M;
      sTempPosX_S = sAnchorPosX_S;
      sTempPosY_S = sAnchorPosY_S;

      if (bXOddFlag > 0) sTempPosX_S += 4;

      do {
        usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

        if (usTileIndex < GRIDSIZE) {
          sHeight = (GetOffsetLandHeight(usTileIndex) / 5);
          sModifiedHeight = (GetModifiedOffsetLandHeight(usTileIndex) / 5);

          pNode = gpWorldLevelData[usTileIndex].pObjectHead;
          while (pNode != NULL) {
            if (pNode->usIndex < NUMBEROFTILES) {
              // Don't render itempools!
              if (!(pNode->uiFlags & LEVELNODE_ITEM)) {
                pTile = &(gSmTileDB[pNode->usIndex]);

                sX = sTempPosX_S;
                sY = sTempPosY_S;

                if (gTileDatabase[pNode->usIndex].uiFlags & IGNORE_WORLD_HEIGHT) {
                  sY -= sModifiedHeight;
                } else {
                  sY -= sHeight;
                }

                sY += (gsRenderHeight / 5);

                pTile->vo->pShadeCurrent =
                    gSmTileSurf[pTile->fType].vo->pShades[pNode->ubShadeLevel];

                // RENDER!
                Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                    pTile->vo, sX, sY, pTile->usSubIndex);
              }
            }

            pNode = pNode->pNext;
          }

          pNode = gpWorldLevelData[usTileIndex].pShadowHead;
          while (pNode != NULL) {
            if (pNode->usIndex < NUMBEROFTILES) {
              pTile = &(gSmTileDB[pNode->usIndex]);
              sX = sTempPosX_S;
              sY = sTempPosY_S - sHeight;

              sY += (gsRenderHeight / 5);

              pTile->vo->pShadeCurrent = gSmTileSurf[pTile->fType].vo->pShades[pNode->ubShadeLevel];

              // RENDER!
              Blt8BPPDataTo16BPPBufferShadow((uint16_t *)pDestBuf, uiDestPitchBYTES, pTile->vo, sX,
                                             sY, pTile->usSubIndex);
            }

            pNode = pNode->pNext;
          }

          pNode = gpWorldLevelData[usTileIndex].pStructHead;

          while (pNode != NULL) {
            if (pNode->usIndex < NUMBEROFTILES) {
              // Don't render itempools!
              if (!(pNode->uiFlags & LEVELNODE_ITEM)) {
                pTile = &(gSmTileDB[pNode->usIndex]);

                sX = sTempPosX_S;
                sY = sTempPosY_S - (gTileDatabase[pNode->usIndex].sOffsetHeight / 5);

                if (gTileDatabase[pNode->usIndex].uiFlags & IGNORE_WORLD_HEIGHT) {
                  sY -= sModifiedHeight;
                } else {
                  sY -= sHeight;
                }

                sY += (gsRenderHeight / 5);

                pTile->vo->pShadeCurrent =
                    gSmTileSurf[pTile->fType].vo->pShades[pNode->ubShadeLevel];

                // RENDER!
                Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                    pTile->vo, sX, sY, pTile->usSubIndex);
              }
            }

            pNode = pNode->pNext;
          }
        }

        sTempPosX_S += 8;
        sTempPosX_M++;
        sTempPosY_M--;

        if (sTempPosX_S >= sEndXS) {
          fEndRenderRow = TRUE;
        }

      } while (!fEndRenderRow);

      if (bXOddFlag > 0) {
        sAnchorPosY_M++;
      } else {
        sAnchorPosX_M++;
      }

      bXOddFlag = !bXOddFlag;
      sAnchorPosY_S += 2;

      if (sAnchorPosY_S >= sEndYS) {
        fEndRenderCol = TRUE;
      }

    } while (!fEndRenderCol);

    {
      // ROOF RENDR LOOP
      // Begin Render Loop
      sAnchorPosX_M = sStartPointX_M;
      sAnchorPosY_M = sStartPointY_M;
      sAnchorPosX_S = sStartPointX_S;
      sAnchorPosY_S = sStartPointY_S;
      bXOddFlag = 0;
      fEndRenderRow = FALSE;
      fEndRenderCol = FALSE;

      do {
        fEndRenderRow = FALSE;
        sTempPosX_M = sAnchorPosX_M;
        sTempPosY_M = sAnchorPosY_M;
        sTempPosX_S = sAnchorPosX_S;
        sTempPosY_S = sAnchorPosY_S;

        if (bXOddFlag > 0) sTempPosX_S += 4;

        do {
          usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

          if (usTileIndex < GRIDSIZE) {
            sHeight = (GetOffsetLandHeight(usTileIndex) / 5);

            pNode = gpWorldLevelData[usTileIndex].pRoofHead;
            while (pNode != NULL) {
              if (pNode->usIndex < NUMBEROFTILES) {
                if (!(pNode->uiFlags & LEVELNODE_HIDDEN)) {
                  pTile = &(gSmTileDB[pNode->usIndex]);

                  sX = sTempPosX_S;
                  sY = sTempPosY_S - (gTileDatabase[pNode->usIndex].sOffsetHeight / 5) - sHeight;

                  sY -= (WALL_HEIGHT / 5);

                  sY += (gsRenderHeight / 5);

                  pTile->vo->pShadeCurrent =
                      gSmTileSurf[pTile->fType].vo->pShades[pNode->ubShadeLevel];

                  // RENDER!
                  Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                      pTile->vo, sX, sY, pTile->usSubIndex);
                }
              }
              pNode = pNode->pNext;
            }
          }

          sTempPosX_S += 8;
          sTempPosX_M++;
          sTempPosY_M--;

          if (sTempPosX_S >= sEndXS) {
            fEndRenderRow = TRUE;
          }

        } while (!fEndRenderRow);

        if (bXOddFlag > 0) {
          sAnchorPosY_M++;
        } else {
          sAnchorPosX_M++;
        }

        bXOddFlag = !bXOddFlag;
        sAnchorPosY_S += 2;

        if (sAnchorPosY_S >= sEndYS) {
          fEndRenderCol = TRUE;
        }

      } while (!fEndRenderCol);
    }

    VSurfaceUnlock(vsFB);

    // OK, blacken out edges of smaller maps...
    if (gMapInformation.ubRestrictedScrollID != 0) {
      CalculateRestrictedMapCoords(NORTH, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS);
      VSurfaceColorFill(vsFB, sX1, sY1, sX2, sY2, Get16BPPColor(FROMRGB(0, 0, 0)));

      CalculateRestrictedMapCoords(WEST, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS);
      VSurfaceColorFill(vsFB, sX1, sY1, sX2, sY2, Get16BPPColor(FROMRGB(0, 0, 0)));

      CalculateRestrictedMapCoords(SOUTH, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS);
      VSurfaceColorFill(vsFB, sX1, sY1, sX2, sY2, Get16BPPColor(FROMRGB(0, 0, 0)));

      CalculateRestrictedMapCoords(EAST, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS);
      VSurfaceColorFill(vsFB, sX1, sY1, sX2, sY2, Get16BPPColor(FROMRGB(0, 0, 0)));
    }

    // Render border!
    BltVObjectFromIndex(vsFB, uiOVERMAP, 0, 0, 0);

    // Update the save buffer
    {
      uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
      uint8_t *pDestBuf, *pSrcBuf;
      uint16_t usWidth, usHeight;
      uint8_t ubBitDepth;

      // Update saved buffer - do for the viewport size ony!
      GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);

      pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);
      pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

      Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES,
                      0, 0, 0, 0, usWidth, usHeight);

      VSurfaceUnlock(vsFB);
      VSurfaceUnlock(vsSB);
    }
  }
}

void RenderOverheadOverlays() {
  uint32_t uiDestPitchBYTES;
  WORLDITEM *pWorldItem;
  uint32_t i;
  struct SOLDIERTYPE *pSoldier;
  struct VObject *hVObject;
  int16_t sX, sY;
  uint16_t end;
  uint16_t usLineColor = 0;
  uint8_t *pDestBuf;
  uint8_t ubPassengers = 0;

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  GetVideoObject(&hVObject, uiPERSONS);

  // SOLDIER OVERLAY
  if (gfTacticalPlacementGUIActive) {  // loop through only the player soldiers
    end = gTacticalStatus.Team[OUR_TEAM].bLastID;
  } else {  // loop through all soldiers.
    end = MAX_NUM_SOLDIERS;
  }
  for (i = 0; i < end; i++) {
    // First, check to see if the soldier exists and is in the sector.
    pSoldier = MercPtrs[i];
    if (!IsSolActive(pSoldier) || !pSoldier->bInSector) continue;
    // Soldier is here.  Calculate his screen position based on his current gridno.
    GetOverheadScreenXYFromGridNo(pSoldier->sGridNo, &sX, &sY);
    // Now, draw his "doll"

    // adjust for position.
    sX += 2;
    sY -= 5;
    // sScreenY -= 7;	//height of doll

    if (!gfTacticalPlacementGUIActive && pSoldier->bLastRenderVisibleValue == -1 &&
        !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
      continue;
    }

    if (pSoldier->sGridNo == NOWHERE) {
      continue;
    }

    sY -= (GetOffsetLandHeight(pSoldier->sGridNo) / 5);

    // Adjust for height...
    sY -= (pSoldier->sHeightAdjustment / 5);

    sY += (gsRenderHeight / 5);

    // Adjust shade a bit...
    SetObjectShade(hVObject, 0);

    // If on roof....
    if (pSoldier->sHeightAdjustment) {
      SetObjectShade(hVObject, 1);
    }

    if (pSoldier->ubID == gusSelectedSoldier) {
      if (gfRadarCurrentGuyFlash && !gfTacticalPlacementGUIActive) {
        SetObjectShade(hVObject, 2);
      }
    }
#ifdef JA2EDITOR
    if (gfEditMode && gpSelected &&
        gpSelected->pSoldier ==
            pSoldier) {  // editor:  show the selected edited merc as the yellow one.
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          0);
    } else
#endif
        if (!gfTacticalPlacementGUIActive) {  // normal
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          pSoldier->bTeam);
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sX, sY, (int16_t)(sX + 3), (int16_t)(sY + 9));
    } else if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {  // vehicle
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          9);
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (int16_t)(sX - 6), (int16_t)(sY),
                             (int16_t)(sX + 9), (int16_t)(sY + 10));
    }
    // else if( pSoldier->uiStatusFlags & (SOLDIER_PASSENGER | SOLDIER_DRIVER) )
    //{// //don't draw person, because they are inside the vehicle.
    //	ubPassengers++;
    //}
    else if (gpTacticalPlacementSelectedSoldier == pSoldier) {  // tactical placement selected merc
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          7);
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (int16_t)(sX - 2), (int16_t)(sY - 2),
                             (int16_t)(sX + 5), (int16_t)(sY + 11));
    } else if (gpTacticalPlacementHilightedSoldier == pSoldier &&
               pSoldier->uiStatusFlags) {  // tactical placement hilighted merc
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          8);
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (int16_t)(sX - 2), (int16_t)(sY - 2),
                             (int16_t)(sX + 5), (int16_t)(sY + 11));
    } else {  // normal
      Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY,
                                          pSoldier->bTeam);
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sX, sY, (int16_t)(sX + 3), (int16_t)(sY + 9));
    }
    if (ubPassengers) {
      SetFont(SMALLCOMPFONT);
      SetFontForeground(FONT_WHITE);
      gprintfdirty((int16_t)(sX - 3), sY, L"%d", ubPassengers);
      mprintf_buffer(pDestBuf, uiDestPitchBYTES, SMALLCOMPFONT, sX - 3, sY, L"%d", ubPassengers);
    }
  }

  // ITEMS OVERLAY
  if (!gfTacticalPlacementGUIActive) {
    for (i = 0; i < guiNumWorldItems; i++) {
      pWorldItem = &gWorldItems[i];
      if (!pWorldItem || !pWorldItem->fExists ||
          (pWorldItem->bVisible != VISIBLE && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS))) {
        continue;
      }

      GetOverheadScreenXYFromGridNo(pWorldItem->sGridNo, &sX, &sY);

      // adjust for position.
      // sX += 2;
      sY += 6;
      sY -= (GetOffsetLandHeight(pWorldItem->sGridNo) / 5);

      sY += (gsRenderHeight / 5);

      if (gfRadarCurrentGuyFlash) {
        usLineColor = Get16BPPColor(FROMRGB(0, 0, 0));
      } else
        switch (pWorldItem->bVisible) {
          case HIDDEN_ITEM:
            usLineColor = Get16BPPColor(FROMRGB(0, 0, 255));
            break;
          case BURIED:
            usLineColor = Get16BPPColor(FROMRGB(255, 0, 0));
            break;
          case HIDDEN_IN_OBJECT:
            usLineColor = Get16BPPColor(FROMRGB(0, 0, 255));
            break;
          case INVISIBLE:
            usLineColor = Get16BPPColor(FROMRGB(0, 255, 0));
            break;
          case VISIBLE:
            usLineColor = Get16BPPColor(FROMRGB(255, 255, 255));
            break;
        }

      if (gfOverItemPool && gsOveritemPoolGridNo == pWorldItem->sGridNo) {
        usLineColor = Get16BPPColor(FROMRGB(255, 0, 0));
      }

      PixelDraw(FALSE, sX, sY, usLineColor, pDestBuf);

      InvalidateRegion(sX, sY, (int16_t)(sX + 1), (int16_t)(sY + 1));
    }
  }

  VSurfaceUnlock(vsFB);
}

void MoveInOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason) {}

void ClickOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason) {
  int32_t uiCellX, uiCellY;
  int16_t sWorldScreenX, sWorldScreenY;

  if (gfTacticalPlacementGUIActive) {
    HandleTacticalPlacementClicksInOverheadMap(reg, reason);
    return;
  }

  if (!(reg->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    reg->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    reg->uiFlags &= (~BUTTON_CLICKED_ON);
    sWorldScreenX = (gusMouseXPos - gsStartRestrictedX) * 5;
    sWorldScreenY = (gusMouseYPos - gsStartRestrictedY) * 5;

    // Get new proposed center location.
    GetFromAbsoluteScreenXYWorldXY(&uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY);

    SetRenderCenter((int16_t)uiCellX, (int16_t)uiCellY);

    KillOverheadMap();

  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    KillOverheadMap();
  }
}

void MoveOverheadRegionCallback(struct MOUSE_REGION *reg, int32_t reason) {}

void GetOverheadScreenXYFromGridNo(int16_t sGridNo, int16_t *psScreenX, int16_t *psScreenY) {
  GetWorldXYAbsoluteScreenXY((int16_t)(CenterX(sGridNo) / CELL_X_SIZE),
                             (int16_t)(CenterY(sGridNo) / CELL_Y_SIZE), psScreenX, psScreenY);
  *psScreenX /= 5;
  *psScreenY /= 5;

  *psScreenX += 5;
  *psScreenY += 5;

  // Subtract the height....
  //*psScreenY -= gpWorldLevelData[ sGridNo ].sHeight / 5;
}

BOOLEAN GetOverheadMouseGridNo(int16_t *psGridNo) {
  int32_t uiCellX, uiCellY;
  int16_t sWorldScreenX, sWorldScreenY;

  if ((OverheadRegion.uiFlags & MSYS_MOUSE_IN_AREA)) {
    // ATE: Adjust alogrithm values a tad to reflect map positioning
    sWorldScreenX = gsStartRestrictedX + (gusMouseXPos - 5) * 5;
    sWorldScreenY = gsStartRestrictedY + (gusMouseYPos - 8) * 5;

    // Get new proposed center location.
    GetFromAbsoluteScreenXYWorldXY(&uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY);

    // Get gridNo
    (*psGridNo) = (int16_t)MAPROWCOLTOPOS((uiCellY / CELL_Y_SIZE), (uiCellX / CELL_X_SIZE));

    // Adjust for height.....
    sWorldScreenY = sWorldScreenY + gpWorldLevelData[(*psGridNo)].sHeight;

    GetFromAbsoluteScreenXYWorldXY(&uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY);

    // Get gridNo
    (*psGridNo) = (int16_t)MAPROWCOLTOPOS((uiCellY / CELL_Y_SIZE), (uiCellX / CELL_X_SIZE));

    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN GetOverheadMouseGridNoForFullSoldiersGridNo(int16_t *psGridNo) {
  int32_t uiCellX, uiCellY;
  int16_t sWorldScreenX, sWorldScreenY;

  if ((OverheadRegion.uiFlags & MSYS_MOUSE_IN_AREA)) {
    // ATE: Adjust alogrithm values a tad to reflect map positioning
    sWorldScreenX = gsStartRestrictedX + (gusMouseXPos - 5) * 5;
    sWorldScreenY = gsStartRestrictedY + (gusMouseYPos) * 5;

    // Get new proposed center location.
    GetFromAbsoluteScreenXYWorldXY(&uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY);

    // Get gridNo
    (*psGridNo) = (int16_t)MAPROWCOLTOPOS((uiCellY / CELL_Y_SIZE), (uiCellX / CELL_X_SIZE));

    // Adjust for height.....
    sWorldScreenY = sWorldScreenY + gpWorldLevelData[(*psGridNo)].sHeight;

    GetFromAbsoluteScreenXYWorldXY(&uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY);

    // Get gridNo
    (*psGridNo) = (int16_t)MAPROWCOLTOPOS((uiCellY / CELL_Y_SIZE), (uiCellX / CELL_X_SIZE));

    return (TRUE);
  } else {
    return (FALSE);
  }
}

void CalculateRestrictedMapCoords(int8_t bDirection, int16_t *psX1, int16_t *psY1, int16_t *psX2,
                                  int16_t *psY2, int16_t sEndXS, int16_t sEndYS) {
  switch (bDirection) {
    case NORTH:

      *psX1 = 0;
      *psX2 = sEndXS;
      *psY1 = 0;
      *psY2 = (abs(NORMAL_MAP_SCREEN_TY - gsTLY) / 5);
      break;

    case WEST:

      *psX1 = 0;
      *psX2 = (abs(-NORMAL_MAP_SCREEN_X - gsTLX) / 5);
      *psY1 = 0;
      *psY2 = sEndYS;
      break;

    case SOUTH:

      *psX1 = 0;
      *psX2 = sEndXS;
      *psY1 = (NORMAL_MAP_SCREEN_HEIGHT - abs(NORMAL_MAP_SCREEN_BY - gsBLY)) / 5;
      *psY2 = sEndYS;
      break;

    case EAST:

      *psX1 = (NORMAL_MAP_SCREEN_WIDTH - abs(NORMAL_MAP_SCREEN_X - gsTRX)) / 5;
      *psX2 = sEndXS;
      *psY1 = 0;
      *psY2 = sEndYS;
      break;
  }
}

void CalculateRestrictedScaleFactors(int16_t *pScaleX, int16_t *pScaleY) {}

void CopyOverheadDBShadetablesFromTileset() {
  uint32_t uiLoop, uiLoop2;
  struct TILE_IMAGERY *pTileSurf;

  // Loop through tileset
  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    pTileSurf = (gTileSurfaceArray[uiLoop]);

    gSmTileSurf[uiLoop].vo->fFlags |= VOBJECT_FLAG_SHADETABLE_SHARED;

    for (uiLoop2 = 0; uiLoop2 < HVOBJECT_SHADE_TABLES; uiLoop2++) {
      gSmTileSurf[uiLoop].vo->pShades[uiLoop2] = pTileSurf->vo->pShades[uiLoop2];
    }
  }
}

void TrashOverheadMap() {
  // If loaded, unload!
  if (gfSmTileLoaded) {
    // Unload
    DeleteOverheadDB();

    // Force load
    gfSmTileLoaded = FALSE;
  }
}
