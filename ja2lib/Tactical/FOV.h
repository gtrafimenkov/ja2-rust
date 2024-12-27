// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __FOV_H
#define __FOV_H

#include "SGP/Types.h"
#include "Tactical/Overhead.h"

void RevealRoofsAndItems(struct SOLDIERTYPE *pSoldier, uint32_t itemsToo, BOOLEAN fShowLocators,
                         uint8_t ubLevel, BOOLEAN fForce);

int32_t GetFreeSlantRoof(void);
void RecountSlantRoofs(void);
void ClearSlantRoofs(void);
BOOLEAN FindSlantRoofSlot(int16_t sGridNo);
void AddSlantRoofFOVSlot(int16_t sGridNo);
void ExamineSlantRoofFOVSlots();

#endif
