// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

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
