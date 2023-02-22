#ifndef _JA2_LOAD_SCREEN_PUB__
#define _JA2_LOAD_SCREEN_PUB__

#include "SGP/Types.h"

struct FileDialogList;

extern struct FileDialogList *AddToFDlgList(struct FileDialogList *pList, struct GetFile *pInfo);

extern void TrashFDlgList(struct FileDialogList *pList);

#endif
