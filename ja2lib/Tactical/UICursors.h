#ifndef __UI_CURSORS_H
#define __UI_CURSORS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define REFINE_PUNCH_1 0
#define REFINE_PUNCH_2 6

#define REFINE_KNIFE_1 0
#define REFINE_KNIFE_2 6

uint8_t GetProperItemCursor(uint8_t ubSoldierID, uint16_t ubItemIndex, uint16_t usMapPos,
                            BOOLEAN fActivated);
void DetermineCursorBodyLocation(uint8_t ubSoldierID, BOOLEAN fDisplay, BOOLEAN fRecalc);

void HandleLeftClickCursor(struct SOLDIERTYPE *pSoldier);
void HandleRightClickAdjustCursor(struct SOLDIERTYPE *pSoldier, int16_t usMapPos);

uint8_t GetActionModeCursor(struct SOLDIERTYPE *pSoldier);

extern BOOLEAN gfCannotGetThrough;

void HandleUICursorRTFeedback(struct SOLDIERTYPE *pSoldier);
void HandleEndConfirmCursor(struct SOLDIERTYPE *pSoldier);

BOOLEAN GetMouseRecalcAndShowAPFlags(uint32_t *puiCursorFlags, BOOLEAN *pfShowAPs);

#endif
