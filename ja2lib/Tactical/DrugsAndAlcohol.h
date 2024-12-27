// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __DRUGS_AND_ALCOHOL_H
#define __DRUGS_AND_ALCOHOL_H

#include "SGP/Types.h"

struct SOLDIERTYPE;
struct OBJECTTYPE;

#define DRUG_TYPE_ADRENALINE 0
#define DRUG_TYPE_ALCOHOL 1
#define NO_DRUG 2
#define NUM_COMPLEX_DRUGS 2
#define DRUG_TYPE_REGENERATION 3

#define SOBER 0
#define FEELING_GOOD 1
#define BORDERLINE 2
#define DRUNK 3
#define HUNGOVER 4

#define REGEN_POINTS_PER_BOOSTER 4
#define LIFE_GAIN_PER_REGEN_POINT 10

uint8_t GetDrugType(uint16_t usItem);
BOOLEAN ApplyDrugs(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject);

void HandleEndTurnDrugAdjustments(struct SOLDIERTYPE *pSoldier);
void HandleAPEffectDueToDrugs(struct SOLDIERTYPE *pSoldier, uint8_t *pubPoints);
void HandleBPEffectDueToDrugs(struct SOLDIERTYPE *pSoldier, int16_t *psPoints);

int8_t GetDrugEffect(struct SOLDIERTYPE *pSoldier, uint8_t ubDrugType);
int8_t GetDrugSideEffect(struct SOLDIERTYPE *pSoldier, uint8_t ubDrugType);
int8_t GetDrunkLevel(struct SOLDIERTYPE *pSoldier);
int32_t EffectStatForBeingDrunk(struct SOLDIERTYPE *pSoldier, int32_t iStat);
BOOLEAN MercUnderTheInfluence(struct SOLDIERTYPE *pSoldier);

#endif
