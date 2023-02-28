#ifndef _CINEMATICS_H_
#define _CINEMATICS_H_

#include "SGP/Types.h"

struct SmackTag;
struct SmackBufTag;
struct SmkFlic;

void SmkInitialize(UINT32 uiWidth, UINT32 uiHeight);
void SmkShutdown(void);
struct SmkFlic *SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
struct SmkFlic *SmkOpenFlic(CHAR8 *cFilename);
void SmkSetBlitPosition(struct SmkFlic *pSmack, UINT32 uiLeft, UINT32 uiTop);
void SmkCloseFlic(struct SmkFlic *pSmack);
struct SmkFlic *SmkGetFreeFlic(void);

#endif
