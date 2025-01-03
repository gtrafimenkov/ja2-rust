// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

// Implementation of the platform layer file operations on Windows.

#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "SGP/Container.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"
#include "platform.h"
#include "platform_win.h"
#include "rust_debug.h"

// Gets the free hard drive space from the drive letter passed in.  It has to be the root dir.  (
// eg. c:\ )
static uint32_t GetFreeSpaceOnHardDrive(char *pzDriveLetter);

uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() {
  struct Str512 zExecDir;
  STRING512 zDrive;
  STRING512 zDir;
  STRING512 zFileName;
  STRING512 zExt;

  uint32_t uiFreeSpace = 0;

  if (!Plat_GetExecutableDirectory(&zExecDir)) {
    return 0;
  }

  // get the drive letter from the exec dir
  _splitpath(zExecDir.buf, zDrive, zDir, zFileName, zExt);

  sprintf(zDrive, "%s\\", zDrive);

  uiFreeSpace = GetFreeSpaceOnHardDrive(zDrive);

  return (uiFreeSpace);
}

static uint32_t GetFreeSpaceOnHardDrive(char *pzDriveLetter) {
  DWORD uiBytesFree = 0;
  DWORD uiSectorsPerCluster = 0;
  DWORD uiBytesPerSector = 0;
  DWORD uiNumberOfFreeClusters = 0;
  DWORD uiTotalNumberOfClusters = 0;

  if (!GetDiskFreeSpace(pzDriveLetter, &uiSectorsPerCluster, &uiBytesPerSector,
                        &uiNumberOfFreeClusters, &uiTotalNumberOfClusters)) {
    uint32_t uiLastError = GetLastError();
    char zString[1024];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

    return (TRUE);
  }

  uiBytesFree = uiBytesPerSector * uiNumberOfFreeClusters * uiSectorsPerCluster;

  return (uiBytesFree);
}

// GetFile file attributes
#define FILE_IS_READONLY 1
#define FILE_IS_DIRECTORY 2
#define FILE_IS_HIDDEN 4
#define FILE_IS_NORMAL 8
#define FILE_IS_ARCHIVE 16
#define FILE_IS_SYSTEM 32
#define FILE_IS_TEMPORARY 64
#define FILE_IS_COMPRESSED 128
#define FILE_IS_OFFLINE 256

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_READONLY;
}

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_SYSTEM;
}

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_HIDDEN;
}

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_DIRECTORY;
}

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_OFFLINE;
}

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) {
  return gfs->uiFileAttribs & FILE_IS_TEMPORARY;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define FILENAME_LENGTH 600

//**************************************************************************
//
//				Variables
//
//**************************************************************************

WIN32_FIND_DATA Win32FindInfo[20];
BOOLEAN fFindInfoInUse[20] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                              FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
HANDLE hFindInfoHandle[20] = {
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

static void W32toSGPFileFind(struct GetFile *pGFStruct, WIN32_FIND_DATA *pW32Struct);

#ifdef JA2TESTVERSION
extern uint32_t uiTotalFileReadTime;
extern uint32_t uiTotalFileReadCalls;
#include "Utils/TimerControl.h"
#endif

BOOLEAN Plat_GetFileFirst(char *pSpec, struct GetFile *pGFStruct) {
  int32_t x, iWhich = 0;
  BOOLEAN fFound;

  if (!(pSpec != NULL)) {
    return FALSE;
  }
  if (!(pGFStruct != NULL)) {
    return FALSE;
  }

  fFound = FALSE;
  for (x = 0; x < 20 && !fFound; x++) {
    if (!fFindInfoInUse[x]) {
      iWhich = x;
      fFound = TRUE;
    }
  }

  if (!fFound) return (FALSE);

  pGFStruct->iFindHandle = iWhich;

  hFindInfoHandle[iWhich] = FindFirstFile(pSpec, &Win32FindInfo[iWhich]);

  if (hFindInfoHandle[iWhich] == INVALID_HANDLE_VALUE) return (FALSE);
  fFindInfoInUse[iWhich] = TRUE;

  W32toSGPFileFind(pGFStruct, &Win32FindInfo[iWhich]);

  return (TRUE);
}

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) {
  if (!(pGFStruct != NULL)) {
    return FALSE;
  }

  if (FindNextFile(hFindInfoHandle[pGFStruct->iFindHandle],
                   &Win32FindInfo[pGFStruct->iFindHandle])) {
    W32toSGPFileFind(pGFStruct, &Win32FindInfo[pGFStruct->iFindHandle]);
    return (TRUE);
  }
  return (FALSE);
}

void Plat_GetFileClose(struct GetFile *pGFStruct) {
  if (pGFStruct == NULL) return;

  FindClose(hFindInfoHandle[pGFStruct->iFindHandle]);
  hFindInfoHandle[pGFStruct->iFindHandle] = INVALID_HANDLE_VALUE;
  fFindInfoInUse[pGFStruct->iFindHandle] = FALSE;

  return;
}

void W32toSGPFileFind(struct GetFile *pGFStruct, WIN32_FIND_DATA *pW32Struct) {
  uint32_t uiAttribMask;

  // Copy the filename
  strcpy(pGFStruct->zFileName, pW32Struct->cFileName);

  // Get file size
  if (pW32Struct->nFileSizeHigh != 0)
    pGFStruct->uiFileSize = 0xffffffff;
  else
    pGFStruct->uiFileSize = pW32Struct->nFileSizeLow;

  // Copy the file attributes
  pGFStruct->uiFileAttribs = 0;

  for (uiAttribMask = 0x80000000; uiAttribMask > 0; uiAttribMask >>= 1) {
    switch (pW32Struct->dwFileAttributes & uiAttribMask) {
      case FILE_ATTRIBUTE_ARCHIVE:
        pGFStruct->uiFileAttribs |= FILE_IS_ARCHIVE;
        break;

      case FILE_ATTRIBUTE_DIRECTORY:
        pGFStruct->uiFileAttribs |= FILE_IS_DIRECTORY;
        break;

      case FILE_ATTRIBUTE_HIDDEN:
        pGFStruct->uiFileAttribs |= FILE_IS_HIDDEN;
        break;

      case FILE_ATTRIBUTE_NORMAL:
        pGFStruct->uiFileAttribs |= FILE_IS_NORMAL;
        break;

      case FILE_ATTRIBUTE_READONLY:
        pGFStruct->uiFileAttribs |= FILE_IS_READONLY;
        break;

      case FILE_ATTRIBUTE_SYSTEM:
        pGFStruct->uiFileAttribs |= FILE_IS_SYSTEM;
        break;

      case FILE_ATTRIBUTE_TEMPORARY:
        pGFStruct->uiFileAttribs |= FILE_IS_TEMPORARY;
        break;

      case FILE_ATTRIBUTE_COMPRESSED:
        pGFStruct->uiFileAttribs |= FILE_IS_COMPRESSED;
        break;

      case FILE_ATTRIBUTE_OFFLINE:
        pGFStruct->uiFileAttribs |= FILE_IS_OFFLINE;
        break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
