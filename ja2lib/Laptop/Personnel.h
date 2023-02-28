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
INT32 GetFundsOnMerc(struct SOLDIERTYPE *pSoldier);

BOOLEAN TransferFundsFromMercToBank(struct SOLDIERTYPE *pSoldier, INT32 iCurrentBalance);
BOOLEAN TransferFundsFromBankToMerc(struct SOLDIERTYPE *pSoldier, INT32 iCurrentBalance);

BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList(UINT8 ubProfile);

#endif
