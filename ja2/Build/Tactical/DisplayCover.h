#ifndef _DISPLAY_COVER__H_
#define _DISPLAY_COVER__H_

#include "SGP/Types.h"
#include "Tactical/SoldierControl.h"

void DisplayCoverOfSelectedGridNo();
void RemoveCoverOfSelectedGridNo();

void DisplayRangeToTarget(SOLDIERTYPE *pSoldier, INT16 sTargetGridNo);

void RemoveVisibleGridNoAtSelectedGridNo();
void DisplayGridNoVisibleToSoldierGrid();

#ifdef JA2TESTVERSION
void DisplayLosAndDisplayCoverUsageScreenMsg();
#endif

void ChangeSizeOfDisplayCover(INT32 iNewSize);

void ChangeSizeOfLOS(INT32 iNewSize);

#endif
