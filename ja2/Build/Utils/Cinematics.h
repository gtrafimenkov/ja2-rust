#ifndef _CINEMATICS_H_
#define _CINEMATICS_H_

#include "SGP/Types.h"

#if 1
// must go after other includes
#include <ddraw.h>
#endif

struct SmackTag;
struct SmackBufTag;

typedef struct {
  CHAR8 *cFilename;
  HWFILE hFileHandle;
  struct SmackTag *SmackHandle;
  struct SmackBufTag *SmackBuffer;
  UINT32 uiFlags;
  LPDIRECTDRAWSURFACE2 lpDDS;
  HWND hWindow;
  UINT32 uiFrame;
  UINT32 uiLeft, uiTop;
} SMKFLIC;

void SmkInitialize(UINT32 uiWidth, UINT32 uiHeight);
void SmkShutdown(void);
SMKFLIC *SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
SMKFLIC *SmkOpenFlic(CHAR8 *cFilename);
void SmkSetBlitPosition(SMKFLIC *pSmack, UINT32 uiLeft, UINT32 uiTop);
void SmkCloseFlic(SMKFLIC *pSmack);
SMKFLIC *SmkGetFreeFlic(void);

#endif
