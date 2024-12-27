// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/SmartMethod.h"

#include "BuildDefines.h"
#include "Editor/EditorUndo.h"
#include "Editor/SmoothingUtils.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldDef.h"
#include "TileEngine/WorldMan.h"

uint8_t gubDoorUIValue = 0;
uint8_t gubWindowUIValue = 0;
uint8_t gubWallUIValue = FIRSTWALL;
uint8_t gubBrokenWallUIValue = 0;

void CalcSmartWallDefault(uint16_t *pusObjIndex, uint16_t *pusUseIndex) {
  *pusUseIndex = 0;
  *pusObjIndex = gubWallUIValue;
}

void CalcSmartDoorDefault(uint16_t *pusObjIndex, uint16_t *pusUseIndex) {
  *pusUseIndex = 4 * (gubDoorUIValue % 2);  // open or closed -- odd or even
  *pusObjIndex = FIRSTDOOR + gubDoorUIValue / 2;
}

void CalcSmartWindowDefault(uint16_t *pusObjIndex, uint16_t *pusUseIndex) {
  *pusUseIndex = 44 + gubWindowUIValue;  // first exterior top right oriented window
  *pusObjIndex = FIRSTWALL;
}

void CalcSmartBrokenWallDefault(uint16_t *pusObjIndex, uint16_t *pusUseIndex) {
  switch (gubBrokenWallUIValue) {
    case 0:
    case 1:
      *pusUseIndex = 49 + gubBrokenWallUIValue;
      break;
    case 3:
      *pusUseIndex = 62;
      break;
    case 4:
      *pusUseIndex = 64;
      break;
  }
  *pusObjIndex = FIRSTWALL;
}

uint16_t CalcSmartWindowIndex(uint16_t usWallOrientation) {
  return (uint16_t)(33 + usWallOrientation * 3 + gubWindowUIValue);
}

uint16_t CalcSmartDoorIndex(uint16_t usWallOrientation) {
  // convert the orientation values as the graphics are in reverse order
  // orientation values:   INSIDE_TOP_LEFT=1,  INSIDE_TOP_RIGHT=2,  OUTSIDE_TOP_LEFT=3,
  // OUTSIDE_TOP_RIGHT=4 door graphics order:	INSIDE_TOP_LEFT=15, INSIDE_TOP_RIGHT=10,
  // OUTSIDE_TOP_LEFT=5, OUTSIDE_TOP_RIGHT=0
  usWallOrientation = (4 - usWallOrientation) * 5;
  // 4 * (gubDoorUIValue%2) evaluates to +4 if the door is open, 0 if closed
  return (uint16_t)(1 + usWallOrientation + 4 * (gubDoorUIValue % 2));
}

uint16_t CalcSmartDoorType() { return (uint16_t)(FIRSTDOOR + gubDoorUIValue / 2); }

uint16_t CalcSmartBrokenWallIndex(uint16_t usWallOrientation) {
  if (gubBrokenWallUIValue == 2)  // the hole in the wall
    return 0xffff;
  if (gubBrokenWallUIValue < 2)  // broken walls
  {
    // convert the orientation value as the graphics are in a different order.
    // orientation values:   INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2, OUTSIDE_TOP_LEFT=3,
    // OUTSIDE_TOP_RIGHT=4
    //																			4
    // 6 8 10 door graphics order:  INSIDE_TOP_LEFT=4, INSIDE_TOP_RIGHT=6, OUTSIDE_TOP_LEFT=0,
    // OUTSIDE_TOP_RIGHT=2
    usWallOrientation = usWallOrientation * 2 + 2;
    usWallOrientation -= usWallOrientation > 6 ? 8 : 0;
    return (uint16_t)(usWallOrientation + 48 + gubBrokenWallUIValue);
  }

  // cracked and smudged walls

  // convert the orientation value as the graphics are in a different order.
  // orientation values:   INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2, OUTSIDE_TOP_LEFT=3,
  // OUTSIDE_TOP_RIGHT=4 door graphics order:  INSIDE_TOP_LEFT=1, INSIDE_TOP_RIGHT=2,
  // OUTSIDE_TOP_LEFT=5, OUTSIDE_TOP_RIGHT=6
  usWallOrientation += usWallOrientation > 1 ? 2 : 0;
  usWallOrientation +=
      gubBrokenWallUIValue == 4 ? 2 : 0;  // smudged type which is 2 index values higher.
  return (uint16_t)(usWallOrientation + 57);
}

