// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/GameEvents.h"

#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"

// #include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "Strategic/GameClock.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "rust_fileman.h"

#ifdef JA2TESTVERSION

wchar_t gEventName[NUMBER_OF_EVENT_TYPES_PLUS_ONE][40] = {
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"Null",
    L"ChangeLightValue",
    L"WeatherStart",
    L"WeatherEnd",
    L"CheckForQuests",
    L"Ambient",
    L"AIMResetMercAnnoyance",
    L"BobbyRayPurchase",
    L"DailyUpdateBobbyRayInventory",
    L"UpdateBobbyRayInventory",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"DailyUpdateOfMercSite",
    L"Day3AddEMailFromSpeck",
    L"DelayedHiringOfMerc",
    L"HandleInsuredMercs",
    L"PayLifeInsuranceForDeadMerc",
    L"MercDailyUpdate",
    L"MercAboutToLeaveComment",
    L"MercContractOver",
    L"GroupArrival",
    L"Day2AddEMailFromIMP",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"MercComplainEquipment",
    L"HourlyUpdate",
    L"HandleMineIncome",
    L"SetupMineIncome",
    L"QueuedBattle",
    L"LeavingMercArriveInDrassen",
    L"LeavingMercArriveInOmerta",
    L"SetByNPCSystem",
    L"SecondAirportAttendantArrived",
    L"HelicopterHoverTooLong",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"HelicopterHoverWayTooLong",
    L"HelicopterDoneRefuelling",
    L"MercLeaveEquipInOmerta",
    L"MercLeaveEquipInDrassen",
    L"DailyEarlyMorningEvents",
    L"GroupAboutToArrive",
    L"ProcessTacticalSchedule",
    L"BeginRainStorm",
    L"EndRainStorm",
    L"HandleTownOpinion",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"SetupTownOpinion",
    L"DelayedDeathHandling",
    L"BeginAirRaid",
    L"TownLoyaltyUpdate",
    L"Meanwhile",
    L"BeginCreatureQuest",
    L"CreatureSpread",
    L"DecayCreatures",
    L"CreatureNightPlanning",
    L"CreatureAttack",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"EvaluateQueenSituation",
    L"CheckEnemyControlledSector",
    L"TurnOnNightLights",
    L"TurnOffNightLights",
    L"TurnOnPrimeLights",
    L"TurnOffPrimeLights",
    L"MercAboutToLeaveComment",
    L"ForceTimeInterupt",
    L"EnricoEmailEvent",
    L"InsuranceInvestigationStarted",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"InsuranceInvestigationOver",
    L"HandleMinuteUpdate",
    L"TemperatureUpdate",
    L"Keith going out of business",
    L"MERC site back online",
    L"Investigate Sector",
    L"CheckIfMineCleared",
    L"RemoveAssassin",
    L"BandageBleedingMercs",
    L"ShowUpdateMenu",
    // 1234567890123456789012345678901234567890 (increase size of array if necessary)
    L"SetMenuReason",
    L"AddSoldierToUpdateBox",
    L"BeginContractRenewalSequence",
    L"RPC_WHINE_ABOUT_PAY",
    L"HaventMadeImpCharacterEmail",
    L"Rainstorm",
    L"Quarter Hour Update",
    L"MERC Merc went up level email delay",
    L".",
};

#endif

void ValidateGameEvents();

STRATEGICEVENT *gpEventList = NULL;

extern BOOLEAN gfTimeInterruptPause;
BOOLEAN gfPreventDeletionOfAnyEvent = FALSE;
BOOLEAN gfEventDeletionPending = FALSE;

BOOLEAN gfProcessingGameEvents = FALSE;
uint32_t guiTimeStampOfCurrentlyExecutingEvent = 0;

// Determines if there are any events that will be processed between the current global time,
// and the beginning of the next global time.
BOOLEAN GameEventsPending(uint32_t uiAdjustment) {
  if (!gpEventList) return FALSE;
  if (gpEventList->uiTimeStamp <= GetGameTimeInSec() + uiAdjustment) return TRUE;
  return FALSE;
}

