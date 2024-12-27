#ifndef __SMOOTHING_UTILS_H
#define __SMOOTHING_UTILS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

// Use these values when specifically replacing a wall with new type.
enum {                 // Wall tile types
  INTERIOR_L,          // interior wall with a top left orientation
  INTERIOR_R,          // interior wall with a top right orientation
  EXTERIOR_L,          // exterior wall with a top left orientation
  EXTERIOR_R,          // exterior wall with a top right orientation
  INTERIOR_CORNER,     // special interior end piece with top left orientation.
                       // The rest of these walls are special wall tiles for top right orientations.
  INTERIOR_BOTTOMEND,  // interior wall for bottom corner
  EXTERIOR_BOTTOMEND,  // exterior wall for bottom corner
  INTERIOR_EXTENDED,   // extended interior wall for top corner
  EXTERIOR_EXTENDED,   // extended exterior wall for top corner
  INTERIOR_EXTENDED_BOTTOMEND,  // extended interior wall for both top and bottom corners.
  EXTERIOR_EXTENDED_BOTTOMEND,  // extended exterior wall for both top and bottom corners.
  NUM_WALL_TYPES
};

// Use these values when passing a ubWallPiece to BuildWallPieces.
enum {
  NO_WALLS,
  INTERIOR_TOP,
  INTERIOR_BOTTOM,
  INTERIOR_LEFT,
  INTERIOR_RIGHT,
  EXTERIOR_TOP,
  EXTERIOR_BOTTOM,
  EXTERIOR_LEFT,
  EXTERIOR_RIGHT,
};

// in newsmooth.c
extern INT8 gbWallTileLUT[NUM_WALL_TYPES][7];
extern void EraseWalls(uint32_t iMapIndex);
extern void BuildWallPiece(uint32_t iMapIndex, UINT8 ubWallPiece, UINT16 usWallType);
// in Smoothing Utils
void RestoreWalls(uint32_t iMapIndex);
UINT16 SearchForRoofType(uint32_t iMapIndex);
UINT16 SearchForWallType(uint32_t iMapIndex);
BOOLEAN RoofAtGridNo(uint32_t iMapIndex);
BOOLEAN BuildingAtGridNo(uint32_t iMapIndex);
struct LEVELNODE* GetHorizontalWall(uint32_t iMapIndex);
struct LEVELNODE* GetVerticalWall(uint32_t iMapIndex);
struct LEVELNODE* GetVerticalFence(uint32_t iMapIndex);
struct LEVELNODE* GetHorizontalFence(uint32_t iMapIndex);
UINT16 GetHorizontalWallType(uint32_t iMapIndex);
UINT16 GetVerticalWallType(uint32_t iMapIndex);
void EraseHorizontalWall(uint32_t iMapIndex);
void EraseVerticalWall(uint32_t iMapIndex);
void ChangeHorizontalWall(uint32_t iMapIndex, UINT16 usNewPiece);
void ChangeVerticalWall(uint32_t iMapIndex, UINT16 usNewPiece);
UINT16 GetWallClass(struct LEVELNODE* pWall);
UINT16 GetVerticalWallClass(UINT16 iMapIndex);
UINT16 GetHorizontalWallClass(UINT16 iMapIndex);
BOOLEAN ValidDecalPlacement(uint32_t iMapIndex);

#endif
