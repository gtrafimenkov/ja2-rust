#include "SGP/LibraryDataBase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/MemMan.h"
#include "SGP/WCheck.h"
#include "platform.h"
#include "platform_strings.h"

// The FileDatabaseHeader
DatabaseManagerHeaderStruct gFileDataBase;

struct _FILETIME {
  uint32_t Lo;
  uint32_t Hi;
};

typedef struct {
  char sFileName[FILENAME_SIZE];
  uint32_t uiOffset;
  uint32_t uiLength;
  uint8_t ubState;
  uint8_t ubReserved;
  struct _FILETIME sFileTime;
  uint16_t usReserved2;
} DIRENTRY;

BOOLEAN IsLibraryRealFile(HWFILE hFile) {
  return DB_EXTRACT_LIBRARY(hFile) == REAL_FILE_LIBRARY_ID;
}

// used when doing the binary search of the libraries
int16_t gsCurrentLibrary = -1;

static int CompareFileNames(char *arg1, FileHeaderStruct *arg2);
BOOLEAN GetFileHeaderFromLibrary(int16_t sLibraryID, char *pstrFileName,
                                 FileHeaderStruct **pFileHeader);
void AddSlashToPath(char *pName);
HWFILE CreateLibraryFileHandle(int16_t sLibraryID, uint32_t uiFileNum);
BOOLEAN CheckIfFileIsAlreadyOpen(char *pFileName, int16_t sLibraryID);

int32_t CompareDirEntryFileNames(char *arg1, DIRENTRY *arg2);

//************************************************************************
//
//	 InitializeFileDatabase():  Call this function to initialize the file
//	database.  It will use the gGameLibaries[] array for the list of libraries
//	and the define NUMBER_OF_LIBRARIES for the number of libraries.  The gGameLibaries
//	array is an array of structure, one of the fields determines if the library
//	will be initialized and game start.
//
//************************************************************************
BOOLEAN InitializeFileDatabase() {
  int16_t i;
  uint32_t uiSize;
  BOOLEAN fLibraryInited = FALSE;

  // if all the libraries exist, set them up
  gFileDataBase.usNumberOfLibraries = NUMBER_OF_LIBRARIES;

  // allocate memory for the each of the library headers
  uiSize = NUMBER_OF_LIBRARIES * sizeof(LibraryHeaderStruct);
  if (uiSize) {
    gFileDataBase.pLibraries = (LibraryHeaderStruct *)MemAlloc(uiSize);
    CHECKF(gFileDataBase.pLibraries);

    // set all the memrory to 0
    memset(gFileDataBase.pLibraries, 0, uiSize);

    // Load up each library
    for (i = 0; i < NUMBER_OF_LIBRARIES; i++) {
      // if you want to init the library at the begining of the game
      if (gGameLibaries[i].fInitOnStart) {
        // if the library exists
        if (OpenLibrary(i)) fLibraryInited = TRUE;

        // else the library doesnt exist
        else {
          FastDebugMsg(
              String("Warning in InitializeFileDatabase( ): Library Id #%d (%s) is to be loaded "
                     "but cannot be found.\n",
                     i, gGameLibaries[i].sLibraryName));
          gFileDataBase.pLibraries[i].fLibraryOpen = FALSE;
        }
      }
    }
    // signify that the database has been initialized ( only if there was a library loaded )
    gFileDataBase.fInitialized = fLibraryInited;
  }

  // allocate memory for the handles of the 'real files' that will be open
  // This is needed because the the code wouldnt be able to tell the difference between a 'real'
  // handle and a made up one
  uiSize = INITIAL_NUM_HANDLES * sizeof(RealFileOpenStruct);
  gFileDataBase.RealFiles.pRealFilesOpen = (RealFileOpenStruct *)MemAlloc(uiSize);
  CHECKF(gFileDataBase.RealFiles.pRealFilesOpen);

  // clear the memory
  memset(gFileDataBase.RealFiles.pRealFilesOpen, 0, uiSize);

  // set the initial number how many files can be opened at the one time
  gFileDataBase.RealFiles.iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;

  return (TRUE);
}

//************************************************************************
//
//	 ShutDownFileDatabase():  Call this function to close down the file
//	database.
//
//************************************************************************