// returns TRUE if any events were deleted
BOOLEAN DeleteEventsWithDeletionPending() {
  STRATEGICEVENT *curr, *prev, *temp;
  BOOLEAN fEventDeleted = FALSE;
  // ValidateGameEvents();
  curr = gpEventList;
  prev = NULL;
  while (curr) {
    // ValidateGameEvents();
    if (curr->ubFlags & SEF_DELETION_PENDING) {
      if (prev) {  // deleting node in middle
        prev->next = curr->next;
        temp = curr;
        curr = curr->next;
        MemFree(temp);
        fEventDeleted = TRUE;
        // ValidateGameEvents();
        continue;
      } else {  // deleting head
        gpEventList = gpEventList->next;
        temp = curr;
        prev = NULL;
        curr = curr->next;
        MemFree(temp);
        fEventDeleted = TRUE;
        // ValidateGameEvents();
        continue;
      }
    }
    prev = curr;
    curr = curr->next;
  }
  gfEventDeletionPending = FALSE;
  return fEventDeleted;
}

static void AdjustClockToEventStamp(STRATEGICEVENT *pEvent, uint32_t *puiAdjustment) {
  uint32_t uiDiff = pEvent->uiTimeStamp - GetGameTimeInSec();
  MoveGameTimeForward(uiDiff);
  *puiAdjustment -= uiDiff;

  UpdateGameClockString2();
}

// If there are any events pending, they are processed, until the time limit is reached, or
// a major event is processed (one that requires the player's attention).
void ProcessPendingGameEvents(uint32_t uiAdjustment, uint8_t ubWarpCode) {
  STRATEGICEVENT *curr, *pEvent, *prev, *temp;
  BOOLEAN fDeleteEvent = FALSE, fDeleteQueuedEvent = FALSE;

  gfTimeInterrupt = FALSE;
  gfProcessingGameEvents = TRUE;

  // While we have events inside the time range to be updated, process them...
  curr = gpEventList;
  prev = NULL;  // prev only used when warping time to target time.
  while (!gfTimeInterrupt && curr && curr->uiTimeStamp <= GetGameTimeInSec() + uiAdjustment) {
    fDeleteEvent = FALSE;
    // Update the time by the difference, but ONLY if the event comes after the current time.
    // In the beginning of the game, series of events are created that are placed in the list
    // BEFORE the start time.  Those events will be processed without influencing the actual time.
    if (curr->uiTimeStamp > GetGameTimeInSec() &&
        ubWarpCode != WARPTIME_PROCESS_TARGET_TIME_FIRST) {
      AdjustClockToEventStamp(curr, &uiAdjustment);
    }
    // Process the event
    if (ubWarpCode != WARPTIME_PROCESS_TARGET_TIME_FIRST) {
      fDeleteEvent = ExecuteStrategicEvent(curr);
    } else if (curr->uiTimeStamp == GetGameTimeInSec() + uiAdjustment) {
      // if we are warping to the target time to
      // process that event first,
      if (!curr->next || curr->next->uiTimeStamp > GetGameTimeInSec() + uiAdjustment) {
        // make sure that we are processing
        // the last event for that second
        AdjustClockToEventStamp(curr, &uiAdjustment);

        fDeleteEvent = ExecuteStrategicEvent(curr);

        if (curr && prev && fDeleteQueuedEvent) {
          // The only case where we are deleting a node in
          // the middle of the list
          prev->next = curr->next;
        }
      } else {
        // We are at the current target warp time however, there are still other events
        // following in this time cycle.
        // We will only target the final event in this time.  NOTE:  Events are posted using a FIFO
        // method
        prev = curr;
        curr = curr->next;
        continue;
      }
    } else {
      // We are warping time to the target time.  We haven't found the event yet,
      // so continuing will keep processing the list until we find it.  NOTE:  Events are posted
      // using a FIFO method
      prev = curr;
      curr = curr->next;
      continue;
    }
    if (fDeleteEvent) {
      // Determine if event node is a special event requiring reposting
      switch (curr->ubEventType) {
        case RANGED_EVENT:
          AddAdvancedStrategicEvent(ENDRANGED_EVENT, curr->ubCallbackID,
                                    curr->uiTimeStamp + curr->uiTimeOffset, curr->uiParam);
          break;
        case PERIODIC_EVENT:
          pEvent = AddAdvancedStrategicEvent(PERIODIC_EVENT, curr->ubCallbackID,
                                             curr->uiTimeStamp + curr->uiTimeOffset, curr->uiParam);
          if (pEvent) pEvent->uiTimeOffset = curr->uiTimeOffset;
          break;
        case EVERYDAY_EVENT:
          AddAdvancedStrategicEvent(EVERYDAY_EVENT, curr->ubCallbackID,
                                    curr->uiTimeStamp + NUM_SEC_IN_DAY, curr->uiParam);
          break;
      }
      if (curr == gpEventList) {
        gpEventList = gpEventList->next;
        MemFree(curr);
        curr = gpEventList;
        prev = NULL;
        // ValidateGameEvents();
      } else {
        temp = curr;
        prev->next = curr->next;
        curr = curr->next;
        MemFree(temp);
        // ValidateGameEvents();
      }
    } else {
      prev = curr;
      curr = curr->next;
    }
  }

  gfProcessingGameEvents = FALSE;

  if (gfEventDeletionPending) {
    DeleteEventsWithDeletionPending();
  }

  if (uiAdjustment && !gfTimeInterrupt) MoveGameTimeForward(uiAdjustment);
}

