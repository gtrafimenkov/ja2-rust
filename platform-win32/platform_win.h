// Some platform services.
// This file should be included only to other platform files.

#ifndef __PLATFORM_WIN_H
#define __PLATFORM_WIN_H

#include <windows.h>

#include "SGP/Types.h"

struct PlatformInitParams {
  HINSTANCE hInstance;
  UINT16 usCommandShow;
  void *WindowProc;
  UINT16 iconID;
};

extern HWND ghWindow;

#endif
