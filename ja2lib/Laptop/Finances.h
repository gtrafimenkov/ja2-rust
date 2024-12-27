#ifndef __FINANCES_H
#define __FINANCES_H

#include "SGP/Types.h"

void GameInitFinances();
void EnterFinances();
void ExitFinances();
void HandleFinances();
void RenderFinances();

#define FINANCES_DATA_FILE "TEMP\\finances.dat"

// the financial structure
struct finance {
  uint8_t ubCode;        // the code index in the finance code table
  uint32_t uiIdNumber;   // unique id number
  uint8_t ubSecondCode;  // secondary code
  uint32_t uiDate;       // time in the world in global time
  int32_t iAmount;       // the amount of the transaction
  int32_t iBalanceToDate;
  struct finance *Next;  // next unit in the list
};

enum {
  ACCRUED_INTEREST,
  ANONYMOUS_DEPOSIT,
  TRANSACTION_FEE,
  HIRED_MERC,
  BOBBYR_PURCHASE,
  PAY_SPECK_FOR_MERC,
  MEDICAL_DEPOSIT,
  IMP_PROFILE,
  PURCHASED_INSURANCE,
  REDUCED_INSURANCE,
  EXTENDED_INSURANCE,
  CANCELLED_INSURANCE,
  INSURANCE_PAYOUT,
  EXTENDED_CONTRACT_BY_1_DAY,
  EXTENDED_CONTRACT_BY_1_WEEK,
  EXTENDED_CONTRACT_BY_2_WEEKS,
  DEPOSIT_FROM_GOLD_MINE,
  DEPOSIT_FROM_SILVER_MINE,
  PURCHASED_FLOWERS,
  FULL_MEDICAL_REFUND,
  PARTIAL_MEDICAL_REFUND,
  NO_MEDICAL_REFUND,
  PAYMENT_TO_NPC,
  TRANSFER_FUNDS_TO_MERC,
  TRANSFER_FUNDS_FROM_MERC,
  TRAIN_TOWN_MILITIA,
  PURCHASED_ITEM_FROM_DEALER,
  MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT,

};

typedef struct finance FinanceUnit;
typedef struct finance *FinanceUnitPtr;

extern FinanceUnitPtr pFinanceListHead;
uint32_t AddTransactionToPlayersBook(uint8_t ubCode, uint8_t ubSecondCode, int32_t iAmount);
void InsertDollarSignInToString(STR16 pString);
void InsertCommasForDollarFigure(STR16 pString);
int32_t GetTodaysDaysIncome(void);
int32_t GetProjectedTotalDailyIncome(void);

#endif