BOOLEAN AddSameDayStrategicEvent(uint8_t ubCallbackID, uint32_t uiMinStamp, uint32_t uiParam) {
  return (AddStrategicEvent(ubCallbackID, uiMinStamp + GetGameTimeInDays() * 24 * 60, uiParam));
}

BOOLEAN AddSameDayStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiSecondStamp,
                                             uint32_t uiParam) {
  return (AddStrategicEventUsingSeconds(
      ubCallbackID, uiSecondStamp + GetGameTimeInDays() * NUM_SEC_IN_DAY, uiParam));
}

BOOLEAN AddFutureDayStrategicEvent(uint8_t ubCallbackID, uint32_t uiMinStamp, uint32_t uiParam,
                                   uint32_t uiNumDaysFromPresent) {
  uint32_t uiDay;
  uiDay = GetGameTimeInDays();
  return (AddStrategicEvent(ubCallbackID, uiMinStamp + 24 * 60 * (uiDay + uiNumDaysFromPresent),
                            uiParam));
}

BOOLEAN AddFutureDayStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiSecondStamp,
                                               uint32_t uiParam, uint32_t uiNumDaysFromPresent) {
  uint32_t uiDay;
  uiDay = GetGameTimeInDays();
  return (AddStrategicEventUsingSeconds(
      ubCallbackID, uiSecondStamp + 24 * 60 * (uiDay + uiNumDaysFromPresent) * 60, uiParam));
}

