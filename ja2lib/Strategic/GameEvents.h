#ifndef __GAME_EVENTS_H
#define __GAME_EVENTS_H

#include "Strategic/GameEventHook.h"

#define SEF_PREVENT_DELETION 0x01
#define SEF_DELETION_PENDING 0x02

typedef struct STRATEGICEVENT {
  struct STRATEGICEVENT *next;
  uint32_t uiTimeStamp;
  uint32_t uiParam;
  uint32_t uiTimeOffset;
  uint8_t ubEventType;
  uint8_t ubCallbackID;
  uint8_t ubFlags;
  INT8 bPadding[6];
} STRATEGICEVENT;

enum { ONETIME_EVENT, RANGED_EVENT, ENDRANGED_EVENT, EVERYDAY_EVENT, PERIODIC_EVENT, QUEUED_EVENT };

void LockStrategicEventFromDeletion(STRATEGICEVENT *pEvent);
void UnlockStrategicEventFromDeletion(STRATEGICEVENT *pEvent);

// part of the game.sav files (not map files)
BOOLEAN SaveStrategicEventsToSavedGame(HWFILE hFile);
BOOLEAN LoadStrategicEventsFromSavedGame(HWFILE hFile);

STRATEGICEVENT *AddAdvancedStrategicEvent(uint8_t ubEventType, uint8_t ubCallbackID,
                                          uint32_t uiTimeStamp, uint32_t uiParam);

BOOLEAN ExecuteStrategicEvent(STRATEGICEVENT *pEvent);

extern BOOLEAN gfEventDeletionPending;

BOOLEAN DeleteEventsWithDeletionPending();

extern STRATEGICEVENT *gpEventList;

#endif
