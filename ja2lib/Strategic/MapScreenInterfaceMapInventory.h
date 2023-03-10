#ifndef _MAP_INTERFACE_MAP_INVEN_H
#define _MAP_INTERFACE_MAP_INVEN_H

#include "SGP/Types.h"
#include "Tactical/WorldItems.h"

// this is how close one has to be in the loaded sector to pickup an item
#define MAX_DISTANCE_TO_PICKUP_ITEM 5

// number of inventory slots
#define MAP_INVENTORY_POOL_SLOT_COUNT 45

// whether we are showing the inventory pool graphic
extern BOOLEAN fShowMapInventoryPool;

// load inventory pool graphic
BOOLEAN LoadInventoryPoolGraphic(void);

// remove inventory pool graphic
void RemoveInventoryPoolGraphic(void);

// blit the inventory graphic
void BlitInventoryPoolGraphic(void);

// which buttons in map invneotyr panel?
void HandleButtonStatesWhileMapInventoryActive(void);

// handle creation and destruction of map inventory pool buttons
void CreateDestroyMapInventoryPoolButtons(BOOLEAN fExitFromMapScreen);

// bail out of sector inventory mode if it is on
void CancelSectorInventoryDisplayIfOn(BOOLEAN fExitFromMapScreen);

INT32 GetSizeOfStashInSector(u8 sMapX, u8 sMapY, i8 sMapZ, BOOLEAN fCountStacksAsOne);

// get total number of items in sector
INT32 GetTotalNumberOfItems(void);

// handle flash of inventory items
void HandleFlashForHighLightedItem(void);

// the list for the inventory
extern WORLDITEM *pInventoryPoolList;

// autoplace down object
BOOLEAN AutoPlaceObjectInInventoryStash(struct OBJECTTYPE *pItemPtr);

// the current inventory item
extern INT32 iCurrentlyHighLightedItem;
extern BOOLEAN fFlashHighLightInventoryItemOnradarMap;
extern INT16 sObjectSourceGridNo;
extern WORLDITEM *pInventoryPoolList;
extern INT32 iCurrentInventoryPoolPage;
extern BOOLEAN fMapInventoryItemCompatable[];

BOOLEAN IsMapScreenWorldItemInvisibleInMapInventory(WORLDITEM *pWorldItem);
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(WORLDITEM *pWorldItem);

#endif
