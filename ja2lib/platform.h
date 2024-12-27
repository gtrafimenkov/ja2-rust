// OS-independent interface to the platform layer.

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "Rect.h"
#include "SGP/Types.h"

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

u32 Plat_GetFileSize(SYS_FILE_HANDLE handle);
BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, u32 bytesToRead, u32 *readBytes);
BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle);
void Plat_CloseFile(SYS_FILE_HANDLE handle);

// Given a path, fill outputBuf with the file name.
void Plat_FileBaseName(const char *path, char *outputBuf, u32 bufSize);

// Change file pointer.
// In case of an error returns 0xFFFFFFFF
u32 Plat_SetFilePointer(SYS_FILE_HANDLE handle, i32 distance, int seekType);

BOOLEAN Plat_GetExecutableDirectory(char *buf, u16 bufSize);

// Gets the amount of free space on the hard drive that the main executeablt is runnning from
uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

BOOLEAN Plat_GetCurrentDirectory(STRING512 pcDirectory);
BOOLEAN Plat_SetCurrentDirectory(const char *pcDirectory);

BOOLEAN Plat_DirectoryExists(const char *pcDirectory);
BOOLEAN Plat_CreateDirectory(const char *pcDirectory);

BOOLEAN Plat_RemoveDirectory(const char *pcDirectory, BOOLEAN fRecursive);
BOOLEAN Plat_EraseDirectory(const char *pcDirectory);

BOOLEAN Plat_ClearFileAttributes(STR filename);

// Check if file (or directory) exists.
BOOLEAN Plat_FileEntityExists(const char *path);

BOOLEAN Plat_DeleteFile(const char *filename);

struct GetFile {
  INT32 iFindHandle;
  CHAR8 zFileName[260];  // changed from UINT16, Alex Meduna, Mar-20'98
  uint32_t uiFileSize;
  uint32_t uiFileAttribs;
};

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct);
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

extern u32 Plat_GetTickCount();

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
