// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __QUANTIZE_WRAP_H
#define __QUANTIZE_WRAP_H

#include "SGP/HImage.h"
#include "SGP/Types.h"

BOOLEAN QuantizeImage(uint8_t *pDest, uint8_t *pSrc, int16_t sWidth, int16_t sHeight,
                      struct SGPPaletteEntry *pPalette);
void MapPalette(uint8_t *pDest, uint8_t *pSrc, int16_t sWidth, int16_t sHeight, int16_t sNumColors,
                struct SGPPaletteEntry *pTable);

#endif
