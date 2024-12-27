// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _JA2_LOAD_SCREEN_PUB__
#define _JA2_LOAD_SCREEN_PUB__

#include "SGP/Types.h"

struct FileDialogList;
struct GetFile;

extern struct FileDialogList *AddToFDlgList(struct FileDialogList *pList, struct GetFile *pInfo);

extern void TrashFDlgList(struct FileDialogList *pList);

#endif