STRATEGICEVENT *AddAdvancedStrategicEvent(uint8_t ubEventType, uint8_t ubCallbackID, uint32_t uiTimeStamp,
                                          uint32_t uiParam) {
  STRATEGICEVENT *pNode, *pNewNode, *pPrevNode;

  if (gfProcessingGameEvents &&
      uiTimeStamp <= guiTimeStampOfCurrentlyExecutingEvent) {  // Prevents infinite loops of posting
                                                               // events that are the same time or
                                                               // earlier than the event
// currently being processed.
#ifdef JA2TESTVERSION
    // if (ubCallbackID == EVENT_PROCESS_TACTICAL_SCHEDULE )
    {
      ScreenMsg(FONT_RED, MSG_DEBUG,
                L"%s Event Rejected:  Can't post events <= time while inside an event callback.  "
                L"This is a special case situation that isn't a bug.",
                gEventName[ubCallbackID]);
    }
    // else
    //{
    //	AssertMsg( 0, String( "%S Event Rejected:  Can't post events <= time while inside an event
    // callback.", gEventName[ ubCallbackID ] ) );
    //}
#endif
    return NULL;
  }

  pNewNode = (STRATEGICEVENT *)MemAlloc(sizeof(STRATEGICEVENT));
  Assert(pNewNode);
  memset(pNewNode, 0, sizeof(STRATEGICEVENT));
  pNewNode->ubCallbackID = ubCallbackID;
  pNewNode->uiParam = uiParam;
  pNewNode->ubEventType = ubEventType;
  pNewNode->uiTimeStamp = uiTimeStamp;
  pNewNode->uiTimeOffset = 0;

  // Search list for a place to insert
  pNode = gpEventList;

  // If it's the first head, do this!
  if (!pNode) {
    gpEventList = pNewNode;
    pNewNode->next = NULL;
  } else {
    pPrevNode = NULL;
    while (pNode) {
      if (uiTimeStamp < pNode->uiTimeStamp) {
        break;
      }
      pPrevNode = pNode;
      pNode = pNode->next;
    }

    // If we are at the end, set at the end!
    if (!pNode) {
      pPrevNode->next = pNewNode;
      pNewNode->next = NULL;
    } else {
      // We have a previous node here
      // Insert IN FRONT!
      if (pPrevNode) {
        pNewNode->next = pPrevNode->next;
        pPrevNode->next = pNewNode;
      } else {  // It's the head
        pNewNode->next = gpEventList;
        gpEventList = pNewNode;
      }
    }
  }

  return pNewNode;
}

BOOLEAN AddStrategicEvent(uint8_t ubCallbackID, uint32_t uiMinStamp, uint32_t uiParam) {
  if (AddAdvancedStrategicEvent(ONETIME_EVENT, ubCallbackID, uiMinStamp * 60, uiParam)) return TRUE;
  return FALSE;
}

BOOLEAN AddStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiSecondStamp, uint32_t uiParam) {
  if (AddAdvancedStrategicEvent(ONETIME_EVENT, ubCallbackID, uiSecondStamp, uiParam)) return TRUE;
  return FALSE;
}

BOOLEAN AddRangedStrategicEvent(uint8_t ubCallbackID, uint32_t uiStartMin, uint32_t uiLengthMin,
                                uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(RANGED_EVENT, ubCallbackID, uiStartMin * 60, uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiLengthMin * 60;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN AddSameDayRangedStrategicEvent(uint8_t ubCallbackID, uint32_t uiStartMin, uint32_t uiLengthMin,
                                       uint32_t uiParam) {
  return AddRangedStrategicEvent(ubCallbackID, uiStartMin + GetGameTimeInDays() * 24 * 60,
                                 uiLengthMin, uiParam);
}

BOOLEAN AddFutureDayRangedStrategicEvent(uint8_t ubCallbackID, uint32_t uiStartMin, uint32_t uiLengthMin,
                                         uint32_t uiParam, uint32_t uiNumDaysFromPresent) {
  return AddRangedStrategicEvent(
      ubCallbackID, uiStartMin + 24 * 60 * (GetGameTimeInDays() + uiNumDaysFromPresent),
      uiLengthMin, uiParam);
}

BOOLEAN AddRangedStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiStartSeconds,
                                            uint32_t uiLengthSeconds, uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(RANGED_EVENT, ubCallbackID, uiStartSeconds, uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiLengthSeconds;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN AddSameDayRangedStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiStartSeconds,
                                                   uint32_t uiLengthSeconds, uint32_t uiParam) {
  return AddRangedStrategicEventUsingSeconds(ubCallbackID,
                                             uiStartSeconds + GetGameTimeInDays() * NUM_SEC_IN_DAY,
                                             uiLengthSeconds, uiParam);
}

BOOLEAN AddFutureDayRangedStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiStartSeconds,
                                                     uint32_t uiLengthSeconds, uint32_t uiParam,
                                                     uint32_t uiNumDaysFromPresent) {
  return AddRangedStrategicEventUsingSeconds(
      ubCallbackID, uiStartSeconds + 24 * 60 * (GetGameTimeInDays() + uiNumDaysFromPresent) * 60,
      uiLengthSeconds, uiParam);
}

BOOLEAN AddEveryDayStrategicEvent(uint8_t ubCallbackID, uint32_t uiStartMin, uint32_t uiParam) {
  if (AddAdvancedStrategicEvent(EVERYDAY_EVENT, ubCallbackID,
                                GetGameTimeInDays() * NUM_SEC_IN_DAY + uiStartMin * 60, uiParam))
    return TRUE;
  return FALSE;
}

BOOLEAN AddEveryDayStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiStartSeconds,
                                              uint32_t uiParam) {
  if (AddAdvancedStrategicEvent(EVERYDAY_EVENT, ubCallbackID,
                                GetGameTimeInDays() * NUM_SEC_IN_DAY + uiStartSeconds, uiParam))
    return TRUE;
  return FALSE;
}

