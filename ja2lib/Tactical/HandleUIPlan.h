// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __HANDLEUIPLAN_H
#define __HANDLEUIPLAN_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define UIPLAN_ACTION_MOVETO 1
#define UIPLAN_ACTION_FIRE 2

BOOLEAN BeginUIPlan(struct SOLDIERTYPE *pSoldier);
BOOLEAN AddUIPlan(uint16_t sGridNo, uint8_t ubPlanID);
void EndUIPlan();
BOOLEAN InUIPlanMode();

#endif
