#ifndef _JA2_LOAD_SCREEN_
#define _JA2_LOAD_SCREEN_

#include "BuildDefines.h"
#include "Editor/LoadScreenPub.h"
#include "SGP/Types.h"

extern INT32 iFDlgState;
extern INT32 iFDlgSelectedSlot;

extern BOOLEAN gfAskForName;
extern BOOLEAN gfCreatedFDlg;
extern BOOLEAN gfDestroyFDlg;

UINT32 WaitForFileName(void);
void RemoveFileDialog(void);
void CreateFileDialog(CHAR16 *zTitle);

void SelectFileDialogYPos(UINT16 usRelativeYPos);

void BuildFilenameWithCoordinate();
void BuildCoordinateWithFilename();
BOOLEAN ExtractFilenameFromFields();
BOOLEAN ValidCoordinate();
BOOLEAN ValidFilename();

BOOLEAN ExternalLoadMap(STR16 szFilename);
BOOLEAN ExternalSaveMap(STR16 szFilename);

extern BOOLEAN gfErrorCatch;
extern CHAR16 gzErrorCatchString[256];

#endif
