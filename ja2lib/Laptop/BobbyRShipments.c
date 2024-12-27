// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/BobbyRShipments.h"

#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
#include "Laptop/BobbyRMailOrder.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define BOBBYR_SHIPMENT_TITLE_TEXT_FONT FONT14ARIAL
#define BOBBYR_SHIPMENT_TITLE_TEXT_COLOR 157

#define BOBBYR_SHIPMENT_STATIC_TEXT_FONT FONT12ARIAL
#define BOBBYR_SHIPMENT_STATIC_TEXT_COLOR 145

#define BOBBYR_BOBBY_RAY_TITLE_X LAPTOP_SCREEN_UL_X + 171
#define BOBBYR_BOBBY_RAY_TITLE_Y LAPTOP_SCREEN_WEB_UL_Y + 3

#define BOBBYR_ORDER_FORM_TITLE_X BOBBYR_BOBBY_RAY_TITLE_X
#define BOBBYR_ORDER_FORM_TITLE_Y BOBBYR_BOBBY_RAY_TITLE_Y + 37
#define BOBBYR_ORDER_FORM_TITLE_WIDTH 159

#define BOBBYR_SHIPMENT_DELIVERY_GRID_X LAPTOP_SCREEN_UL_X + 2
#define BOBBYR_SHIPMENT_DELIVERY_GRID_Y BOBBYR_SHIPMENT_ORDER_GRID_Y
#define BOBBYR_SHIPMENT_DELIVERY_GRID_WIDTH 183

#define BOBBYR_SHIPMENT_ORDER_GRID_X LAPTOP_SCREEN_UL_X + 223
#define BOBBYR_SHIPMENT_ORDER_GRID_Y LAPTOP_SCREEN_WEB_UL_Y + 62

#define BOBBYR_SHIPMENT_BACK_BUTTON_X 130
#define BOBBYR_SHIPMENT_BACK_BUTTON_Y 400 + LAPTOP_SCREEN_WEB_DELTA_Y + 4

#define BOBBYR_SHIPMENT_HOME_BUTTON_X 515
#define BOBBYR_SHIPMENT_HOME_BUTTON_Y BOBBYR_SHIPMENT_BACK_BUTTON_Y

#define BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS 13

#define BOBBYR_SHIPMENT_ORDER_NUM_X 116  // LAPTOP_SCREEN_UL_X + 9
#define BOBBYR_SHIPMENT_ORDER_NUM_START_Y 144
#define BOBBYR_SHIPMENT_ORDER_NUM_WIDTH 64

#define BOBBYR_SHIPMENT_GAP_BTN_LINES 20

#define BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_X BOBBYR_SHIPMENT_ORDER_NUM_X
#define BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y 117

#define BOBBYR_SHIPMENT_NUM_ITEMS_X \
  183  // BOBBYR_SHIPMENT_ORDER_NUM_X+BOBBYR_SHIPMENT_ORDER_NUM_WIDTH+2
#define BOBBYR_SHIPMENT_NUM_ITEMS_Y BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y
#define BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH 116

// #define		BOBBYR_SHIPMENT_

uint32_t guiBobbyRShipmentGrid;

BOOLEAN gfBobbyRShipmentsDirty = FALSE;

int32_t giBobbyRShipmentSelectedShipment = -1;

// Back Button
void BtnBobbyRShipmentBackCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRShipmetBack;
int32_t guiBobbyRShipmentBackImage;

// Home Button
void BtnBobbyRShipmentHomeCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRShipmentHome;
int32_t giBobbyRShipmentHomeImage;

struct MOUSE_REGION gSelectedPreviousShipmentsRegion[BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS];
void SelectPreviousShipmentsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

//
// Function Prototypes
//

void DisplayShipmentGrid();
void DisplayPreviousShipments();
void DisplayShipmentTitles();
void RemovePreviousShipmentsMouseRegions();
void CreatePreviousShipmentsMouseRegions();
int32_t CountNumberValidShipmentForTheShipmentsPage();
// ppp

//
// Function
//

void GameInitBobbyRShipments() {}

