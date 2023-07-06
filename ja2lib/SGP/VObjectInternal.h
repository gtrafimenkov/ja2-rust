#ifndef __VOBJECT_INTERNAL_H
#define __VOBJECT_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VObject.h"

// VObject is similar to Image, but has following additions: shades, glow, zstrip.
// VObject is always created from an HImage, that HImage must have subimages.
struct VObject {
  uint8_t ubBitDepth;
  void *image_data;
  uint32_t image_data_size;
  struct SGPPaletteEntry *pPaletteEntry;     // 8BPP Palette
  uint16_t *p16BPPPalette;                   // A 16BPP palette used for 8->16 blits
  struct Subimage *subimages;                // Object offset data etc
  uint16_t number_of_subimages;              // Total number of objects
  uint16_t *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  uint16_t *pShadeCurrent;                   // this is 16bit palette
  bool shared_shadetable;
  uint16_t *pGlow;            // glow highlight table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays
};

#endif
