// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "MessageBoxScreen.h"

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Types.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "SysGlobals.h"
#include "Tactical/Interface.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RenderWorld.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#define MSGBOX_DEFAULT_WIDTH 300

#define MSGBOX_BUTTON_WIDTH 61
#define MSGBOX_BUTTON_HEIGHT 20
#define MSGBOX_BUTTON_X_SEP 15

#define MSGBOX_SMALL_BUTTON_WIDTH 31
#define MSGBOX_SMALL_BUTTON_X_SEP 8

typedef void (*MSGBOX_CALLBACK)(uint8_t bExitValue);

// old mouse x and y positions
SGPPoint pOldMousePosition;
struct GRect MessageBoxRestrictedCursorRegion;

// if the cursor was locked to a region
BOOLEAN fCursorLockedToArea = FALSE;
BOOLEAN gfInMsgBox = FALSE;

// extern BOOLEAN fMapExitDueToMessageBox;
extern BOOLEAN fInMapMode;
extern BOOLEAN gfOverheadMapDirty;

void OKMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void YESMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void ContractMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void LieMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void NOMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void NumberedMsgBoxCallback(GUI_BUTTON *btn, int32_t reason);
void MsgBoxClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
uint32_t ExitMsgBox(int8_t ubExitCode);
uint16_t GetMSgBoxButtonWidth(int32_t iButtonImage);

struct GRect gOldCursorLimitRectangle;

MESSAGE_BOX_STRUCT gMsgBox;
BOOLEAN gfNewMessageBox = FALSE;
BOOLEAN gfStartedFromGameScreen = FALSE;
BOOLEAN gfStartedFromMapScreen = FALSE;
BOOLEAN fRestoreBackgroundForMessageBox = FALSE;
BOOLEAN gfDontOverRideSaveBuffer = TRUE;  // this variable can be unset if ur in a non gamescreen
                                          // and DONT want the msg box to use the save buffer
extern void HandleTacticalUILoseCursorFromOtherScreen();
extern wchar_t* pUpdatePanelButtons[];

wchar_t gzUserDefinedButton1[128];
wchar_t gzUserDefinedButton2[128];

