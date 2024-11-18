#ifndef __STORE_INVENTORY_H_
#define __STORE_INVENTORY_H_

#include "SGP/Types.h"
#include "Tactical/Items.h"

typedef struct {
  uint16_t usItemIndex;  // Index into the item table
  uint8_t ubQtyOnHand;
  uint8_t ubQtyOnOrder;           // The number of items on order
  uint8_t ubItemQuality;          // the % damaged listed from 0 to 100
  BOOLEAN fPreviouslyEligible;  // whether or not dealer has been eligible to sell this item in days
                                // prior to today

  uint8_t filler;

} STORE_INVENTORY;

// Enums used for the access the MAX dealers array
enum {
  BOBBY_RAY_NEW,
  BOBBY_RAY_USED,

  BOBBY_RAY_LISTS,
};

extern uint8_t StoreInventory[MAXITEMS][BOBBY_RAY_LISTS];
extern int16_t WeaponROF[MAX_WEAPONS];

void SetupStoreInventory(STORE_INVENTORY *pInventoryArray, BOOLEAN fUsed);
BOOLEAN DoesGunOfSameClassExistInInventory(uint8_t ubItemIndex, uint8_t ubDealerID);
STORE_INVENTORY *GetPtrToStoreInventory(uint8_t ubDealerID);
// int16_t	CountNumberOfItemsInStoreInventory( uint8_t ubArmsDealerID );

#endif