BOOLEAN EnterBobbyRShipments() {
  InitBobbyRWoodBackground();

  // load the Order Grid graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\BobbyRay_OnOrder.sti", &guiBobbyRShipmentGrid)) {
    return FALSE;
  }

  guiBobbyRShipmentBackImage = LoadButtonImage("LAPTOP\\CatalogueButton.sti", -1, 0, -1, 1, -1);
  guiBobbyRShipmetBack = CreateIconAndTextButton(
      guiBobbyRShipmentBackImage, BobbyROrderFormText[BOBBYR_BACK], BOBBYR_GUNS_BUTTON_FONT,
      BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR, BOBBYR_GUNS_TEXT_COLOR_OFF,
      BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_SHIPMENT_BACK_BUTTON_X,
      BOBBYR_SHIPMENT_BACK_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnBobbyRShipmentBackCallback);
  SetButtonCursor(guiBobbyRShipmetBack, CURSOR_LAPTOP_SCREEN);

  giBobbyRShipmentHomeImage = UseLoadedButtonImage(guiBobbyRShipmentBackImage, -1, 0, -1, 1, -1);
  guiBobbyRShipmentHome = CreateIconAndTextButton(
      giBobbyRShipmentHomeImage, BobbyROrderFormText[BOBBYR_HOME], BOBBYR_GUNS_BUTTON_FONT,
      BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR, BOBBYR_GUNS_TEXT_COLOR_OFF,
      BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_SHIPMENT_HOME_BUTTON_X,
      BOBBYR_SHIPMENT_HOME_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnBobbyRShipmentHomeCallback);
  SetButtonCursor(guiBobbyRShipmentHome, CURSOR_LAPTOP_SCREEN);

  CreateBobbyRayOrderTitle();

  giBobbyRShipmentSelectedShipment = -1;

  // if there are shipments
  if (giNumberOfNewBobbyRShipment != 0) {
    int32_t iCnt;

    // get the first shipment #
    for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
      if (gpNewBobbyrShipments[iCnt].fActive) giBobbyRShipmentSelectedShipment = iCnt;
    }
  }

  CreatePreviousShipmentsMouseRegions();

  return (TRUE);
}

void ExitBobbyRShipments() {
  DeleteBobbyRWoodBackground();
  DestroyBobbyROrderTitle();

  DeleteVideoObjectFromIndex(guiBobbyRShipmentGrid);

  UnloadButtonImage(guiBobbyRShipmentBackImage);
  UnloadButtonImage(giBobbyRShipmentHomeImage);
  RemoveButton(guiBobbyRShipmetBack);
  RemoveButton(guiBobbyRShipmentHome);

  RemovePreviousShipmentsMouseRegions();
}

void HandleBobbyRShipments() {
  if (gfBobbyRShipmentsDirty) {
    gfBobbyRShipmentsDirty = FALSE;

    RenderBobbyRShipments();
  }
}

void RenderBobbyRShipments() {
  //  struct VObject* hPixHandle;

  DrawBobbyRWoodBackground();

  DrawBobbyROrderTitle();

  // Output the title
  DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__TITLE], BOBBYR_ORDER_FORM_TITLE_X,
                   BOBBYR_ORDER_FORM_TITLE_Y, BOBBYR_ORDER_FORM_TITLE_WIDTH,
                   BOBBYR_SHIPMENT_TITLE_TEXT_FONT, BOBBYR_SHIPMENT_TITLE_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  DisplayShipmentGrid();

  if (giBobbyRShipmentSelectedShipment != -1 &&
      gpNewBobbyrShipments[giBobbyRShipmentSelectedShipment].fActive &&
      gpNewBobbyrShipments[giBobbyRShipmentSelectedShipment].fDisplayedInShipmentPage) {
    //		DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X,
    // BOBBYR_SHIPMENT_ORDER_GRID_Y,
    //&LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0],
    // FALSE );
    DisplayPurchasedItems(
        FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y,
        &gpNewBobbyrShipments[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0], FALSE,
        giBobbyRShipmentSelectedShipment);
  } else {
    //		DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X,
    // BOBBYR_SHIPMENT_ORDER_GRID_Y,
    //&LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0],
    // TRUE );
    DisplayPurchasedItems(FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y, NULL,
                          TRUE, giBobbyRShipmentSelectedShipment);
  }

  DisplayShipmentTitles();
  DisplayPreviousShipments();

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnBobbyRShipmentBackCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R_MAILORDER;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnBobbyRShipmentHomeCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void DisplayShipmentGrid() {
  struct VObject *hPixHandle;

  GetVideoObject(&hPixHandle, guiBobbyRShipmentGrid);

  // Shipment Order Grid
  BltVObject(vsFB, hPixHandle, 0, BOBBYR_SHIPMENT_DELIVERY_GRID_X, BOBBYR_SHIPMENT_DELIVERY_GRID_Y);

  // Order Grid
  BltVObject(vsFB, hPixHandle, 1, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y);
}

void DisplayShipmentTitles() {
  // output the order #
  DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__ORDERED_ON],
                   BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_X, BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y,
                   BOBBYR_SHIPMENT_ORDER_NUM_WIDTH, BOBBYR_SHIPMENT_STATIC_TEXT_FONT,
                   BOBBYR_SHIPMENT_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Output the # of items
  DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__NUM_ITEMS], BOBBYR_SHIPMENT_NUM_ITEMS_X,
                   BOBBYR_SHIPMENT_NUM_ITEMS_Y, BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH,
                   BOBBYR_SHIPMENT_STATIC_TEXT_FONT, BOBBYR_SHIPMENT_STATIC_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}

