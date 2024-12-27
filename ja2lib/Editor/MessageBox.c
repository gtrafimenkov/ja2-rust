// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/MessageBox.h"

#include "BuildDefines.h"
#include "SGP/ButtonSystem.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/Video.h"
#include "Utils/FontControl.h"

// internal variables.
int32_t iMsgBoxNum;
int32_t iMsgBoxOkImg, iMsgBoxCancelImg;
int32_t iMsgBoxBgrnd, iMsgBoxOk, iMsgBoxCancel;
struct GRect MsgBoxRect;

BOOLEAN gfMessageBoxResult = FALSE;
uint8_t gubMessageBoxStatus = MESSAGEBOX_NONE;

void MsgBoxOkClkCallback(GUI_BUTTON *butn, int32_t reason);
void MsgBoxCnclClkCallback(GUI_BUTTON *butn, int32_t reason);

void CreateMessageBox(wchar_t *wzString) {
  int16_t sPixLen;
  int16_t sStartX, sStartY;

  sPixLen = StringPixLength(wzString, (uint16_t)gpLargeFontType1) + 10;
  if (sPixLen > 600) sPixLen = 600;

  sStartX = (640 - sPixLen) / 2;
  sStartY = (480 - 96) / 2;

  gfMessageBoxResult = FALSE;

  // Fake button for background w/ text
  iMsgBoxBgrnd =
      CreateTextButton(wzString, (uint16_t)gpLargeFontType1, FONT_LTKHAKI, FONT_DKKHAKI,
                       BUTTON_USE_DEFAULT, sStartX, sStartY, sPixLen, 96, BUTTON_NO_TOGGLE,
                       MSYS_PRIORITY_HIGHEST - 2, BUTTON_NO_CALLBACK, BUTTON_NO_CALLBACK);
  DisableButton(iMsgBoxBgrnd);
  SpecifyDisabledButtonStyle(iMsgBoxBgrnd, DISABLED_STYLE_NONE);

  iMsgBoxOkImg = LoadButtonImage("EDITOR//ok.sti", 0, 1, 2, 3, 4);
  iMsgBoxCancelImg = LoadButtonImage("EDITOR//cancel.sti", 0, 1, 2, 3, 4);

  iMsgBoxOk =
      QuickCreateButton(iMsgBoxOkImg, (int16_t)(sStartX + (sPixLen / 2) - 35),
                        (int16_t)(sStartY + 58), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        BUTTON_NO_CALLBACK, (GUI_CALLBACK)MsgBoxOkClkCallback);

  iMsgBoxCancel =
      QuickCreateButton(iMsgBoxCancelImg, (int16_t)(sStartX + (sPixLen / 2) + 5),
                        (int16_t)(sStartY + 58), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        BUTTON_NO_CALLBACK, (GUI_CALLBACK)MsgBoxCnclClkCallback);

  MsgBoxRect.iLeft = sStartX;
  MsgBoxRect.iTop = sStartY;
  MsgBoxRect.iRight = sStartX + sPixLen;
  MsgBoxRect.iBottom = sStartY + 96;

  RestrictMouseCursor(&MsgBoxRect);

  gubMessageBoxStatus = MESSAGEBOX_WAIT;
}

BOOLEAN MessageBoxHandled() {
  InputAtom DummyEvent;

  while (DequeueEvent(&DummyEvent)) {
    if (DummyEvent.usEvent == KEY_DOWN) {
      switch (DummyEvent.usParam) {
        case ENTER:
        case 'y':
        case 'Y':
          gubMessageBoxStatus = MESSAGEBOX_DONE;
          gfMessageBoxResult = TRUE;
          break;
        case ESC:
        case 'n':
        case 'N':
          gubMessageBoxStatus = MESSAGEBOX_DONE;
          gfMessageBoxResult = FALSE;
          break;
      }
    }
  }

  if (gubMessageBoxStatus == MESSAGEBOX_DONE) {
    while (DequeueEvent(&DummyEvent)) continue;
  }
  MarkButtonsDirty();
  RenderButtons();
  EndFrameBufferRender();
  return gubMessageBoxStatus == MESSAGEBOX_DONE;
}

void RemoveMessageBox() {
  FreeMouseCursor();
  RemoveButton(iMsgBoxCancel);
  RemoveButton(iMsgBoxOk);
  RemoveButton(iMsgBoxBgrnd);
  UnloadButtonImage(iMsgBoxOkImg);
  UnloadButtonImage(iMsgBoxCancelImg);
  gubMessageBoxStatus = MESSAGEBOX_NONE;
}

//----------------------------------------------------------------------------------------------
//	Quick Message Box button callback functions.
//----------------------------------------------------------------------------------------------

void MsgBoxOkClkCallback(GUI_BUTTON *butn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    butn->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubMessageBoxStatus = MESSAGEBOX_DONE;
    gfMessageBoxResult = TRUE;
  }
}

void MsgBoxCnclClkCallback(GUI_BUTTON *butn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    butn->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubMessageBoxStatus = MESSAGEBOX_DONE;
    gfMessageBoxResult = FALSE;
  }
}

//----------------------------------------------------------------------------------------------
//	End of the quick message box callback functions
//----------------------------------------------------------------------------------------------
