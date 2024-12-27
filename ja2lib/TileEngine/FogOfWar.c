#include "TileEngine/FogOfWar.h"

#include "SGP/Types.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/WorldMan.h"

// When line of sight reaches a gridno, and there is a light there, it turns it on.
// This is only done in the cave levels.
void RemoveFogFromGridNo(uint32_t uiGridNo) {
  INT32 i;
  INT32 x, y;
  x = uiGridNo % WORLD_COLS;
  y = uiGridNo / WORLD_COLS;
  for (i = 0; i < MAX_LIGHT_SPRITES; i++) {
    if (LightSprites[i].iX == x && LightSprites[i].iY == y) {
      if (!(LightSprites[i].uiFlags & LIGHT_SPR_ON)) {
        LightSpritePower(i, TRUE);
        LightDraw(LightSprites[i].uiLightType, LightSprites[i].iTemplate, LightSprites[i].iX,
                  LightSprites[i].iY, i);
        MarkWorldDirty();
        return;
      }
    }
  }
}
