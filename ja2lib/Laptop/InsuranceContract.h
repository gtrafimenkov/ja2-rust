#ifndef __INSURANCE_CONTRACT_H
#define __INSURANCE_CONTRACT_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void GameInitInsuranceContract();
BOOLEAN EnterInsuranceContract();
void ExitInsuranceContract();
void HandleInsuranceContract();
void RenderInsuranceContract();

extern INT16 gsCurrentInsuranceMercIndex;

// determines if a merc will run out of there insurance contract
void DailyUpdateOfInsuredMercs();

// void InsuranceContractPayLifeInsuranceForDeadMerc( LIFE_INSURANCE_PAYOUT *pPayoutStruct );

BOOLEAN AddLifeInsurancePayout(struct SOLDIERTYPE *pSoldier);
void InsuranceContractPayLifeInsuranceForDeadMerc(uint8_t ubPayoutID);
void StartInsuranceInvestigation(uint8_t ubPayoutID);
void EndInsuranceInvestigation(uint8_t ubPayoutID);

INT32 CalculateInsuranceContractCost(INT32 iLength, uint8_t ubMercID);

void InsuranceContractEndGameShutDown();

void PurchaseOrExtendInsuranceForSoldier(struct SOLDIERTYPE *pSoldier, uint32_t uiInsuranceLength);

#endif
