#ifndef __PALETTE_ENTRY_H
#define __PALETTE_ENTRY_H

#include "SGP/Types.h"

// Palette structure, mimics that of Win32
struct SGPPaletteEntry {
  uint8_t peRed;
  uint8_t peGreen;
  uint8_t peBlue;
  uint8_t peFlags;
};

#endif
