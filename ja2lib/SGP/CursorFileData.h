#ifndef __CURSOR_FILE_DATA
#define __CURSOR_FILE_DATA

#include "SGP/Types.h"

struct CursorFileData {
  CHAR8 ubFilename[48];
  BOOLEAN fLoaded;
  uint32_t uiIndex;
  UINT8 ubFlags;
  UINT8 ubNumberOfFrames;
  struct VObject* hVObject;
};

#endif
