// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __WORLD_ITEMS
#define __WORLD_ITEMS

#include "Tactical/Items.h"

#define WORLD_ITEM_DONTRENDER 0x0001
#define WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO 0x0002
#define WORLD_ITEM_ARMED_BOMB 0x0040
#define WORLD_ITEM_SCIFI_ONLY 0x0080
#define WORLD_ITEM_REALISTIC_ONLY 0x0100
#define WORLD_ITEM_REACHABLE 0x0200
#define WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT 0x0400

typedef struct {
  BOOLEAN fExists;
  int16_t sGridNo;
  uint8_t ubLevel;
  struct OBJECTTYPE o;
  uint16_t usFlags;
  int8_t bRenderZHeightAboveLevel;

  int8_t bVisible;

  // This is the chance associated with an item or a trap not-existing in the world.  The reason why
  // this is reversed (10 meaning item has 90% chance of appearing, is because the order that the
  // map is saved, we don't know if the version is older or not until after the items are loaded and
  // added. Because this value is zero in the saved maps, we can't change it to 100, hence the
  // reversal method. This check is only performed the first time a map is loaded.  Later, it is
  // entirely skipped.
  uint8_t ubNonExistChance;

} WORLDITEM;

extern WORLDITEM *gWorldItems;
extern uint32_t guiNumWorldItems;

int32_t AddItemToWorld(int16_t sGridNo, struct OBJECTTYPE *pObject, uint8_t ubLevel, uint16_t usFlags,
                     int8_t bRenderZHeightAboveLevel, int8_t bVisible);
void RemoveItemFromWorld(int32_t iItemIndex);
int32_t FindWorldItem(uint16_t usItem);

void LoadWorldItemsFromMap(int8_t **hBuffer);
void SaveWorldItemsToMap(FileID fp);

void TrashWorldItems();

typedef struct {
  BOOLEAN fExists;
  int32_t iItemIndex;
} WORLDBOMB;

extern WORLDBOMB *gWorldBombs;
extern uint32_t guiNumWorldBombs;

extern int32_t AddBombToWorld(int32_t iItemIndex);
extern void FindPanicBombsAndTriggers(void);
extern int32_t FindWorldItemForBombInGridNo(int16_t sGridNo, int8_t bLevel);

void RefreshWorldItemsIntoItemPools(WORLDITEM *pItemList, int32_t iNumberOfItems);

#endif
