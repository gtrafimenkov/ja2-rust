#ifndef _ARMS_DEALER_INV_INIT__H_
#define _ARMS_DEALER_INV_INIT__H_

#include "SGP/Types.h"

#define LAST_DEALER_ITEM -1
#define NO_DEALER_ITEM 0

// item suitability categories for dealer inventory initialization, virtual customer sales, and
// re-ordering
#define ITEM_SUITABILITY_NONE 0
#define ITEM_SUITABILITY_LOW 1
#define ITEM_SUITABILITY_MEDIUM 2
#define ITEM_SUITABILITY_HIGH 3
#define ITEM_SUITABILITY_ALWAYS 4

#define DEALER_BUYING 0
#define DEALER_SELLING 1

typedef struct {
  int16_t sItemIndex;
  uint8_t ubOptimalNumber;

} DEALER_POSSIBLE_INV;

typedef struct {
  uint32_t uiItemClass;
  uint8_t ubWeaponClass;
  BOOLEAN fAllowUsed;
} ITEM_SORT_ENTRY;

int8_t GetDealersMaxItemAmount(uint8_t ubDealerID, uint16_t usItemIndex);

DEALER_POSSIBLE_INV *GetPointerToDealersPossibleInventory(uint8_t ubArmsDealerID);

uint8_t GetCurrentSuitabilityForItem(int8_t bArmsDealer, uint16_t usItemIndex);
uint8_t ChanceOfItemTransaction(int8_t bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerSelling,
                                BOOLEAN fUsed);
BOOLEAN ItemTransactionOccurs(int8_t bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerSelling,
                              BOOLEAN fUsed);
uint8_t DetermineInitialInvItems(int8_t bArmsDealerID, uint16_t usItemIndex, uint8_t ubChances,
                                 BOOLEAN fUsed);
uint8_t HowManyItemsAreSold(int8_t bArmsDealerID, uint16_t usItemIndex, uint8_t ubNumInStock,
                            BOOLEAN fUsed);
uint8_t HowManyItemsToReorder(uint8_t ubWanted, uint8_t ubStillHave);

int BobbyRayItemQsortCompare(const void *pArg1, const void *pArg2);
int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2);
int RepairmanItemQsortCompare(const void *pArg1, const void *pArg2);
int CompareItemsForSorting(uint16_t usItem1Index, uint16_t usItem2Index, uint8_t ubItem1Quality,
                           uint8_t ubItem2Quality);
uint8_t GetDealerItemCategoryNumber(uint16_t usItemIndex);
BOOLEAN CanDealerItemBeSoldUsed(uint16_t usItemIndex);

#endif
