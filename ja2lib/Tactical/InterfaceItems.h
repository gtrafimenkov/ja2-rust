#ifndef __INTERFACE_ITEMS_H
#define __INTERFACE_ITEMS_H

#include "SGP/MouseSystem.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Items.h"

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
  INT16 sBarDx;
  INT16 sBarDy;
  INT16 sWidth;
  INT16 sHeight;
  INT16 sX;  // starts at 0, gets set via InitInvSlotInterface()
  INT16 sY;  // starts at 0, gets set via InitInvSlotInterface()
} INV_REGIONS;

// USED TO SETUP REGION POSITIONS, ETC
typedef struct {
  INT16 sX;
  INT16 sY;
} INV_REGION_DESC;

// Itempickup stuff
BOOLEAN InitializeItemPickupMenu(struct SOLDIERTYPE *pSoldier, INT16 sGridNo,
                                 struct ITEM_POOL *pItemPool, INT16 sScreenX, INT16 sScreenY,
                                 INT8 bZLevel);
void RenderItemPickupMenu();
void RemoveItemPickupMenu();
void SetItemPickupMenuDirty(BOOLEAN fDirtyLevel);
BOOLEAN HandleItemPickupMenu();
void SetPickUpMenuDirtyLevel(BOOLEAN fDirtyLevel);

// FUNCTIONS FOR INTERFACEING WITH ITEM PANEL STUFF
void INVRenderINVPanelItem(struct SOLDIERTYPE *pSoldier, INT16 sPocket, UINT8 fDirtyLevel);
BOOLEAN InitInvSlotInterface(INV_REGION_DESC *pRegionDesc, INV_REGION_DESC *pCamoRegion,
                             MOUSE_CALLBACK INVMoveCallback, MOUSE_CALLBACK INVClickCallback,
                             MOUSE_CALLBACK INVMoveCammoCallback,
                             MOUSE_CALLBACK INVClickCammoCallback, BOOLEAN fSetHighestPrioity);
void ShutdownInvSlotInterface();
void GetSlotInvXY(UINT8 ubPos, INT16 *psX, INT16 *psY);
void GetSlotInvHeightWidth(UINT8 ubPos, INT16 *psWidth, INT16 *psHeight);
void HandleRenderInvSlots(struct SOLDIERTYPE *pSoldier, UINT8 fDirtyLevel);
void HandleNewlyAddedItems(struct SOLDIERTYPE *pSoldier, BOOLEAN *fDirtyLevel);
void RenderInvBodyPanel(struct SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY);
void DisableInvRegions(BOOLEAN fDisable);

void DegradeNewlyAddedItems();
void CheckForAnyNewlyAddedItems(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleCompatibleAmmoUI(struct SOLDIERTYPE *pSoldier, INT8 bInvPos, BOOLEAN fOn);

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

void INVRenderItem(UINT32 uiBuffer, struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject,
                   INT16 sX, INT16 sY, INT16 sWidth, INT16 sHeight, UINT8 fDirtyLevel,
                   UINT8 *pubHighlightCounter, UINT8 ubStatusIndex, BOOLEAN fOutline,
                   INT16 sOutlineColor);

extern BOOLEAN gfInItemDescBox;

BOOLEAN InItemDescriptionBox();
BOOLEAN InitItemDescriptionBox(struct SOLDIERTYPE *pSoldier, UINT8 ubPosition, INT16 sX, INT16 sY,
                               UINT8 ubStatusIndex);
BOOLEAN InternalInitItemDescriptionBox(struct OBJECTTYPE *pObject, INT16 sX, INT16 sY,
                                       UINT8 ubStatusIndex, struct SOLDIERTYPE *pSoldier);
BOOLEAN InitKeyItemDescriptionBox(struct SOLDIERTYPE *pSoldier, UINT8 ubPosition, INT16 sX,
                                  INT16 sY, UINT8 ubStatusIndex);
void RenderItemDescriptionBox();
void HandleItemDescriptionBox(BOOLEAN *pfDirty);
void DeleteItemDescriptionBox();

BOOLEAN InItemStackPopup();
BOOLEAN InitItemStackPopup(struct SOLDIERTYPE *pSoldier, UINT8 ubPosition, INT16 sInvX, INT16 sInvY,
                           INT16 sInvWidth, INT16 sInvHeight);
void RenderItemStackPopup(BOOLEAN fFullRender);
void HandleItemStackPopup();
void DeleteItemStackPopup();
void EndItemStackPopupWithItemInHand();

// keyring handlers
BOOLEAN InitKeyRingPopup(struct SOLDIERTYPE *pSoldier, INT16 sInvX, INT16 sInvY, INT16 sInvWidth,
                         INT16 sInvHeight);
void RenderKeyRingPopup(BOOLEAN fFullRender);
void InitKeyRingInterface(MOUSE_CALLBACK KeyRingClickCallback);
void InitMapKeyRingInterface(MOUSE_CALLBACK KeyRingClickCallback);
void DeleteKeyRingPopup();

void ShutdownKeyRingInterface(void);
BOOLEAN InKeyRingPopup(void);
void BeginKeyRingItemPointer(struct SOLDIERTYPE *pSoldier, UINT8 ubKeyRingPosition);

extern struct OBJECTTYPE *gpItemPointer;
extern struct OBJECTTYPE gItemPointer;
extern struct SOLDIERTYPE *gpItemPointerSoldier;
extern UINT16 usItemSnapCursor;
extern UINT16 us16BPPItemCyclePlacedItemColors[20];
extern BOOLEAN gfItemPointerDifferentThanDefault;

void BeginItemPointer(struct SOLDIERTYPE *pSoldier, UINT8 ubHandPos);
void InternalBeginItemPointer(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObject,
                              INT8 bHandPos);
void EndItemPointer();
void DrawItemFreeCursor();
void DrawItemTileCursor();
void HideItemTileCursor();
void InitItemInterface();
BOOLEAN ItemCursorInLobRange(UINT16 usMapPos);
BOOLEAN HandleItemPointerClick(UINT16 usMapPos);
UINT32 GetInterfaceGraphicForItem(INVTYPE *pItem);
UINT16 GetTileGraphicForItem(INVTYPE *pItem);
BOOLEAN LoadTileGraphicForItem(INVTYPE *pItem, UINT32 *puiVo);

void GetHelpTextForItem(CHAR16 *pzStr, size_t bufSize, struct OBJECTTYPE *pObject,
                        struct SOLDIERTYPE *pSoldier);

BOOLEAN AttemptToApplyCamo(struct SOLDIERTYPE *pSoldier, UINT16 usItemIndex);

UINT8 GetPrefferedItemSlotGraphicNum(UINT16 usItem);

void CancelItemPointer();

BOOLEAN LoadItemCursorFromSavedGame(FileID hFile);
BOOLEAN SaveItemCursorToSavedGame(FileID hFile);

void EnableKeyRing(BOOLEAN fEnable);

// handle compatable items for merc and map inventory
BOOLEAN HandleCompatibleAmmoUIForMapScreen(struct SOLDIERTYPE *pSoldier, INT32 bInvPos, BOOLEAN fOn,
                                           BOOLEAN fFromMerc);
BOOLEAN HandleCompatibleAmmoUIForMapInventory(struct SOLDIERTYPE *pSoldier, INT32 bInvPos,
                                              INT32 iStartSlotNumber, BOOLEAN fOn,
                                              BOOLEAN fFromMerc);
void ResetCompatibleItemArray();

void CycleItemDescriptionItem();

#endif
