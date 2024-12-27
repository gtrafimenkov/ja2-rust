// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPPersonalityEntrance.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// IMP personality entrance buttons
int32_t giIMPPersonalityEntranceButton[1];
int32_t giIMPPersonalityEntranceButtonImage[1];

// function definitions
void CreateIMPPersonalityEntranceButtons(void);
void DestroyIMPPersonalityEntranceButtons(void);
void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, int32_t reason);

void EnterIMPPersonalityEntrance(void) {
  // create buttons needed
  CreateIMPPersonalityEntranceButtons();

  return;
}

void RenderIMPPersonalityEntrance(void) {
  // the background
  RenderProfileBackGround();

  // the IMP symbol
  // RenderIMPSymbol( 112, 30 );

  // indent
  RenderAvgMercIndentFrame(90, 40);
  return;
}

void ExitIMPPersonalityEntrance(void) {
  // destroy buttons needed
  DestroyIMPPersonalityEntranceButtons();

  return;
}

void HandleIMPPersonalityEntrance(void) { return; }

void CreateIMPPersonalityEntranceButtons(void) {
  // this function will create the buttons needed for the IMP personality Page

  // ths begin button
  giIMPPersonalityEntranceButtonImage[0] =
      LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*giIMPPersonalityEntranceButton[0] = QuickCreateButton( giIMPPersonalityEntranceButtonImage[0],
     LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE,
     MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPPersonalityEntranceDoneCallback);
*/
  giIMPPersonalityEntranceButton[0] = CreateIconAndTextButton(
      giIMPPersonalityEntranceButtonImage[0], pImpButtonText[1], FONT12ARIAL, FONT_WHITE,
      DEFAULT_SHADOW, FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (136),
      LAPTOP_SCREEN_WEB_UL_Y + (314), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityEntranceDoneCallback);

  SetButtonCursor(giIMPPersonalityEntranceButton[0], CURSOR_WWW);

  return;
}

void DestroyIMPPersonalityEntranceButtons(void) {
  // this function will destroy the buttons needed for the IMP personality page

  // the begin button
  RemoveButton(giIMPPersonalityEntranceButton[0]);
  UnloadButtonImage(giIMPPersonalityEntranceButtonImage[0]);

  return;
}

void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP Begin Screen done button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // done with begin screen, next screen
      iCurrentImpPage = IMP_PERSONALITY_QUIZ;
      fButtonPendingFlag = TRUE;
    }
  }
}
