#ifndef _JA2_LOAD_SCREEN_
#define _JA2_LOAD_SCREEN_

#include "BuildDefines.h"
#include "Editor/LoadScreenPub.h"
#include "SGP/Types.h"

extern int32_t iFDlgState;
extern int32_t iFDlgSelectedSlot;

extern BOOLEAN gfAskForName;
extern BOOLEAN gfCreatedFDlg;
extern BOOLEAN gfDestroyFDlg;

uint32_t WaitForFileName(void);
void RemoveFileDialog(void);
void CreateFileDialog(wchar_t *zTitle);

void SelectFileDialogYPos(uint16_t usRelativeYPos);

void BuildFilenameWithCoordinate();
void BuildCoordinateWithFilename();
BOOLEAN ExtractFilenameFromFields();
BOOLEAN ValidCoordinate();
BOOLEAN ValidFilename();

BOOLEAN ExternalLoadMap(wchar_t* szFilename);
BOOLEAN ExternalSaveMap(wchar_t* szFilename);

extern BOOLEAN gfErrorCatch;
extern wchar_t gzErrorCatchString[256];

#endif
