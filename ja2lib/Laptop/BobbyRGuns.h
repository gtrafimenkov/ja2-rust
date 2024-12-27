#ifndef __BOBBYRGUNS_H
#define __BOBBYRGUNS_H

#include "SGP/Types.h"

#define BOBBYRDESCFILE "BINARYDATA\\BrayDesc.edt"

#define BOBBYR_ITEM_DESC_NAME_SIZE 160
#define BOBBYR_ITEM_DESC_INFO_SIZE 640
#define BOBBYR_ITEM_DESC_FILE_SIZE 800

#define BOBBYR_USED_ITEMS 0xFFFFFFFF

#define BOBBYR_GUNS_BUTTON_FONT FONT10ARIAL
#define BOBBYR_GUNS_TEXT_COLOR_ON FONT_NEARBLACK
#define BOBBYR_GUNS_TEXT_COLOR_OFF FONT_NEARBLACK

#define BOBBYR_GUNS_SHADOW_COLOR 169

#define BOBBYR_NO_ITEMS 65535

extern uint16_t gusCurWeaponIndex;
extern uint8_t gubLastGunIndex;
extern INT32 giBobbyRHomeImage;

void GameInitBobbyRGuns();
BOOLEAN EnterBobbyRGuns();
void ExitBobbyRGuns();
void HandleBobbyRGuns();
void RenderBobbyRGuns();

BOOLEAN DisplayBobbyRBrTitle();
BOOLEAN DeleteBobbyBrTitle();
BOOLEAN InitBobbyBrTitle();
BOOLEAN InitBobbyMenuBar();
BOOLEAN DeleteBobbyMenuBar();

// BOOLEAN DisplayWeaponInfo();
BOOLEAN DisplayItemInfo(uint32_t uiItemClass);
void DeleteMouseRegionForBigImage();
void UpdateButtonText(uint32_t uiCurPage);
void EnterInitBobbyRGuns();
void DailyUpdateOfBobbyRaysUsedInventory();
uint16_t CalcBobbyRayCost(uint16_t usIndex, uint16_t usBobbyIndex, BOOLEAN fUsed);
// void CalculateFirstAndLastIndexs();
void SetFirstLastPagesForUsed();
void SetFirstLastPagesForNew(uint32_t uiClass);

#endif
