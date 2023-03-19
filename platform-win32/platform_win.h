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

#define FILE_ACCESS_READ 0x01
#define FILE_ACCESS_WRITE 0x02

#define FILE_CREATE_NEW 0x0010         // create new file. fail if exists
#define FILE_CREATE_ALWAYS 0x0020      // create new file. overwrite existing
#define FILE_OPEN_EXISTING 0x0040      // open a file. fail if doesn't exist
#define FILE_OPEN_ALWAYS 0x0080        // open a file, create if doesn't exist
#define FILE_TRUNCATE_EXISTING 0x0100  // open a file, truncate to size 0. fail if no exist

#define FILE_SEEK_FROM_START 0x01
#define FILE_SEEK_FROM_END 0x02
#define FILE_SEEK_FROM_CURRENT 0x04

extern HWFILE FileMan_Open(const char *strFilename, UINT32 uiOptions, BOOLEAN fDeleteOnClose);
extern HWFILE FileMan_OpenForReading(const char *path);
extern void FileMan_Close(HWFILE);
extern BOOLEAN FileMan_Read(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead);
extern UINT32 FileMan_GetSize(HWFILE);

#endif
