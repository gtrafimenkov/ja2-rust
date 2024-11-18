#ifndef __CURSOR_FILE_DATA
#define __CURSOR_FILE_DATA

#include "SGP/Types.h"

struct CursorFileData {
  char ubFilename[48];
  BOOLEAN fLoaded;
  uint8_t ubFlags;
  uint8_t ubNumberOfFrames;
  struct VObject* hVObject;
};

#endif
