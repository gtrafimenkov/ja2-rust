#ifndef __BOBBYR_H
#define __BOBBYR_H

#include "Laptop/StoreInventory.h"
#include "SGP/Types.h"
#include "Tactical/ItemTypes.h"

void GameInitBobbyR();
BOOLEAN EnterBobbyR();
void ExitBobbyR();
void HandleBobbyR();
void RenderBobbyR();

#define BOBBYR_BACKGROUND_WIDTH 125
#define BOBBYR_BACKGROUND_HEIGHT 100
#define BOBBYR_NUM_HORIZONTAL_TILES 4
#define BOBBYR_NUM_VERTICAL_TILES 4

#define BOBBYR_GRIDLOC_X LAPTOP_SCREEN_UL_X + 4
#define BOBBYR_GRIDLOC_Y LAPTOP_SCREEN_WEB_UL_Y + 45

/*
extern	uint16_t	gusFirstGunIndex;
extern	uint16_t	gusLastGunIndex;
extern	uint8_t		gubNumGunPages;

extern	uint16_t	gusFirstAmmoIndex;
extern	uint16_t	gusLastAmmoIndex;
extern	uint8_t		gubNumAmmoPages;

extern	uint16_t	gusFirstMiscIndex;
extern	uint16_t	gusLastMiscIndex;
extern	uint8_t		gubNumMiscPages;

extern	uint16_t  gusFirstArmourIndex;
extern	uint16_t  gusLastArmourIndex;
extern	uint8_t		gubNumArmourPages;

extern	uint16_t  gusFirstUsedIndex;
extern	uint16_t  gusLastUsedIndex;
extern	uint8_t		gubNumUsedPages;
*/

extern uint32_t guiLastBobbyRayPage;

// BOOLEAN WebPageTileBackground(uint8_t ubNumX, uint8_t ubNumY, uint16_t usWidth, uint16_t usHeight, uint32_t
// uiBackGround);
BOOLEAN DrawBobbyRWoodBackground();
BOOLEAN DeleteBobbyRWoodBackground();
BOOLEAN InitBobbyRWoodBackground();
void DailyUpdateOfBobbyRaysNewInventory();
void DailyUpdateOfBobbyRaysUsedInventory();
void OrderBobbyRItem(uint16_t usItemIndex);
void AddFreshBobbyRayInventory(uint16_t usItemIndex);
void InitBobbyRayInventory();
BOOLEAN InitBobbyRayNewInventory();
BOOLEAN InitBobbyRayUsedInventory();
uint8_t HowManyBRItemsToOrder(uint16_t usItemIndex, uint8_t ubCurrentlyOnHand, uint8_t ubBobbyRayNewUsed);
void CancelAllPendingBRPurchaseOrders(void);
int16_t GetInventorySlotForItem(STORE_INVENTORY *pInventoryArray, uint16_t usItemIndex, BOOLEAN fUsed);

#endif
