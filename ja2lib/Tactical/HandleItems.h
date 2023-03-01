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

  INT32 iItemIndex;
  INT8 bVisible;
  INT8 bFlashColor;
  UINT32 uiTimerID;
  INT16 sGridNo;
  UINT8 ubLevel;
  UINT16 usFlags;
  INT8 bRenderZHeightAboveLevel;
  struct LEVELNODE *pLevelNode;
};

typedef struct {
  struct ITEM_POOL *pItemPool;

  // Additional info for locators
  INT8 bRadioFrame;
  UINT32 uiLastFrameUpdate;
  ITEM_POOL_LOCATOR_HOOK Callback;
  BOOLEAN fAllocated;
  UINT8 ubFlags;

} ITEM_POOL_LOCATOR;

INT32 HandleItem(struct SOLDIERTYPE *pSoldier, UINT16 usGridNo, INT8 bLevel, UINT16 usHandItem,
                 BOOLEAN fFromUI);
void SoldierPickupItem(struct SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel);
void HandleSoldierPickupItem(struct SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo,
                             INT8 bZLevel);
void HandleFlashingItems();

BOOLEAN SoldierDropItem(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj);

void HandleSoldierThrowItem(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
BOOLEAN VerifyGiveItem(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE **ppTargetSoldier);
void SoldierGiveItemFromAnimation(struct SOLDIERTYPE *pSoldier);
void SoldierGiveItem(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                     struct OBJECTTYPE *pObject, INT8 bInvPos);

void NotifySoldiersToLookforItems();
void AllSoldiersLookforItems(BOOLEAN RevealRoofsAndItems);

void SoldierGetItemFromWorld(struct SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo,
                             INT8 bZLevel, BOOLEAN *pfSelectionList);

struct OBJECTTYPE *AddItemToPool(INT16 sGridNo, struct OBJECTTYPE *pObject, INT8 bVisible,
                                 UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel);
struct OBJECTTYPE *AddItemToPoolAndGetIndex(INT16 sGridNo, struct OBJECTTYPE *pObject,
                                            INT8 bVisible, UINT8 ubLevel, UINT16 usFlags,
                                            INT8 bRenderZHeightAboveLevel, INT32 *piItemIndex);
struct OBJECTTYPE *InternalAddItemToPool(INT16 *psGridNo, struct OBJECTTYPE *pObject, INT8 bVisible,
                                         UINT8 ubLevel, UINT16 usFlags,
                                         INT8 bRenderZHeightAboveLevel, INT32 *piItemIndex);

INT16 AdjustGridNoForItemPlacement(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
BOOLEAN GetItemPool(UINT16 usMapPos, struct ITEM_POOL **ppItemPool, UINT8 ubLevel);
BOOLEAN DrawItemPoolList(struct ITEM_POOL *pItemPool, INT16 sGridNo, UINT8 bCommand, INT8 bZLevel,
                         INT16 sXPos, INT16 sYPos);
BOOLEAN RemoveItemFromPool(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel);
BOOLEAN ItemExistsAtLocation(INT16 sGridNo, INT32 iItemIndex, UINT8 ubLevel);
BOOLEAN MoveItemPools(INT16 sStartPos, INT16 sEndPos);

void SetItemPoolLocator(struct ITEM_POOL *pItemPool);
void SetItemPoolLocatorWithCallback(struct ITEM_POOL *pItemPool, ITEM_POOL_LOCATOR_HOOK Callback);
BOOLEAN SetItemPoolVisibilityOn(struct ITEM_POOL *pItemPool, INT8 bAllGreaterThan,
                                BOOLEAN fSetLocator);
void AdjustItemPoolVisibility(struct ITEM_POOL *pItemPool);

void SetItemPoolVisibilityHiddenInObject(struct ITEM_POOL *pItemPool);
void SetItemPoolVisibilityHidden(struct ITEM_POOL *pItemPool);

INT32 GetItemOfClassTypeInPool(INT16 sGridNo, UINT32 uiItemClass, UINT8 ubLevel);
void RemoveItemPool(INT16 sGridNo, UINT8 ubLevel);
void RenderTopmostFlashingItems();

void RemoveAllUnburiedItems(INT16 sGridNo, UINT8 ubLevel);

BOOLEAN DoesItemPoolContainAnyHiddenItems(struct ITEM_POOL *pItemPool);
BOOLEAN DoesItemPoolContainAllHiddenItems(struct ITEM_POOL *pItemPool);

void HandleSoldierDropBomb(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);
void HandleSoldierUseRemote(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

BOOLEAN DoesItemPoolContainAllItemsOfZeroZLevel(struct ITEM_POOL *pItemPool);
BOOLEAN DoesItemPoolContainAllItemsOfHigherZLevel(struct ITEM_POOL *pItemPool);

BOOLEAN ItemPoolOKForDisplay(struct ITEM_POOL *pItemPool, INT8 bZLevel);
INT16 GetNumOkForDisplayItemsInPool(struct ITEM_POOL *pItemPool, INT8 bZLevel);

void SoldierHandleDropItem(struct SOLDIERTYPE *pSoldier);

BOOLEAN LookForHiddenItems(INT16 sGridNo, INT8 ubLevel, BOOLEAN fSetLocator, INT8 bZLevel);

INT8 GetZLevelOfItemPoolGivenStructure(INT16 sGridNo, UINT8 ubLevel, struct STRUCTURE *pStructure);

INT8 GetLargestZLevelOfItemPool(struct ITEM_POOL *pItemPool);

BOOLEAN NearbyGroundSeemsWrong(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                               BOOLEAN fCheckAroundGridno, UINT16 *psProblemGridNo);
void MineSpottedDialogueCallBack(void);

extern INT16 gsBoobyTrapGridNo;
extern struct SOLDIERTYPE *gpBoobyTrapSoldier;
void AddBlueFlag(INT16 sGridNo, INT8 bLevel);
void RemoveBlueFlag(INT16 sGridNo, INT8 bLevel);

// check if item is booby trapped
BOOLEAN ContinuePastBoobyTrapInMapScreen(struct OBJECTTYPE *pObject, struct SOLDIERTYPE *pSoldier);

// set off the booby trap in mapscreen
void SetOffBoobyTrapInMapScreen(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject);

void RefreshItemPools(WORLDITEM *pItemList, INT32 iNumberOfItems);

BOOLEAN HandItemWorks(struct SOLDIERTYPE *pSoldier, INT8 bSlot);

BOOLEAN ItemTypeExistsAtLocation(INT16 sGridNo, UINT16 usItem, UINT8 ubLevel, INT32 *piItemIndex);

INT16 FindNearestAvailableGridNoForItem(INT16 sSweetGridNo, INT8 ubRadius);

BOOLEAN CanPlayerUseRocketRifle(struct SOLDIERTYPE *pSoldier, BOOLEAN fDisplay);

void MakeNPCGrumpyForMinorOffense(struct SOLDIERTYPE *pSoldier,
                                  struct SOLDIERTYPE *pOffendingSoldier);

#endif
