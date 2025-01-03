// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SOLDIER_FUNCTIONS_H
#define _SOLDIER_FUNCTIONS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void ContinueMercMovement(struct SOLDIERTYPE *pSoldier);

BOOLEAN IsValidStance(struct SOLDIERTYPE *pSoldier, int8_t bNewStance);
void SelectMoveAnimationFromStance(struct SOLDIERTYPE *pSoldier);
BOOLEAN IsValidMovementMode(struct SOLDIERTYPE *pSoldier, int16_t usMovementMode);
float CalcSoldierNextBleed(struct SOLDIERTYPE *pSoldier);
float CalcSoldierNextUnmovingBleed(struct SOLDIERTYPE *pSoldier);
void SoldierCollapse(struct SOLDIERTYPE *pSoldier);

BOOLEAN ReevaluateEnemyStance(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState);

void HandlePlacingRoofMarker(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, BOOLEAN fSet,
                             BOOLEAN fForce);

void PickPickupAnimation(struct SOLDIERTYPE *pSoldier, int32_t iItemIndex, int16_t sGridNo,
                         int8_t bZLevel);

void MercStealFromMerc(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget);

void HandleCrowShadowVisibility(struct SOLDIERTYPE *pSoldier);

#endif