void IncSmartWallUIValue() { gubWallUIValue += gubWallUIValue < LASTWALL ? 1 : -3; }

void DecSmartWallUIValue() { gubWallUIValue -= gubWallUIValue > FIRSTWALL ? 1 : -3; }

void IncSmartDoorUIValue() { gubDoorUIValue += gubDoorUIValue < 7 ? 1 : -7; }

void DecSmartDoorUIValue() { gubDoorUIValue -= gubDoorUIValue > 0 ? 1 : -7; }

void IncSmartWindowUIValue() { gubWindowUIValue += gubWindowUIValue < 2 ? 1 : -2; }

void DecSmartWindowUIValue() { gubWindowUIValue -= gubWindowUIValue > 0 ? 1 : -2; }

void IncSmartBrokenWallUIValue() { gubBrokenWallUIValue += gubBrokenWallUIValue < 4 ? 1 : -4; }

void DecSmartBrokenWallUIValue() { gubBrokenWallUIValue -= gubBrokenWallUIValue > 0 ? 1 : -4; }

BOOLEAN CalcWallInfoUsingSmartMethod(uint32_t iMapIndex, uint16_t *pusWallType,
                                     uint16_t *pusIndex) {
  return FALSE;
}

BOOLEAN CalcDoorInfoUsingSmartMethod(uint32_t iMapIndex, uint16_t *pusDoorType,
                                     uint16_t *pusIndex) {
  struct LEVELNODE *pWall = NULL;
  uint16_t usWallOrientation;
  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartDoorIndex(usWallOrientation) - 1;
    *pusDoorType = CalcSmartDoorType();
    return TRUE;
  }
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartDoorIndex(usWallOrientation) - 1;
    *pusDoorType = CalcSmartDoorType();
    return TRUE;
  }
  return FALSE;
}

BOOLEAN CalcWindowInfoUsingSmartMethod(uint32_t iMapIndex, uint16_t *pusWallType,
                                       uint16_t *pusIndex) {
  struct LEVELNODE *pWall = NULL;
  uint32_t uiTileType;
  uint16_t usWallOrientation;

  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    *pusWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a window, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      *pusWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartWindowIndex(usWallOrientation) - 1;
    return TRUE;
  }
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    *pusWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a window, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      *pusWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartWindowIndex(usWallOrientation) - 1;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN CalcBrokenWallInfoUsingSmartMethod(uint32_t iMapIndex, uint16_t *pusWallType,
                                           uint16_t *pusIndex) {
  struct LEVELNODE *pWall = NULL;
  uint32_t uiTileType;
  uint16_t usWallOrientation;

  if (gubBrokenWallUIValue == 2)  // the hole in the wall
  {
    *pusWallType = 0xffff;
    *pusIndex = 0xffff;  // but it won't draw it.
    return TRUE;
  }

  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    *pusWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a walltype, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      *pusWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartBrokenWallIndex(usWallOrientation) - 1;
    return TRUE;
  }
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    *pusWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a walltype, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      *pusWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    *pusIndex = CalcSmartBrokenWallIndex(usWallOrientation) - 1;
    return TRUE;
  }
  return FALSE;
}

