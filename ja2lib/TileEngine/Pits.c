#include "TileEngine/Pits.h"

#include "SGP/Types.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "SysGlobals.h"
#include "Tactical/ActionItems.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/WorldMan.h"
#include "Utils/SoundControl.h"

// used by editor
BOOLEAN gfShowPits = FALSE;

BOOLEAN gfLoadPitsWithoutArming = FALSE;

void Add3X3Pit(INT32 iMapIndex) {
  EXITGRID ExitGrid;
  if (!gfEditMode) ApplyMapChangesToMapTempFile(TRUE);
  AddObjectToTail(iMapIndex + 159, REGWATERTEXTURE1);
  AddObjectToTail(iMapIndex - 1, REGWATERTEXTURE2);
  AddObjectToTail(iMapIndex - 161, REGWATERTEXTURE3);
  AddObjectToTail(iMapIndex + 160, REGWATERTEXTURE4);
  AddObjectToTail(iMapIndex, REGWATERTEXTURE5);
  AddObjectToTail(iMapIndex - 160, REGWATERTEXTURE6);
  AddObjectToTail(iMapIndex + 161, REGWATERTEXTURE7);
  AddObjectToTail(iMapIndex + 1, REGWATERTEXTURE8);
  AddObjectToTail(iMapIndex - 159, REGWATERTEXTURE9);
  if (!gfEditMode) {  // Add the exitgrids associated with the pit.
    ExitGrid.ubGotoSectorX = (UINT8)gWorldSectorX;
    ExitGrid.ubGotoSectorY = (UINT8)gWorldSectorY;
    ExitGrid.ubGotoSectorZ = (UINT8)(gbWorldSectorZ + 1);
    ExitGrid.usGridNo = (UINT16)iMapIndex;
    AddExitGridToWorld(iMapIndex + 159, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 1, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 161, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 160, &ExitGrid);
    AddExitGridToWorld(iMapIndex, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 160, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 161, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 1, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 159, &ExitGrid);
    RecompileLocalMovementCostsFromRadius((INT16)iMapIndex, 2);
  }

  MarkWorldDirty();
  if (!gfEditMode) ApplyMapChangesToMapTempFile(FALSE);
}

void Add5X5Pit(INT32 iMapIndex) {
  EXITGRID ExitGrid;
  if (!gfEditMode) ApplyMapChangesToMapTempFile(TRUE);
  AddObjectToTail(iMapIndex + 318, REGWATERTEXTURE10);
  AddObjectToTail(iMapIndex + 158, REGWATERTEXTURE11);
  AddObjectToTail(iMapIndex - 2, REGWATERTEXTURE12);
  AddObjectToTail(iMapIndex - 162, REGWATERTEXTURE13);
  AddObjectToTail(iMapIndex - 322, REGWATERTEXTURE14);
  AddObjectToTail(iMapIndex + 319, REGWATERTEXTURE15);
  AddObjectToTail(iMapIndex + 159, REGWATERTEXTURE16);
  AddObjectToTail(iMapIndex - 1, REGWATERTEXTURE17);
  AddObjectToTail(iMapIndex - 161, REGWATERTEXTURE18);
  AddObjectToTail(iMapIndex - 321, REGWATERTEXTURE19);
  AddObjectToTail(iMapIndex + 320, REGWATERTEXTURE20);
  AddObjectToTail(iMapIndex + 160, REGWATERTEXTURE21);
  AddObjectToTail(iMapIndex, REGWATERTEXTURE22);
  AddObjectToTail(iMapIndex - 160, REGWATERTEXTURE23);
  AddObjectToTail(iMapIndex - 320, REGWATERTEXTURE24);
  AddObjectToTail(iMapIndex + 321, REGWATERTEXTURE25);
  AddObjectToTail(iMapIndex + 161, REGWATERTEXTURE26);
  AddObjectToTail(iMapIndex + 1, REGWATERTEXTURE27);
  AddObjectToTail(iMapIndex - 159, REGWATERTEXTURE28);
  AddObjectToTail(iMapIndex - 319, REGWATERTEXTURE29);
  AddObjectToTail(iMapIndex + 322, REGWATERTEXTURE30);
  AddObjectToTail(iMapIndex + 162, REGWATERTEXTURE31);
  AddObjectToTail(iMapIndex + 2, REGWATERTEXTURE32);
  AddObjectToTail(iMapIndex - 158, REGWATERTEXTURE33);
  AddObjectToTail(iMapIndex - 318, REGWATERTEXTURE34);
  if (!gfEditMode) {  // Add the exitgrids associated with the pit.
    ExitGrid.ubGotoSectorX = (UINT8)gWorldSectorX;
    ExitGrid.ubGotoSectorY = (UINT8)gWorldSectorY;
    ExitGrid.ubGotoSectorZ = (UINT8)(gbWorldSectorZ + 1);
    ExitGrid.usGridNo = (UINT16)iMapIndex;
    AddExitGridToWorld(iMapIndex + 318, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 158, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 2, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 162, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 322, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 319, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 159, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 1, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 161, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 321, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 320, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 160, &ExitGrid);
    AddExitGridToWorld(iMapIndex, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 160, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 320, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 321, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 161, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 1, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 159, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 319, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 322, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 162, &ExitGrid);
    AddExitGridToWorld(iMapIndex + 2, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 158, &ExitGrid);
    AddExitGridToWorld(iMapIndex - 318, &ExitGrid);
    RecompileLocalMovementCostsFromRadius((INT16)iMapIndex, 3);
  }
  MarkWorldDirty();
  if (!gfEditMode) ApplyMapChangesToMapTempFile(FALSE);
}