BOOLEAN ShutDownFileDatabase() {
  uint16_t sLoop1;

  // Free up the memory used for each library
  for (sLoop1 = 0; sLoop1 < gFileDataBase.usNumberOfLibraries; sLoop1++) CloseLibrary(sLoop1);

  // Free up the memory used for all the library headers
  if (gFileDataBase.pLibraries) {
    MemFree(gFileDataBase.pLibraries);
    gFileDataBase.pLibraries = NULL;
  }

  // loop through all the 'opened files' ( there should be no files open )
  for (sLoop1 = 0; sLoop1 < gFileDataBase.RealFiles.iNumFilesOpen; sLoop1++) {
    FastDebugMsg(
        String("ShutDownFileDatabase( ):  ERROR:  real file id still exists, wasnt closed"));
    Plat_CloseFile(gFileDataBase.RealFiles.pRealFilesOpen[sLoop1].hRealFileHandle);
  }

  // Free up the memory used for the real files array for the opened files
  if (gFileDataBase.RealFiles.pRealFilesOpen) {
    MemFree(gFileDataBase.RealFiles.pRealFilesOpen);
    gFileDataBase.RealFiles.pRealFilesOpen = NULL;
  }

  return (TRUE);
}

BOOLEAN CheckForLibraryExistence(char *pLibraryName) {
  SYS_FILE_HANDLE handle;
  if (Plat_OpenForReading(pLibraryName, &handle)) {
    Plat_CloseFile(handle);
    return TRUE;
  }
  return FALSE;
}

