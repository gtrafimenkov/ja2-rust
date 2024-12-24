#ifndef __MERCS_ACCOUNT_H
#define __MERCS_ACCOUNT_H

#include "SGP/Types.h"

void GameInitMercsAccount();
BOOLEAN EnterMercsAccount();
void ExitMercsAccount();
void HandleMercsAccount();
void RenderMercsAccount();

uint32_t CalculateHowMuchPlayerOwesSpeck();

#endif
