// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __INSURANCE_CONTRACT_H
#define __INSURANCE_CONTRACT_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

void GameInitInsuranceContract();
BOOLEAN EnterInsuranceContract();
void ExitInsuranceContract();
void HandleInsuranceContract();
void RenderInsuranceContract();

extern int16_t gsCurrentInsuranceMercIndex;

// determines if a merc will run out of there insurance contract
void DailyUpdateOfInsuredMercs();

// void InsuranceContractPayLifeInsuranceForDeadMerc( LIFE_INSURANCE_PAYOUT *pPayoutStruct );

BOOLEAN AddLifeInsurancePayout(struct SOLDIERTYPE *pSoldier);
void InsuranceContractPayLifeInsuranceForDeadMerc(uint8_t ubPayoutID);
void StartInsuranceInvestigation(uint8_t ubPayoutID);
void EndInsuranceInvestigation(uint8_t ubPayoutID);

int32_t CalculateInsuranceContractCost(int32_t iLength, uint8_t ubMercID);

void InsuranceContractEndGameShutDown();

void PurchaseOrExtendInsuranceForSoldier(struct SOLDIERTYPE *pSoldier, uint32_t uiInsuranceLength);

#endif