void DisplayPreviousShipments() {
  uint32_t uiCnt;
  wchar_t zText[512];
  uint16_t usPosY = BOBBYR_SHIPMENT_ORDER_NUM_START_Y;
  uint32_t uiNumItems = CountNumberValidShipmentForTheShipmentsPage();
  uint32_t uiNumberItemsInShipments = 0;
  uint32_t uiItemCnt;
  uint8_t ubFontColor = BOBBYR_SHIPMENT_STATIC_TEXT_COLOR;

  // loop through all the shipments
  for (uiCnt = 0; uiCnt < uiNumItems; uiCnt++) {
    // if it is a valid shipment, and can be displayed at bobby r
    if (gpNewBobbyrShipments[uiCnt].fActive &&
        gpNewBobbyrShipments[giBobbyRShipmentSelectedShipment].fDisplayedInShipmentPage) {
      if (uiCnt == (uint32_t)giBobbyRShipmentSelectedShipment) {
        ubFontColor = FONT_MCOLOR_WHITE;
      } else {
        ubFontColor = BOBBYR_SHIPMENT_STATIC_TEXT_COLOR;
      }

      // Display the "ordered on day num"
      swprintf(zText, ARR_SIZE(zText), L"%s %d", gpGameClockString[0],
               gpNewBobbyrShipments[uiCnt].uiOrderedOnDayNum);
      DrawTextToScreen(zText, BOBBYR_SHIPMENT_ORDER_NUM_X, usPosY, BOBBYR_SHIPMENT_ORDER_NUM_WIDTH,
                       BOBBYR_SHIPMENT_STATIC_TEXT_FONT, ubFontColor, 0, FALSE, CENTER_JUSTIFIED);

      uiNumberItemsInShipments = 0;

      //		for( uiItemCnt=0; uiItemCnt<LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[
      // uiCnt ].ubNumberPurchases; uiItemCnt++ )
      for (uiItemCnt = 0; uiItemCnt < gpNewBobbyrShipments[uiCnt].ubNumberPurchases; uiItemCnt++) {
        //			uiNumberItemsInShipments +=
        // LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ uiCnt
        //].BobbyRayPurchase[uiItemCnt].ubNumberPurchased;
        uiNumberItemsInShipments +=
            gpNewBobbyrShipments[uiCnt].BobbyRayPurchase[uiItemCnt].ubNumberPurchased;
      }

      // Display the # of items
      swprintf(zText, ARR_SIZE(zText), L"%d", uiNumberItemsInShipments);
      DrawTextToScreen(zText, BOBBYR_SHIPMENT_NUM_ITEMS_X, usPosY, BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH,
                       BOBBYR_SHIPMENT_STATIC_TEXT_FONT, ubFontColor, 0, FALSE, CENTER_JUSTIFIED);
      usPosY += BOBBYR_SHIPMENT_GAP_BTN_LINES;
    }
  }
}

void CreatePreviousShipmentsMouseRegions() {
  uint32_t uiCnt;
  uint16_t usPosY = BOBBYR_SHIPMENT_ORDER_NUM_START_Y;
  uint16_t usWidth = BOBBYR_SHIPMENT_DELIVERY_GRID_WIDTH;
  uint16_t usHeight = GetFontHeight(BOBBYR_SHIPMENT_STATIC_TEXT_FONT);
  uint32_t uiNumItems = CountNumberOfBobbyPurchasesThatAreInTransit();

  for (uiCnt = 0; uiCnt < uiNumItems; uiCnt++) {
    MSYS_DefineRegion(&gSelectedPreviousShipmentsRegion[uiCnt], BOBBYR_SHIPMENT_ORDER_NUM_X, usPosY,
                      (uint16_t)(BOBBYR_SHIPMENT_ORDER_NUM_X + usWidth), (uint16_t)(usPosY + usHeight),
                      MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
                      SelectPreviousShipmentsRegionCallBack);
    MSYS_AddRegion(&gSelectedPreviousShipmentsRegion[uiCnt]);
    MSYS_SetRegionUserData(&gSelectedPreviousShipmentsRegion[uiCnt], 0, uiCnt);

    usPosY += BOBBYR_SHIPMENT_GAP_BTN_LINES;
  }
}

void RemovePreviousShipmentsMouseRegions() {
  uint32_t uiCnt;
  uint32_t uiNumItems = CountNumberOfBobbyPurchasesThatAreInTransit();

  for (uiCnt = 0; uiCnt < uiNumItems; uiCnt++) {
    MSYS_RemoveRegion(&gSelectedPreviousShipmentsRegion[uiCnt]);
  }
}

void SelectPreviousShipmentsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    int32_t iSlotID = MSYS_GetRegionUserData(pRegion, 0);

    if (CountNumberOfBobbyPurchasesThatAreInTransit() > iSlotID) {
      int32_t iCnt;
      int32_t iValidShipmentCounter = 0;

      giBobbyRShipmentSelectedShipment = -1;

      // loop through and get the "x" iSlotID shipment
      for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
        if (gpNewBobbyrShipments[iCnt].fActive) {
          if (iValidShipmentCounter == iSlotID) {
            giBobbyRShipmentSelectedShipment = iCnt;
          }

          iValidShipmentCounter++;
        }
      }
    }

    gfBobbyRShipmentsDirty = TRUE;
  }
}

int32_t CountNumberValidShipmentForTheShipmentsPage() {
  if (giNumberOfNewBobbyRShipment > BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS)
    return (BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS);
  else
    return (giNumberOfNewBobbyRShipment);
}
