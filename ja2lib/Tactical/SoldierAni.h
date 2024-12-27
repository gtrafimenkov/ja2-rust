#ifndef __SOLDIER_ANI_H
#define __SOLDIER_ANI_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

BOOLEAN AdjustToNextAnimationFrame(struct SOLDIERTYPE *pSoldier);

BOOLEAN CheckForAndHandleSoldierDeath(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfMadeCorpse);

BOOLEAN CheckForAndHandleSoldierDyingNotFromHit(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleSoldierDeath(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfMadeCorpse);

BOOLEAN OKFallDirection(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bLevel,
                        int8_t bTestDirection, uint16_t usAnimState);

BOOLEAN HandleCheckForDeathCommonCode(struct SOLDIERTYPE *pSoldier);

void KickOutWheelchair(struct SOLDIERTYPE *pSoldier);

#endif