// This is a very difficult function to document properly.  The reason being is that it is sooo
// subliminal by nature.  I have thought up of priorities and choose the best piece to draw based
// on the surrounding conditions.  Here are the priorities which are referenced below via comments:
// A)  If there is currently a bottom piece and a right piece, immediately exit.
// B)  We are currently over a bottom piece.  Now, we don't automatically want to draw a right piece
// here 		for multiple reasons.  First, the UI will be too quick and place bottom and
// right pieces for
// every 		place the user clicks, which isn't what we want.  Therefore, we look to see
// if there is a right
//    piece in the y-1 gridno.  It would then make sense to place a right piece down here.
//    Regardless,
//		if we encounter a bottom piece here, we will exit.
// C)  This is the counterpart to B, but we are looking at a current right piece, and are
// determining if 		we should place a bottom piece based on another bottom piece
// existing in the x-1 gridno. D)  Now, we analyse the neighboring tiles and determine the
// orientations that would add weight to the
//    current tile either towards drawing a horizontal piece or a vertical piece.
// E)  Now that we have the information, we give the highest priority to any weights that match the
// current 		wall piece type selected by the user.  Based on that, we will only consider
// the best match
// of the 		type and use it.  If there are no matches on type, we continue. F)  We
// failed to find weights matching the current wall type, but before we give up using the user's
// wall type, there are two more cases.  When there is a bottom wall in the y+1 position or a right
// wall in the x+1 position.  If there are matching walls, there, then we draw two pieces to connect
// the current gridno with the respective position.
void PasteSmartWall(uint32_t iMapIndex) {
  uint16_t usWallType;

  // These are the counters for the walls of each type
  uint16_t usNumV[4] = {0, 0, 0, 0};  // vertical wall weights
  uint16_t usNumH[4] = {0, 0, 0, 0};  // horizontal wall weights

  //*A* See above documentation
  if (GetVerticalWall(iMapIndex) && GetHorizontalWall(iMapIndex)) return;
  //*B* See above documentation
  usWallType = GetHorizontalWallType(iMapIndex);
  if (usWallType) {
    if (usWallType == gubWallUIValue) {
      usWallType = GetVerticalWallType(iMapIndex - WORLD_COLS);
      if (usWallType == gubWallUIValue) {
        if (FloorAtGridNo(iMapIndex + 1))
          BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, gubWallUIValue);
        else
          BuildWallPiece(iMapIndex, INTERIOR_RIGHT, gubWallUIValue);
        return;
      }
      usWallType = GetHorizontalWallType(iMapIndex - WORLD_COLS);
      if (usWallType == gubWallUIValue) {
        if (FloorAtGridNo(iMapIndex + 1)) {
          BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, gubWallUIValue);
          if (!GetHorizontalWall(iMapIndex - WORLD_COLS + 1))
            ChangeVerticalWall(iMapIndex, INTERIOR_EXTENDED);
        } else {
          BuildWallPiece(iMapIndex, INTERIOR_RIGHT, gubWallUIValue);
          if (!GetHorizontalWall(iMapIndex - WORLD_COLS + 1))
            ChangeVerticalWall(iMapIndex, EXTERIOR_EXTENDED);
        }
      }
    }
    return;
  }
  //*C* See above documentation
  usWallType = GetVerticalWallType(iMapIndex);
  if (usWallType) {
    if (usWallType == gubWallUIValue) {
      usWallType = GetHorizontalWallType(iMapIndex - 1);
      if (usWallType == gubWallUIValue) {
        if (FloorAtGridNo(iMapIndex + WORLD_COLS))
          BuildWallPiece(iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue);
        else
          BuildWallPiece(iMapIndex, INTERIOR_BOTTOM, gubWallUIValue);
      }
    }
    return;
  }
  //*D* See above documentation
  // Evaluate left adjacent tile
  if ((usWallType = GetVerticalWallType(iMapIndex - 1))) usNumH[usWallType - FIRSTWALL]++;
  if ((usWallType = GetHorizontalWallType(iMapIndex - 1))) usNumH[usWallType - FIRSTWALL]++;
  // Evaluate right adjacent tile
  if ((usWallType = GetHorizontalWallType(iMapIndex + 1))) usNumH[usWallType - FIRSTWALL]++;
  // Evaluate upper adjacent tile
  if ((usWallType = GetVerticalWallType(iMapIndex - WORLD_COLS))) usNumV[usWallType - FIRSTWALL]++;
  if ((usWallType = GetHorizontalWallType(iMapIndex - WORLD_COLS)))
    usNumV[usWallType - FIRSTWALL]++;
  // Evaluate lower adjacent tile
  if ((usWallType = GetVerticalWallType(iMapIndex + WORLD_COLS))) usNumV[usWallType - FIRSTWALL]++;
  //*E* See above documentation
  if (usNumV[gubWallUIValue - FIRSTWALL] | usNumH[gubWallUIValue - FIRSTWALL]) {
    if (usNumV[gubWallUIValue - FIRSTWALL] >= usNumH[gubWallUIValue - FIRSTWALL]) {
      if (FloorAtGridNo(iMapIndex + 1)) {  // inside
        BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, gubWallUIValue);
        // Change to extended piece if it is a new top right corner to cover the end part.
        if (GetHorizontalWall(iMapIndex - WORLD_COLS) &&
            !GetHorizontalWall(iMapIndex - WORLD_COLS + 1) &&
            !GetVerticalWall(iMapIndex - WORLD_COLS))
          ChangeVerticalWall(iMapIndex, INTERIOR_EXTENDED);
        else if (GetHorizontalWall(iMapIndex - WORLD_COLS) &&
                 !GetHorizontalWall(iMapIndex - WORLD_COLS - 1) &&
                 !GetVerticalWall(iMapIndex - WORLD_COLS - 1)) {
          ChangeVerticalWall(iMapIndex, INTERIOR_EXTENDED);
          EraseHorizontalWall(iMapIndex - WORLD_COLS);
        }
      } else {  // outside
        BuildWallPiece(iMapIndex, INTERIOR_RIGHT, gubWallUIValue);
        if (GetHorizontalWall(iMapIndex - WORLD_COLS) &&
            !GetHorizontalWall(iMapIndex - WORLD_COLS + 1) &&
            !GetVerticalWall(iMapIndex - WORLD_COLS))
          ChangeVerticalWall(iMapIndex, EXTERIOR_EXTENDED);
        else if (GetHorizontalWall(iMapIndex - WORLD_COLS) &&
                 !GetHorizontalWall(iMapIndex - WORLD_COLS - 1) &&
                 !GetVerticalWall(iMapIndex - WORLD_COLS - 1)) {
          ChangeVerticalWall(iMapIndex, EXTERIOR_EXTENDED);
          EraseHorizontalWall(iMapIndex - WORLD_COLS);
        }
      }
    } else {
      if (GetVerticalWall(iMapIndex - 1) && !GetVerticalWall(iMapIndex - WORLD_COLS - 1) &&
          !GetHorizontalWall(iMapIndex - WORLD_COLS - 1))
        EraseVerticalWall(iMapIndex - 1);
      if (FloorAtGridNo(iMapIndex + WORLD_COLS)) {  // inside
        BuildWallPiece(iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue);
        if (GetVerticalWall(iMapIndex + WORLD_COLS))
          ChangeVerticalWall(iMapIndex + WORLD_COLS, INTERIOR_EXTENDED);
        if (GetVerticalWall(iMapIndex + WORLD_COLS - 1) && !GetVerticalWall(iMapIndex - 1))
          ChangeVerticalWall(iMapIndex + WORLD_COLS - 1, INTERIOR_EXTENDED);
        else if (GetVerticalWall(iMapIndex - 1) && !GetVerticalWall(iMapIndex + WORLD_COLS - 1) &&
                 FloorAtGridNo(iMapIndex))
          ChangeVerticalWall(iMapIndex - 1, INTERIOR_BOTTOMEND);
      } else {  // outside
        BuildWallPiece(iMapIndex, INTERIOR_BOTTOM, gubWallUIValue);
        if (GetVerticalWall(iMapIndex + WORLD_COLS))
          ChangeVerticalWall(iMapIndex + WORLD_COLS, EXTERIOR_EXTENDED);
        if (GetVerticalWall(iMapIndex + WORLD_COLS - 1) && !GetVerticalWall(iMapIndex - 1))
          ChangeVerticalWall(iMapIndex + WORLD_COLS - 1, EXTERIOR_EXTENDED);
        else if (GetVerticalWall(iMapIndex - 1) && !GetVerticalWall(iMapIndex + WORLD_COLS - 1) &&
                 FloorAtGridNo(iMapIndex))
          ChangeVerticalWall(iMapIndex - 1, EXTERIOR_BOTTOMEND);
      }
    }
    return;
  }
  //*F* See above documentation
  usWallType = GetHorizontalWallType(iMapIndex + WORLD_COLS);
  if (usWallType == gubWallUIValue) {
    if (!GetHorizontalWall(iMapIndex + WORLD_COLS - 1)) EraseHorizontalWall(iMapIndex + WORLD_COLS);
    if (FloorAtGridNo(iMapIndex + 1)) {  // inside
      BuildWallPiece(iMapIndex + WORLD_COLS, EXTERIOR_RIGHT, gubWallUIValue);
      BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, gubWallUIValue);
      if (!GetVerticalWall(iMapIndex + WORLD_COLS * 2) &&
          FloorAtGridNo(iMapIndex + WORLD_COLS * 2 + 1))
        ChangeVerticalWall(iMapIndex + WORLD_COLS, INTERIOR_BOTTOMEND);
      else  // override the damn other smoothing.
        ChangeVerticalWall(iMapIndex + WORLD_COLS, INTERIOR_R);
    } else {  // outside
      BuildWallPiece(iMapIndex + WORLD_COLS, INTERIOR_RIGHT, gubWallUIValue);
      BuildWallPiece(iMapIndex, INTERIOR_RIGHT, gubWallUIValue);
      if (!GetVerticalWall(iMapIndex + WORLD_COLS * 2) &&
          !FloorAtGridNo(iMapIndex + WORLD_COLS * 2 + 1))
        ChangeVerticalWall(iMapIndex + WORLD_COLS, EXTERIOR_BOTTOMEND);
      else  // override the damn other smoothing.
        ChangeVerticalWall(iMapIndex + WORLD_COLS, EXTERIOR_R);
    }
    return;
  }
  usWallType = GetVerticalWallType(iMapIndex + 1);
  if (usWallType == gubWallUIValue) {
    if (FloorAtGridNo(iMapIndex + WORLD_COLS)) {  // inside
      BuildWallPiece(iMapIndex + 1, EXTERIOR_BOTTOM, gubWallUIValue);
      BuildWallPiece(iMapIndex, EXTERIOR_BOTTOM, gubWallUIValue);
      if (!GetVerticalWall(iMapIndex - WORLD_COLS + 1)) {
        EraseVerticalWall(iMapIndex + 1);
        ChangeVerticalWall(iMapIndex + WORLD_COLS + 1, INTERIOR_EXTENDED);
      }
      if (!GetVerticalWall(iMapIndex + WORLD_COLS + 1)) {
        if (!GetHorizontalWall(iMapIndex - WORLD_COLS + 1) &&
            !GetVerticalWall(iMapIndex - WORLD_COLS + 1) &&
            GetHorizontalWall(iMapIndex - WORLD_COLS + 2))
          ChangeVerticalWall(iMapIndex + 1, INTERIOR_EXTENDED);
        else
          ChangeVerticalWall(iMapIndex + 1, INTERIOR_BOTTOMEND);
      }
    } else {  // outside
      BuildWallPiece(iMapIndex + 1, INTERIOR_BOTTOM, gubWallUIValue);
      BuildWallPiece(iMapIndex, INTERIOR_BOTTOM, gubWallUIValue);
      if (!GetVerticalWall(iMapIndex - WORLD_COLS + 1)) {
        EraseVerticalWall(iMapIndex + 1);
        ChangeVerticalWall(iMapIndex + WORLD_COLS + 1, EXTERIOR_EXTENDED);
      }
      if (!GetVerticalWall(iMapIndex + WORLD_COLS + 1)) {
        if (!GetHorizontalWall(iMapIndex - WORLD_COLS + 1) &&
            !GetVerticalWall(iMapIndex - WORLD_COLS + 1) &&
            GetHorizontalWall(iMapIndex - WORLD_COLS + 2))
          ChangeVerticalWall(iMapIndex + 1, EXTERIOR_EXTENDED);
        else
          ChangeVerticalWall(iMapIndex + 1, EXTERIOR_BOTTOMEND);
      }
    }
    return;
  }
  // Check for the highest weight value.
}

