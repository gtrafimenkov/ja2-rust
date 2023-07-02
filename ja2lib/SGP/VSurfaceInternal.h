#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  uint16_t usHeight;   // Height of Video Surface
  uint16_t usWidth;    // Width of Video Surface
  uint8_t ubBitDepth;  // can only be 8 or 16
  // when VSurface is created from an image, it will have the bit depth of this image
  // when an empty VSurface is created, it will have bit depth of 16

  // A void pointer, but for this implementation, is really a
  // lpDirectDrawSurface;
  void* pSurfaceData;

  // Direct Draw One Interface
  void* pSurfaceData1;

  // A void pointer, but for this implementation a DDPalette
  void* pPalette;

  uint32_t fFlags;            // Used to describe memory usage, etc
  uint16_t* p16BPPPalette;    // A 16BPP palette used for 8->16 blits
  COLORVAL TransparentColor;  // Defaults to 0,0,0
  void* pClipper;             // A void pointer encapsolated as a clipper Surface
};

#endif