BOOLEAN InitializeLibrary(char *pLibraryName, LibraryHeaderStruct *pLibHeader,
                          BOOLEAN fCanBeOnCDrom) {
  SYS_FILE_HANDLE hFile;
  uint16_t usNumEntries = 0;
  uint32_t uiNumBytesRead;
  uint32_t uiLoop;
  DIRENTRY DirEntry;
  LIBHEADER LibFileHeader;
  uint32_t uiCount = 0;

  // open the library for reading ( if it exists )
  if (!Plat_OpenForReading(pLibraryName, &hFile)) {
    return FALSE;
  }

  // Read in the library header ( at the begining of the library )
  if (!Plat_ReadFile(hFile, &LibFileHeader, sizeof(LIBHEADER), &uiNumBytesRead)) return (FALSE);

  if (uiNumBytesRead != sizeof(LIBHEADER)) {
    // Error Reading the file database header.
    return (FALSE);
  }

  // place the file pointer at the begining of the file headers ( they are at the end of the file )
  Plat_SetFilePointer(hFile, -(LibFileHeader.iEntries * (int32_t)sizeof(DIRENTRY)),
                      FILE_SEEK_FROM_END);

  // loop through the library and determine the number of files that are FILE_OK
  // ie.  so we dont load the old or deleted files
  usNumEntries = 0;
  for (uiLoop = 0; uiLoop < (uint32_t)LibFileHeader.iEntries; uiLoop++) {
    // read in the file header
    if (!Plat_ReadFile(hFile, &DirEntry, sizeof(DIRENTRY), &uiNumBytesRead)) return (FALSE);

    if (DirEntry.ubState == FILE_OK) usNumEntries++;
  }

  // Allocate enough memory for the library header
  pLibHeader->pFileHeader = (FileHeaderStruct *)MemAlloc(sizeof(FileHeaderStruct) * usNumEntries);

#ifdef JA2TESTVERSION
  pLibHeader->uiTotalMemoryAllocatedForLibrary = sizeof(FileHeaderStruct) * usNumEntries;
#endif

  // place the file pointer at the begining of the file headers ( they are at the end of the file )
  Plat_SetFilePointer(hFile, -(LibFileHeader.iEntries * (int32_t)sizeof(DIRENTRY)),
                      FILE_SEEK_FROM_END);

  // loop through all the entries
  uiCount = 0;
  for (uiLoop = 0; uiLoop < (uint32_t)LibFileHeader.iEntries; uiLoop++) {
    // read in the file header
    if (!Plat_ReadFile(hFile, &DirEntry, sizeof(DIRENTRY), &uiNumBytesRead)) return (FALSE);

    if (DirEntry.ubState == FILE_OK) {
      // Check to see if the file is not longer then it should be
      if ((strlen(DirEntry.sFileName) + 1) >= FILENAME_SIZE)
        FastDebugMsg(
            String("\n*******InitializeLibrary():  Warning!:  '%s' from the library '%s' has name "
                   "whose size (%d) is bigger then it should be (%s)",
                   DirEntry.sFileName, pLibHeader->sLibraryPath, (strlen(DirEntry.sFileName) + 1),
                   FILENAME_SIZE));

      // allocate memory for the files name
      pLibHeader->pFileHeader[uiCount].pFileName = (char *)MemAlloc(strlen(DirEntry.sFileName) + 1);

      // if we couldnt allocate memory
      if (!pLibHeader->pFileHeader[uiCount].pFileName) {
        // report an error
        return (FALSE);
      }

#ifdef JA2TESTVERSION
      pLibHeader->uiTotalMemoryAllocatedForLibrary += strlen(DirEntry.sFileName) + 1;
#endif

      // copy the file name, offset and length into the header
      strcpy(pLibHeader->pFileHeader[uiCount].pFileName, DirEntry.sFileName);
      pLibHeader->pFileHeader[uiCount].uiFileOffset = DirEntry.uiOffset;
      pLibHeader->pFileHeader[uiCount].uiFileLength = DirEntry.uiLength;

      uiCount++;
    }
  }

  pLibHeader->usNumberOfEntries = usNumEntries;

  // allocate memory for the library path
  //	if( strlen( LibFileHeader.sPathToLibrary ) == 0 )
  {
    //		FastDebugMsg( String("The %s library file does not contain a path.  Use 'n' argument
    // to name the library when you create it\n", LibFileHeader.sLibName ) ); 		Assert( 0 );
  }

  // if the library has a path
  if (strlen(LibFileHeader.sPathToLibrary) != 0) {
    pLibHeader->sLibraryPath = (char *)MemAlloc(strlen(LibFileHeader.sPathToLibrary) + 1);
    strcpy(pLibHeader->sLibraryPath, LibFileHeader.sPathToLibrary);
  } else {
    // else the library name does not contain a path ( most likely either an error or it is the
    // default path )
    pLibHeader->sLibraryPath = (char *)MemAlloc(1);
    pLibHeader->sLibraryPath[0] = '\0';
  }

#ifdef JA2TESTVERSION
  pLibHeader->uiTotalMemoryAllocatedForLibrary += strlen(LibFileHeader.sPathToLibrary) + 1;
#endif

  // allocate space for the open files array
  pLibHeader->pOpenFiles = (FileOpenStruct *)MemAlloc(INITIAL_NUM_HANDLES * sizeof(FileOpenStruct));
  if (!pLibHeader->pOpenFiles) {
    // report an error
    return (FALSE);
  }

  memset(pLibHeader->pOpenFiles, 0, INITIAL_NUM_HANDLES * sizeof(FileOpenStruct));

#ifdef JA2TESTVERSION
  pLibHeader->uiTotalMemoryAllocatedForLibrary += INITIAL_NUM_HANDLES * sizeof(FileOpenStruct);
#endif

  pLibHeader->hLibraryHandle = hFile;
  pLibHeader->usNumberOfEntries = usNumEntries;
  pLibHeader->fLibraryOpen = TRUE;
  pLibHeader->iNumFilesOpen = 0;
  pLibHeader->iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;

  return (TRUE);
}