void PasteSmartDoor(uint32_t iMapIndex) {
  struct LEVELNODE *pWall = NULL;
  uint16_t usTileIndex;
  uint16_t usDoorType;
  uint16_t usIndex;
  uint16_t usWallOrientation;

  if ((pWall = GetVerticalWall(iMapIndex))) {
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartDoorIndex(usWallOrientation);
    usDoorType = CalcSmartDoorType();
    AddToUndoList(iMapIndex);
    GetTileIndexFromTypeSubIndex(usDoorType, usIndex, &usTileIndex);
    ReplaceStructIndex(iMapIndex, pWall->usIndex, usTileIndex);
  }
  if ((pWall = GetHorizontalWall(iMapIndex))) {
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartDoorIndex(usWallOrientation);
    usDoorType = CalcSmartDoorType();
    AddToUndoList(iMapIndex);
    GetTileIndexFromTypeSubIndex(usDoorType, usIndex, &usTileIndex);
    ReplaceStructIndex(iMapIndex, pWall->usIndex, usTileIndex);
  }
}

void PasteSmartWindow(uint32_t iMapIndex) {
  uint16_t usNewWallIndex;

  struct LEVELNODE *pWall = NULL;
  uint32_t uiTileType;
  uint16_t usWallType;
  uint16_t usIndex;
  uint16_t usWallOrientation;

  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a window, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      usWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartWindowIndex(usWallOrientation);
    // Calculate the new graphic for the window type selected.

    AddToUndoList(iMapIndex);
    GetTileIndexFromTypeSubIndex(usWallType, usIndex, &usNewWallIndex);
    ReplaceStructIndex(iMapIndex, pWall->usIndex, usNewWallIndex);
  }
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a window, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      usWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartWindowIndex(usWallOrientation);
    // Calculate the new graphic for the window type selected.
    AddToUndoList(iMapIndex);
    GetTileIndexFromTypeSubIndex(usWallType, usIndex, &usNewWallIndex);
    ReplaceStructIndex(iMapIndex, pWall->usIndex, usNewWallIndex);
  }
}

