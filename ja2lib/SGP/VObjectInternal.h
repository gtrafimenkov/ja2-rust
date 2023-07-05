#ifndef __VOBJECT_INTERNAL_H
#define __VOBJECT_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VObject.h"

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
struct VObject {
  UINT32 fFlags;                          // Special flags
  UINT32 image_data_size;                 // ETRLE data size
  struct SGPPaletteEntry *pPaletteEntry;  // 8BPP Palette
  UINT16 *p16BPPPalette;                  // A 16BPP palette used for 8->16 blits

  PTR image_data;                          // ETRLE pixel data
  struct Subimage *subimages;              // Object offset data etc
  UINT16 *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  UINT16 *pShadeCurrent;
  UINT16 *pGlow;              // glow highlight table
  UINT8 *pShade8;             // 8-bit shading index table
  UINT8 *pGlow8;              // 8-bit glow table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays

  UINT16 number_of_subimages;  // Total number of objects
  UINT8 ubBitDepth;            // BPP

  // Reserved for added room and 32-byte boundaries
  BYTE bReserved[1];
};

#endif
