#ifndef __FOV_H
#define __FOV_H

#include "SGP/Types.h"
#include "Tactical/Overhead.h"

void RevealRoofsAndItems(struct SOLDIERTYPE *pSoldier, uint32_t itemsToo, BOOLEAN fShowLocators,
                         uint8_t ubLevel, BOOLEAN fForce);

INT32 GetFreeSlantRoof(void);
void RecountSlantRoofs(void);
void ClearSlantRoofs(void);
BOOLEAN FindSlantRoofSlot(INT16 sGridNo);
void AddSlantRoofFOVSlot(INT16 sGridNo);
void ExamineSlantRoofFOVSlots();

#endif
