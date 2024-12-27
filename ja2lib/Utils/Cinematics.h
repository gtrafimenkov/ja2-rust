#ifndef _CINEMATICS_H_
#define _CINEMATICS_H_

#include "SGP/Types.h"

struct SmackTag;
struct SmackBufTag;
struct SmkFlic;

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight);
void SmkShutdown(void);
struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
struct SmkFlic *SmkOpenFlic(char *cFilename);
void SmkSetBlitPosition(struct SmkFlic *pSmack, uint32_t uiLeft, uint32_t uiTop);
void SmkCloseFlic(struct SmkFlic *pSmack);
struct SmkFlic *SmkGetFreeFlic(void);

#endif
