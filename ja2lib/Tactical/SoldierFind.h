#ifndef __SOLDIER_FIND_H
#define __SOLDIER_FIND_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

#define FIND_SOLDIER_FULL 0x000000002
#define FIND_SOLDIER_GRIDNO 0x000000004
#define FIND_SOLDIER_SAMELEVEL 0x000000008
#define FIND_SOLDIER_SELECTIVE 0x000000020
#define FIND_SOLDIER_BEGINSTACK 0x000000040

// RETURN FLAGS FOR FINDSOLDIER
#define SELECTED_MERC 0x000000002
#define OWNED_MERC 0x000000004
#define ENEMY_MERC 0x000000008
#define UNCONSCIOUS_MERC 0x000000020
#define DEAD_MERC 0x000000040
#define VISIBLE_MERC 0x000000080
#define ONDUTY_MERC 0x000000100
#define NOINTERRUPT_MERC 0x000000200
#define NEUTRAL_MERC 0x000000400

#define FINDSOLDIERSAMELEVEL(l) (((FIND_SOLDIER_FULL | FIND_SOLDIER_SAMELEVEL) | (l << 16)))

#define FINDSOLDIERSELECTIVESAMELEVEL(l) \
  (((FIND_SOLDIER_SELECTIVE | FIND_SOLDIER_SAMELEVEL) | (l << 16)))

BOOLEAN FindSoldierFromMouse(uint16_t *pusSoldierIndex, uint32_t *pMercFlags);
BOOLEAN SelectiveFindSoldierFromMouse(uint16_t *pusSoldierIndex, uint32_t *pMercFlags);
BOOLEAN FindSoldier(INT16 sGridNo, uint16_t *pusSoldierIndex, uint32_t *pMercFlags,
                    uint32_t uiFlags);
struct SOLDIERTYPE *SimpleFindSoldier(INT16 sGridNo, INT8 bLevel);

BOOLEAN CycleSoldierFindStack(uint16_t usMapPos);

BOOLEAN GridNoOnScreen(INT16 sGridNo);

BOOLEAN SoldierOnScreen(uint16_t usID);
BOOLEAN SoldierLocationRelativeToScreen(INT16 sGridNo, uint16_t usReasonID, INT8 *pbDirection,
                                        uint32_t *puiScrollFlags);
void GetSoldierScreenPos(struct SOLDIERTYPE *pSoldier, INT16 *psScreenX, INT16 *psScreenY);
void GetSoldierAnimDims(struct SOLDIERTYPE *pSoldier, INT16 *psHeight, INT16 *psWidth);
void GetSoldierAnimOffsets(struct SOLDIERTYPE *pSoldier, INT16 *sOffsetX, INT16 *sOffsetY);
void GetSoldierTRUEScreenPos(struct SOLDIERTYPE *pSoldier, INT16 *psScreenX, INT16 *psScreenY);
BOOLEAN IsPointInSoldierBoundingBox(struct SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY);
BOOLEAN FindRelativeSoldierPosition(struct SOLDIERTYPE *pSoldier, uint16_t *usFlags, INT16 sX,
                                    INT16 sY);

uint8_t QuickFindSoldier(INT16 sGridNo);
void GetGridNoScreenPos(INT16 sGridNo, uint8_t ubLevel, INT16 *psScreenX, INT16 *psScreenY);

#endif
