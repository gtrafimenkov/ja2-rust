#ifndef _AILIST_H
#define _AILIST_H
#include "SGP/Types.h"

typedef struct AILIST {
  uint8_t ubID;
  INT8 bPriority;
  uint8_t ubUnused;
  struct AILIST* pNext;
} AILIST;

#define MAX_AI_PRIORITY 100

extern void ClearAIList(void);
extern AILIST* CreateNewAIListEntry(uint8_t ubNewEntry, uint8_t ubID, INT8 bAlertStatus);
extern BOOLEAN InsertIntoAIList(uint8_t ubID, INT8 bAlertStatus);
extern uint8_t RemoveFirstAIListEntry(void);
extern BOOLEAN BuildAIListForTeam(INT8 bTeam);
extern BOOLEAN MoveToFrontOfAIList(uint8_t ubID);
#endif
