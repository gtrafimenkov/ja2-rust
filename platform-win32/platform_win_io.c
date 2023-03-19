// Implementation of the platform layer file operations on Windows.

#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "SGP/Container.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/LibraryDataBase.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "StrUtils.h"
#include "platform.h"
#include "rust_debug.h"

u32 Plat_GetFileSize(SYS_FILE_HANDLE handle) { return GetFileSize(handle, NULL); }
BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, u32 bytesToRead, u32 *readBytes) {
  return ReadFile(handle, buffer, bytesToRead, (LPDWORD)readBytes, NULL);
}
void Plat_CloseFile(SYS_FILE_HANDLE handle) { CloseHandle(handle); }
BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle) {
  *handle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_FLAG_RANDOM_ACCESS, NULL);
  return *handle != INVALID_HANDLE_VALUE;
}

// Change file pointer.
// In case of an error returns 0xFFFFFFFF
u32 Plat_SetFilePointer(SYS_FILE_HANDLE handle, i32 distance, int seekType) {
  DWORD moveMethod;
  switch (seekType) {
    case FILE_SEEK_FROM_START:
      moveMethod = FILE_BEGIN;
      break;
    case FILE_SEEK_FROM_END:
      moveMethod = FILE_END;
      break;
    case FILE_SEEK_FROM_CURRENT:
      moveMethod = FILE_CURRENT;
      break;
    default:
      return 0xFFFFFFFF;
  }
  return SetFilePointer(handle, distance, NULL, moveMethod);
}

// Gets the free hard drive space from the drive letter passed in.  It has to be the root dir.  (
// eg. c:\ )
static UINT32 GetFreeSpaceOnHardDrive(STR pzDriveLetter);

UINT32 Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() {
  struct Str512 zExecDir;
  STRING512 zDrive;
  STRING512 zDir;
  STRING512 zFileName;
  STRING512 zExt;

  UINT32 uiFreeSpace = 0;

  if (!Plat_GetExecutableDirectory(&zExecDir)) {
    return 0;
  }

  // get the drive letter from the exec dir
  _splitpath(zExecDir.buf, zDrive, zDir, zFileName, zExt);

  sprintf(zDrive, "%s\\", zDrive);

  uiFreeSpace = GetFreeSpaceOnHardDrive(zDrive);

  return (uiFreeSpace);
}

