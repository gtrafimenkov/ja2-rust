#ifndef __SCHEDULING_H
#define __SCHEDULING_H

#include "BuildDefines.h"
#include "SGP/Types.h"
#include "Tactical/SoldierInitList.h"

// Merc scheduling actions
// NOTE:  Any modifications to this enumeration also require updating the text in EditorMercs.c used
//			 in the editor for merc schedule purposes.
enum {
  SCHEDULE_ACTION_NONE,
  SCHEDULE_ACTION_LOCKDOOR,
  SCHEDULE_ACTION_UNLOCKDOOR,
  SCHEDULE_ACTION_OPENDOOR,
  SCHEDULE_ACTION_CLOSEDOOR,
  SCHEDULE_ACTION_GRIDNO,
  SCHEDULE_ACTION_LEAVESECTOR,
  SCHEDULE_ACTION_ENTERSECTOR,
  SCHEDULE_ACTION_STAYINSECTOR,
  SCHEDULE_ACTION_SLEEP,
  SCHEDULE_ACTION_WAKE,
  NUM_SCHEDULE_ACTIONS
};

#define SCHEDULE_FLAGS_VARIANCE1 0x0001
#define SCHEDULE_FLAGS_VARIANCE2 0x0002
#define SCHEDULE_FLAGS_VARIANCE3 0x0004
#define SCHEDULE_FLAGS_VARIANCE4 0x0008
#define SCHEDULE_FLAGS_ACTIVE1 0x0010
#define SCHEDULE_FLAGS_ACTIVE2 0x0020
#define SCHEDULE_FLAGS_ACTIVE3 0x0040
#define SCHEDULE_FLAGS_ACTIVE4 0x0080
#define SCHEDULE_FLAGS_TEMPORARY 0x0100        // for default schedules -- not saved.
#define SCHEDULE_FLAGS_SLEEP_CONVERTED 0x0200  // converted (needs to be uncoverted before saving)
#define SCHEDULE_FLAGS_NPC_SLEEPING 0x0400  // if processing a sleep command, this flag will be set.

// combo flag for turning active flags off
#define SCHEDULE_FLAGS_ACTIVE_ALL 0x00F0

#define MAX_SCHEDULE_ACTIONS 4

typedef struct SCHEDULENODE {
  struct SCHEDULENODE *next;
  UINT16 usTime[MAX_SCHEDULE_ACTIONS];   // converted to minutes 12:30PM would be 12*60 + 30 = 750
  UINT16 usData1[MAX_SCHEDULE_ACTIONS];  // typically the gridno, but depends on the action
  UINT16 usData2[MAX_SCHEDULE_ACTIONS];  // secondary information, not used by most actions
  UINT8 ubAction[MAX_SCHEDULE_ACTIONS];
  UINT8 ubScheduleID;
  UINT8 ubSoldierID;
  UINT16 usFlags;
} SCHEDULENODE;

extern UINT8 gubScheduleID;
extern SCHEDULENODE *gpScheduleList;

// Access functions
SCHEDULENODE *GetSchedule(UINT8 ubScheduleID);

// Removes all schedules from the event list, and cleans out the list.
void DestroyAllSchedules();
void DestroyAllSchedulesWithoutDestroyingEvents();

// This is the callback whenever a schedule is processed
void ProcessTacticalSchedule(UINT8 ubScheduleID);

void DeleteSchedule(UINT8 ubScheduleID);

void LoadSchedules(INT8 **hBuffer);
BOOLEAN LoadSchedulesFromSave(FileID hFile);
BOOLEAN SaveSchedules(FileID hFile);

void PostSchedule(struct SOLDIERTYPE *pSoldier);
void PostDefaultSchedule(struct SOLDIERTYPE *pSoldier);
void PostNextSchedule(struct SOLDIERTYPE *pSoldier);

// After the world is loaded and the temp modifications have been applied,
// we then need to post the events and process schedules for the time that we have been gone.
void PostSchedules();

// Sorts the schedule in chronological order.  Returns TRUE if any sorting took place.
BOOLEAN SortSchedule(SCHEDULENODE *pSchedule);
// Adds a schedule to the list.  COPIES THE DATA OVER (ALLOCATES NEW NODE!)
void CopyScheduleToList(SCHEDULENODE *pSchedule, SOLDIERINITNODE *pNode);
// Entering the editor automatically removes all events posted.
void PrepareSchedulesForEditorEntry();
// Leaving the editor and entering the game posts the events.
void PrepareSchedulesForEditorExit();
// Packs all of the scheduleIDs, and updates the links.  This is done whenever necessary and
// before saving the map, as this forces the IDs to align with the SOLDIERINITNODE->ubScheduleID's.
void OptimizeSchedules();

void PerformActionOnDoorAdjacentToGridNo(UINT8 ubScheduleAction, UINT16 usMapIndex);

BOOLEAN ExtractScheduleEntryAndExitInfo(struct SOLDIERTYPE *pSoldier, UINT32 *puiEntryTime,
                                        UINT32 *puiExitTime);
BOOLEAN ExtractScheduleDoorLockAndUnlockInfo(struct SOLDIERTYPE *pSoldier, UINT32 *puiOpeningTime,
                                             UINT32 *puiClosingTime);

void ReconnectSchedules(void);

void SecureSleepSpot(struct SOLDIERTYPE *pSoldier, UINT16 usSleepSpot);

BOOLEAN BumpAnyExistingMerc(INT16 sGridNo);

#endif
