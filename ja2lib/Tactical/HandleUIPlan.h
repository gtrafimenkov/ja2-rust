#ifndef __HANDLEUIPLAN_H
#define __HANDLEUIPLAN_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define UIPLAN_ACTION_MOVETO 1
#define UIPLAN_ACTION_FIRE 2

BOOLEAN BeginUIPlan(struct SOLDIERTYPE *pSoldier);
BOOLEAN AddUIPlan(UINT16 sGridNo, UINT8 ubPlanID);
void EndUIPlan();
BOOLEAN InUIPlanMode();

#endif
