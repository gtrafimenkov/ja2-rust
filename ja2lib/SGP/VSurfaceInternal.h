#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  UINT16 usHeight;  // Height of Video Surface
  UINT16 usWidth;   // Width of Video Surface

  // A void pointer, but for this implementation, is really a lpDirectDrawSurface
  PTR pSurfaceData;

  // Direct Draw One Interface
  PTR pSurfaceData1;
};

#endif