BOOLEAN LoadDataFromLibrary(int16_t sLibraryID, uint32_t uiFileNum, void *pData,
                            uint32_t uiBytesToRead, uint32_t *pBytesRead) {
  uint32_t uiOffsetInLibrary, uiLength;
  SYS_FILE_HANDLE hLibraryFile;
  uint32_t uiNumBytesRead;
  uint32_t uiCurPos;

  // get the offset into the library, the length and current position of the file pointer.
  uiOffsetInLibrary =
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileOffset;
  uiLength = gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileLength;
  hLibraryFile = gFileDataBase.pLibraries[sLibraryID].hLibraryHandle;
  uiCurPos = gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFilePosInFile;

  // set the file pointer to the right location
  Plat_SetFilePointer(hLibraryFile, (uiOffsetInLibrary + uiCurPos), FILE_SEEK_FROM_START);

  // if we are trying to read more data then the size of the file, return an error
  if (uiBytesToRead + uiCurPos > uiLength) {
    *pBytesRead = 0;
    return (FALSE);
  }

  // get the data
  if (!Plat_ReadFile(hLibraryFile, pData, uiBytesToRead, &uiNumBytesRead)) return (FALSE);

  if (uiBytesToRead != uiNumBytesRead) {
    //		Gets the reason why the function failed
    //		uint32_t uiLastError = GetLastError();
    //		char zString[1024];
    //		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

    return (FALSE);
  }

  gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFilePosInFile += uiNumBytesRead;

  //	Plat_CloseFile( hLibraryFile );

  *pBytesRead = uiNumBytesRead;

  return (TRUE);
}

//************************************************************************
//
// CheckIfFileExistInLibrary() determines if a file exists in a library.
//
//************************************************************************

BOOLEAN CheckIfFileExistInLibrary(char *pFileName) {
  int16_t sLibraryID;
  FileHeaderStruct *pFileHeader;

  // get thelibrary that file is in
  sLibraryID = GetLibraryIDFromFileName(pFileName);
  if (sLibraryID == -1) {
    // not in any library
    return (FALSE);
  }

  if (GetFileHeaderFromLibrary(sLibraryID, pFileName, &pFileHeader))
    return (TRUE);
  else
    return (FALSE);
}

//************************************************************************
//
//	This function finds out if the file CAN be in a library.  It determines
//	if the library that the file MAY be in is open.
//	( eg. File is  Laptop\Test.sti, if the Laptop\ library is open, it returns true
//
//************************************************************************
int16_t GetLibraryIDFromFileName(char *pFileName) {
  int16_t sLoop1, sBestMatch = -1;

  // loop through all the libraries to check which library the file is in
  for (sLoop1 = 0; sLoop1 < gFileDataBase.usNumberOfLibraries; sLoop1++) {
    // if the library is not loaded, dont try to access the array
    if (IsLibraryOpened(sLoop1)) {
      // if the library path name is of size zero, ( the library is for the default path )
      if (strlen(gFileDataBase.pLibraries[sLoop1].sLibraryPath) == 0) {
        // determine if there is a directory in the file name
        if (strchr(pFileName, '\\') == NULL && strchr(pFileName, '/') == NULL) {
          // There is no directory in the file name
          return (sLoop1);
        }
      }

      // compare the library name to the file name that is passed in
      else {
        // if the directory paths are the same, to the length of the lib's path
        if (strncasecmp(gFileDataBase.pLibraries[sLoop1].sLibraryPath, pFileName,
                        strlen(gFileDataBase.pLibraries[sLoop1].sLibraryPath)) == 0) {
          // if we've never matched, or this match's path is longer than the previous match (meaning
          // it's more exact)
          if ((sBestMatch == (-1)) || (strlen(gFileDataBase.pLibraries[sLoop1].sLibraryPath) >
                                       strlen(gFileDataBase.pLibraries[sBestMatch].sLibraryPath)))
            sBestMatch = sLoop1;
        }
      }
    }
  }

  // no library was found, return an error
  return (sBestMatch);
}

//************************************************************************
//
//	GetFileHeaderFromLibrary() performsperforms a binary search of the
//	library.  It adds the libraries path to the file in the
//	library and then string compared that to the name that we are
//	searching for.
//
//************************************************************************

