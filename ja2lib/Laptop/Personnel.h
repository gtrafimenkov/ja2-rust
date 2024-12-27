// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __PERSONNEL_H
#define __PERSONNEL_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// delay for change in ATM mode
#define DELAY_PER_MODE_CHANGE_IN_ATM 2000

void GameInitPersonnel();
void EnterPersonnel();
void ExitPersonnel();
void HandlePersonnel();
void RenderPersonnel();

// add character to:

// leaving for odd reasons
void AddCharacterToOtherList(struct SOLDIERTYPE *pSoldier);

// killed and removed
void AddCharacterToDeadList(struct SOLDIERTYPE *pSoldier);

// simply fired...but alive
void AddCharacterToFiredList(struct SOLDIERTYPE *pSoldier);

// get the total amt of money on this guy
int32_t GetFundsOnMerc(struct SOLDIERTYPE *pSoldier);

BOOLEAN TransferFundsFromMercToBank(struct SOLDIERTYPE *pSoldier, int32_t iCurrentBalance);
BOOLEAN TransferFundsFromBankToMerc(struct SOLDIERTYPE *pSoldier, int32_t iCurrentBalance);

BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList(uint8_t ubProfile);

#endif
