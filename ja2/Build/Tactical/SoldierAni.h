#ifndef __SOLDIER_ANI_H
#define __SOLDIER_ANI_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

BOOLEAN AdjustToNextAnimationFrame(struct SOLDIERTYPE *pSoldier);

BOOLEAN CheckForAndHandleSoldierDeath(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfMadeCorpse);

BOOLEAN CheckForAndHandleSoldierDyingNotFromHit(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleSoldierDeath(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfMadeCorpse);

BOOLEAN OKFallDirection(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bLevel,
                        INT8 bTestDirection, UINT16 usAnimState);

BOOLEAN HandleCheckForDeathCommonCode(struct SOLDIERTYPE *pSoldier);

void KickOutWheelchair(struct SOLDIERTYPE *pSoldier);

#endif