int32_t DoMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen, uint16_t usFlags,
                   MSGBOX_CALLBACK ReturnCallback, const struct GRect *pCenteringRect) {
  uint16_t usTextBoxWidth;
  uint16_t usTextBoxHeight;
  struct GRect aRect;
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;
  int16_t sButtonX, sButtonY, sBlankSpace;
  uint8_t ubMercBoxBackground = BASIC_MERC_POPUP_BACKGROUND,
        ubMercBoxBorder = BASIC_MERC_POPUP_BORDER;
  uint8_t ubFontColor, ubFontShadowColor;
  uint16_t usCursor;
  int32_t iId = -1;

  GetMousePos(&pOldMousePosition);

  // this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save
  // buffer
  gfDontOverRideSaveBuffer = TRUE;

  SetCurrentCursorFromDatabase(CURSOR_NORMAL);

  if (gMsgBox.BackRegion.uiFlags & MSYS_REGION_EXISTS) {
    return (0);
  }

  // Based on style....
  switch (ubStyle) {
    // default
    case MSG_BOX_BASIC_STYLE:

      ubMercBoxBackground = DIALOG_MERC_POPUP_BACKGROUND;
      ubMercBoxBorder = DIALOG_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\popupbuttons.sti", -1, 0, -1, 1, -1);
      ubFontColor = FONT_MCOLOR_WHITE;
      ubFontShadowColor = DEFAULT_SHADOW;
      usCursor = CURSOR_NORMAL;

      break;

    case MSG_BOX_RED_ON_WHITE:
      ubMercBoxBackground = WHITE_MERC_POPUP_BACKGROUND;
      ubMercBoxBorder = RED_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\msgboxRedButtons.sti", -1, 0, -1, 1, -1);

      ubFontColor = 2;
      ubFontShadowColor = NO_SHADOW;
      usCursor = CURSOR_LAPTOP_SCREEN;
      break;

    case MSG_BOX_BLUE_ON_GREY:
      ubMercBoxBackground = GREY_MERC_POPUP_BACKGROUND;
      ubMercBoxBorder = BLUE_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\msgboxGreyButtons.sti", -1, 0, -1, 1, -1);

      ubFontColor = 2;
      ubFontShadowColor = FONT_MCOLOR_WHITE;
      usCursor = CURSOR_LAPTOP_SCREEN;
      break;
    case MSG_BOX_IMP_STYLE:
      ubMercBoxBackground = IMP_POPUP_BACKGROUND;
      ubMercBoxBorder = DIALOG_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\msgboxGreyButtons.sti", -1, 0, -1, 1, -1);

      ubFontColor = 2;
      ubFontShadowColor = FONT_MCOLOR_WHITE;
      usCursor = CURSOR_LAPTOP_SCREEN;
      break;
    case MSG_BOX_BASIC_SMALL_BUTTONS:

      ubMercBoxBackground = DIALOG_MERC_POPUP_BACKGROUND;
      ubMercBoxBorder = DIALOG_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\popupbuttons.sti", -1, 2, -1, 3, -1);
      ubFontColor = FONT_MCOLOR_WHITE;
      ubFontShadowColor = DEFAULT_SHADOW;
      usCursor = CURSOR_NORMAL;

      break;

    case MSG_BOX_LAPTOP_DEFAULT:
      ubMercBoxBackground = LAPTOP_POPUP_BACKGROUND;
      ubMercBoxBorder = LAPTOP_POP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\popupbuttons.sti", -1, 0, -1, 1, -1);
      ubFontColor = FONT_MCOLOR_WHITE;
      ubFontShadowColor = DEFAULT_SHADOW;
      usCursor = CURSOR_LAPTOP_SCREEN;
      break;

    default:
      ubMercBoxBackground = BASIC_MERC_POPUP_BACKGROUND;
      ubMercBoxBorder = BASIC_MERC_POPUP_BORDER;

      // Add button images
      gMsgBox.iButtonImages = LoadButtonImage("INTERFACE\\msgboxbuttons.sti", -1, 0, -1, 1, -1);
      ubFontColor = FONT_MCOLOR_WHITE;
      ubFontShadowColor = DEFAULT_SHADOW;
      usCursor = CURSOR_NORMAL;
      break;
  }

  if (usFlags & MSG_BOX_FLAG_USE_CENTERING_RECT && pCenteringRect != NULL) {
    aRect.iTop = pCenteringRect->iTop;
    aRect.iLeft = pCenteringRect->iLeft;
    aRect.iBottom = pCenteringRect->iBottom;
    aRect.iRight = pCenteringRect->iRight;
  } else {
    // Use default!
    aRect.iTop = 0;
    aRect.iLeft = 0;
    aRect.iBottom = 480;
    aRect.iRight = 640;
  }

  // Set some values!
  gMsgBox.usFlags = usFlags;
  gMsgBox.uiExitScreen = uiExitScreen;
  gMsgBox.ExitCallback = ReturnCallback;
  gMsgBox.fRenderBox = TRUE;
  gMsgBox.bHandled = 0;

  // Init message box
  gMsgBox.iBoxId =
      PrepareMercPopupBox(iId, ubMercBoxBackground, ubMercBoxBorder, zString, MSGBOX_DEFAULT_WIDTH,
                          40, 10, 30, &usTextBoxWidth, &usTextBoxHeight);

  if (gMsgBox.iBoxId == -1) {
#ifdef JA2BETAVERSION
    AssertMsg(0,
              "Failed in DoMessageBox().  Probable reason is because the string was too large to "
              "fit in max message box size.");
#endif
    return 0;
  }

  // Save height,width
  gMsgBox.usWidth = usTextBoxWidth;
  gMsgBox.usHeight = usTextBoxHeight;

  // Determine position ( centered in rect )
  gMsgBox.sX = (int16_t)((((aRect.iRight - aRect.iLeft) - usTextBoxWidth) / 2) + aRect.iLeft);
  gMsgBox.sY = (int16_t)((((aRect.iBottom - aRect.iTop) - usTextBoxHeight) / 2) + aRect.iTop);

  if (IsTacticalMode()) {
    gfStartedFromGameScreen = TRUE;
  }

  if ((fInMapMode == TRUE)) {
    //		fMapExitDueToMessageBox = TRUE;
    gfStartedFromMapScreen = TRUE;
    SetMapPanelDirty(true);
  }

  // Set pending screen
  SetPendingNewScreen(MSG_BOX_SCREEN);

  // Init save buffer
  VSURFACE_DESC vs_desc;
  vs_desc.usWidth = usTextBoxWidth;
  vs_desc.usHeight = usTextBoxHeight;
  if (AddVideoSurface(&vs_desc, &gMsgBox.uiSaveBuffer) == FALSE) {
    return (-1);
  }

  // Save what we have under here...
  pDestBuf = VSurfaceLockOld(GetVSByID(gMsgBox.uiSaveBuffer), &uiDestPitchBYTES);
  pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);

  Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES, 0, 0,
                  NewGRect(gMsgBox.sX, gMsgBox.sY, usTextBoxWidth, usTextBoxHeight));

  VSurfaceUnlock(GetVSByID(gMsgBox.uiSaveBuffer));
  VSurfaceUnlock(vsFB);

  // Create top-level mouse region
  MSYS_DefineRegion(&(gMsgBox.BackRegion), 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, usCursor,
                    MSYS_NO_CALLBACK, MsgBoxClickCallback);

  if (gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE] == FALSE) {
    if (usFlags & MSG_BOX_FLAG_OK) {
      SimulateMouseMovement((gMsgBox.sX + (usTextBoxWidth / 2) + 27),
                            (gMsgBox.sY + (usTextBoxHeight - 10)));
    } else {
      SimulateMouseMovement(gMsgBox.sX + usTextBoxWidth / 2, gMsgBox.sY + usTextBoxHeight - 4);
    }
  }

  // Add region
  MSYS_AddRegion(&(gMsgBox.BackRegion));

  // findout if cursor locked, if so, store old params and store, restore when done
  if (IsCursorRestricted()) {
    fCursorLockedToArea = TRUE;
    GetRestrictedClipCursor(&MessageBoxRestrictedCursorRegion);
    FreeMouseCursor();
  }

  // Create four numbered buttons
  if (usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
    // This is exclusive of any other buttons... no ok, no cancel, no nothing

    sBlankSpace = usTextBoxWidth - MSGBOX_SMALL_BUTTON_WIDTH * 4 - MSGBOX_SMALL_BUTTON_X_SEP * 3;
    sButtonX = sBlankSpace / 2;
    sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

    gMsgBox.uiButton[0] = CreateIconAndTextButton(
        gMsgBox.iButtonImages, L"1", FONT12ARIAL, ubFontColor, ubFontShadowColor, ubFontColor,
        ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
        (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK,
        (GUI_CALLBACK)NumberedMsgBoxCallback);
    MSYS_SetBtnUserData(gMsgBox.uiButton[0], 0, 1);
    SetButtonCursor(gMsgBox.uiButton[0], usCursor);

    sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
    gMsgBox.uiButton[1] = CreateIconAndTextButton(
        gMsgBox.iButtonImages, L"2", FONT12ARIAL, ubFontColor, ubFontShadowColor, ubFontColor,
        ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
        (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK,
        (GUI_CALLBACK)NumberedMsgBoxCallback);
    MSYS_SetBtnUserData(gMsgBox.uiButton[1], 0, 2);
    SetButtonCursor(gMsgBox.uiButton[1], usCursor);

    sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
    gMsgBox.uiButton[2] = CreateIconAndTextButton(
        gMsgBox.iButtonImages, L"3", FONT12ARIAL, ubFontColor, ubFontShadowColor, ubFontColor,
        ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
        (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK,
        (GUI_CALLBACK)NumberedMsgBoxCallback);
    MSYS_SetBtnUserData(gMsgBox.uiButton[2], 0, 3);
    SetButtonCursor(gMsgBox.uiButton[2], usCursor);

    sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
    gMsgBox.uiButton[3] = CreateIconAndTextButton(
        gMsgBox.iButtonImages, L"4", FONT12ARIAL, ubFontColor, ubFontShadowColor, ubFontColor,
        ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
        (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK,
        (GUI_CALLBACK)NumberedMsgBoxCallback);
    MSYS_SetBtnUserData(gMsgBox.uiButton[3], 0, 4);
    SetButtonCursor(gMsgBox.uiButton[3], usCursor);
    ForceButtonUnDirty(gMsgBox.uiButton[3]);
    ForceButtonUnDirty(gMsgBox.uiButton[2]);
    ForceButtonUnDirty(gMsgBox.uiButton[1]);
    ForceButtonUnDirty(gMsgBox.uiButton[0]);

  } else {
    // Create text button
    if (usFlags & MSG_BOX_FLAG_OK) {
      //			sButtonX = ( usTextBoxWidth - MSGBOX_BUTTON_WIDTH ) / 2;
      sButtonX = (usTextBoxWidth - GetMSgBoxButtonWidth(gMsgBox.iButtonImages)) / 2;

      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiOKButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_OK], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)OKMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiOKButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiOKButton);
    }

    // Create text button
    if (usFlags & MSG_BOX_FLAG_CANCEL) {
      sButtonX = (usTextBoxWidth - GetMSgBoxButtonWidth(gMsgBox.iButtonImages)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiOKButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_CANCEL], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)OKMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiOKButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiOKButton);
    }

    if (usFlags & MSG_BOX_FLAG_YESNO) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_YES], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);

      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_NO], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);
    }

    if (usFlags & MSG_BOX_FLAG_CONTINUESTOP) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pUpdatePanelButtons[0], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);

      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pUpdatePanelButtons[1], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);
    }

    if (usFlags & MSG_BOX_FLAG_OKCONTRACT) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_OK], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)OKMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);

      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_REHIRE], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)ContractMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);
    }

    if (usFlags & MSG_BOX_FLAG_YESNOCONTRACT) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 3;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_YES], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_NO], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);

      gMsgBox.uiOKButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_REHIRE], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + 2 * (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)ContractMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiOKButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiOKButton);
    }

    if (usFlags & MSG_BOX_FLAG_GENERICCONTRACT) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 3;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, gzUserDefinedButton1, FONT12ARIAL, ubFontColor, ubFontShadowColor,
          ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, gzUserDefinedButton2, FONT12ARIAL, ubFontColor, ubFontShadowColor,
          ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);

      gMsgBox.uiOKButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_REHIRE], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + 2 * (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)ContractMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiOKButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiOKButton);
    }

    if (usFlags & MSG_BOX_FLAG_GENERIC) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, gzUserDefinedButton1, FONT12ARIAL, ubFontColor, ubFontShadowColor,
          ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED, (int16_t)(gMsgBox.sX + sButtonX),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, gzUserDefinedButton2, FONT12ARIAL, ubFontColor, ubFontShadowColor,
          ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);
    }

    if (usFlags & MSG_BOX_FLAG_YESNOLIE) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 3;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_YES], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_NO], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);

      gMsgBox.uiOKButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_LIE], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + 2 * (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)LieMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiOKButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiOKButton);
    }

    if (usFlags & MSG_BOX_FLAG_OKSKIP) {
      sButtonX =
          (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)) / 2;
      sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

      gMsgBox.uiYESButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_OK], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX), (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)YESMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiYESButton, usCursor);

      ForceButtonUnDirty(gMsgBox.uiYESButton);

      gMsgBox.uiNOButton = CreateIconAndTextButton(
          gMsgBox.iButtonImages, pMessageStrings[MSG_SKIP], FONT12ARIAL, ubFontColor,
          ubFontShadowColor, ubFontColor, ubFontShadowColor, TEXT_CJUSTIFIED,
          (int16_t)(gMsgBox.sX + sButtonX + (MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP)),
          (int16_t)(gMsgBox.sY + sButtonY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
          DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)NOMsgBoxCallback);
      SetButtonCursor(gMsgBox.uiNOButton, usCursor);
      ForceButtonUnDirty(gMsgBox.uiNOButton);
    }
  }

  InterruptTime();
  PauseGame();
  LockPause();
  // Pause timers as well....
  PauseTime(TRUE);

  // Save mouse restriction region...
  GetRestrictedClipCursor(&gOldCursorLimitRectangle);
  FreeMouseCursor();

  gfNewMessageBox = TRUE;

  gfInMsgBox = TRUE;

  return (iId);
}

void MsgBoxClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  /// if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
  //{
  //	gMsgBox.bHandled = MSG_BOX_RETURN_NO;
  //}
  //
}

void OKMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    fLButtonDown = TRUE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_UP) && fLButtonDown) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = MSG_BOX_RETURN_OK;
  } else if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void YESMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    fLButtonDown = TRUE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_UP) && fLButtonDown) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = MSG_BOX_RETURN_YES;
  } else if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void NOMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    fLButtonDown = TRUE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_UP) && fLButtonDown) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = MSG_BOX_RETURN_NO;
  } else if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void ContractMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    fLButtonDown = TRUE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_UP) && fLButtonDown) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = MSG_BOX_RETURN_CONTRACT;
  } else if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void LieMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    fLButtonDown = TRUE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_UP) && fLButtonDown) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = MSG_BOX_RETURN_LIE;
  } else if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void NumberedMsgBoxCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    gMsgBox.bHandled = (int8_t)MSYS_GetBtnUserData(btn, 0);
  }
}

uint32_t ExitMsgBox(int8_t ubExitCode) {
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;
  SGPPoint pPosition;

  // Delete popup!
  RemoveMercPopupBoxFromIndex(gMsgBox.iBoxId);
  gMsgBox.iBoxId = -1;

  // Delete buttons!
  if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
    RemoveButton(gMsgBox.uiButton[0]);
    RemoveButton(gMsgBox.uiButton[1]);
    RemoveButton(gMsgBox.uiButton[2]);
    RemoveButton(gMsgBox.uiButton[3]);
  } else {
    if (gMsgBox.usFlags & MSG_BOX_FLAG_OK) {
      RemoveButton(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNO) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_OKCONTRACT) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNOCONTRACT) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
      RemoveButton(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_GENERICCONTRACT) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
      RemoveButton(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_GENERIC) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNOLIE) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
      RemoveButton(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_OKSKIP) {
      RemoveButton(gMsgBox.uiYESButton);
      RemoveButton(gMsgBox.uiNOButton);
    }
  }

  // Delete button images
  UnloadButtonImage(gMsgBox.iButtonImages);

  // Unpause game....
  UnlockPause();
  UnPauseGame();
  // UnPause timers as well....
  PauseTime(FALSE);

  // Restore mouse restriction region...
  RestrictMouseCursor(&gOldCursorLimitRectangle);

  gfInMsgBox = FALSE;

  // Call done callback!
  if (gMsgBox.ExitCallback != NULL) {
    (*(gMsgBox.ExitCallback))(ubExitCode);
  }

  // if ur in a non gamescreen and DONT want the msg box to use the save buffer, unset
  // gfDontOverRideSaveBuffer in ur callback
  if (((gMsgBox.uiExitScreen != GAME_SCREEN) || (fRestoreBackgroundForMessageBox == TRUE)) &&
      gfDontOverRideSaveBuffer) {
    // restore what we have under here...
    pSrcBuf = VSurfaceLockOld(GetVSByID(gMsgBox.uiSaveBuffer), &uiSrcPitchBYTES);
    pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);

    Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES,
                    gMsgBox.sX, gMsgBox.sY, NewGRect(0, 0, gMsgBox.usWidth, gMsgBox.usHeight));

    VSurfaceUnlock(GetVSByID(gMsgBox.uiSaveBuffer));
    VSurfaceUnlock(vsFB);

    InvalidateRegion(gMsgBox.sX, gMsgBox.sY, (int16_t)(gMsgBox.sX + gMsgBox.usWidth),
                     (int16_t)(gMsgBox.sY + gMsgBox.usHeight));
  }

  fRestoreBackgroundForMessageBox = FALSE;
  gfDontOverRideSaveBuffer = TRUE;

  if (fCursorLockedToArea == TRUE) {
    GetMousePos(&pPosition);

    if ((pPosition.iX > MessageBoxRestrictedCursorRegion.iRight) ||
        ((pPosition.iX > MessageBoxRestrictedCursorRegion.iLeft) &&
         (pPosition.iY < MessageBoxRestrictedCursorRegion.iTop) &&
         (pPosition.iY > MessageBoxRestrictedCursorRegion.iBottom))) {
      SimulateMouseMovement(pOldMousePosition.iX, pOldMousePosition.iY);
    }

    fCursorLockedToArea = FALSE;
    RestrictMouseCursor(&MessageBoxRestrictedCursorRegion);
  }

  // Remove region
  MSYS_RemoveRegion(&(gMsgBox.BackRegion));

  // Remove save buffer!
  DeleteVideoSurfaceFromIndex(gMsgBox.uiSaveBuffer);

  switch (gMsgBox.uiExitScreen) {
    case GAME_SCREEN:

      if (InOverheadMap()) {
        gfOverheadMapDirty = TRUE;
      } else {
        SetRenderFlags(RENDER_FLAG_FULL);
      }
      break;
    case MAP_SCREEN:
      SetMapPanelDirty(true);
      break;
  }

  if (gfFadeInitialized) {
    SetPendingNewScreen(FADE_SCREEN);
    return (FADE_SCREEN);
  }

  return (gMsgBox.uiExitScreen);
}