// NEW:  Period Events
// Event will get processed automatically once every X minutes.
BOOLEAN AddPeriodStrategicEvent(uint8_t ubCallbackID, uint32_t uiOnceEveryXMinutes, uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(
      PERIODIC_EVENT, ubCallbackID, GetGameTimeInDays() * NUM_SEC_IN_DAY + uiOnceEveryXMinutes * 60,
      uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiOnceEveryXMinutes * 60;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN AddPeriodStrategicEventUsingSeconds(uint8_t ubCallbackID, uint32_t uiOnceEveryXSeconds,
                                            uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(PERIODIC_EVENT, ubCallbackID,
                                     GetGameTimeInDays() * NUM_SEC_IN_DAY + uiOnceEveryXSeconds,
                                     uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiOnceEveryXSeconds;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN AddPeriodStrategicEventWithOffset(uint8_t ubCallbackID, uint32_t uiOnceEveryXMinutes,
                                          uint32_t uiOffsetFromCurrent, uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(
      PERIODIC_EVENT, ubCallbackID, GetGameTimeInDays() * NUM_SEC_IN_DAY + uiOffsetFromCurrent * 60,
      uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiOnceEveryXMinutes * 60;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN AddPeriodStrategicEventUsingSecondsWithOffset(uint8_t ubCallbackID,
                                                      uint32_t uiOnceEveryXSeconds,
                                                      uint32_t uiOffsetFromCurrent, uint32_t uiParam) {
  STRATEGICEVENT *pEvent;
  pEvent = AddAdvancedStrategicEvent(PERIODIC_EVENT, ubCallbackID,
                                     GetGameTimeInDays() * NUM_SEC_IN_DAY + uiOffsetFromCurrent,
                                     uiParam);
  if (pEvent) {
    pEvent->uiTimeOffset = uiOnceEveryXSeconds;
    return TRUE;
  }
  return FALSE;
}

void DeleteAllStrategicEventsOfType(uint8_t ubCallbackID) {
  STRATEGICEVENT *curr, *prev, *temp;
  prev = NULL;
  curr = gpEventList;
  while (curr) {
    if (curr->ubCallbackID == ubCallbackID && !(curr->ubFlags & SEF_DELETION_PENDING)) {
      if (gfPreventDeletionOfAnyEvent) {
        curr->ubFlags |= SEF_DELETION_PENDING;
        gfEventDeletionPending = TRUE;
        prev = curr;
        curr = curr->next;
        continue;
      }
      // Detach the node
      if (prev)
        prev->next = curr->next;
      else
        gpEventList = curr->next;

      // isolate and remove curr
      temp = curr;
      curr = curr->next;
      MemFree(temp);
      // ValidateGameEvents();
    } else {  // Advance all the nodes
      prev = curr;
      curr = curr->next;
    }
  }
}

void DeleteAllStrategicEvents() {
  STRATEGICEVENT *temp;
  while (gpEventList) {
    temp = gpEventList;
    gpEventList = gpEventList->next;
    MemFree(temp);
    // ValidateGameEvents();
    temp = NULL;
  }
  gpEventList = NULL;
}

// Searches for and removes the first event matching the supplied information.  There may very well
// be a need for more specific event removal, so let me know (Kris), of any support needs.  Function
// returns FALSE if no events were found or if the event wasn't deleted due to delete lock,
BOOLEAN DeleteStrategicEvent(uint8_t ubCallbackID, uint32_t uiParam) {
  STRATEGICEVENT *curr, *prev;
  curr = gpEventList;
  prev = NULL;
  while (curr) {  // deleting middle
    if (curr->ubCallbackID == ubCallbackID && curr->uiParam == uiParam) {
      if (!(curr->ubFlags & SEF_DELETION_PENDING)) {
        if (gfPreventDeletionOfAnyEvent) {
          curr->ubFlags |= SEF_DELETION_PENDING;
          gfEventDeletionPending = TRUE;
          return FALSE;
        }
        if (prev) {
          prev->next = curr->next;
        } else {
          gpEventList = gpEventList->next;
        }
        MemFree(curr);
        // ValidateGameEvents();
        return TRUE;
      }
    }
    prev = curr;
    curr = curr->next;
  }
  return FALSE;
}

// part of the game.sav files (not map files)
BOOLEAN SaveStrategicEventsToSavedGame(FileID hFile) {
  uint32_t uiNumBytesWritten = 0;
  STRATEGICEVENT sGameEvent;

  uint32_t uiNumGameEvents = 0;
  STRATEGICEVENT *pTempEvent = gpEventList;

  // Go through the list and determine the number of events
  while (pTempEvent) {
    pTempEvent = pTempEvent->next;
    uiNumGameEvents++;
  }

  // write the number of strategic events
  File_Write(hFile, &uiNumGameEvents, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  // loop through all the events and save them.
  pTempEvent = gpEventList;
  while (pTempEvent) {
    // save the current structure
    memcpy(&sGameEvent, pTempEvent, sizeof(STRATEGICEVENT));

    // write the current strategic event
    File_Write(hFile, &sGameEvent, sizeof(STRATEGICEVENT), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(STRATEGICEVENT)) {
      return (FALSE);
    }

    pTempEvent = pTempEvent->next;
  }

  return (TRUE);
}

BOOLEAN LoadStrategicEventsFromSavedGame(FileID hFile) {
  uint32_t uiNumGameEvents;
  STRATEGICEVENT sGameEvent;
  uint32_t cnt;
  uint32_t uiNumBytesRead = 0;
  STRATEGICEVENT *pTemp = NULL;

  // erase the old Game Event queue
  DeleteAllStrategicEvents();

  // Read the number of strategic events
  File_Read(hFile, &uiNumGameEvents, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  pTemp = NULL;

  // loop through all the events and save them.
  for (cnt = 0; cnt < uiNumGameEvents; cnt++) {
    STRATEGICEVENT *pTempEvent = NULL;

    // allocate memory for the event
    pTempEvent = (STRATEGICEVENT *)MemAlloc(sizeof(STRATEGICEVENT));
    if (pTempEvent == NULL) return (FALSE);

    // Read the current strategic event
    File_Read(hFile, &sGameEvent, sizeof(STRATEGICEVENT), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(STRATEGICEVENT)) {
      return (FALSE);
    }

    memcpy(pTempEvent, &sGameEvent, sizeof(STRATEGICEVENT));

    // Add the new node to the list

    // if its the first node,
    if (cnt == 0) {
      // assign it as the head node
      gpEventList = pTempEvent;

      // assign the 'current node' to the head node
      pTemp = gpEventList;
    } else {
      // add the new node to the next field of the current node
      pTemp->next = pTempEvent;

      // advance the current node to the next node
      pTemp = pTemp->next;
    }

    // NULL out the next field ( cause there is no next field yet )
    pTempEvent->next = NULL;
  }

  return (TRUE);
}

void LockStrategicEventFromDeletion(STRATEGICEVENT *pEvent) {
  pEvent->ubFlags |= SEF_PREVENT_DELETION;
}

void UnlockStrategicEventFromDeletion(STRATEGICEVENT *pEvent) {
  pEvent->ubFlags &= ~SEF_PREVENT_DELETION;
}

void ValidateGameEvents() {
  STRATEGICEVENT *curr;
  curr = gpEventList;
  while (curr) {
    curr = curr->next;
    if (curr == (STRATEGICEVENT *)0xdddddddd) {
      return;
    }
  }
}
