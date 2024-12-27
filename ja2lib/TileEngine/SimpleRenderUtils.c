// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Types.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"

void MarkMapIndexDirty(int32_t iMapIndex) {
  gpWorldLevelData[iMapIndex].uiFlags |= MAPELEMENT_REDRAW;
  SetRenderFlags(RENDER_FLAG_MARKED);
}

void CenterScreenAtMapIndex(int32_t iMapIndex) {
  int16_t sWorldX, sWorldY;
  int16_t sCellX, sCellY;

  // Get X, Y world GRID Coordinates
  sWorldY = (iMapIndex / WORLD_COLS);
  sWorldX = iMapIndex - (sWorldY * WORLD_COLS);

  // Convert into cell coords
  sCellY = sWorldY * CELL_Y_SIZE;
  sCellX = sWorldX * CELL_X_SIZE;

  // Set the render values, so that the screen will render here next frame.
  gsRenderCenterX = sCellX;
  gsRenderCenterY = sCellY;

  SetRenderFlags(RENDER_FLAG_FULL);
}

void MarkWorldDirty() { SetRenderFlags(RENDER_FLAG_FULL); }
