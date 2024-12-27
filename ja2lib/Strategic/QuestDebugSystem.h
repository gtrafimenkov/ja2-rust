// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _QUEST_DEBUG_SYSTEM_H_
#define _QUEST_DEBUG_SYSTEM_H_

#include "TacticalAI/NPC.h"

extern BOOLEAN gfNpcLogButton;
extern int16_t gsQdsEnteringGridNo;

void NpcRecordLoggingInit(uint8_t ubNpcID, uint8_t ubMercID, uint8_t ubQuoteNum, uint8_t ubApproach);
void NpcRecordLogging(uint8_t ubApproach, char* pStringA, ...);

#endif
