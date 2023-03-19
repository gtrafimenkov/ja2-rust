// Some platform services.
// This file should be included only to other platform files.

#ifndef __PLATFORM_WIN_H
#define __PLATFORM_WIN_H

#include <windows.h>

#include "SGP/Types.h"

// This is a handle to an open file.
// The file can be a regular file on a disk, or a file from an slf archive.
typedef UINT32 HWFILE;

//	Pass in the Fileman file handle of an OPEN file and it will return..
//		if its a Real File, the return will be the handle of the REAL file
//		if its a LIBRARY file, the return will be the handle of the LIBRARY
HANDLE GetRealFileHandleFromFileManFileHandle(HWFILE hFile);

struct PlatformInitParams {
  HINSTANCE hInstance;
  UINT16 usCommandShow;
  void *WindowProc;
  UINT16 iconID;
};

extern HWND ghWindow;

#define FILE_SEEK_FROM_START 0x01
#define FILE_SEEK_FROM_END 0x02
#define FILE_SEEK_FROM_CURRENT 0x04

#endif
