#include "Editor/SmoothingUtils.h"

#include <stdlib.h>

#include "BuildDefines.h"
#include "Editor/EditSys.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorUndo.h"
#include "Editor/SmartMethod.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"  //for GridNoOnVisibleWorldTile()
// #include "TileEngine/WorldDef.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"  //for RemoveXXXX()

extern uint16_t PickAWallPiece(uint16_t usWallPieceType);

// This method isn't foolproof, but because erasing large areas of buildings could result in
// multiple wall types for each building.  When processing the region, it is necessary to
// calculate the roof type by searching for the nearest roof tile.
uint16_t SearchForWallType(uint32_t iMapIndex) {
  uint32_t uiTileType;
  struct LEVELNODE *pWall;
  INT16 sOffset;
  INT16 x, y, sRadius = 0;
  if (gfBasement) {
    uint16_t usWallType;
    usWallType = GetRandomIndexByRange(FIRSTWALL, LASTWALL);
    if (usWallType == 0xffff) usWallType = FIRSTWALL;
    return usWallType;
  }
  while (sRadius < 32) {
    // NOTE:  start at the higher y value and go negative because it is possible to have another
    // structure type one tile north, but not one tile south -- so it'll find the correct wall
    // first.
    for (y = sRadius; y >= -sRadius; y--)
      for (x = -sRadius; x <= sRadius; x++) {
        if (abs(x) == abs(sRadius) || abs(y) == abs(sRadius)) {
          sOffset = y * WORLD_COLS + x;
          if (!GridNoOnVisibleWorldTile((INT16)(iMapIndex + sOffset))) {
            continue;
          }
          pWall = gpWorldLevelData[iMapIndex + sOffset].pStructHead;
          while (pWall) {
            GetTileType(pWall->usIndex, &uiTileType);
            if (uiTileType >= FIRSTWALL &&
                uiTileType <= LASTWALL) {  // found a roof, so return its type.
              return (uint16_t)uiTileType;
            }
            // if( uiTileType >= FIRSTWINDOW && uiTileType <= LASTWINDOW )
            //{	//Window types can be converted to a wall type.
            //	return (uint16_t)(FIRSTWALL + uiTileType - FIRSTWINDOW );
            //}
            pWall = pWall->pNext;
          }
        }
      }
    sRadius++;
  }
  return 0xffff;
}

// This method isn't foolproof, but because erasing large areas of buildings could result in
// multiple roof types for each building.  When processing the region, it is necessary to
// calculate the roof type by searching for the nearest roof tile.
uint16_t SearchForRoofType(uint32_t iMapIndex) {
  uint32_t uiTileType;
  struct LEVELNODE *pRoof;
  INT16 x, y, sRadius = 0;
  INT16 sOffset;
  while (sRadius < 32) {
    for (y = -sRadius; y <= sRadius; y++)
      for (x = -sRadius; x <= sRadius; x++) {
        if (abs(x) == abs(sRadius) || abs(y) == abs(sRadius)) {
          sOffset = y * WORLD_COLS + x;
          if (!GridNoOnVisibleWorldTile((INT16)(iMapIndex + sOffset))) {
            continue;
          }
          pRoof = gpWorldLevelData[iMapIndex + sOffset].pRoofHead;
          while (pRoof) {
            GetTileType(pRoof->usIndex, &uiTileType);
            if (uiTileType >= FIRSTROOF &&
                uiTileType <= LASTROOF) {  // found a roof, so return its type.
              return (uint16_t)uiTileType;
            }
            pRoof = pRoof->pNext;
          }
        }
      }
    sRadius++;
  }
  return 0xffff;
}

BOOLEAN RoofAtGridNo(uint32_t iMapIndex) {
  struct LEVELNODE *pRoof;
  uint32_t uiTileType;
  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;
  // Look through all objects and Search for type
  while (pRoof) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &uiTileType);
      if (uiTileType >= FIRSTROOF && uiTileType <= SECONDSLANTROOF) return TRUE;
      pRoof = pRoof->pNext;
    }
  }
  return FALSE;
}

BOOLEAN BuildingAtGridNo(uint32_t iMapIndex) {
  if (RoofAtGridNo(iMapIndex)) return TRUE;
  if (FloorAtGridNo(iMapIndex)) return TRUE;
  return FALSE;
}

