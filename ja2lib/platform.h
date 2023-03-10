// OS-independent interface to the platform layer.

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "Rect.h"
#include "SGP/Types.h"
#include "rust_platform.h"

/////////////////////////////////////////////////////////////////////////////////
// Files
/////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#define FS_SEPARATOR '\\'
#else
#define FS_SEPARATOR '/'
#endif

// Handler to underlying OS file handle.
typedef void *SYS_FILE_HANDLE;

#define FILE_SEEK_FROM_START 0x01
#define FILE_SEEK_FROM_END 0x02
#define FILE_SEEK_FROM_CURRENT 0x04

uint32_t Plat_GetFileSize(SYS_FILE_HANDLE handle);
BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, uint32_t bytesToRead,
                      uint32_t *readBytes);
BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle);
void Plat_CloseFile(SYS_FILE_HANDLE handle);

// Change file pointer.
// In case of an error returns 0xFFFFFFFF
uint32_t Plat_SetFilePointer(SYS_FILE_HANDLE handle, int32_t distance, int seekType);

// Gets the amount of free space on the hard drive that the main executeablt is runnning from
uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

struct GetFile {
  int32_t iFindHandle;
  char zFileName[260];  // changed from uint16_t, Alex Meduna, Mar-20'98
  uint32_t uiFileSize;
  uint32_t uiFileAttribs;
};

BOOLEAN Plat_GetFileFirst(char *pSpec, struct GetFile *pGFStruct);
BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct);
void Plat_GetFileClose(struct GetFile *pGFStruct);
BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs);

struct FileDialogList {
  struct GetFile FileInfo;
  struct FileDialogList *pNext;
  struct FileDialogList *pPrev;
};

/////////////////////////////////////////////////////////////////////////////////
// Timers
/////////////////////////////////////////////////////////////////////////////////

extern uint32_t Plat_GetTickCount();

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

extern void Plat_OnSGPExit();

/////////////////////////////////////////////////////////////////////////////////
// Mouse
/////////////////////////////////////////////////////////////////////////////////

void Plat_ClipCursor(const struct Rect *rect);

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#endif
