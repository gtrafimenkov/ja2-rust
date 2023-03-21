#include "SGP/Video.h"

#include "SGP/VSurfaceInternal.h"
#include "SGP/VideoInternal.h"

UINT16 gusScreenWidth;
UINT16 gusScreenHeight;

void GetCurrentVideoSettings(UINT16 *usWidth, UINT16 *usHeight, UINT8 *ubBitDepth) {
  *usWidth = (UINT16)gusScreenWidth;
  *usHeight = (UINT16)gusScreenHeight;
  *ubBitDepth = 16;
}
