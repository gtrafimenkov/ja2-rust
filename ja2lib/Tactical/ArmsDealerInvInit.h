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
  INT16 sItemIndex;
  UINT8 ubOptimalNumber;

} DEALER_POSSIBLE_INV;

typedef struct {
  uint32_t uiItemClass;
  UINT8 ubWeaponClass;
  BOOLEAN fAllowUsed;
} ITEM_SORT_ENTRY;

INT8 GetDealersMaxItemAmount(UINT8 ubDealerID, uint16_t usItemIndex);

DEALER_POSSIBLE_INV *GetPointerToDealersPossibleInventory(UINT8 ubArmsDealerID);

UINT8 GetCurrentSuitabilityForItem(INT8 bArmsDealer, uint16_t usItemIndex);
UINT8 ChanceOfItemTransaction(INT8 bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerSelling,
                              BOOLEAN fUsed);
BOOLEAN ItemTransactionOccurs(INT8 bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerSelling,
                              BOOLEAN fUsed);
UINT8 DetermineInitialInvItems(INT8 bArmsDealerID, uint16_t usItemIndex, UINT8 ubChances,
                               BOOLEAN fUsed);
UINT8 HowManyItemsAreSold(INT8 bArmsDealerID, uint16_t usItemIndex, UINT8 ubNumInStock,
                          BOOLEAN fUsed);
UINT8 HowManyItemsToReorder(UINT8 ubWanted, UINT8 ubStillHave);

int BobbyRayItemQsortCompare(const void *pArg1, const void *pArg2);
int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2);
int RepairmanItemQsortCompare(const void *pArg1, const void *pArg2);
int CompareItemsForSorting(uint16_t usItem1Index, uint16_t usItem2Index, UINT8 ubItem1Quality,
                           UINT8 ubItem2Quality);
UINT8 GetDealerItemCategoryNumber(uint16_t usItemIndex);
BOOLEAN CanDealerItemBeSoldUsed(uint16_t usItemIndex);

#endif
