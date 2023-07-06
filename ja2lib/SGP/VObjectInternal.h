#ifndef __VOBJECT_INTERNAL_H
#define __VOBJECT_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VObject.h"

// VObject is similar to Image, but has following additions: shades, glow, zstrip.
// VObject is always created from an HImage, that HImage must have subimages.
struct VObject {
  UINT8 ubBitDepth;
  void *image_data;
  UINT32 image_data_size;
  struct SGPPaletteEntry *pPaletteEntry;   // 8BPP Palette
  UINT16 *p16BPPPalette;                   // A 16BPP palette used for 8->16 blits
  struct Subimage *subimages;              // Object offset data etc
  UINT16 number_of_subimages;              // Total number of objects
  UINT16 *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  UINT16 *pShadeCurrent;                   // this is 16bit palette
  bool shared_shadetable;
  UINT16 *pGlow;              // glow highlight table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays
};

#endif
