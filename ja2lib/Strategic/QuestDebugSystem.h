#ifndef _QUEST_DEBUG_SYSTEM_H_
#define _QUEST_DEBUG_SYSTEM_H_

#include "TacticalAI/NPC.h"

extern BOOLEAN gfNpcLogButton;
extern INT16 gsQdsEnteringGridNo;

void NpcRecordLoggingInit(UINT8 ubNpcID, UINT8 ubMercID, UINT8 ubQuoteNum, UINT8 ubApproach);
void NpcRecordLogging(UINT8 ubApproach, STR pStringA, ...);

#endif
