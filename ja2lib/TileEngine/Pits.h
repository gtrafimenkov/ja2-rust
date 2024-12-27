#ifndef __PITS_H
#define __PITS_H

#include "SGP/Types.h"

void Add3X3Pit(INT32 iMapIndex);
void Add5X5Pit(INT32 iMapIndex);
void Remove3X3Pit(INT32 iMapIndex);
void Remove5X5Pit(INT32 iMapIndex);

void SearchForOtherMembersWithinPitRadiusAndMakeThemFall(int16_t sGridNo, int16_t sRadius);

void AddAllPits();
void RemoveAllPits();

extern BOOLEAN gfShowPits;
extern BOOLEAN gfLoadPitsWithoutArming;

void HandleFallIntoPitFromAnimation(uint8_t ubID);

#endif
