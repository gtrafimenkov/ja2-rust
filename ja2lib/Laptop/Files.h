#ifndef __FILES_H
#define __FILES_H

#include "SGP/Types.h"

#define FILES_DAT_FILE "TEMP\\files.dat"

void GameInitFiles();
void EnterFiles();
void ExitFiles();
void HandleFiles();
void RenderFiles();

struct files {
  uint8_t ubCode;       // the code index in the files code table
  uint8_t ubFormat;     // layout format
  uint32_t uiIdNumber;  // unique id number
  uint32_t uiDate;      // time in the world in global time (resolution, minutes)
  BOOLEAN fRead;
  STR8 pPicFileNameList[2];

  struct files *Next;  // next unit in the list
};

struct filestring {
  STR16 pString;
  struct filestring *Next;
};

typedef struct filestring FileString;
typedef struct filestring *FileStringPtr;

// files codes
enum {
  FILES_WELCOME_NOTICE,
};

// special codes for special files
enum {
  ENRICO_BACKGROUND = 0,
  SLAY_BACKGROUND,
  MATRON_BACKGROUND,
  IMPOSTER_BACKGROUND,
  TIFFANY_BACKGROUND,
  REXALL_BACKGROUND,
  ELGIN_BACKGROUND,

};
extern uint8_t ubFileRecordsLength[];
extern BOOLEAN fEnteredFileViewerFromNewFileIcon;
extern BOOLEAN fNewFilesInFileViewer;

typedef struct files FilesUnit;
typedef struct files *FilesUnitPtr;

extern FilesUnitPtr pFilesListHead;

struct filerecordwidth {
  int32_t iRecordNumber;
  int32_t iRecordWidth;
  int32_t iRecordHeightAdjustment;
  uint8_t ubFlags;
  struct filerecordwidth *Next;
};

typedef struct filerecordwidth FileRecordWidth;
typedef struct filerecordwidth *FileRecordWidthPtr;
uint32_t AddFilesToPlayersLog(uint8_t ubCode, uint32_t uiDate, uint8_t ubFormat, STR8 pFirstPicFile,
                              STR8 pSecondPicFile);

// add a file about this terrorist
BOOLEAN AddFileAboutTerrorist(int32_t iProfileId);

#endif