BOOLEAN GetFileHeaderFromLibrary(int16_t sLibraryID, char *pstrFileName,
                                 FileHeaderStruct **pFileHeader) {
  FileHeaderStruct **ppFileHeader;
  char sFileNameWithPath[FILENAME_SIZE];

  // combine the library path to the file name (need it for the search of the library )
  strcpy(sFileNameWithPath, pstrFileName);

  gsCurrentLibrary = sLibraryID;

  /* try to find the filename using a binary search algorithm: */
  ppFileHeader = (FileHeaderStruct **)bsearch(
      (char *)&sFileNameWithPath,
      (FileHeaderStruct *)gFileDataBase.pLibraries[sLibraryID].pFileHeader,
      gFileDataBase.pLibraries[sLibraryID].usNumberOfEntries, sizeof(FileHeaderStruct),
      (int (*)(const void *, const void *))CompareFileNames);

  if (ppFileHeader) {
    *pFileHeader = (FileHeaderStruct *)ppFileHeader;
    return (TRUE);
  } else {
    pFileHeader = NULL;
    return (FALSE);
  }
}

static int CompareFileNames(char *arg1, FileHeaderStruct *arg2) {
  char sSearchKey[FILENAME_SIZE];
  char sFileNameWithPath[FILENAME_SIZE];

  sprintf(sSearchKey, "%s", arg1);

  sprintf(sFileNameWithPath, "%s%s", gFileDataBase.pLibraries[gsCurrentLibrary].sLibraryPath,
          arg2->pFileName);

  /* Compare all of both strings: */
  return strcasecmp(sSearchKey, sFileNameWithPath);
}

void AddSlashToPath(char *pName) {
  uint32_t uiLoop, uiCounter;
  BOOLEAN fDone = FALSE;
  char sNewName[FILENAME_SIZE];

  // find out if there is a '\' in the file name

  uiCounter = 0;
  for (uiLoop = 0; uiLoop < strlen(pName) && !fDone; uiLoop++) {
    if (pName[uiLoop] == '\\') {
      sNewName[uiCounter] = pName[uiLoop];
      uiCounter++;
      sNewName[uiCounter] = '\\';
    } else
      sNewName[uiCounter] = pName[uiLoop];

    uiCounter++;
  }
  sNewName[uiCounter] = '\0';

  strcpy(pName, sNewName);
}

//************************************************************************
//
// This function will see if a file is in a library.  If it is, the file will be opened and a file
// handle will be created for it.
//
//************************************************************************

HWFILE OpenFileFromLibrary(char *pName) {
  FileHeaderStruct *pFileHeader;
  HWFILE hLibFile;
  int16_t sLibraryID;
  uint16_t uiLoop1;
  uint32_t uiFileNum = 0;

  // Check if the file can be contained from an open library ( the path to the file a library path )
  sLibraryID = GetLibraryIDFromFileName(pName);

  if (sLibraryID != -1) {
    // Check if another file is already open in the library ( report a warning if so )

    if (gFileDataBase.pLibraries[sLibraryID].uiIdOfOtherFileAlreadyOpenedLibrary != 0) {
    }

    // check if the file is already open
    if (CheckIfFileIsAlreadyOpen(pName, sLibraryID)) return (0);

    // if the file is in a library, get the file
    if (GetFileHeaderFromLibrary(sLibraryID, pName, &pFileHeader)) {
      // increment the number of open files
      gFileDataBase.pLibraries[sLibraryID].iNumFilesOpen++;

      // if there isnt enough space to put the file, realloc more space
      if (gFileDataBase.pLibraries[sLibraryID].iNumFilesOpen >=
          gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray) {
        FileOpenStruct *pOpenFiles;

        // reallocate more space for the array
        pOpenFiles = (FileOpenStruct *)MemRealloc(
            gFileDataBase.pLibraries[sLibraryID].pOpenFiles,
            gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME);

        if (!pOpenFiles) return (0);

        // increment the number of open files that we can have open
        gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;

        gFileDataBase.pLibraries[sLibraryID].pOpenFiles = pOpenFiles;
      }

      // loop through to find a new spot in the array
      uiFileNum = 0;
      for (uiLoop1 = 1; uiLoop1 < gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray;
           uiLoop1++) {
        if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiLoop1].uiFileID == 0) {
          uiFileNum = uiLoop1;
          break;
        }
      }

      // if for some reason we couldnt find a spot, return an error
      if (uiFileNum == 0) return (0);

      // Create a library handle for the new file
      hLibFile = CreateLibraryFileHandle(sLibraryID, uiFileNum);

      // Set the current file data into the array of open files
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFileID = hLibFile;
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFilePosInFile = 0;
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader = pFileHeader;

      // Save the current file position in the library
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiActualPositionInLibrary =
          Plat_SetFilePointer(gFileDataBase.pLibraries[sLibraryID].hLibraryHandle, 0,
                              FILE_SEEK_FROM_CURRENT);

      // Set the file position in the library to the begining of the 'file' in the library
      Plat_SetFilePointer(
          gFileDataBase.pLibraries[sLibraryID].hLibraryHandle,
          gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileOffset,
          FILE_SEEK_FROM_START);

      Plat_GetFileSize(gFileDataBase.pLibraries[sLibraryID].hLibraryHandle);

    } else {
      // Failed to find the file in a library
      return (0);
    }
  } else {
    // Library is not open, or doesnt exist
    return (0);
  }

  // Set the fact the a file is currently open in the library
  //	gFileDataBase.pLibraries[ sLibraryID ].fAnotherFileAlreadyOpenedLibrary = TRUE;
  gFileDataBase.pLibraries[sLibraryID].uiIdOfOtherFileAlreadyOpenedLibrary = uiFileNum;

  return (hLibFile);
}

