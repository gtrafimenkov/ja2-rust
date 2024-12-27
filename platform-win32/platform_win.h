// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

// Some platform services.
// This file should be included only to other platform files.

#ifndef __PLATFORM_WIN_H
#define __PLATFORM_WIN_H

#include <windows.h>

#include "SGP/Types.h"

struct PlatformInitParams {
  HINSTANCE hInstance;
  uint16_t usCommandShow;
  void *WindowProc;
  uint16_t iconID;
};

extern HWND ghWindow;

#endif
