// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __INTERFACE_ITEMS_H
#define __INTERFACE_ITEMS_H

#include "SGP/MouseSystem.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Items.h"

struct VSurface;

// DEFINES FOR ITEM SLOT SIZES IN PIXELS
#define BIG_INV_SLOT_WIDTH 61
#define BIG_INV_SLOT_HEIGHT 22
#define SM_INV_SLOT_WIDTH 30
#define SM_INV_SLOT_HEIGHT 23
#define VEST_INV_SLOT_WIDTH 43
#define VEST_INV_SLOT_HEIGHT 24
#define LEGS_INV_SLOT_WIDTH 43
#define LEGS_INV_SLOT_HEIGHT 24
#define HEAD_INV_SLOT_WIDTH 43
#define HEAD_INV_SLOT_HEIGHT 24

extern BOOLEAN fShowDescriptionFlag;

// A STRUCT USED INTERNALLY FOR INV SLOT REGIONS
typedef struct {
  BOOLEAN fBigPocket;
  int16_t sBarDx;
  int16_t sBarDy;
  int16_t sWidth;
  int16_t sHeight;
  int16_t sX;  // starts at 0, gets set via InitInvSlotInterface()
  int16_t sY;  // starts at 0, gets set via InitInvSlotInterface()
} INV_REGIONS;

// USED TO SETUP REGION POSITIONS, ETC
typedef struct {
  int16_t sX;
  int16_t sY;
} INV_REGION_DESC;

// Itempickup stuff
BOOLEAN InitializeItemPickupMenu(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                 struct ITEM_POOL *pItemPool, int16_t sScreenX, int16_t sScreenY,
                                 int8_t bZLevel);
void RenderItemPickupMenu();
void RemoveItemPickupMenu();
void SetItemPickupMenuDirty(BOOLEAN fDirtyLevel);
BOOLEAN HandleItemPickupMenu();
void SetPickUpMenuDirtyLevel(BOOLEAN fDirtyLevel);

// FUNCTIONS FOR INTERFACEING WITH ITEM PANEL STUFF
void INVRenderINVPanelItem(struct SOLDIERTYPE *pSoldier, int16_t sPocket, uint8_t fDirtyLevel);
BOOLEAN InitInvSlotInterface(INV_REGION_DESC *pRegionDesc, INV_REGION_DESC *pCamoRegion,
                             MOUSE_CALLBACK INVMoveCallback, MOUSE_CALLBACK INVClickCallback,
                             MOUSE_CALLBACK INVMoveCammoCallback,
                             MOUSE_CALLBACK INVClickCammoCallback, BOOLEAN fSetHighestPrioity);
void ShutdownInvSlotInterface();
void GetSlotInvXY(uint8_t ubPos, int16_t *psX, int16_t *psY);
void GetSlotInvHeightWidth(uint8_t ubPos, int16_t *psWidth, int16_t *psHeight);
void HandleRenderInvSlots(struct SOLDIERTYPE *pSoldier, uint8_t fDirtyLevel);
void HandleNewlyAddedItems(struct SOLDIERTYPE *pSoldier, BOOLEAN *fDirtyLevel);
void RenderInvBodyPanel(struct SOLDIERTYPE *pSoldier, int16_t sX, int16_t sY);
void DisableInvRegions(BOOLEAN fDisable);

