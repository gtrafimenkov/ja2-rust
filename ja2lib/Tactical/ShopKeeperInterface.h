#ifndef _SHOPKEEPER_INTERFACE__H_
#define _SHOPKEEPER_INTERFACE__H_

#include "Laptop/StoreInventory.h"
#include "SGP/Types.h"

#define SKI_NUM_TRADING_INV_SLOTS 12
#define SKI_NUM_TRADING_INV_ROWS 2
#define SKI_NUM_TRADING_INV_COLS 6

// Enums used for when the user clicks on an item and the item goes to..
enum {
  ARMS_DEALER_INVENTORY,
  ARMS_DEALER_OFFER_AREA,
  PLAYERS_OFFER_AREA,
  PLAYERS_INVENTORY,
};

#define ARMS_INV_ITEM_SELECTED 0x00000001  // The item has been placed into the offer area
//#define	ARMS_INV_PLAYERS_ITEM_SELECTED						0x00000002
//// The source location for the item has been selected
#define ARMS_INV_PLAYERS_ITEM_HAS_VALUE \
  0x00000004  // The Players item is worth something to this dealer
//#define	ARMS_INV_ITEM_HIGHLIGHTED
// 0x00000008			// If the items is highlighted
#define ARMS_INV_ITEM_NOT_REPAIRED_YET \
  0x00000010                                // The item is in for repairs but not repaired yet
#define ARMS_INV_ITEM_REPAIRED 0x00000020   // The item is repaired
#define ARMS_INV_JUST_PURCHASED 0x00000040  // The item was just purchased
#define ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED 0x00000080  // The Players item has been evaluated

typedef struct {
  BOOLEAN fActive;
  int16_t sItemIndex;
  uint32_t uiFlags;
  struct OBJECTTYPE ItemObject;
  uint8_t ubLocationOfObject;  // An enum value for the location of the item ( either in the arms
                             // dealers inventory, one of the offer areas or in the users inventory)
  int8_t bSlotIdInOtherLocation;

  uint8_t ubIdOfMercWhoOwnsTheItem;
  uint32_t uiItemPrice;  // Only used for the players item that have been evaluated

  int16_t sSpecialItemElement;  // refers to which special item element an item in a dealer's
                              // inventory area occupies.  -1 Means the item is "perfect" and has no
                              // associated special item.

} INVENTORY_IN_SLOT;

extern INVENTORY_IN_SLOT PlayersOfferArea[SKI_NUM_TRADING_INV_SLOTS];
extern int32_t giShopKeepDialogueEventinProgress;

// extern	BOOLEAN		gfRedrawSkiScreen;

enum {
  SKI_DIRTY_LEVEL0,  // no redraw
  SKI_DIRTY_LEVEL1,  // redraw only items
  SKI_DIRTY_LEVEL2,  // redraw everything
};

extern uint8_t gubSkiDirtyLevel;

extern struct OBJECTTYPE *gpHighLightedItemObject;

extern INVENTORY_IN_SLOT gMoveingItem;

extern struct OBJECTTYPE *pShopKeeperItemDescObject;

uint32_t ShopKeeperScreenInit(void);
uint32_t ShopKeeperScreenHandle(void);
uint32_t ShopKeeperScreenShutdown(void);

void EnterShopKeeperInterfaceScreen(uint8_t ubArmsDealer);

void DrawHatchOnInventory(uint32_t uiSurface, uint16_t usPosX, uint16_t usPosY, uint16_t usWidth,
                          uint16_t usHeight);
BOOLEAN ShouldSoldierDisplayHatchOnItem(uint8_t ubProfileID, int16_t sSlotNum);
int8_t AddItemToPlayersOfferArea(uint8_t ubProfileID, INVENTORY_IN_SLOT *pInvSlot,
                               int8_t bSlotIdInOtherLocation);
void ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack(uint8_t bExitValue);
void ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack(uint8_t bExitValue);

void SkiHelpTextDoneCallBack(void);
void SetSkiCursor(uint16_t usCursor);

void InitShopKeeperSubTitledText(wchar_t* pString);

void AddItemToPlayersOfferAreaAfterShopKeeperOpen(struct OBJECTTYPE *pItemObject,
                                                  int8_t bPreviousInvPos);

void BeginSkiItemPointer(uint8_t ubSource, int8_t bSlotNum, BOOLEAN fOfferToDealerFirst);

void DeleteShopKeeperItemDescBox();

BOOLEAN CanMercInteractWithSelectedShopkeeper(struct SOLDIERTYPE *pSoldier);

void DealerGetsBribed(uint8_t ubProfileId, uint32_t uiMoneyAmount);

#ifdef JA2TESTVERSION
void AddShopkeeperToGridNo(uint8_t ubProfile, int16_t sGridNo);
#endif

void RestrictSkiMouseCursor();

#endif