static UINT32 GetFreeSpaceOnHardDrive(STR pzDriveLetter) {
  DWORD uiBytesFree = 0;
  DWORD uiSectorsPerCluster = 0;
  DWORD uiBytesPerSector = 0;
  DWORD uiNumberOfFreeClusters = 0;
  DWORD uiTotalNumberOfClusters = 0;

  if (!GetDiskFreeSpace(pzDriveLetter, &uiSectorsPerCluster, &uiBytesPerSector,
                        &uiNumberOfFreeClusters, &uiTotalNumberOfClusters)) {
    UINT32 uiLastError = GetLastError();
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

#define CHECKF(exp) \
  if (!(exp)) {     \
    return (FALSE); \
  }
#define CHECKV(exp) \
  if (!(exp)) {     \
    return;         \
  }
#define CHECKN(exp) \
  if (!(exp)) {     \
    return (NULL);  \
  }
#define CHECKBI(exp) \
  if (!(exp)) {      \
    return (-1);     \
  }

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

//**************************************************************************
//
// FileMan_Open
//
//		Opens a file.
//
// Parameter List :
//
//		STR	   -> filename
//		UIN32		-> access - read or write, or both
//		BOOLEAN	-> delete on close
//
// Return Value :
//
//		HWFILE	-> handle of opened file
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

static BOOLEAN FileMan_ExistsNoDB(STR strFilename) {
  BOOLEAN fExists = FALSE;
  FILE *file;

  // open up the file to see if it exists on the disk
  file = fopen(strFilename, "r");
  if (file) {
    fExists = TRUE;
    fclose(file);
  }

  return (fExists);
}

HWFILE FileMan_Open(const char *strFilename, UINT32 uiOptions, BOOLEAN fDeleteOnClose) {
  HWFILE hFile;
  HANDLE hRealFile;
  DWORD dwAccess;
  DWORD dwFlagsAndAttributes;
  BOOLEAN fExists;
  DWORD dwCreationFlags;
  HWFILE hLibFile;

  hFile = 0;
  dwCreationFlags = 0;

  // check if the file exists - note that we use the function FileMan_ExistsNoDB
  // because it doesn't check the databases, and we don't want to do that here
  fExists = FileMan_ExistsNoDB(strFilename);

  dwAccess = 0;
  if (uiOptions & FILE_ACCESS_READ) dwAccess |= GENERIC_READ;
  if (uiOptions & FILE_ACCESS_WRITE) dwAccess |= GENERIC_WRITE;

  dwFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS;
  if (fDeleteOnClose) dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;

  // if the file is on the disk
  if (fExists) {
    hRealFile = CreateFile(strFilename, dwAccess, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                           dwFlagsAndAttributes, NULL);

    if (hRealFile == INVALID_HANDLE_VALUE) {
      return (0);
    }

    // create a file handle for the 'real file'
    hFile = CreateRealFileHandle(hRealFile);
  }
  // if the file did not exist, try to open it from the database
  else if (gFileDataBase.fInitialized) {
    // if the file is to be opened for writing, return an error cause you cant write a file that is
    // in the database library
    if (fDeleteOnClose) {
      return (0);
    }

    // if the file doesnt exist on the harddrive, but it is to be created, dont try to load it from
    // the file database
    if (uiOptions & FILE_ACCESS_WRITE) {
      // if the files is to be written to
      if ((uiOptions & FILE_CREATE_NEW) || (uiOptions & FILE_OPEN_ALWAYS) ||
          (uiOptions & FILE_CREATE_ALWAYS) || (uiOptions & FILE_TRUNCATE_EXISTING)) {
        hFile = 0;
      }
    } else {
      // If the file is in the library, get a handle to it.
      hLibFile = OpenFileFromLibrary(strFilename);

      // tried to open a file that wasnt in the database
      if (!hLibFile)
        return (0);
      else
        return (hLibFile);  // return the file handle
    }
  }

  if (!hFile) {
    if (uiOptions & FILE_CREATE_NEW) {
      dwCreationFlags = CREATE_NEW;
    } else if (uiOptions & FILE_CREATE_ALWAYS) {
      dwCreationFlags = CREATE_ALWAYS;
    } else if (uiOptions & FILE_OPEN_EXISTING || uiOptions & FILE_ACCESS_READ) {
      dwCreationFlags = OPEN_EXISTING;
    } else if (uiOptions & FILE_OPEN_ALWAYS) {
      dwCreationFlags = OPEN_ALWAYS;
    } else if (uiOptions & FILE_TRUNCATE_EXISTING) {
      dwCreationFlags = TRUNCATE_EXISTING;
    } else {
      dwCreationFlags = OPEN_ALWAYS;
    }

    hRealFile = CreateFile(strFilename, dwAccess, FILE_SHARE_READ, NULL, dwCreationFlags,
                           dwFlagsAndAttributes, NULL);
    if (hRealFile == INVALID_HANDLE_VALUE) {
      UINT32 uiLastError = GetLastError();
      char zString[1024];
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

      return (0);
    }

    hFile = CreateRealFileHandle(hRealFile);
  }

  if (!hFile) return (0);

  return (hFile);
}

//**************************************************************************
//
// FileMan_Close
//
//
// Parameter List :
//
//		HWFILE hFile	-> handle to file to close
//
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

void FileMan_Close(HWFILE hFile) {
  INT16 sLibraryID;
  UINT32 uiFileNum;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  // if its the 'real file' library
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    // if its not already closed
    if (gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].uiFileID != 0) {
      Plat_CloseFile(gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle);
      gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].uiFileID = 0;
      gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle = 0;
      gFileDataBase.RealFiles.iNumFilesOpen--;
      if (gFileDataBase.RealFiles.iNumFilesOpen < 0) {
        // if for some reason we are below 0, report an error ( should never be )
        Assert(0);
      }
    }
  } else {
    // if the database is initialized
    if (gFileDataBase.fInitialized) CloseLibraryFile(sLibraryID, uiFileNum);
  }
}

//**************************************************************************
//
// FileMan_Read
//
//		To read a file.
//
// Parameter List :
//
//		HWFILE		-> handle to file to read from
//		void	*	-> source buffer
//		UINT32	-> num bytes to read
//		UINT32	-> num bytes read
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//		08Dec97:ARM		-> return FALSE if bytes to read != bytes read
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

#ifdef JA2TESTVERSION
extern UINT32 uiTotalFileReadTime;
extern UINT32 uiTotalFileReadCalls;
#include "Utils/TimerControl.h"
#endif

BOOLEAN FileMan_Read(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead) {
  HANDLE hRealFile;
  DWORD dwNumBytesToRead;
  u32 dwNumBytesRead;
  BOOLEAN fRet = FALSE;
  INT16 sLibraryID;
  UINT32 uiFileNum;

#ifdef JA2TESTVERSION
  UINT32 uiStartTime = GetJA2Clock();
#endif

  // init the variables
  dwNumBytesToRead = dwNumBytesRead = 0;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  dwNumBytesToRead = (DWORD)uiBytesToRead;

  // if its a real file, read the data from the file
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    // if the file is opened
    if (uiFileNum != 0) {
      hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle;

      fRet = Plat_ReadFile(hRealFile, pDest, dwNumBytesToRead, &dwNumBytesRead);
      if (dwNumBytesToRead != dwNumBytesRead) {
        UINT32 uiLastError = GetLastError();
        char zString[1024];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

        fRet = FALSE;
      }

      if (puiBytesRead) *puiBytesRead = (UINT32)dwNumBytesRead;
    }
  } else {
    // if the database is initialized
    if (gFileDataBase.fInitialized) {
      // if the library is open
      if (IsLibraryOpened(sLibraryID)) {
        // if the file is opened
        if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFileID != 0) {
          // read the data from the library
          fRet = LoadDataFromLibrary(sLibraryID, uiFileNum, pDest, dwNumBytesToRead,
                                     (UINT32 *)&dwNumBytesRead);
          if (puiBytesRead) {
            *puiBytesRead = (UINT32)dwNumBytesRead;
          }
        }
      }
    }
  }
