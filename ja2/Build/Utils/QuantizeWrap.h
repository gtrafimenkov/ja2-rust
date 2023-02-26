#ifndef __QUANTIZE_WRAP_H
#define __QUANTIZE_WRAP_H

#include "SGP/HImage.h"
#include "SGP/Types.h"

BOOLEAN QuantizeImage(UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight,
                      struct SGPPaletteEntry *pPalette);
void MapPalette(UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, INT16 sNumColors,
                struct SGPPaletteEntry *pTable);

#endif
