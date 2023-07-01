#ifndef __PALETTE_ENTRY_H
#define __PALETTE_ENTRY_H

#include "SGP/Types.h"

// Palette structure, mimics that of Win32
struct SGPPaletteEntry {
  UINT8 peRed;
  UINT8 peGreen;
  UINT8 peBlue;
  UINT8 _unused;
};

#endif
