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
extern int8_t gbWallTileLUT[NUM_WALL_TYPES][7];
extern void EraseWalls(uint32_t iMapIndex);
extern void BuildWallPiece(uint32_t iMapIndex, uint8_t ubWallPiece, uint16_t usWallType);
// in Smoothing Utils
void RestoreWalls(uint32_t iMapIndex);
uint16_t SearchForRoofType(uint32_t iMapIndex);
uint16_t SearchForWallType(uint32_t iMapIndex);
BOOLEAN RoofAtGridNo(uint32_t iMapIndex);
BOOLEAN BuildingAtGridNo(uint32_t iMapIndex);
struct LEVELNODE* GetHorizontalWall(uint32_t iMapIndex);
struct LEVELNODE* GetVerticalWall(uint32_t iMapIndex);
struct LEVELNODE* GetVerticalFence(uint32_t iMapIndex);
struct LEVELNODE* GetHorizontalFence(uint32_t iMapIndex);
uint16_t GetHorizontalWallType(uint32_t iMapIndex);
uint16_t GetVerticalWallType(uint32_t iMapIndex);
void EraseHorizontalWall(uint32_t iMapIndex);
void EraseVerticalWall(uint32_t iMapIndex);
void ChangeHorizontalWall(uint32_t iMapIndex, uint16_t usNewPiece);
void ChangeVerticalWall(uint32_t iMapIndex, uint16_t usNewPiece);
uint16_t GetWallClass(struct LEVELNODE* pWall);
uint16_t GetVerticalWallClass(uint16_t iMapIndex);
uint16_t GetHorizontalWallClass(uint16_t iMapIndex);
BOOLEAN ValidDecalPlacement(uint32_t iMapIndex);

#endif
