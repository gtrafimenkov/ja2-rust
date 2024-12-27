// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/EventManager.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "SGP/Container.h"
#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "Utils/TimerControl.h"

HLIST hEventQueue = NULL;
HLIST hDelayEventQueue = NULL;
HLIST hDemandEventQueue = NULL;

#define QUEUE_RESIZE 20

// LOCAL FUNCTIONS
HLIST GetQueue(uint8_t ubQueueID);
void SetQueue(uint8_t ubQueueID, HLIST hQueue);

BOOLEAN InitializeEventManager() {
  // Create Queue
  hEventQueue = CreateList(QUEUE_RESIZE, sizeof(PTR));

  if (hEventQueue == NULL) {
    return (FALSE);
  }

  // Create Delay Queue
  hDelayEventQueue = CreateList(QUEUE_RESIZE, sizeof(PTR));

  if (hDelayEventQueue == NULL) {
    return (FALSE);
  }

  // Create Demand Queue (events on this queue are only processed when specifically
  // called for by code)
  hDemandEventQueue = CreateList(QUEUE_RESIZE, sizeof(PTR));

  if (hDemandEventQueue == NULL) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN ShutdownEventManager() {
  if (hEventQueue != NULL) {
    DeleteList(hEventQueue);
  }

  if (hDelayEventQueue != NULL) {
    DeleteList(hDelayEventQueue);
  }

  if (hDemandEventQueue != NULL) {
    DeleteList(hDemandEventQueue);
  }

  return (TRUE);
}

BOOLEAN AddEvent(uint32_t uiEvent, uint16_t usDelay, PTR pEventData, uint32_t uiDataSize,
                 uint8_t ubQueueID) {
  EVENT *pEvent;
  uint32_t uiEventSize = sizeof(EVENT);
  HLIST hQueue;

  // Allocate new event
  pEvent = (EVENT *)MemAlloc(uiEventSize + uiDataSize);

  if (!(pEvent != NULL)) {
    return FALSE;
  }

  // Set values
  pEvent->TimeStamp = GetJA2Clock();
  pEvent->usDelay = usDelay;
  pEvent->uiEvent = uiEvent;
  pEvent->uiFlags = 0;
  pEvent->uiDataSize = uiDataSize;
  pEvent->pData = (BYTE *)pEvent;
  pEvent->pData = pEvent->pData + uiEventSize;

  memcpy(pEvent->pData, pEventData, uiDataSize);

  // Add event to queue
  hQueue = GetQueue(ubQueueID);
  hQueue = AddtoList(hQueue, &pEvent, ListSize(hQueue));
  SetQueue(ubQueueID, hQueue);

  return (TRUE);
}

BOOLEAN RemoveEvent(EVENT **ppEvent, uint32_t uiIndex, uint8_t ubQueueID) {
  uint32_t uiQueueSize;
  HLIST hQueue;

  // Get an event from queue, if one exists
  //

  hQueue = GetQueue(ubQueueID);

  // Get Size
  uiQueueSize = ListSize(hQueue);

  if (uiQueueSize > 0) {
    // Get
    if (!(RemfromList(hQueue, ppEvent, uiIndex) != FALSE)) {
      return FALSE;
    }
  } else {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN PeekEvent(EVENT **ppEvent, uint32_t uiIndex, uint8_t ubQueueID) {
  uint32_t uiQueueSize;
  HLIST hQueue;

  // Get an event from queue, if one exists
  //

  hQueue = GetQueue(ubQueueID);

  // Get Size
  uiQueueSize = ListSize(hQueue);

  if (uiQueueSize > 0) {
    // Get
    if (!(PeekList(hQueue, ppEvent, uiIndex) != FALSE)) {
      return FALSE;
    }
  } else {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN FreeEvent(EVENT *pEvent) {
  if (!(pEvent != NULL)) {
    return FALSE;
  }

  // Delete event
  MemFree(pEvent);

  return (TRUE);
}

uint32_t EventQueueSize(uint8_t ubQueueID) {
  uint32_t uiQueueSize;
  HLIST hQueue;

  // Get an event from queue, if one exists
  //

  hQueue = GetQueue(ubQueueID);

  // Get Size
  uiQueueSize = ListSize(hQueue);

  return (uiQueueSize);
}

HLIST GetQueue(uint8_t ubQueueID) {
  switch (ubQueueID) {
    case PRIMARY_EVENT_QUEUE:
      return (hEventQueue);
      break;

    case SECONDARY_EVENT_QUEUE:
      return (hDelayEventQueue);
      break;

    case DEMAND_EVENT_QUEUE:
      return (hDemandEventQueue);
      break;

    default:
      Assert(FALSE);
      return (0);
      break;
  }
}

void SetQueue(uint8_t ubQueueID, HQUEUE hQueue) {
  switch (ubQueueID) {
    case PRIMARY_EVENT_QUEUE:
      hEventQueue = hQueue;
      break;

    case SECONDARY_EVENT_QUEUE:
      hDelayEventQueue = hQueue;
      break;

    case DEMAND_EVENT_QUEUE:
      hDemandEventQueue = hQueue;
      break;

    default:
      Assert(FALSE);
      break;
  }
}
