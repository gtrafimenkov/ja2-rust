#ifndef _SOLDIER_FUNCTIONS_H
#define _SOLDIER_FUNCTIONS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void ContinueMercMovement(struct SOLDIERTYPE *pSoldier);

BOOLEAN IsValidStance(struct SOLDIERTYPE *pSoldier, INT8 bNewStance);
void SelectMoveAnimationFromStance(struct SOLDIERTYPE *pSoldier);
BOOLEAN IsValidMovementMode(struct SOLDIERTYPE *pSoldier, INT16 usMovementMode);
FLOAT CalcSoldierNextBleed(struct SOLDIERTYPE *pSoldier);
FLOAT CalcSoldierNextUnmovingBleed(struct SOLDIERTYPE *pSoldier);
void SoldierCollapse(struct SOLDIERTYPE *pSoldier);

BOOLEAN ReevaluateEnemyStance(struct SOLDIERTYPE *pSoldier, UINT16 usAnimState);

void HandlePlacingRoofMarker(struct SOLDIERTYPE *pSoldier, INT16 sGridNo, BOOLEAN fSet,
                             BOOLEAN fForce);

void PickPickupAnimation(struct SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo,
                         INT8 bZLevel);

void MercStealFromMerc(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget);

void HandleCrowShadowVisibility(struct SOLDIERTYPE *pSoldier);

#endif
