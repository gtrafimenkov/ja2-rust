#ifndef __VOBJECT_INTERNAL_H
#define __VOBJECT_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VObject.h"

typedef struct {
  uint16_t *p16BPPData;
  uint16_t usRegionIndex;
  uint8_t ubShadeLevel;
  uint16_t usWidth;
  uint16_t usHeight;
  int16_t sOffsetX;
  int16_t sOffsetY;
} SixteenBPPObjectInfo;

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
struct VObject {
  uint32_t fFlags;                        // Special flags
  uint32_t uiSizePixData;                 // ETRLE data size
  struct SGPPaletteEntry *pPaletteEntry;  // 8BPP Palette
  COLORVAL TransparentColor;              // Defaults to 0,0,0
  uint16_t *p16BPPPalette;                // A 16BPP palette used for 8->16 blits

  void *pPixData;              // ETRLE pixel data
  struct Subimage *subimages;  // Object offset data etc
  SixteenBPPObjectInfo *p16BPPObject;
  uint16_t *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  uint16_t *pShadeCurrent;
  uint16_t *pGlow;            // glow highlight table
  uint8_t *pShade8;           // 8-bit shading index table
  uint8_t *pGlow8;            // 8-bit glow table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays

  uint16_t usNumberOf16BPPObjects;
  uint16_t usNumberOfObjects;  // Total number of objects
  uint8_t ubBitDepth;          // BPP

  // Reserved for added room and 32-byte boundaries
  BYTE bReserved[1];
};

#endif
