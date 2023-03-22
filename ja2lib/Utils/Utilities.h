#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "SGP/Types.h"
#include "Tactical/OverheadTypes.h"

struct SGPPaletteEntry;

BOOLEAN CreateSGPPaletteFromCOLFile(struct SGPPaletteEntry *pPalette, SGPFILENAME ColFile);
BOOLEAN DisplayPaletteRep(PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface);

void CopyFilename(STR pFilename, STR pDestination);

BOOLEAN WrapString(STR16 pStr, STR16 pStr2, size_t buf2Size, UINT16 usWidth, INT32 uiFont);

#endif