uint32_t MessageBoxScreenInit() { return (TRUE); }

uint32_t MessageBoxScreenHandle() {
  InputAtom InputEvent;

  if (gfNewMessageBox) {
    // If in game screen....
    if ((gfStartedFromGameScreen) || (gfStartedFromMapScreen)) {
      // uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
      // uint8_t	 *pDestBuf, *pSrcBuf;

      if (gfStartedFromGameScreen) {
        HandleTacticalUILoseCursorFromOtherScreen();
      } else {
        HandleMAPUILoseCursorFromOtherScreen();
      }

      gfStartedFromGameScreen = FALSE;
      gfStartedFromMapScreen = FALSE;
    }

    gfNewMessageBox = FALSE;

    return (MSG_BOX_SCREEN);
  }

  UnmarkButtonsDirty();

  // Render the box!
  if (gMsgBox.fRenderBox) {
    if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
      MarkAButtonDirty(gMsgBox.uiButton[0]);
      MarkAButtonDirty(gMsgBox.uiButton[1]);
      MarkAButtonDirty(gMsgBox.uiButton[2]);
      MarkAButtonDirty(gMsgBox.uiButton[3]);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_OK) {
      MarkAButtonDirty(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_CANCEL) {
      MarkAButtonDirty(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNO) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_OKCONTRACT) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNOCONTRACT) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
      MarkAButtonDirty(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_GENERICCONTRACT) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
      MarkAButtonDirty(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_GENERIC) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP) {
      // Exit messagebox
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNOLIE) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
      MarkAButtonDirty(gMsgBox.uiOKButton);
    }

    if (gMsgBox.usFlags & MSG_BOX_FLAG_OKSKIP) {
      MarkAButtonDirty(gMsgBox.uiYESButton);
      MarkAButtonDirty(gMsgBox.uiNOButton);
    }

    RenderMercPopUpBoxFromIndex(gMsgBox.iBoxId, gMsgBox.sX, gMsgBox.sY, FRAME_BUFFER);
    // gMsgBox.fRenderBox = FALSE;
    // ATE: Render each frame...
  }

  // Render buttons
  RenderButtons();

  EndFrameBufferRender();

  // carter, need key shortcuts for clearing up message boxes
  // Check for esc
  while (DequeueEvent(&InputEvent) == TRUE) {
    if (InputEvent.usEvent == KEY_UP) {
      if ((InputEvent.usParam == ESC) || (InputEvent.usParam == 'n')) {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNO) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_NO;
        }
      }

      if (InputEvent.usParam == ENTER) {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNO) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_YES;
        } else if (gMsgBox.usFlags & MSG_BOX_FLAG_OK) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_OK;
        } else if (gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_OK;
        }
      }
      if (InputEvent.usParam == 'o') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_OK) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_OK;
        }
      }
      if (InputEvent.usParam == 'y') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_YESNO) {
          // Exit messagebox
          gMsgBox.bHandled = MSG_BOX_RETURN_YES;
        }
      }
      if (InputEvent.usParam == '1') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
          // Exit messagebox
          gMsgBox.bHandled = 1;
        }
      }
      if (InputEvent.usParam == '2') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
          // Exit messagebox
          gMsgBox.bHandled = 1;
        }
      }
      if (InputEvent.usParam == '3') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
          // Exit messagebox
          gMsgBox.bHandled = 1;
        }
      }
      if (InputEvent.usParam == '4') {
        if (gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS) {
          // Exit messagebox
          gMsgBox.bHandled = 1;
        }
      }
    }
  }

  if (gMsgBox.bHandled) {
    SetRenderFlags(RENDER_FLAG_FULL);
    return (ExitMsgBox(gMsgBox.bHandled));
  }

  return (MSG_BOX_SCREEN);
}

