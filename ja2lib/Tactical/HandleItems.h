#ifndef __HANDLE_ITEMS_H
#define __HANDLE_ITEMS_H

#include "SGP/Types.h"
#include "Tactical/WorldItems.h"

struct SOLDIERTYPE;
struct STRUCTURE;

#define ITEM_HANDLE_OK 1
#define ITEM_HANDLE_RELOADING -1
#define ITEM_HANDLE_UNCONSCIOUS -2
#define ITEM_HANDLE_NOAPS -3
#define ITEM_HANDLE_NOAMMO -4
#define ITEM_HANDLE_CANNOT_GETTO_LOCATION -5
#define ITEM_HANDLE_BROKEN -6
#define ITEM_HANDLE_NOROOM -7
#define ITEM_HANDLE_REFUSAL -8

// Define for code to try and pickup all items....
#define ITEM_PICKUP_ACTION_ALL 32000
#define ITEM_PICKUP_SELECTION 31000

#define ITEM_IGNORE_Z_LEVEL -1

#define ITEMLIST_INIT_HANDLE 1
#define ITEMLIST_DISPLAY 2
#define ITEMLIST_HANDLE 3
#define ITEMLIST_END_HANDLE 4
#define ITEMLIST_HANDLE_SELECTION 5

// visibility defines
#define ANY_VISIBILITY_VALUE -10
#define HIDDEN_ITEM -4
#define BURIED -3
#define HIDDEN_IN_OBJECT -2
#define INVISIBLE -1
#define VISIBLE 1

#define ITEM_LOCATOR_DELAY 0x01
#define ITEM_LOCATOR_LOCKED 0x02

// MACRO FOR DEFINING OF ITEM IS VISIBLE
#define ITEMPOOL_VISIBLE(pItemPool) \
  ((pItemPool->bVisible >= 1) || (gTacticalStatus.uiFlags & SHOW_ALL_ITEMS))

typedef void (*ITEM_POOL_LOCATOR_HOOK)(void);

struct ITEM_POOL {
  struct ITEM_POOL *pNext;
  struct ITEM_POOL *pPrev;

  int32_t iItemIndex;
  int8_t bVisible;
  int8_t bFlashColor;
  uint32_t uiTimerID;
  int16_t sGridNo;
  uint8_t ubLevel;
  uint16_t usFlags;
  int8_t bRenderZHeightAboveLevel;
  struct LEVELNODE *pLevelNode;
};

typedef struct {
  struct ITEM_POOL *pItemPool;

  // Additional info for locators
  int8_t bRadioFrame;
  uint32_t uiLastFrameUpdate;
  ITEM_POOL_LOCATOR_HOOK Callback;
  BOOLEAN fAllocated;
  uint8_t ubFlags;

} ITEM_POOL_LOCATOR;

int32_t HandleItem(struct SOLDIERTYPE *pSoldier, uint16_t usGridNo, int8_t bLevel, uint16_t usHandItem,
                 BOOLEAN fFromUI);
void SoldierPickupItem(struct SOLDIERTYPE *pSoldier, int32_t iItemIndex, int16_t sGridNo, int8_t bZLevel);
void HandleSoldierPickupItem(struct SOLDIERTYPE *pSoldier, int32_t iItemIndex, int16_t sGridNo,
                             int8_t bZLevel);
void HandleFlashingItems();

BOOLEAN SoldierDropItem(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj);

void HandleSoldierThrowItem(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);
BOOLEAN VerifyGiveItem(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE **ppTargetSoldier);
void SoldierGiveItemFromAnimation(struct SOLDIERTYPE *pSoldier);
void SoldierGiveItem(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                     struct OBJECTTYPE *pObject, int8_t bInvPos);

void NotifySoldiersToLookforItems();
void AllSoldiersLookforItems(BOOLEAN RevealRoofsAndItems);

void SoldierGetItemFromWorld(struct SOLDIERTYPE *pSoldier, int32_t iItemIndex, int16_t sGridNo,
                             int8_t bZLevel, BOOLEAN *pfSelectionList);

struct OBJECTTYPE *AddItemToPool(int16_t sGridNo, struct OBJECTTYPE *pObject, int8_t bVisible,
                                 uint8_t ubLevel, uint16_t usFlags, int8_t bRenderZHeightAboveLevel);
struct OBJECTTYPE *AddItemToPoolAndGetIndex(int16_t sGridNo, struct OBJECTTYPE *pObject,
                                            int8_t bVisible, uint8_t ubLevel, uint16_t usFlags,
                                            int8_t bRenderZHeightAboveLevel, int32_t *piItemIndex);
struct OBJECTTYPE *InternalAddItemToPool(int16_t *psGridNo, struct OBJECTTYPE *pObject, int8_t bVisible,
                                         uint8_t ubLevel, uint16_t usFlags,
                                         int8_t bRenderZHeightAboveLevel, int32_t *piItemIndex);