void Remove3X3Pit(INT32 iMapIndex) {
  RemoveAllObjectsOfTypeRange(iMapIndex + 159, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 1, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 161, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 160, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 160, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 161, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 1, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 159, REGWATERTEXTURE, REGWATERTEXTURE);
  MarkWorldDirty();
}

void Remove5X5Pit(INT32 iMapIndex) {
  RemoveAllObjectsOfTypeRange(iMapIndex + 318, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 158, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 2, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 162, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 322, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 319, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 159, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 1, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 161, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 321, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 320, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 160, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 160, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 320, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 321, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 161, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 1, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 159, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 319, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 322, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 162, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex + 2, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 158, REGWATERTEXTURE, REGWATERTEXTURE);
  RemoveAllObjectsOfTypeRange(iMapIndex - 318, REGWATERTEXTURE, REGWATERTEXTURE);
  MarkWorldDirty();
}

void AddAllPits() {
  UINT32 i;
  for (i = 0; i < guiNumWorldItems; i++) {
    if (gWorldItems[i].o.usItem == ACTION_ITEM) {
      if (gWorldItems[i].o.bActionValue == ACTION_ITEM_SMALL_PIT)
        Add3X3Pit(gWorldItems[i].sGridNo);
      else if (gWorldItems[i].o.bActionValue == ACTION_ITEM_LARGE_PIT)
        Add5X5Pit(gWorldItems[i].sGridNo);
    }
  }
}

void RemoveAllPits() {
  UINT32 i;
  for (i = 0; i < guiNumWorldItems; i++) {
    if (gWorldItems[i].o.usItem == ACTION_ITEM) {
      if (gWorldItems[i].o.bActionValue == ACTION_ITEM_SMALL_PIT)
        Remove3X3Pit(gWorldItems[i].sGridNo);
      else if (gWorldItems[i].o.bActionValue == ACTION_ITEM_LARGE_PIT)
        Remove5X5Pit(gWorldItems[i].sGridNo);
    }
  }
}

void SearchForOtherMembersWithinPitRadiusAndMakeThemFall(INT16 sGridNo, INT16 sRadius) {
  INT16 x, y, sNewGridNo;
  UINT8 ubID;
  struct SOLDIERTYPE *pSoldier;

  PlayJA2Sample(CAVE_COLLAPSE, RATE_11025, SoundVolume(HIGHVOLUME, sGridNo), 1, SoundDir(sGridNo));
  for (y = -sRadius; y <= sRadius; y++)
    for (x = -sRadius; x <= sRadius; x++) {
      sNewGridNo = sGridNo + y * WORLD_COLS + x;
      // Validate gridno location, and check if there are any mercs here.  If there are
      // any mercs, we want them to fall below.  The exitgrid already exists at this location
      if (GridNoOnVisibleWorldTile(sNewGridNo)) {
        // Check if buddy exists here.....
        ubID = WhoIsThere2(sNewGridNo, 0);

        if (ubID != NOBODY) {
          // OK, make guy fall...
          // Set data to look for exit grid....
          pSoldier = MercPtrs[ubID];

          pSoldier->uiPendingActionData4 = sNewGridNo;

          EVENT_InitNewSoldierAnim(pSoldier, FALL_INTO_PIT, 0, FALSE);
        }
      }
    }
}

void HandleFallIntoPitFromAnimation(UINT8 ubID) {
  struct SOLDIERTYPE *pSoldier = MercPtrs[ubID];
  EXITGRID ExitGrid;
  INT16 sPitGridNo;
  // OK, get exit grid...

  sPitGridNo = (INT16)pSoldier->uiPendingActionData4;

  GetExitGrid(sPitGridNo, &ExitGrid);

  // Given exit grid, make buddy move to next sector....
  pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
  pSoldier->usStrategicInsertionData = ExitGrid.usGridNo;

  pSoldier->sSectorX = ExitGrid.ubGotoSectorX;
  pSoldier->sSectorY = ExitGrid.ubGotoSectorY;
  pSoldier->bSectorZ = ExitGrid.ubGotoSectorZ;

  // Remove from world......
  RemoveSoldierFromTacticalSector(pSoldier, TRUE);

  HandleSoldierLeavingSectorByThemSelf(pSoldier);

  SetSoldierHeight(pSoldier, 0);
}
