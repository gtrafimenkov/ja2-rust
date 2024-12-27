// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H

#include "SGP/Types.h"

typedef struct {
  uint32_t TimeStamp;
  uint32_t uiFlags;
  uint16_t usDelay;
  uint32_t uiEvent;
  uint32_t uiDataSize;
  BYTE *pData;

} EVENT;

#define PRIMARY_EVENT_QUEUE 0
#define SECONDARY_EVENT_QUEUE 1
#define DEMAND_EVENT_QUEUE 2

#define EVENT_EXPIRED 0x00000002

// Management fucntions
BOOLEAN InitializeEventManager();
BOOLEAN ShutdownEventManager();

BOOLEAN AddEvent(uint32_t uiEvent, uint16_t usDelay, PTR pEventData, uint32_t uiDataSize,
                 uint8_t ubQueueID);
BOOLEAN RemoveEvent(EVENT **ppEvent, uint32_t uiIndex, uint8_t ubQueueID);
BOOLEAN PeekEvent(EVENT **ppEvent, uint32_t uiIndex, uint8_t ubQueueID);
BOOLEAN FreeEvent(EVENT *pEvent);
uint32_t EventQueueSize(uint8_t ubQueueID);

#endif