HWFILE CreateLibraryFileHandle(int16_t sLibraryID, uint32_t uiFileNum) {
  HWFILE hLibFile;

  hLibFile = uiFileNum;
  hLibFile |= DB_ADD_LIBRARY_ID(sLibraryID);

  return (hLibFile);
}

HWFILE CreateRealFileHandle(SYS_FILE_HANDLE hFile) {
  HWFILE hLibFile;
  int32_t iLoop1;
  uint32_t uiFileNum = 0;
  uint32_t uiSize;

  // if there isnt enough space to put the file, realloc more space
  if (gFileDataBase.RealFiles.iNumFilesOpen >= (gFileDataBase.RealFiles.iSizeOfOpenFileArray - 1)) {
    uiSize = (gFileDataBase.RealFiles.iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME) *
             sizeof(RealFileOpenStruct);

    gFileDataBase.RealFiles.pRealFilesOpen =
        (RealFileOpenStruct *)MemRealloc(gFileDataBase.RealFiles.pRealFilesOpen, uiSize);
    CHECKF(gFileDataBase.RealFiles.pRealFilesOpen);

    // Clear out the new part of the array
    memset(&gFileDataBase.RealFiles.pRealFilesOpen[gFileDataBase.RealFiles.iSizeOfOpenFileArray], 0,
           (NUM_FILES_TO_ADD_AT_A_TIME * sizeof(RealFileOpenStruct)));

    gFileDataBase.RealFiles.iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;
  }

  // loop through to find a new spot in the array
  uiFileNum = 0;
  for (iLoop1 = 1; iLoop1 < gFileDataBase.RealFiles.iSizeOfOpenFileArray; iLoop1++) {
    if (gFileDataBase.RealFiles.pRealFilesOpen[iLoop1].uiFileID == 0) {
      uiFileNum = iLoop1;
      break;
    }
  }

  // if for some reason we couldnt find a spot, return an error
  if (uiFileNum == 0) return (0);

  hLibFile = uiFileNum;
  hLibFile |= DB_ADD_LIBRARY_ID(REAL_FILE_LIBRARY_ID);

  gFileDataBase.RealFiles.pRealFilesOpen[iLoop1].uiFileID = hLibFile;
  gFileDataBase.RealFiles.pRealFilesOpen[iLoop1].hRealFileHandle = hFile;

  gFileDataBase.RealFiles.iNumFilesOpen++;

  return (hLibFile);
}

BOOLEAN GetLibraryAndFileIDFromLibraryFileHandle(HWFILE hlibFile, int16_t *pLibraryID,
                                                 uint32_t *pFileNum) {
  *pFileNum = DB_EXTRACT_FILE_ID(hlibFile);
  *pLibraryID = (uint16_t)DB_EXTRACT_LIBRARY(hlibFile);

  // TEST: qq
  /*	if( *pLibraryID == LIBRARY_SOUNDS )
          {
                  int q=5;
          }
  */
  return (TRUE);
}

//************************************************************************
//
//	Close an individual file that is contained in the library
//
//************************************************************************

