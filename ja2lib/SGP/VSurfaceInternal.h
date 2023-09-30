#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  UINT16 usHeight;
  UINT16 usWidth;

  void* _platformData1;       // platform-specific data
  void* _platformData2;       // platform-specific data
  COLORVAL TransparentColor;  // Defaults to 0,0,0
};

#endif
