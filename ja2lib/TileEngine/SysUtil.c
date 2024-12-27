#include "TileEngine/SysUtil.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"

uint32_t guiBOTTOMPANEL = 0;
uint32_t guiRIGHTPANEL = 0;
uint32_t guiRENDERBUFFER = 0;
uint32_t guiSAVEBUFFER = 0;
uint32_t guiEXTRABUFFER = 0;

BOOLEAN gfExtraBuffer = FALSE;

BOOLEAN InitializeSystemVideoObjects() { return (TRUE); }

BOOLEAN InitializeGameVideoObjects() {
  VSURFACE_DESC vs_desc;
  uint16_t usWidth;
  uint16_t usHeight;
  uint8_t ubBitDepth;

  // Create render buffer
  GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);
  vs_desc.fCreateFlags = VSURFACE_CREATE_DEFAULT | VSURFACE_SYSTEM_MEM_USAGE;
  vs_desc.usWidth = usWidth;
  vs_desc.usHeight = usHeight;
  vs_desc.ubBitDepth = ubBitDepth;

  CHECKF(AddVideoSurface(&vs_desc, &guiSAVEBUFFER));

  CHECKF(AddVideoSurface(&vs_desc, &guiEXTRABUFFER));
  gfExtraBuffer = TRUE;

  guiRENDERBUFFER = FRAME_BUFFER;

  return (TRUE);
}
