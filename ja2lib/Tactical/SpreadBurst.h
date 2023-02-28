#ifndef _SPREAD_BURST_H
#define _SPREAD_BURST_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void ResetBurstLocations();
void AccumulateBurstLocation(INT16 sGridNo);
void PickBurstLocations(struct SOLDIERTYPE *pSoldier);
void AIPickBurstLocations(struct SOLDIERTYPE *pSoldier, INT8 bTargets,
                          struct SOLDIERTYPE *pTargets[5]);

void RenderAccumulatedBurstLocations();

#endif
