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
BOOLEAN FindSoldier(int16_t sGridNo, uint16_t *pusSoldierIndex, uint32_t *pMercFlags,
                    uint32_t uiFlags);
struct SOLDIERTYPE *SimpleFindSoldier(int16_t sGridNo, int8_t bLevel);

BOOLEAN CycleSoldierFindStack(uint16_t usMapPos);

BOOLEAN GridNoOnScreen(int16_t sGridNo);

BOOLEAN SoldierOnScreen(uint16_t usID);
BOOLEAN SoldierLocationRelativeToScreen(int16_t sGridNo, uint16_t usReasonID, int8_t *pbDirection,
                                        uint32_t *puiScrollFlags);
void GetSoldierScreenPos(struct SOLDIERTYPE *pSoldier, int16_t *psScreenX, int16_t *psScreenY);
void GetSoldierAnimDims(struct SOLDIERTYPE *pSoldier, int16_t *psHeight, int16_t *psWidth);
void GetSoldierAnimOffsets(struct SOLDIERTYPE *pSoldier, int16_t *sOffsetX, int16_t *sOffsetY);
void GetSoldierTRUEScreenPos(struct SOLDIERTYPE *pSoldier, int16_t *psScreenX, int16_t *psScreenY);
BOOLEAN IsPointInSoldierBoundingBox(struct SOLDIERTYPE *pSoldier, int16_t sX, int16_t sY);
BOOLEAN FindRelativeSoldierPosition(struct SOLDIERTYPE *pSoldier, uint16_t *usFlags, int16_t sX,
                                    int16_t sY);

uint8_t QuickFindSoldier(int16_t sGridNo);
void GetGridNoScreenPos(int16_t sGridNo, uint8_t ubLevel, int16_t *psScreenX, int16_t *psScreenY);

#endif
