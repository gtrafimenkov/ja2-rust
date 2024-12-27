#ifndef _SPREAD_BURST_H
#define _SPREAD_BURST_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void ResetBurstLocations();
void AccumulateBurstLocation(int16_t sGridNo);
void PickBurstLocations(struct SOLDIERTYPE *pSoldier);
void AIPickBurstLocations(struct SOLDIERTYPE *pSoldier, int8_t bTargets,
                          struct SOLDIERTYPE *pTargets[5]);

void RenderAccumulatedBurstLocations();

#endif