uint32_t MessageBoxScreenShutdown() { return (FALSE); }

// a basic box that don't care what screen we came from
void DoScreenIndependantMessageBox(wchar_t *zString, uint16_t usFlags,
                                   MSGBOX_CALLBACK ReturnCallback) {
  DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, GetMapCenteringRect());
}

// a basic box that don't care what screen we came from
void DoUpperScreenIndependantMessageBox(wchar_t *zString, uint16_t usFlags,
                                        MSGBOX_CALLBACK ReturnCallback) {
  struct GRect CenteringRect = {0, 0, 640, INV_INTERFACE_START_Y / 2};
  DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, &CenteringRect);
}

// a basic box that don't care what screen we came from
void DoLowerScreenIndependantMessageBox(wchar_t *zString, uint16_t usFlags,
                                        MSGBOX_CALLBACK ReturnCallback) {
  struct GRect CenteringRect = {0, INV_INTERFACE_START_Y / 2, 640, INV_INTERFACE_START_Y};
  DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, &CenteringRect);
}

void DoScreenIndependantMessageBoxWithRect(wchar_t *zString, uint16_t usFlags,
                                           MSGBOX_CALLBACK ReturnCallback,
                                           const struct GRect *pCenteringRect) {
  /// which screen are we in?

  // Map Screen (excluding AI Viewer)
#ifdef JA2BETAVERSION
  if ((IsMapScreen()) && (guiCurrentScreen != AIVIEWER_SCREEN))
#else
  if ((IsMapScreen()))
#endif
  {

    // auto resolve is a special case
    if (guiCurrentScreen == AUTORESOLVE_SCREEN) {
      DoMessageBox(MSG_BOX_BASIC_STYLE, zString, AUTORESOLVE_SCREEN, usFlags, ReturnCallback,
                   pCenteringRect);
    } else {
      // set up for mapscreen
      DoMapMessageBoxWithRect(MSG_BOX_BASIC_STYLE, zString, MAP_SCREEN, usFlags, ReturnCallback,
                              pCenteringRect);
    }
  }

  // Laptop
  else if (guiCurrentScreen == LAPTOP_SCREEN) {
    // set up for laptop
    DoLapTopSystemMessageBoxWithRect(MSG_BOX_LAPTOP_DEFAULT, zString, LAPTOP_SCREEN, usFlags,
                                     ReturnCallback, pCenteringRect);
  }

  // Save Load Screen
  else if (guiCurrentScreen == SAVE_LOAD_SCREEN) {
    DoSaveLoadMessageBoxWithRect(MSG_BOX_BASIC_STYLE, zString, SAVE_LOAD_SCREEN, usFlags,
                                 ReturnCallback, pCenteringRect);
  }

  // Options Screen
  else if (guiCurrentScreen == OPTIONS_SCREEN) {
    DoOptionsMessageBoxWithRect(MSG_BOX_BASIC_STYLE, zString, OPTIONS_SCREEN, usFlags,
                                ReturnCallback, pCenteringRect);
  }

  // Tactical
  else if (IsTacticalMode()) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, zString, guiCurrentScreen, usFlags, ReturnCallback,
                 pCenteringRect);
  }
}

uint16_t GetMSgBoxButtonWidth(int32_t iButtonImage) {
  return (GetWidthOfButtonPic((uint16_t)iButtonImage, ButtonPictures[iButtonImage].OnNormal));
}
