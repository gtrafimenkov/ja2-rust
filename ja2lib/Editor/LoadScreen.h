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
void CreateFileDialog(CHAR16 *zTitle);

void SelectFileDialogYPos(uint16_t usRelativeYPos);

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