BOOLEAN CloseLibraryFile(int16_t sLibraryID, uint32_t uiFileID) {
  if (IsLibraryOpened(sLibraryID)) {
    // if the uiFileID is invalid
    if ((uiFileID >= (uint32_t)gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray))
      return (FALSE);

    // if the file is not opened, dont close it
    if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileID].uiFileID != 0) {
      // reset the variables
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileID].uiFileID = 0;
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileID].uiFilePosInFile = 0;
      gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileID].pFileHeader = NULL;

      // reset the libraries file pointer to the positon it was in prior to opening the current file
      Plat_SetFilePointer(
          gFileDataBase.pLibraries[sLibraryID].hLibraryHandle,
          gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileID].uiActualPositionInLibrary,
          FILE_SEEK_FROM_CURRENT);

      // decrement the number of files that are open
      gFileDataBase.pLibraries[sLibraryID].iNumFilesOpen--;

      // Reset the fact that a file is accessing the library
      //			gFileDataBase.pLibraries[ sLibraryID
      //].fAnotherFileAlreadyOpenedLibrary = FALSE;
      gFileDataBase.pLibraries[sLibraryID].uiIdOfOtherFileAlreadyOpenedLibrary = 0;
    }
  }

  return (TRUE);
}

BOOLEAN LibraryFileSeek(int16_t sLibraryID, uint32_t uiFileNum, uint32_t uiDistance,
                        uint8_t uiHowToSeek) {
  uint32_t uiCurPos, uiSize;

  // if the library is not open, return an error
  if (!IsLibraryOpened(sLibraryID)) return (FALSE);

  uiCurPos = gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFilePosInFile;
  uiSize = gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileLength;

  if (uiHowToSeek == FILE_SEEK_FROM_START)
    uiCurPos = uiDistance;
  else if (uiHowToSeek == FILE_SEEK_FROM_END)
    uiCurPos = uiSize - uiDistance;
  else if (uiHowToSeek == FILE_SEEK_FROM_CURRENT)
    uiCurPos += uiDistance;
  else
    return (FALSE);

  gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].uiFilePosInFile = uiCurPos;
  return (TRUE);
}

//************************************************************************
//
//	OpenLibrary() Opens a library from the 'array' of library names
//	that was passd in at game initialization.  Pass in an enum for the
//	library.
//
//************************************************************************

BOOLEAN OpenLibrary(int16_t sLibraryID) {
  // if the library is already opened, report an error
  if (gFileDataBase.pLibraries[sLibraryID].fLibraryOpen) return (FALSE);

  // if we are trying to do something with an invalid library id
  if (sLibraryID >= gFileDataBase.usNumberOfLibraries) return (FALSE);

  // if we cant open the library
  if (!InitializeLibrary(gGameLibaries[sLibraryID].sLibraryName,
                         &gFileDataBase.pLibraries[sLibraryID], gGameLibaries[sLibraryID].fOnCDrom))
    return (FALSE);

  return (TRUE);
}

