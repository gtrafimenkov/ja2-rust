#ifndef _LIBRARY_DATABASE_H
#define _LIBRARY_DATABASE_H

#include "SGP/FileMan.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/Types.h"
#include "platform.h"

#define FILENAME_SIZE 256

#define NUM_FILES_TO_ADD_AT_A_TIME 20
#define INITIAL_NUM_HANDLES 20

#define REAL_FILE_LIBRARY_ID 1022

#define DB_BITS_FOR_LIBRARY 10
#define DB_BITS_FOR_FILE_ID 22

#define DB_EXTRACT_LIBRARY(exp) (exp >> DB_BITS_FOR_FILE_ID)
#define DB_EXTRACT_FILE_ID(exp) (exp & 0x3FFFFF)

#define DB_ADD_LIBRARY_ID(exp) (exp << DB_BITS_FOR_FILE_ID)
#define DB_ADD_FILE_ID(exp) (exp & 0xC00000)

typedef struct {
  char sLibraryName[FILENAME_SIZE];  // The name of the library file on the disk
  BOOLEAN fOnCDrom;  // A flag specifying if its a cdrom library ( not implemented yet )
  BOOLEAN
  fInitOnStart;  // Flag specifying if the library is to Initialized at the begining of the game

} LibraryInitHeader;

#include "SGP/Ja2Libs.h"

extern LibraryInitHeader gGameLibaries[];

typedef struct {
  uint32_t uiFileID;                // id of the file ( they start at 1 )
  SYS_FILE_HANDLE hRealFileHandle;  // if the file is a Real File, this its handle
} RealFileOpenStruct;

typedef struct {
  char *pFileName;
  uint32_t uiFileLength;
  uint32_t uiFileOffset;
} FileHeaderStruct;

typedef struct {
  uint32_t uiFileID;                   // id of the file ( they start at 1 )
  uint32_t uiFilePosInFile;            // current position in the file
  uint32_t uiActualPositionInLibrary;  // Current File pointer position in actuall library
  FileHeaderStruct *pFileHeader;
} FileOpenStruct;

typedef struct {
  char *sLibraryPath;
  SYS_FILE_HANDLE hLibraryHandle;
  uint16_t usNumberOfEntries;
  BOOLEAN fLibraryOpen;
  //	BOOLEAN	fAnotherFileAlreadyOpenedLibrary;				//this variable is
  // set when a file is opened from the library and reset when the file is close.  No 2 files can
  // have
  // access
  // to the library at 1 time.
  uint32_t
      uiIdOfOtherFileAlreadyOpenedLibrary;  // this variable is set when a file is opened from
                                            // the library and reset when the file is close.  No
                                            // 2 files can have access to the library at 1 time.
  int32_t iNumFilesOpen;
  int32_t iSizeOfOpenFileArray;
  FileHeaderStruct *pFileHeader;
  FileOpenStruct *pOpenFiles;

  //
  //	Temp:	Total memory used for each library ( all memory allocated
  //
#ifdef JA2TESTVERSION
  uint32_t uiTotalMemoryAllocatedForLibrary;
#endif

} LibraryHeaderStruct;

typedef struct {
  int32_t iNumFilesOpen;
  int32_t iSizeOfOpenFileArray;
  RealFileOpenStruct *pRealFilesOpen;

} RealFileHeaderStruct;

typedef struct {
  char *sManagerName;
  LibraryHeaderStruct *pLibraries;
  uint16_t usNumberOfLibraries;
  BOOLEAN fInitialized;
  RealFileHeaderStruct RealFiles;
} DatabaseManagerHeaderStruct;

//*************************************************************************
//
//  NOTE!  The following structs are also used by the datalib98 utility
//
//*************************************************************************

#define FILE_OK 0
#define FILE_DELETED 0xff
#define FILE_OLD 1
#define FILE_DOESNT_EXIST 0xfe

typedef struct {
  char sLibName[FILENAME_SIZE];
  char sPathToLibrary[FILENAME_SIZE];
  int32_t iEntries;
  int32_t iUsed;
  uint16_t iSort;
  uint16_t iVersion;
  BOOLEAN fContainsSubDirectories;
  int32_t iReserved;
} LIBHEADER;

// The FileDatabaseHeader
extern DatabaseManagerHeaderStruct gFileDataBase;

// Function Prototypes

BOOLEAN CheckForLibraryExistence(char *pLibraryName);
BOOLEAN InitializeLibrary(char *pLibraryName, LibraryHeaderStruct *pLibheader,
                          BOOLEAN fCanBeOnCDrom);

BOOLEAN CheckIfFileExistInLibrary(char *pFileName);
int16_t GetLibraryIDFromFileName(char *pFileName);
HWFILE OpenFileFromLibrary(char *pName);
HWFILE CreateRealFileHandle(SYS_FILE_HANDLE hFile);
BOOLEAN CloseLibraryFile(int16_t sLibraryID, uint32_t uiFileID);
BOOLEAN GetLibraryAndFileIDFromLibraryFileHandle(HWFILE hlibFile, int16_t *pLibraryID,
                                                 uint32_t *pFileNum);
BOOLEAN LoadDataFromLibrary(int16_t sLibraryID, uint32_t uiFileIndex, void *pData,
                            uint32_t uiBytesToRead, uint32_t *pBytesRead);
BOOLEAN LibraryFileSeek(int16_t sLibraryID, uint32_t uiFileNum, uint32_t uiDistance,
                        uint8_t uiHowToSeek);

// used to open and close libraries during the game
BOOLEAN CloseLibrary(int16_t sLibraryID);
BOOLEAN OpenLibrary(int16_t sLibraryID);

#endif
