#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  uint16_t usHeight;  // Height of Video Surface
  uint16_t usWidth;   // Width of Video Surface

  // A void pointer, but for this implementation, is really a lpDirectDrawSurface
  void* pSurfaceData;

  // Direct Draw One Interface
  void* pSurfaceData1;

  COLORVAL TransparentColor;  // Defaults to 0,0,0
};

#endif
