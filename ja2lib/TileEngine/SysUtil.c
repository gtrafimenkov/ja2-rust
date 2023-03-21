#include "TileEngine/SysUtil.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"

UINT32 guiBOTTOMPANEL = 0;
UINT32 guiRIGHTPANEL = 0;
UINT32 guiSAVEBUFFER = 0;
UINT32 guiEXTRABUFFER = 0;

BOOLEAN InitializeSystemVideoObjects() { return (TRUE); }

BOOLEAN InitializeGameVideoObjects() {
  VSURFACE_DESC vs_desc;
  UINT16 usWidth;
  UINT16 usHeight;
  UINT8 ubBitDepth;

  // Create render buffer
  GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);
  vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT;
  vs_desc.usWidth = usWidth;
  vs_desc.usHeight = usHeight;
  vs_desc.ubBitDepth = ubBitDepth;

  if (!(AddVideoSurface(&vs_desc, &guiSAVEBUFFER))) {
    return FALSE;
  }

  if (!(AddVideoSurface(&vs_desc, &guiEXTRABUFFER))) {
    return FALSE;
  }

  return (TRUE);
}