void DegradeNewlyAddedItems();
void CheckForAnyNewlyAddedItems(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleCompatibleAmmoUI(struct SOLDIERTYPE *pSoldier, int8_t bInvPos, BOOLEAN fOn);

// THIS FUNCTION IS CALLED TO RENDER AN ITEM.
// uiBuffer - The Dest Video Surface - can only be FRAME_BUFFER or guiSAVEBUFFER
// pSoldier - used for determining whether burst mode needs display
// pObject	- Usually taken from pSoldier->inv[HANDPOS]
// sX, sY, Width, Height,  - Will Center it in the Width
// fDirtyLevel  if == DIRTYLEVEL2 will render everything
//							if == DIRTYLEVEL1 will render bullets and
// status only
//
//	pubHighlightCounter - if not null, and == 2 - will display name above item
//											-	if
//== 1 will only dirty the name space and then set counter to 0
//  Last parameter used mainly for when mouse is over item

void INVRenderItem(struct VSurface *dest, struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject,
                   int16_t sX, int16_t sY, int16_t sWidth, int16_t sHeight, uint8_t fDirtyLevel,
                   uint8_t *pubHighlightCounter, uint8_t ubStatusIndex, BOOLEAN fOutline,
                   int16_t sOutlineColor);

extern BOOLEAN gfInItemDescBox;

BOOLEAN InItemDescriptionBox();
BOOLEAN InitItemDescriptionBox(struct SOLDIERTYPE *pSoldier, uint8_t ubPosition, int16_t sX,
                               int16_t sY, uint8_t ubStatusIndex);
BOOLEAN InternalInitItemDescriptionBox(struct OBJECTTYPE *pObject, int16_t sX, int16_t sY,
                                       uint8_t ubStatusIndex, struct SOLDIERTYPE *pSoldier);
BOOLEAN InitKeyItemDescriptionBox(struct SOLDIERTYPE *pSoldier, uint8_t ubPosition, int16_t sX,
                                  int16_t sY, uint8_t ubStatusIndex);
void RenderItemDescriptionBox();
void HandleItemDescriptionBox(BOOLEAN *pfDirty);
void DeleteItemDescriptionBox();

BOOLEAN InItemStackPopup();
BOOLEAN InitItemStackPopup(struct SOLDIERTYPE *pSoldier, uint8_t ubPosition, int16_t sInvX,
                           int16_t sInvY, int16_t sInvWidth, int16_t sInvHeight);
void RenderItemStackPopup(BOOLEAN fFullRender);
void HandleItemStackPopup();
void DeleteItemStackPopup();
void EndItemStackPopupWithItemInHand();

// keyring handlers
BOOLEAN InitKeyRingPopup(struct SOLDIERTYPE *pSoldier, int16_t sInvX, int16_t sInvY,
                         int16_t sInvWidth, int16_t sInvHeight);
void RenderKeyRingPopup(BOOLEAN fFullRender);
void InitKeyRingInterface(MOUSE_CALLBACK KeyRingClickCallback);
void InitMapKeyRingInterface(MOUSE_CALLBACK KeyRingClickCallback);
void DeleteKeyRingPopup();

void ShutdownKeyRingInterface(void);
BOOLEAN InKeyRingPopup(void);
void BeginKeyRingItemPointer(struct SOLDIERTYPE *pSoldier, uint8_t ubKeyRingPosition);

extern struct OBJECTTYPE *gpItemPointer;
extern struct OBJECTTYPE gItemPointer;
extern struct SOLDIERTYPE *gpItemPointerSoldier;
extern uint16_t usItemSnapCursor;
extern uint16_t us16BPPItemCyclePlacedItemColors[20];
extern BOOLEAN gfItemPointerDifferentThanDefault;

void BeginItemPointer(struct SOLDIERTYPE *pSoldier, uint8_t ubHandPos);
void InternalBeginItemPointer(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject,
                              int8_t bHandPos);
void EndItemPointer();
void DrawItemFreeCursor();
void DrawItemTileCursor();
void HideItemTileCursor();
void InitItemInterface();
BOOLEAN ItemCursorInLobRange(uint16_t usMapPos);
BOOLEAN HandleItemPointerClick(uint16_t usMapPos);
uint32_t GetInterfaceGraphicForItem(INVTYPE *pItem);
uint16_t GetTileGraphicForItem(INVTYPE *pItem);
BOOLEAN LoadTileGraphicForItem(INVTYPE *pItem, uint32_t *puiVo);

void GetHelpTextForItem(wchar_t *pzStr, size_t bufSize, struct OBJECTTYPE *pObject,
                        struct SOLDIERTYPE *pSoldier);

BOOLEAN AttemptToApplyCamo(struct SOLDIERTYPE *pSoldier, uint16_t usItemIndex);

uint8_t GetPrefferedItemSlotGraphicNum(uint16_t usItem);

void CancelItemPointer();

BOOLEAN LoadItemCursorFromSavedGame(FileID hFile);
BOOLEAN SaveItemCursorToSavedGame(FileID hFile);

void EnableKeyRing(BOOLEAN fEnable);

// handle compatable items for merc and map inventory
BOOLEAN HandleCompatibleAmmoUIForMapScreen(struct SOLDIERTYPE *pSoldier, int32_t bInvPos,
                                           BOOLEAN fOn, BOOLEAN fFromMerc);
BOOLEAN HandleCompatibleAmmoUIForMapInventory(struct SOLDIERTYPE *pSoldier, int32_t bInvPos,
                                              int32_t iStartSlotNumber, BOOLEAN fOn,
                                              BOOLEAN fFromMerc);
void ResetCompatibleItemArray();

void CycleItemDescriptionItem();

#endif