int16_t AdjustGridNoForItemPlacement(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);
BOOLEAN GetItemPool(uint16_t usMapPos, struct ITEM_POOL **ppItemPool, uint8_t ubLevel);
BOOLEAN DrawItemPoolList(struct ITEM_POOL *pItemPool, int16_t sGridNo, uint8_t bCommand, int8_t bZLevel,
                         int16_t sXPos, int16_t sYPos);
BOOLEAN RemoveItemFromPool(int16_t sGridNo, int32_t iItemIndex, uint8_t ubLevel);
BOOLEAN ItemExistsAtLocation(int16_t sGridNo, int32_t iItemIndex, uint8_t ubLevel);
BOOLEAN MoveItemPools(int16_t sStartPos, int16_t sEndPos);

void SetItemPoolLocator(struct ITEM_POOL *pItemPool);
void SetItemPoolLocatorWithCallback(struct ITEM_POOL *pItemPool, ITEM_POOL_LOCATOR_HOOK Callback);
BOOLEAN SetItemPoolVisibilityOn(struct ITEM_POOL *pItemPool, int8_t bAllGreaterThan,
                                BOOLEAN fSetLocator);
void AdjustItemPoolVisibility(struct ITEM_POOL *pItemPool);

void SetItemPoolVisibilityHiddenInObject(struct ITEM_POOL *pItemPool);
void SetItemPoolVisibilityHidden(struct ITEM_POOL *pItemPool);

int32_t GetItemOfClassTypeInPool(int16_t sGridNo, uint32_t uiItemClass, uint8_t ubLevel);
void RemoveItemPool(int16_t sGridNo, uint8_t ubLevel);
void RenderTopmostFlashingItems();

void RemoveAllUnburiedItems(int16_t sGridNo, uint8_t ubLevel);

BOOLEAN DoesItemPoolContainAnyHiddenItems(struct ITEM_POOL *pItemPool);
BOOLEAN DoesItemPoolContainAllHiddenItems(struct ITEM_POOL *pItemPool);

void HandleSoldierDropBomb(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);
void HandleSoldierUseRemote(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);

BOOLEAN DoesItemPoolContainAllItemsOfZeroZLevel(struct ITEM_POOL *pItemPool);
BOOLEAN DoesItemPoolContainAllItemsOfHigherZLevel(struct ITEM_POOL *pItemPool);

BOOLEAN ItemPoolOKForDisplay(struct ITEM_POOL *pItemPool, int8_t bZLevel);
int16_t GetNumOkForDisplayItemsInPool(struct ITEM_POOL *pItemPool, int8_t bZLevel);

void SoldierHandleDropItem(struct SOLDIERTYPE *pSoldier);

BOOLEAN LookForHiddenItems(int16_t sGridNo, int8_t ubLevel, BOOLEAN fSetLocator, int8_t bZLevel);

int8_t GetZLevelOfItemPoolGivenStructure(int16_t sGridNo, uint8_t ubLevel, struct STRUCTURE *pStructure);

int8_t GetLargestZLevelOfItemPool(struct ITEM_POOL *pItemPool);

BOOLEAN NearbyGroundSeemsWrong(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                               BOOLEAN fCheckAroundGridno, uint16_t *psProblemGridNo);
void MineSpottedDialogueCallBack(void);

extern int16_t gsBoobyTrapGridNo;
extern struct SOLDIERTYPE *gpBoobyTrapSoldier;
void AddBlueFlag(int16_t sGridNo, int8_t bLevel);
void RemoveBlueFlag(int16_t sGridNo, int8_t bLevel);

// check if item is booby trapped
BOOLEAN ContinuePastBoobyTrapInMapScreen(struct OBJECTTYPE *pObject, struct SOLDIERTYPE *pSoldier);

// set off the booby trap in mapscreen
void SetOffBoobyTrapInMapScreen(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject);

void RefreshItemPools(WORLDITEM *pItemList, int32_t iNumberOfItems);

BOOLEAN HandItemWorks(struct SOLDIERTYPE *pSoldier, int8_t bSlot);

BOOLEAN ItemTypeExistsAtLocation(int16_t sGridNo, uint16_t usItem, uint8_t ubLevel, int32_t *piItemIndex);

int16_t FindNearestAvailableGridNoForItem(int16_t sSweetGridNo, int8_t ubRadius);

BOOLEAN CanPlayerUseRocketRifle(struct SOLDIERTYPE *pSoldier, BOOLEAN fDisplay);

void MakeNPCGrumpyForMinorOffense(struct SOLDIERTYPE *pSoldier,
                                  struct SOLDIERTYPE *pOffendingSoldier);

#endif