void PasteSmartBrokenWall(uint32_t iMapIndex) {
  uint16_t usNewWallIndex;

  struct LEVELNODE *pWall;
  uint32_t uiTileType;
  uint16_t usWallType;
  uint16_t usIndex;
  uint16_t usWallOrientation;

  pWall = GetVerticalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      usWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartBrokenWallIndex(usWallOrientation);
    if (usIndex == 0xffff) {
      AddToUndoList(iMapIndex);
      RemoveStruct(iMapIndex, pWall->usIndex);
    } else {
      AddToUndoList(iMapIndex);
      GetTileIndexFromTypeSubIndex(usWallType, usIndex, &usNewWallIndex);
      ReplaceStructIndex(iMapIndex, pWall->usIndex, usNewWallIndex);
    }
  }
  pWall = GetHorizontalWall(iMapIndex);
  if (pWall) {
    GetTileType(pWall->usIndex, &uiTileType);
    usWallType = (uint16_t)uiTileType;
    if (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) {
      // We want to be able to replace doors with a window, however, the doors do not
      // contain the wall type, so we have to search for the nearest wall to extract it.
      usWallType = SearchForWallType(iMapIndex);
    }
    GetWallOrientation(pWall->usIndex, &usWallOrientation);
    usIndex = CalcSmartBrokenWallIndex(usWallOrientation);
    if (usIndex == 0xffff) {
      AddToUndoList(iMapIndex);
      RemoveStruct(iMapIndex, pWall->usIndex);
    } else {
      AddToUndoList(iMapIndex);
      GetTileIndexFromTypeSubIndex(usWallType, usIndex, &usNewWallIndex);
      ReplaceStructIndex(iMapIndex, pWall->usIndex, usNewWallIndex);
    }
    // Calculate the new graphic for the window type selected.
  }
}
