#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  uint16_t usHeight;
  uint16_t usWidth;

  void* _platformData1;  // platform-specific data
  void* _platformData2;  // platform-specific data
};

#endif
