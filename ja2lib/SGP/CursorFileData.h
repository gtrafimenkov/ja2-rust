#ifndef __CURSOR_FILE_DATA
#define __CURSOR_FILE_DATA

#include "SGP/Types.h"

struct CursorFileData {
  CHAR8 ubFilename[48];
  BOOLEAN fLoaded;
  uint32_t uiIndex;
  uint8_t ubFlags;
  uint8_t ubNumberOfFrames;
  struct VObject* hVObject;
};

#endif
