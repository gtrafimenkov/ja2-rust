#include "SGP/Video.h"

#include "SGP/VideoInternal.h"

uint16_t gusScreenWidth;
uint16_t gusScreenHeight;

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight, uint8_t *ubBitDepth) {
  *usWidth = (uint16_t)gusScreenWidth;
  *usHeight = (uint16_t)gusScreenHeight;
  *ubBitDepth = 16;
}
