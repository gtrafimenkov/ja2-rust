#ifndef _DISPLAY_COVER__H_
#define _DISPLAY_COVER__H_

#include "SGP/Types.h"

struct SOLDIERTYPE;

void DisplayCoverOfSelectedGridNo();
void RemoveCoverOfSelectedGridNo();

void DisplayRangeToTarget(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo);

void RemoveVisibleGridNoAtSelectedGridNo();
void DisplayGridNoVisibleToSoldierGrid();

#ifdef JA2TESTVERSION
void DisplayLosAndDisplayCoverUsageScreenMsg();
#endif

void ChangeSizeOfDisplayCover(int32_t iNewSize);

void ChangeSizeOfLOS(int32_t iNewSize);

#endif