BOOLEAN CloseLibrary(int16_t sLibraryID) {
  uint32_t uiLoop1;

  // if the library isnt loaded, dont close it
  if (!IsLibraryOpened(sLibraryID)) return (FALSE);

#ifdef JA2TESTVERSION
  FastDebugMsg(
      String("ShutDownFileDatabase( ): %d bytes of ram used for the Library #%3d, path '%s',  in "
             "the File Database System\n",
             gFileDataBase.pLibraries[sLibraryID].uiTotalMemoryAllocatedForLibrary, sLibraryID,
             gFileDataBase.pLibraries[sLibraryID].sLibraryPath));
  gFileDataBase.pLibraries[sLibraryID].uiTotalMemoryAllocatedForLibrary = 0;
#endif

  // if there are any open files, loop through the library and close down whatever file is still
  // open
  if (gFileDataBase.pLibraries[sLibraryID].iNumFilesOpen) {
    // loop though the array of open files to see if any are still open
    for (uiLoop1 = 0; uiLoop1 < (uint32_t)gFileDataBase.pLibraries[sLibraryID].usNumberOfEntries;
         uiLoop1++) {
      if (CheckIfFileIsAlreadyOpen(
              gFileDataBase.pLibraries[sLibraryID].pFileHeader[uiLoop1].pFileName, sLibraryID)) {
        FastDebugMsg(String(
            "CloseLibrary():  ERROR:  %s library file id still exists, wasnt closed, closing now.",
            gFileDataBase.pLibraries[sLibraryID].pFileHeader[uiLoop1].pFileName));
        CloseLibraryFile(sLibraryID, uiLoop1);

        //	Removed because the memory gets freed in the next for loop.  Would only enter here
        // if files were still open 	gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1
        //].pFileName = NULL;
      }
    }
  }

  // Free up the memory used for each file name
  for (uiLoop1 = 0; uiLoop1 < gFileDataBase.pLibraries[sLibraryID].usNumberOfEntries; uiLoop1++) {
    MemFree(gFileDataBase.pLibraries[sLibraryID].pFileHeader[uiLoop1].pFileName);
    gFileDataBase.pLibraries[sLibraryID].pFileHeader[uiLoop1].pFileName = NULL;
  }

  // Free up the memory needed for the Library File Headers
  if (gFileDataBase.pLibraries[sLibraryID].pFileHeader) {
    MemFree(gFileDataBase.pLibraries[sLibraryID].pFileHeader);
    gFileDataBase.pLibraries[sLibraryID].pFileHeader = NULL;
  }

  // Free up the memory used for the library name
  if (gFileDataBase.pLibraries[sLibraryID].sLibraryPath) {
    MemFree(gFileDataBase.pLibraries[sLibraryID].sLibraryPath);
    gFileDataBase.pLibraries[sLibraryID].sLibraryPath = NULL;
  }

  // Free up the space requiered for the open files array
  if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles) {
    MemFree(gFileDataBase.pLibraries[sLibraryID].pOpenFiles);
    gFileDataBase.pLibraries[sLibraryID].pOpenFiles = NULL;
  }

  // set that the library isnt open
  gFileDataBase.pLibraries[sLibraryID].fLibraryOpen = FALSE;

  // close the file ( note libraries are to be closed by the Windows close function )
  Plat_CloseFile(gFileDataBase.pLibraries[sLibraryID].hLibraryHandle);

  return (TRUE);
}

BOOLEAN IsLibraryOpened(int16_t sLibraryID) {
  // if the database is not initialized
  if (!gFileDataBase.fInitialized) return (FALSE);

  // if we are trying to do something with an invalid library id
  if (sLibraryID >= gFileDataBase.usNumberOfLibraries) return (FALSE);

  // if the library is opened
  if (gFileDataBase.pLibraries[sLibraryID].fLibraryOpen)
    return (TRUE);
  else
    return (FALSE);
}

BOOLEAN CheckIfFileIsAlreadyOpen(char *pFileName, int16_t sLibraryID) {
  uint16_t usLoop1 = 0;

  struct Str512 filename;

  if (!Plat_FileBaseName(pFileName, &filename)) {
    return false;
  }

  // loop through all the open files to see if 'new' file to open is already open
  for (usLoop1 = 1; usLoop1 < gFileDataBase.pLibraries[sLibraryID].iSizeOfOpenFileArray;
       usLoop1++) {
    // check if the file is open
    if (gFileDataBase.pLibraries[sLibraryID].pOpenFiles[usLoop1].uiFileID != 0) {
      // Check if the file already exists
      if (strcasecmp(
              filename.buf,
              gFileDataBase.pLibraries[sLibraryID].pOpenFiles[usLoop1].pFileHeader->pFileName) == 0)
        return (TRUE);
    }
  }
  return (FALSE);
}

//************************************************************************
//
//	CompareFileNames() gets called by the binary search function.
//
//************************************************************************

int32_t CompareDirEntryFileNames(char *arg1, DIRENTRY *arg2) {
  char sSearchKey[FILENAME_SIZE];
  char sFileNameWithPath[FILENAME_SIZE];

  sprintf(sSearchKey, "%s", arg1);

  sprintf(sFileNameWithPath, "%s", arg2->sFileName);

  /* Compare all of both strings: */
  return strcasecmp(sSearchKey, sFileNameWithPath);
}
