// Some platform services.
// This file should be included only to other platform files.

#ifndef __PLATFORM_WIN_H
#define __PLATFORM_WIN_H

#include <windows.h>

#include "SGP/Types.h"

//	Pass in the Fileman file handle of an OPEN file and it will return..
//		if its a Real File, the return will be the handle of the REAL file
//		if its a LIBRARY file, the return will be the handle of the LIBRARY
HANDLE GetRealFileHandleFromFileManFileHandle(HWFILE hFile);

struct PlatformInitParams {
  HINSTANCE hInstance;
  uint16_t usCommandShow;
  void *WindowProc;
  uint16_t iconID;
};

extern HWND ghWindow;

#endif