#ifdef JA2TESTVERSION
  // Add the time that we spent in this function to the total.
  uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
  uiTotalFileReadCalls++;
#endif

  return (fRet);
}

//**************************************************************************
//
// FileMan_Write
//
//		To write a file.
//
// Parameter List :
//
//		HWFILE		-> handle to file to write to
//		void	*	-> destination buffer
//		UINT32	-> num bytes to write
//		UINT32	-> num bytes written
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//		08Dec97:ARM		-> return FALSE if dwNumBytesToWrite != dwNumBytesWritten
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

BOOLEAN FileMan_Write(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite, UINT32 *puiBytesWritten) {
  HANDLE hRealFile;
  DWORD dwNumBytesToWrite, dwNumBytesWritten;
  BOOLEAN fRet;
  INT16 sLibraryID;
  UINT32 uiFileNum;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  // if its a real file, read the data from the file
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    dwNumBytesToWrite = (DWORD)uiBytesToWrite;

    // get the real file handle to the file
    hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle;

    fRet = WriteFile(hRealFile, pDest, dwNumBytesToWrite, &dwNumBytesWritten, NULL);

    if (dwNumBytesToWrite != dwNumBytesWritten) fRet = FALSE;

    if (puiBytesWritten) *puiBytesWritten = (UINT32)dwNumBytesWritten;
  } else {
    // we cannot write to a library file
    if (puiBytesWritten) *puiBytesWritten = 0;
    return (FALSE);
  }

  return (fRet);
}

//**************************************************************************
//
// FileMan_Seek
//
//		To seek to a position in a file.
//
// Parameter List :
//
//		HWFILE	-> handle to file to seek in
//		UINT32	-> distance to seek
//		UINT8		-> how to seek
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

BOOLEAN FileMan_Seek(HWFILE hFile, UINT32 uiDistance, UINT8 uiHow) {
  HANDLE hRealFile;
  LONG lDistanceToMove;
  INT32 iDistance = 0;

  INT16 sLibraryID;
  UINT32 uiFileNum;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  // if its a real file, read the data from the file
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    // Get the handle to the real file
    hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle;

    iDistance = (INT32)uiDistance;

    if (uiHow == FILE_SEEK_FROM_END) {
      if (iDistance > 0) iDistance = -(iDistance);
    }

    lDistanceToMove = (LONG)uiDistance;

    if (Plat_SetFilePointer(hRealFile, iDistance, uiHow) == 0xFFFFFFFF) return (FALSE);
  } else {
    // if the database is initialized
    if (gFileDataBase.fInitialized) LibraryFileSeek(sLibraryID, uiFileNum, uiDistance, uiHow);
  }

  return (TRUE);
}

//**************************************************************************
//
// FileMan_GetSize
//
//		To get the current file size.
//
// Parameter List :
//
//		HWFILE	-> handle to file
//
// Return Value :
//
//		INT32		-> file size in file if successful
//					-> 0 if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

UINT32 FileMan_GetSize(HWFILE hFile) {
  HANDLE hRealHandle;
  UINT32 uiFileSize = 0xFFFFFFFF;

  INT16 sLibraryID;
  UINT32 uiFileNum;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  // if its a real file, read the data from the file
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    // Get the handle to a real file
    hRealHandle = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle;

    uiFileSize = Plat_GetFileSize(hRealHandle);
  } else {
    // if the library is open
    if (IsLibraryOpened(sLibraryID))
      uiFileSize =
          gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileLength;
  }

  if (uiFileSize == 0xFFFFFFFF)
    return (0);
  else
    return (uiFileSize);
}

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct) {
  INT32 x, iWhich = 0;
  BOOLEAN fFound;

  CHECKF(pSpec != NULL);
  CHECKF(pGFStruct != NULL);

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
  CHECKF(pGFStruct != NULL);

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
  UINT32 uiAttribMask;

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

HANDLE GetRealFileHandleFromFileManFileHandle(HWFILE hFile) {
  INT16 sLibraryID;
  UINT32 uiFileNum;

  GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

  // if its the 'real file' library
  if (sLibraryID == REAL_FILE_LIBRARY_ID) {
    // if its not already closed
    if (gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].uiFileID != 0) {
      return (gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle);
    }
  } else {
    // if the file is not opened, dont close it
    if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFileID != 0) {
      return (gFileDataBase.pLibraries[sLibraryID].hLibraryHandle);
    }
  }
  return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
