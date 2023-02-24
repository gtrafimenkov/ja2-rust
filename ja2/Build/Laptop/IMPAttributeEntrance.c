#include "Laptop/IMPAttributeEntrance.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// the buttons
UINT32 giIMPAttributeEntranceButtonImage[1];
UINT32 giIMPAttributeEntranceButton[1];

// function definitions
void DestroyIMPAttributeEntranceButtons(void);
void CreateIMPAttributeEntranceButtons(void);

// callbacks
void BtnIMPAttributeBeginCallback(GUI_BUTTON *btn, INT32 reason);

void EnterIMPAttributeEntrance(void) {
  CreateIMPAttributeEntranceButtons();

  return;
}

void RenderIMPAttributeEntrance(void) {
  // the background
  RenderProfileBackGround();

  // avg merc indent
  RenderAvgMercIndentFrame(90, 40);

  return;
}

void ExitIMPAttributeEntrance(void) {
  // destroy the finish buttons
  DestroyIMPAttributeEntranceButtons();

  return;
}

void HandleIMPAttributeEntrance(void) { return; }

void CreateIMPAttributeEntranceButtons(void) {
  // the begin button
  giIMPAttributeEntranceButtonImage[0] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*
  giIMPAttributeEntranceButton[0] = QuickCreateButton( giIMPAttributeEntranceButtonImage[0],
  LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE,
  MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
  (GUI_CALLBACK)BtnIMPAttributeBeginCallback ); */

  giIMPAttributeEntranceButton[0] = CreateIconAndTextButton(
      giIMPAttributeEntranceButtonImage[0], pImpButtonText[13], FONT12ARIAL, FONT_WHITE,
      DEFAULT_SHADOW, FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (136),
      LAPTOP_SCREEN_WEB_UL_Y + (314), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeBeginCallback);

  SetButtonCursor(giIMPAttributeEntranceButton[0], CURSOR_WWW);
  return;
}

void DestroyIMPAttributeEntranceButtons(void) {
  // this function will destroy the buttons needed for the IMP attrib enter page

  // the begin  button
  RemoveButton(giIMPAttributeEntranceButton[0]);
  UnloadButtonImage(giIMPAttributeEntranceButtonImage[0]);

  return;
}

void BtnIMPAttributeBeginCallback(GUI_BUTTON *btn, INT32 reason) {
  // btn callback for IMP attrbite begin button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      iCurrentImpPage = IMP_ATTRIBUTE_PAGE;
      fButtonPendingFlag = TRUE;
    }
  }
}