BOOLEAN ValidDecalPlacement(uint32_t iMapIndex) {
  if (GetVerticalWall(iMapIndex) || GetHorizontalWall(iMapIndex) || GetVerticalFence(iMapIndex) ||
      GetHorizontalFence(iMapIndex))
    return TRUE;
  return FALSE;
}

struct LEVELNODE *GetVerticalWall(uint32_t iMapIndex) {
  struct LEVELNODE *pStruct;
  uint32_t uiTileType;
  uint16_t usWallOrientation;
  pStruct = gpWorldLevelData[iMapIndex].pStructHead;
  while (pStruct) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if ((uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
          (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) {
          return pStruct;
        }
      }
    }
    pStruct = pStruct->pNext;
  }
  return NULL;
}

struct LEVELNODE *GetHorizontalWall(uint32_t iMapIndex) {
  struct LEVELNODE *pStruct;
  uint32_t uiTileType;
  uint16_t usWallOrientation;
  pStruct = gpWorldLevelData[iMapIndex].pStructHead;
  while (pStruct) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if ((uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
          (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) {
          return pStruct;
        }
      }
    }
    pStruct = pStruct->pNext;
  }
  return NULL;
}

uint16_t GetVerticalWallType(uint32_t iMapIndex) {
  struct LEVELNODE *pWall;
  uint32_t uiTileType;
  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)
      uiTileType = SearchForWallType(iMapIndex);
    return (uint16_t)uiTileType;
  }
  return 0;
}

uint16_t GetHorizontalWallType(uint32_t iMapIndex) {
  struct LEVELNODE *pWall;
  uint32_t uiTileType;
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)
      uiTileType = SearchForWallType(iMapIndex);
    return (uint16_t)uiTileType;
  }
  return 0;
}

struct LEVELNODE *GetVerticalFence(uint32_t iMapIndex) {
  struct LEVELNODE *pStruct;
  uint32_t uiTileType;
  uint16_t usWallOrientation;
  pStruct = gpWorldLevelData[iMapIndex].pStructHead;
  while (pStruct) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if (uiTileType == FENCESTRUCT) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) {
          return pStruct;
        }
      }
    }
    pStruct = pStruct->pNext;
  }
  return NULL;
}

struct LEVELNODE *GetHorizontalFence(uint32_t iMapIndex) {
  struct LEVELNODE *pStruct;
  uint32_t uiTileType;
  uint16_t usWallOrientation;
  pStruct = gpWorldLevelData[iMapIndex].pStructHead;
  while (pStruct) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if (uiTileType == FENCESTRUCT) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) {
          return pStruct;
        }
      }
    }
    pStruct = pStruct->pNext;
  }
  return NULL;
}

void EraseHorizontalWall(uint32_t iMapIndex) {
  struct LEVELNODE *pWall;
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    AddToUndoList(iMapIndex);
    RemoveStruct(iMapIndex, pWall->usIndex);
    RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
  }
}

void EraseVerticalWall(uint32_t iMapIndex) {
  struct LEVELNODE *pWall;
  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    AddToUndoList(iMapIndex);
    RemoveStruct(iMapIndex, pWall->usIndex);
    RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
  }
}

void ChangeHorizontalWall(uint32_t iMapIndex, uint16_t usNewPiece) {
  struct LEVELNODE *pWall;
  uint32_t uiTileType;
  uint16_t usTileIndex;
  INT16 sIndex;
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    if (uiTileType >= FIRSTWALL &&
        uiTileType <= LASTWALL) {  // Okay, we have the wall, now change it's type.
      sIndex = PickAWallPiece(usNewPiece);
      AddToUndoList(iMapIndex);
      GetTileIndexFromTypeSubIndex(uiTileType, sIndex, &usTileIndex);
      ReplaceStructIndex(iMapIndex, pWall->usIndex, usTileIndex);
    }
  }
}

