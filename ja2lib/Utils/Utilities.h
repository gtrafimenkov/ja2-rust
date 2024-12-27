// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "SGP/Types.h"
#include "Tactical/OverheadTypes.h"

struct SGPPaletteEntry;

BOOLEAN CreateSGPPaletteFromCOLFile(struct SGPPaletteEntry* pPalette, SGPFILENAME ColFile);
BOOLEAN DisplayPaletteRep(PaletteRepID aPalRep, uint8_t ubXPos, uint8_t ubYPos,
                          uint32_t uiDestSurface);

void CopyFilename(char* pFilename, char* pDestination);

BOOLEAN WrapString(wchar_t* pStr, wchar_t* pStr2, size_t buf2Size, uint16_t usWidth,
                   int32_t uiFont);

#endif
