#ifndef _STRATEGIC_MERC_HANDLER_H_
#define _STRATEGIC_MERC_HANDLER_H_

#include "SGP/Types.h"
#include "Tactical/SoldierControl.h"

void StrategicHandlePlayerTeamMercDeath(SOLDIERTYPE *pSoldier);
void MercDailyUpdate();
void MercsContractIsFinished(UINT8 ubID);
void RPCWhineAboutNoPay(UINT8 ubID);
void MercComplainAboutEquipment(UINT8 ubProfileID);
BOOLEAN SoldierHasWorseEquipmentThanUsedTo(SOLDIERTYPE *pSoldier);
void UpdateBuddyAndHatedCounters(void);
void HourlyCamouflageUpdate(void);
#endif
