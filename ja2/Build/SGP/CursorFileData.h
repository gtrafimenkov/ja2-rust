#ifndef __CURSOR_FILE_DATA
#define __CURSOR_FILE_DATA

#include "SGP/Video.h"

struct CursorFileData {
  CHAR8 ubFilename[48];
  BOOLEAN fLoaded;
  UINT32 uiIndex;
  UINT8 ubFlags;
  UINT8 ubNumberOfFrames;
  HVOBJECT hVObject;
};

#endif
