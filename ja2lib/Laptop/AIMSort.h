#ifndef __AIMSORT_H_
#define __AIMSORT_H_

#include "SGP/Types.h"

extern uint8_t gubCurrentSortMode;
extern uint8_t gubCurrentListMode;
extern uint8_t gbCurrentIndex;

#define AIM_ASCEND 6
#define AIM_DESCEND 7

void GameInitAimSort();
BOOLEAN EnterAimSort();
void ExitAimSort();
void HandleAimSort();
void RenderAimSort();

#endif