void ChangeVerticalWall(uint32_t iMapIndex, uint16_t usNewPiece) {
  struct LEVELNODE *pWall;
  uint32_t uiTileType;
  uint16_t usTileIndex;
  INT16 sIndex;
  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    if (uiTileType >= FIRSTWALL &&
        uiTileType <= LASTWALL) {  // Okay, we have the wall, now change it's type.
      sIndex = PickAWallPiece(usNewPiece);
      AddToUndoList(iMapIndex);
      GetTileIndexFromTypeSubIndex(uiTileType, sIndex, &usTileIndex);
      ReplaceStructIndex(iMapIndex, pWall->usIndex, usTileIndex);
    }
  }
}

void RestoreWalls(uint32_t iMapIndex) {
  struct LEVELNODE *pWall = NULL;
  uint32_t uiTileType;
  uint16_t usWallType;
  uint16_t usWallOrientation;
  uint8_t ubSaveWallUIValue;
  BOOLEAN fDone = FALSE;

  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)
      usWallType = SearchForWallType(iMapIndex);
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    AddToUndoList(iMapIndex);
    RemoveStruct(iMapIndex, pWall->usIndex);
    RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
    switch (usWallOrientation) {
      case OUTSIDE_TOP_LEFT:
        BuildWallPiece(iMapIndex, INTERIOR_BOTTOM, usWallType);
        break;
      case INSIDE_TOP_LEFT:
        BuildWallPiece(iMapIndex, EXTERIOR_BOTTOM, usWallType);
        break;
    }
    fDone = TRUE;
  }
  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)
      usWallType = SearchForWallType(iMapIndex);
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    AddToUndoList(iMapIndex);
    RemoveStruct(iMapIndex, pWall->usIndex);
    RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
    switch (usWallOrientation) {
      case OUTSIDE_TOP_RIGHT:
        BuildWallPiece(iMapIndex, INTERIOR_RIGHT, usWallType);
        break;
      case INSIDE_TOP_RIGHT:
        BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, usWallType);
        break;
    }
    fDone = TRUE;
  }
  if (fDone) {
    return;
  }
  // we are in a special case here.  The user is attempting to restore a wall, though nothing
  // is here.  We will hook into the smart wall method by tricking it into using the local wall
  // type, but only if we have adjacent walls.
  fDone = FALSE;
  if ((pWall = GetHorizontalWall(iMapIndex - 1))) fDone = TRUE;
  if (!fDone && (pWall = GetHorizontalWall(iMapIndex + 1))) fDone = TRUE;
  if (!fDone && (pWall = GetVerticalWall(iMapIndex - WORLD_COLS))) fDone = TRUE;
  if (!fDone && (pWall = GetVerticalWall(iMapIndex + WORLD_COLS))) fDone = TRUE;
  if (!fDone) return;
  // found a wall.  Let's back up the current wall value, and restore it after pasting a smart wall.
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)
      usWallType = SearchForWallType(iMapIndex);
    if (usWallType != 0xffff) {
      ubSaveWallUIValue = gubWallUIValue;    // save the wall UI value.
      gubWallUIValue = (uint8_t)usWallType;  // trick the UI value
      PasteSmartWall(iMapIndex);             // paste smart wall with fake UI value
      gubWallUIValue = ubSaveWallUIValue;    // restore the real UI value.
    }
  }
}

uint16_t GetWallClass(struct LEVELNODE *pWall) {
  uint16_t row, col, rowVariants;
  uint16_t usWallIndex;
  if (!pWall) return 0xffff;
  GetSubIndexFromTileIndex(pWall->usIndex, &usWallIndex);
  for (row = 0; row < NUM_WALL_TYPES; row++) {
    rowVariants = gbWallTileLUT[row][0];
    for (col = 1; col <= rowVariants; col++) {
      if (usWallIndex == gbWallTileLUT[row][col]) {
        return row;  // row is the wall class
      }
    }
  }
  return 0xffff;
}

uint16_t GetVerticalWallClass(uint16_t iMapIndex) {
  struct LEVELNODE *pWall;
  if ((pWall = GetVerticalWall(iMapIndex))) return GetWallClass(pWall);
  return 0xffff;
}

uint16_t GetHorizontalWallClass(uint16_t iMapIndex) {
  struct LEVELNODE *pWall;
  if ((pWall = GetVerticalWall(iMapIndex))) return GetWallClass(pWall);
  return 0xffff;
}
