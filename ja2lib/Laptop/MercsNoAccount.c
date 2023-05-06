#include "Laptop/MercsNoAccount.h"

#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Laptop/Mercs.h"
#include "Laptop/SpeckQuotes.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define MERC_NA_TEXT_FONT FONT12ARIAL
#define MERC_NA_TEXT_COLOR FONT_MCOLOR_WHITE

#define MERC_NO_ACCOUNT_IMAGE_X LAPTOP_SCREEN_UL_X + 23
#define MERC_NO_ACCOUNT_IMAGE_Y LAPTOP_SCREEN_UL_Y + 52

#define MERC_OPEN_BUTTON_X 130
#define MERC_OPEN_BUTTON_Y 380

#define MERC_CANCEL_BUTTON_X 490
#define MERC_CANCEL_BUTTON_Y MERC_OPEN_BUTTON_Y

#define MERC_NA_SENTENCE_X MERC_NO_ACCOUNT_IMAGE_X + 10
#define MERC_NA_SENTENCE_Y MERC_NO_ACCOUNT_IMAGE_Y + 75
#define MERC_NA_SENTENCE_WIDTH 460 - 20

UINT32 guiNoAccountImage;

// The Open Account Box button
void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32 guiOpenAccountBoxButton;
INT32 guiOpenAccountBoxButtonImage;

// The Cancel Account Box button
void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32 guiCancelBoxButton;

void GameInitMercsNoAccount() {}

BOOLEAN EnterMercsNoAccount() {
  InitMercBackGround();

  // load the Account box graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\NoAccountBox.sti", &guiNoAccountImage)) {
    return FALSE;
  }

  // Open Accouint button
  guiOpenAccountBoxButtonImage = LoadButtonImage("LAPTOP\\BigButtons.sti", -1, 0, -1, 1, -1);

  guiOpenAccountBoxButton = CreateIconAndTextButton(
      guiOpenAccountBoxButtonImage, MercNoAccountText[MERC_NO_ACC_OPEN_ACCOUNT], FONT12ARIAL,
      MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW, MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      MERC_OPEN_BUTTON_X, MERC_OPEN_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnOpenAccountBoxButtonCallback);
  SetButtonCursor(guiOpenAccountBoxButton, CURSOR_LAPTOP_SCREEN);
  SpecifyDisabledButtonStyle(guiOpenAccountBoxButton, DISABLED_STYLE_SHADED);

  guiCancelBoxButton = CreateIconAndTextButton(
      guiOpenAccountBoxButtonImage, MercNoAccountText[MERC_NO_ACC_CANCEL], FONT12ARIAL,
      MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW, MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      MERC_CANCEL_BUTTON_X, MERC_CANCEL_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnCancelBoxButtonCallback);
  SetButtonCursor(guiCancelBoxButton, CURSOR_LAPTOP_SCREEN);
  SpecifyDisabledButtonStyle(guiCancelBoxButton, DISABLED_STYLE_SHADED);

  RenderMercsNoAccount();

  return (TRUE);
}

void ExitMercsNoAccount() {
  DeleteVideoObjectFromIndex(guiNoAccountImage);

  UnloadButtonImage(guiOpenAccountBoxButtonImage);
  RemoveButton(guiOpenAccountBoxButton);
  RemoveButton(guiCancelBoxButton);

  RemoveMercBackGround();
}

void HandleMercsNoAccount() {}

void RenderMercsNoAccount() {
  struct VObject *hPixHandle;

  DrawMecBackGround();

  // Title
  GetVideoObject(&hPixHandle, guiNoAccountImage);
  BltVObjectSrcTrans(vsFB, hPixHandle, 0, MERC_NO_ACCOUNT_IMAGE_X, MERC_NO_ACCOUNT_IMAGE_Y);

  // Display the sentence
  DisplayWrappedString(MERC_NA_SENTENCE_X, MERC_NA_SENTENCE_Y, MERC_NA_SENTENCE_WIDTH, 2,
                       MERC_NA_TEXT_FONT, MERC_NA_TEXT_COLOR,
                       MercNoAccountText[MERC_NO_ACC_NO_ACCOUNT_OPEN_ONE], FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      // open an account
      LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID;

      // Get an account number
      LaptopSaveInfo.guiPlayersMercAccountNumber = Random(99999);

      gusMercVideoSpeckSpeech = SPECK_QUOTE_THANK_PLAYER_FOR_OPENING_ACCOUNT;

      guiCurrentLaptopMode = LAPTOP_MODE_MERC;
      gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_MERC;
      gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}
