#include "Laptop/InsuranceContract.h"

#include <stdio.h>

#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Insurance.h"
#include "Laptop/InsuranceText.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Money.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/StrategicStatus.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierProfile.h"
#include "Utils/Cursors.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define INS_CTRCT_ORDER_GRID_WIDTH 132
#define INS_CTRCT_ORDER_GRID_HEIGHT 216
#define INS_CTRCT_ORDER_GRID_OFFSET_X INS_CTRCT_ORDER_GRID_WIDTH + 2

#define INS_CTRCT_ORDER_GRID1_X 76 + LAPTOP_SCREEN_UL_X
#define INS_CTRCT_ORDER_GRID1_Y 126 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_ORDER_GRID2_X INS_CTRCT_ORDER_GRID1_X + INS_CTRCT_ORDER_GRID_OFFSET_X

#define INS_CTRCT_ORDER_GRID3_X INS_CTRCT_ORDER_GRID2_X + INS_CTRCT_ORDER_GRID_OFFSET_X

#define INS_CTRCT_OG_FACE_OFFSET_X 5
#define INS_CTRCT_OG_FACE_OFFSET_Y 4

#define INS_CTRCT_OG_NICK_NAME_OFFSET_X 57
#define INS_CTRCT_OG_NICK_NAME_OFFSET_Y 13

#define INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X INS_CTRCT_OG_NICK_NAME_OFFSET_X
#define INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y INS_CTRCT_OG_NICK_NAME_OFFSET_Y + 13

#define INS_CTRCT_TITLE_Y (48 + LAPTOP_SCREEN_WEB_UL_Y)  // 52 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_FIRST_BULLET_TEXT_X 86 + LAPTOP_SCREEN_UL_X
#define INS_CTRCT_FIRST_BULLET_TEXT_Y 65 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_SECOND_BULLET_TEXT_X INS_CTRCT_FIRST_BULLET_TEXT_X
#define INS_CTRCT_SECOND_BULLET_TEXT_Y 93 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_INTSRUCTION_TEXT_WIDTH 375

#define INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y 123 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X 4
#define INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_Y 54

#define INS_CTRCT_LENGTH_OFFSET_X INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X
#define INS_CTRCT_LENGTH_OFFSET_Y 71

#define INS_CTRCT_DAYS_REMAINING_OFFSET_Y 87

#define INS_CTRCT_INSURANCE_CNTRCT_OFFSET_Y 108

#define INS_CTRCT_PREMIUM_OWING_OFFSET_Y 160

#define INS_CTRCT_OG_BOX_OFFSET_X 92
#define INS_CTRCT_OG_BOX_WIDTH 35

#define INS_CTRCT_ACCEPT_BTN_X (132 / 2 - 43 / 2)  // 6
#define INS_CTRCT_ACCEPT_BTN_Y 193

#define INS_CTRCT_CLEAR_BTN_X 86

#define INS_CTRCT_BOTTON_LINK_Y 351 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CTRCT_BOTTOM_LINK_RED_BAR_X 171 + LAPTOP_SCREEN_UL_X
#define INS_CTRCT_BOTTON_LINK_RED_BAR_Y INS_CTRCT_BOTTON_LINK_Y + 41

#define INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET 117

#define INS_CTRCT_BOTTOM_LINK_RED_WIDTH 97

#define INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH 74

// this is the percentage of daily salary used as a base to calculate daily insurance premiums
#define INSURANCE_PREMIUM_RATE 5

#define INS_CTRCT_SKILL_BASE 42
#define INS_CTRCT_FITNESS_BASE 85
#define INS_CTRCT_EXP_LEVEL_BASE 3
#define INS_CTRCT_SURVIVAL_BASE 90

uint32_t guiInsOrderGridImage;
uint32_t guiInsOrderBulletImage;

int16_t gsForm1InsuranceLengthNumber;
int16_t gsForm2InsuranceLengthNumber;
int16_t gsForm3InsuranceLengthNumber;

uint8_t gubMercIDForMercInForm1;
uint8_t gubMercIDForMercInForm2;
uint8_t gubMercIDForMercInForm3;

uint8_t gubNumberofDisplayedInsuranceGrids;

BOOLEAN gfChangeInsuranceFormButtons = FALSE;

uint8_t gubInsuranceMercArray[20];
int16_t gsCurrentInsuranceMercIndex;
int16_t gsMaxPlayersOnTeam;

// link to the varios pages
struct MOUSE_REGION gSelectedInsuranceContractLinkRegion[2];
void SelectInsuranceContractRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

int32_t guiInsContractPrevButtonImage;
void BtnInsContractPrevButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsContractPrevBackButton;

int32_t guiInsContractNextButtonImage;
void BtnInsContractNextButtonCallBack(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsContractNextBackButton;

// Graphic for Accept, Clear button for form 1
int32_t guiInsuranceAcceptClearForm1ButtonImage;
void BtnInsuranceAcceptClearForm1ButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsuranceAcceptClearForm1Button;

// Graphic for Accept, Clear button for form 2
void BtnInsuranceAcceptClearForm2ButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsuranceAcceptClearForm2Button;

// Graphic for Accept, Clear button for form 3
void BtnInsuranceAcceptClearForm3ButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsuranceAcceptClearForm3Button;

//
//	Function Prototypes
//
BOOLEAN DisplayOrderGrid(uint8_t ubGridNumber, uint8_t ubMercID);
int8_t GetNumberOfHireMercsStartingFromID(uint8_t ubStartMercID);
// int32_t			CalculateInsuranceCost( struct SOLDIERTYPE *pSoldier, BOOLEAN
// fHaveInsurance
// );
void InsuranceContractUserTextFieldCallBack(uint8_t ubID, BOOLEAN fEntering);
int8_t CountInsurableMercs();
void DisableInsuranceContractNextPreviousbuttons();
void CreateDestroyInsuranceContractFormButtons(BOOLEAN fCreate);
void HandleAcceptButton(uint8_t ubSoldierID, uint8_t ubFormID);
float DiffFromNormRatio(int16_t sThisValue, int16_t sNormalValue);
void InsContractNoMercsPopupCallBack(uint8_t bExitValue);
void BuildInsuranceArray();
BOOLEAN MercIsInsurable(struct SOLDIERTYPE *pSoldier);
// uint32_t		GetContractLengthForFormNumber( uint8_t ubFormID );
void EnableDisableInsuranceContractAcceptButtons();
uint32_t GetTimeRemainingOnSoldiersContract(struct SOLDIERTYPE *pSoldier);
uint32_t GetTimeRemainingOnSoldiersInsuranceContract(struct SOLDIERTYPE *pSoldier);
void EnableDisableIndividualInsuranceContractButton(uint8_t ubMercIDForMercInForm1,
                                                    uint32_t *puiAcceptButton);
BOOLEAN CanSoldierExtendInsuranceContract(struct SOLDIERTYPE *pSoldier);
int32_t CalculateSoldiersInsuranceContractLength(struct SOLDIERTYPE *pSoldier);
int32_t CalcStartDayOfInsurance(struct SOLDIERTYPE *pSoldier);

BOOLEAN AreAnyAimMercsOnTeam();
// ppp

void GameInitInsuranceContract() {
  gsCurrentInsuranceMercIndex = gTacticalStatus.Team[gbPlayerNum].bFirstID;
}

void EnterLaptopInitInsuranceContract() {
  wchar_t zTextField[14];

  swprintf(zTextField, ARR_SIZE(zTextField), L"%d", 0);
  SetInputFieldStringWith16BitString(1, zTextField);
  SetInputFieldStringWith16BitString(2, zTextField);
  SetInputFieldStringWith16BitString(3, zTextField);
}

BOOLEAN EnterInsuranceContract() {
  uint16_t usPosX, i;

  // build the list of mercs that are can be displayed
  BuildInsuranceArray();

  gubNumberofDisplayedInsuranceGrids =
      GetNumberOfHireMercsStartingFromID((uint8_t)gsCurrentInsuranceMercIndex);
  if (gubNumberofDisplayedInsuranceGrids > 3) gubNumberofDisplayedInsuranceGrids = 3;

  InitInsuranceDefaults();

  // load the Insurance title graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\InsOrderGrid.sti", &guiInsOrderGridImage)) {
    return FALSE;
  }

  // load the Insurance bullet graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\bullet.sti", &guiInsOrderBulletImage)) {
    return FALSE;
  }

  usPosX = INS_CTRCT_BOTTOM_LINK_RED_BAR_X;
  for (i = 0; i < 2; i++) {
    MSYS_DefineRegion(
        &gSelectedInsuranceContractLinkRegion[i], usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y - 37,
        (uint16_t)(usPosX + INS_CTRCT_BOTTOM_LINK_RED_WIDTH), INS_CTRCT_BOTTON_LINK_RED_BAR_Y + 2,
        MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceContractRegionCallBack);
    MSYS_AddRegion(&gSelectedInsuranceContractLinkRegion[i]);
    MSYS_SetRegionUserData(&gSelectedInsuranceContractLinkRegion[i], 0, i);

    usPosX += INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET;
  }

  // left arrow
  guiInsContractPrevButtonImage = LoadButtonImage("LAPTOP\\InsLeftButton.sti", 2, 0, -1, 1, -1);
  guiInsContractPrevBackButton = CreateIconAndTextButton(
      guiInsContractPrevButtonImage, InsContractText[INS_CONTRACT_PREVIOUS], INS_FONT_BIG,
      INS_FONT_COLOR, INS_FONT_SHADOW, INS_FONT_COLOR, INS_FONT_SHADOW, TEXT_CJUSTIFIED,
      INS_INFO_LEFT_ARROW_BUTTON_X, INS_INFO_LEFT_ARROW_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnInsContractPrevButtonCallback);
  SetButtonCursor(guiInsContractPrevBackButton, CURSOR_WWW);
  SpecifyButtonTextOffsets(guiInsContractPrevBackButton, 17, 16, FALSE);

  // Right arrow
  guiInsContractNextButtonImage = LoadButtonImage("LAPTOP\\InsRightButton.sti", 2, 0, -1, 1, -1);
  guiInsContractNextBackButton = CreateIconAndTextButton(
      guiInsContractNextButtonImage, InsContractText[INS_CONTRACT_NEXT], INS_FONT_BIG,
      INS_FONT_COLOR, INS_FONT_SHADOW, INS_FONT_COLOR, INS_FONT_SHADOW, TEXT_CJUSTIFIED,
      INS_INFO_RIGHT_ARROW_BUTTON_X, INS_INFO_RIGHT_ARROW_BUTTON_Y, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnInsContractNextButtonCallBack);
  SetButtonCursor(guiInsContractNextBackButton, CURSOR_WWW);
  SpecifyButtonTextOffsets(guiInsContractNextBackButton, 18, 16, FALSE);

  // create the new set of buttons
  CreateDestroyInsuranceContractFormButtons(TRUE);

  //	RenderInsuranceContract();
  return (TRUE);
}

void ExitInsuranceContract() {
  uint8_t i;

  RemoveInsuranceDefaults();

  DeleteVideoObjectFromIndex(guiInsOrderGridImage);

  DeleteVideoObjectFromIndex(guiInsOrderBulletImage);

  for (i = 0; i < 2; i++) MSYS_RemoveRegion(&gSelectedInsuranceContractLinkRegion[i]);

  // the previous button
  UnloadButtonImage(guiInsContractPrevButtonImage);
  RemoveButton(guiInsContractPrevBackButton);

  // the next button
  UnloadButtonImage(guiInsContractNextButtonImage);
  RemoveButton(guiInsContractNextBackButton);

  CreateDestroyInsuranceContractFormButtons(FALSE);
}

void HandleInsuranceContract() {
  if (gfChangeInsuranceFormButtons) {
    // remove the old buttons from the page
    CreateDestroyInsuranceContractFormButtons(FALSE);

    // Get the new number of displayed insurance grids
    gubNumberofDisplayedInsuranceGrids =
        GetNumberOfHireMercsStartingFromID((uint8_t)gsCurrentInsuranceMercIndex);
    if (gubNumberofDisplayedInsuranceGrids > 3) gubNumberofDisplayedInsuranceGrids = 3;

    // create the new set of buttons
    CreateDestroyInsuranceContractFormButtons(TRUE);

    // reset the flag
    gfChangeInsuranceFormButtons = FALSE;

    // force a redraw of the screen to erase the old buttons
    fPausedReDrawScreenFlag = TRUE;
    RenderInsuranceContract();

    MarkButtonsDirty();
  }

  EnableDisableInsuranceContractAcceptButtons();
}

void RenderInsuranceContract() {
  struct VObject *hPixHandle;
  wchar_t sText[800];
  uint8_t ubCount = 0;
  int16_t sMercID;
  int16_t sNextMercID;
  uint16_t usPosX;
  struct SOLDIERTYPE *pSoldier = NULL;

  SetFontShadow(INS_FONT_SHADOW);

  DisplayInsuranceDefaults();

  // disable the next or previous button depending on how many more mercs we have to display
  DisableInsuranceContractNextPreviousbuttons();

  usPosX = INS_CTRCT_BOTTOM_LINK_RED_BAR_X;

  // Display the red bar under the link at the bottom.  and the text
  DisplaySmallRedLineWithShadow(usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y,
                                (uint16_t)(usPosX + INS_CTRCT_BOTTOM_LINK_RED_WIDTH),
                                INS_CTRCT_BOTTON_LINK_RED_BAR_Y);
  swprintf(sText, ARR_SIZE(sText), L"%s", pMessageStrings[MSG_HOMEPAGE]);
  DisplayWrappedString(usPosX, INS_CTRCT_BOTTON_LINK_Y + 18, INS_CTRCT_BOTTOM_LINK_RED_WIDTH, 2,
                       INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  usPosX += INS_CTRCT_BOTTOM_LINK_RED_BAR_OFFSET;

  // Display the red bar under the link at the bottom.  and the text
  DisplaySmallRedLineWithShadow(usPosX, INS_CTRCT_BOTTON_LINK_RED_BAR_Y,
                                (uint16_t)(usPosX + INS_CTRCT_BOTTOM_LINK_RED_WIDTH),
                                INS_CTRCT_BOTTON_LINK_RED_BAR_Y);
  GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK, sText);
  DisplayWrappedString(usPosX, INS_CTRCT_BOTTON_LINK_Y + 12, INS_CTRCT_BOTTOM_LINK_RED_WIDTH, 2,
                       INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  // Display the title slogan
  GetInsuranceText(INS_SNGL_ENTERING_REVIEWING_CLAIM, sText);
  DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INS_CTRCT_TITLE_Y,
                   LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiInsOrderBulletImage);
  BltVideoObject2(vsFB, hPixHandle, 0, INS_CTRCT_FIRST_BULLET_TEXT_X, INS_CTRCT_FIRST_BULLET_TEXT_Y,
                  VO_BLT_SRCTRANSPARENCY, NULL);

  // Display the first instruction sentence
  GetInsuranceText(INS_MLTI_TO_PURCHASE_INSURANCE, sText);
  DisplayWrappedString(INS_CTRCT_FIRST_BULLET_TEXT_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                       INS_CTRCT_FIRST_BULLET_TEXT_Y, INS_CTRCT_INTSRUCTION_TEXT_WIDTH, 2,
                       INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       LEFT_JUSTIFIED);

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiInsOrderBulletImage);
  BltVideoObject2(vsFB, hPixHandle, 0, INS_CTRCT_FIRST_BULLET_TEXT_X,
                  INS_CTRCT_SECOND_BULLET_TEXT_Y, VO_BLT_SRCTRANSPARENCY, NULL);

  // Display the second instruction sentence
  GetInsuranceText(INS_MLTI_ONCE_SATISFIED_CLICK_ACCEPT, sText);
  DisplayWrappedString(INS_CTRCT_FIRST_BULLET_TEXT_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                       INS_CTRCT_SECOND_BULLET_TEXT_Y, INS_CTRCT_INTSRUCTION_TEXT_WIDTH, 2,
                       INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       LEFT_JUSTIFIED);

  // Display the red bar under the instruction text
  DisplaySmallRedLineWithShadow(INS_CTRCT_FIRST_BULLET_TEXT_X,
                                INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y,
                                INS_CTRCT_FIRST_BULLET_TEXT_X + INS_CTRCT_INTSRUCTION_TEXT_WIDTH,
                                INS_CTRCT_RED_BAR_UNDER_INSTRUCTION_TEXT_Y);

  sNextMercID = gsCurrentInsuranceMercIndex;
  while ((ubCount < gubNumberofDisplayedInsuranceGrids) &&
         (sNextMercID <= gTacticalStatus.Team[gbPlayerNum].bLastID)) {
    sMercID = gubInsuranceMercArray[sNextMercID];

    pSoldier = GetSoldierByID(GetSoldierIDFromMercID((uint8_t)sMercID));

    if ((sMercID != -1) && MercIsInsurable(pSoldier)) {
      DisplayOrderGrid(ubCount, (uint8_t)sMercID);
      ubCount++;
    }

    sNextMercID++;
  }

  // if there are no valid mercs to insure
  if (ubCount == 0) {
    // if there where AIM mercs ( on short contract )
    if (AreAnyAimMercsOnTeam()) {
      // Display Error Message, all aim mercs are on short contract
      GetInsuranceText(INS_MLTI_ALL_AIM_MERCS_ON_SHORT_CONTRACT, sText);
      DoLapTopMessageBox(MSG_BOX_RED_ON_WHITE, sText, LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
                         InsContractNoMercsPopupCallBack);
    } else {
      // Display Error Message, no valid mercs
      GetInsuranceText(INS_MLTI_NO_QUALIFIED_MERCS, sText);
      DoLapTopMessageBox(MSG_BOX_RED_ON_WHITE, sText, LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
                         InsContractNoMercsPopupCallBack);
    }
  }

  SetFontShadow(DEFAULT_SHADOW);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnInsContractPrevButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      if (gsCurrentInsuranceMercIndex > 2) gsCurrentInsuranceMercIndex -= 3;

      // signal that we want to change the number of forms on the page
      gfChangeInsuranceFormButtons = TRUE;

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

void BtnInsContractNextButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      gsCurrentInsuranceMercIndex += 3;

      // signal that we want to change the number of forms on the page
      gfChangeInsuranceFormButtons = TRUE;

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

BOOLEAN DisplayOrderGrid(uint8_t ubGridNumber, uint8_t ubMercID) {
  struct VObject *hPixHandle;
  uint16_t usPosX, usPosY;
  uint32_t uiInsMercFaceImage;
  int32_t iCostOfContract = 0;
  char sTemp[100];
  wchar_t sText[800];
  BOOLEAN fDisplayMercContractStateTextColorInRed = FALSE;

  struct SOLDIERTYPE *pSoldier;

  pSoldier = GetSoldierByID(GetSoldierIDFromMercID(ubMercID));

  usPosX = usPosY = 0;

  switch (ubGridNumber) {
    case 0:
      usPosX = INS_CTRCT_ORDER_GRID1_X;
      gubMercIDForMercInForm1 = ubMercID;
      gsForm1InsuranceLengthNumber = (int16_t)pSoldier->iTotalLengthOfInsuranceContract;
      break;

    case 1:
      usPosX = INS_CTRCT_ORDER_GRID2_X;
      gubMercIDForMercInForm2 = ubMercID;
      gsForm2InsuranceLengthNumber = (int16_t)pSoldier->iTotalLengthOfInsuranceContract;
      break;

    case 2:
      usPosX = INS_CTRCT_ORDER_GRID3_X;
      gubMercIDForMercInForm3 = ubMercID;
      gsForm3InsuranceLengthNumber = (int16_t)pSoldier->iTotalLengthOfInsuranceContract;
      break;

    default:
      // should never get in here
      Assert(0);
      break;
  }

  // Get and display the insurance order grid #1
  GetVideoObject(&hPixHandle, guiInsOrderGridImage);
  BltVideoObject2(vsFB, hPixHandle, 0, usPosX, INS_CTRCT_ORDER_GRID1_Y, VO_BLT_SRCTRANSPARENCY,
                  NULL);

  // load the mercs face graphic and add it
  sprintf(sTemp, "FACES\\%02d.sti", ubMercID);
  if (!AddVObjectFromFile(sTemp, &uiInsMercFaceImage)) {
    return FALSE;
  }

  // Get the merc's face
  GetVideoObject(&hPixHandle, uiInsMercFaceImage);

  // if the merc is dead, shade the face red
  if (IsMercDead(ubMercID)) {
    // if the merc is dead
    // shade the face red, (to signify that he is dead)
    hPixHandle->pShades[0] =
        Create16BPPPaletteShaded(hPixHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                 DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

    // set the red pallete to the face
    SetObjectHandleShade(uiInsMercFaceImage, 0);
  }

  // Get and display the mercs face
  BltVideoObject2(vsFB, hPixHandle, 0, usPosX + INS_CTRCT_OG_FACE_OFFSET_X,
                  INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_OG_FACE_OFFSET_Y, VO_BLT_SRCTRANSPARENCY,
                  NULL);

  // the face images isn't needed anymore so delete it
  DeleteVideoObjectFromIndex(uiInsMercFaceImage);

  // display the mercs nickname
  DrawTextToScreen(gMercProfiles[ubMercID].zNickname,
                   (uint16_t)(usPosX + INS_CTRCT_OG_NICK_NAME_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_OG_NICK_NAME_OFFSET_Y, 0, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Get the text to display the mercs current insurance contract status
  if (IsMercDead(GetSolProfile(pSoldier))) {
    // if the merc has a contract
    if (pSoldier->usLifeInsurance) {
      // Display the contract text
      GetInsuranceText(INS_SNGL_DEAD_WITH_CONTRACT, sText);
    } else {
      // Display the contract text
      GetInsuranceText(INS_SNGL_DEAD_NO_CONTRACT, sText);
    }
    DisplayWrappedString((uint16_t)(usPosX + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X),
                         INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y,
                         INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH, 2, INS_FONT_SMALL,
                         INS_FONT_COLOR_RED, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  } else {
    // if the merc has a contract
    if (pSoldier->usLifeInsurance) {
      // if the soldier can extend their insurance
      if (CanSoldierExtendInsuranceContract(pSoldier)) {
        // Display the contract text
        GetInsuranceText(INS_SNGL_PARTIALLY_INSURED, sText);
        fDisplayMercContractStateTextColorInRed = TRUE;
      } else {
        // Display the contract text
        GetInsuranceText(INS_SNGL_CONTRACT, sText);
        fDisplayMercContractStateTextColorInRed = FALSE;
      }
    } else {
      // Display the contract text
      GetInsuranceText(INS_SNGL_NOCONTRACT, sText);
      fDisplayMercContractStateTextColorInRed = TRUE;
    }
    if (fDisplayMercContractStateTextColorInRed)
      DisplayWrappedString((uint16_t)(usPosX + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X),
                           INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y,
                           INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH, 2, INS_FONT_SMALL,
                           INS_FONT_COLOR_RED, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    else
      DisplayWrappedString((uint16_t)(usPosX + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_X),
                           INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_OG_HAS_CONTRACT_OFFSET_Y,
                           INS_CTRCT_CONTRACT_STATUS_TEXT_WIDTH, 2, INS_FONT_SMALL, INS_FONT_COLOR,
                           sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }

  // Display the Emplyment contract text
  GetInsuranceText(INS_SNGL_EMPLOYMENT_CONTRACT, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_Y,
                   INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, CENTER_JUSTIFIED);

  // Display the merc contract Length text
  GetInsuranceText(INS_SNGL_LENGTH, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_LENGTH_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_LENGTH_OFFSET_Y, 0, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the mercs contract length
  swprintf(sText, ARR_SIZE(sText), L"%d", pSoldier->iTotalContractLength);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_OG_BOX_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_LENGTH_OFFSET_Y, INS_CTRCT_OG_BOX_WIDTH,
                   INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

  // Display the days remaining for the emplyment contract text
  GetInsuranceText(INS_SNGL_DAYS_REMAINING, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_LENGTH_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_DAYS_REMAINING_OFFSET_Y, 0, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // display the amount of time the merc has left on their Regular contract
  if (IsMercDead(ubMercID))
    swprintf(sText, ARR_SIZE(sText), L"%s", pMessageStrings[MSG_LOWERCASE_NA]);
  else
    swprintf(sText, ARR_SIZE(sText), L"%d", GetTimeRemainingOnSoldiersContract(pSoldier));

  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_OG_BOX_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_DAYS_REMAINING_OFFSET_Y,
                   INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  // Display the Insurqance contract
  GetInsuranceText(INS_SNGL_INSURANCE_CONTRACT, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_INSURANCE_CNTRCT_OFFSET_Y,
                   INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, CENTER_JUSTIFIED);

  GetInsuranceText(INS_SNGL_LENGTH, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_LENGTH_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_LENGTH_OFFSET_Y + 54, 0, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the insurance days remaining text
  GetInsuranceText(INS_SNGL_DAYS_REMAINING, sText);
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_LENGTH_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_DAYS_REMAINING_OFFSET_Y + 54, 0,
                   INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  //
  // display the amount of time the merc has left on the insurance contract
  //

  // if the soldier has insurance, disply the length of time the merc has left
  if (IsMercDead(ubMercID))
    swprintf(sText, ARR_SIZE(sText), L"%s", pMessageStrings[MSG_LOWERCASE_NA]);

  else if (pSoldier->usLifeInsurance != 0)
    swprintf(sText, ARR_SIZE(sText), L"%d", GetTimeRemainingOnSoldiersInsuranceContract(pSoldier));

  else
    swprintf(sText, ARR_SIZE(sText), L"%d", 0);

  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_OG_BOX_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_DAYS_REMAINING_OFFSET_Y + 54,
                   INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  //
  // Calculate the insurance cost
  //

  // if the soldier can get insurance, calculate a new cost
  if (CanSoldierExtendInsuranceContract(pSoldier)) {
    iCostOfContract = CalculateInsuranceContractCost(
        CalculateSoldiersInsuranceContractLength(pSoldier), GetSolProfile(pSoldier));
  }

  else {
    iCostOfContract = 0;
  }

  if (iCostOfContract < 0) {
    // shouldnt get in here now since we can longer give refunds
    Assert(0);
  } else {
    // Display the premium owing text
    GetInsuranceText(INS_SNGL_PREMIUM_OWING, sText);
    DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_EMPLYMNT_CNTRCT_TEXT_OFFSET_X),
                     INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_PREMIUM_OWING_OFFSET_Y,
                     INS_CTRCT_ORDER_GRID_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                     FALSE, CENTER_JUSTIFIED);

    // display the amount of refund
    swprintf(sText, ARR_SIZE(sText), L"%d", iCostOfContract);
    InsertCommasForDollarFigure(sText);
    InsertDollarSignInToString(sText);
  }

  if (IsMercDead(ubMercID)) {
    swprintf(sText, ARR_SIZE(sText), L"0");
    InsertDollarSignInToString(sText);
  }
  // display the amount owing
  DrawTextToScreen(sText, (uint16_t)(usPosX + 32), INS_CTRCT_ORDER_GRID1_Y + 179, 72, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

  //
  // Get the insurance contract length for the merc
  //
  swprintf(sText, ARR_SIZE(sText), L"%d", CalculateSoldiersInsuranceContractLength(pSoldier));

  // Display the length of time the player can get for the insurance contract
  DrawTextToScreen(sText, (uint16_t)(usPosX + INS_CTRCT_OG_BOX_OFFSET_X),
                   INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_LENGTH_OFFSET_Y + 52 + 2,
                   INS_CTRCT_OG_BOX_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  return (TRUE);
}

void BtnInsuranceAcceptClearForm1ButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      uint8_t ubButton = (uint8_t)MSYS_GetBtnUserData(btn, 0);
      uint8_t ubSoldierID = (uint8_t)GetSoldierIDFromMercID(gubMercIDForMercInForm1);

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      // the accept button
      if (ubButton == 0) {
        // handle the accept button, (global to all accept button
        HandleAcceptButton(ubSoldierID, 1);

        // specify the length of the insurance contract
        Menptr[ubSoldierID].iTotalLengthOfInsuranceContract = gsForm1InsuranceLengthNumber;

        // reset the insurance length
        gsForm1InsuranceLengthNumber = 0;
      }

      // redraw the screen
      fPausedReDrawScreenFlag = TRUE;

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

void BtnInsuranceAcceptClearForm2ButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      uint8_t ubButton = (uint8_t)MSYS_GetBtnUserData(btn, 0);
      uint8_t ubSoldierID = (uint8_t)GetSoldierIDFromMercID(gubMercIDForMercInForm2);

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      // the accept button
      if (ubButton == 0) {
        // handle the accept button, (global to all accept button
        HandleAcceptButton(ubSoldierID, 2);

        // specify the length of the insurance contract
        Menptr[ubSoldierID].iTotalLengthOfInsuranceContract = gsForm2InsuranceLengthNumber;

        // reset the insurance length
        gsForm2InsuranceLengthNumber = 0;
      }

      // redraw the screen
      fPausedReDrawScreenFlag = TRUE;

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

void BtnInsuranceAcceptClearForm3ButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      uint8_t ubButton = (uint8_t)MSYS_GetBtnUserData(btn, 0);
      uint8_t ubSoldierID = (uint8_t)GetSoldierIDFromMercID(gubMercIDForMercInForm3);

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      // the accept button
      if (ubButton == 0) {
        // handle the accept button, (global to all accept button
        HandleAcceptButton(ubSoldierID, 3);

        // specify the length of the insurance contract
        Menptr[ubSoldierID].iTotalLengthOfInsuranceContract = gsForm3InsuranceLengthNumber;

        // reset the insurance length
        gsForm3InsuranceLengthNumber = 0;
      }

      // redraw the screen
      fPausedReDrawScreenFlag = TRUE;

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

int8_t GetNumberOfHireMercsStartingFromID(uint8_t ubStartMercID) {
  uint8_t i;
  uint8_t ubCount = 0;

  for (i = 0; i < gsMaxPlayersOnTeam; i++) {
    if (i >= ubStartMercID) {
      ubCount++;
    }
  }

  return (ubCount);
}

/*
int32_t CalculateInsuranceCost( struct SOLDIERTYPE *pSoldier, BOOLEAN fHaveInsurance )
{
        int32_t			iAmount=0;
        uint32_t		uiInsuranceContractLength = 0;

        uiInsuranceContractLength = CalculateSoldiersInsuranceContractLength( pSoldier );

        //If the soldier already has life insurance, then the user is changing the length of the
contract if( pSoldier->usLifeInsurance )
        {
                //if the user is changing the contract length
                if( uiInsuranceContractLength != 0 )
                {
                        iAmount = CalculateInsuranceContractCost( uiInsuranceContractLength,
GetSolProfile(pSoldier));
                }
                //else we are just calculating the new figure
                else
                {
                        iAmount = 0;
                }
        }
        //else the merc doesn't have life insurance
        else
        {
                iAmount = CalculateInsuranceContractCost( uiInsuranceContractLength,
GetSolProfile(pSoldier));
        }

        return( iAmount );
}
*/

void SelectInsuranceContractRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint32_t uiInsuranceLink = MSYS_GetRegionUserData(pRegion, 0);

    if (uiInsuranceLink == 0)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
    else if (uiInsuranceLink == 1)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

int8_t CountInsurableMercs() {
  int16_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int16_t bLastTeamID;
  int8_t bCount = 0;

  // Set locator to first merc
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

  for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
    if (MercIsInsurable(pSoldier)) {
      bCount++;
    }
  }

  return (bCount);
}

void DisableInsuranceContractNextPreviousbuttons() {
  // disable the next button if there is no more mercs to display
  if ((gsCurrentInsuranceMercIndex + gubNumberofDisplayedInsuranceGrids) < CountInsurableMercs()) {
    EnableButton(guiInsContractNextBackButton);
  } else
    DisableButton(guiInsContractNextBackButton);

  // if we are currently displaying the first set of mercs, disable the previous button
  if (gsCurrentInsuranceMercIndex < 3) {
    DisableButton(guiInsContractPrevBackButton);
  } else
    EnableButton(guiInsContractPrevBackButton);
}

void CreateDestroyInsuranceContractFormButtons(BOOLEAN fCreate) {
  static BOOLEAN fButtonsCreated = FALSE;

  if (fCreate && !fButtonsCreated) {
    // place the 3 accept buttons for the different forms

    // The accept button image
    guiInsuranceAcceptClearForm1ButtonImage =
        LoadButtonImage("LAPTOP\\AcceptClearBox.sti", -1, 0, -1, 1, -1);

    if (gubNumberofDisplayedInsuranceGrids >= 1) {
      // the accept button for form 1
      guiInsuranceAcceptClearForm1Button = CreateIconAndTextButton(
          guiInsuranceAcceptClearForm1ButtonImage, InsContractText[INS_CONTRACT_ACCEPT],
          INS_FONT_MED, INS_FONT_BTN_COLOR, INS_FONT_BTN_SHADOW_COLOR, INS_FONT_BTN_COLOR,
          INS_FONT_BTN_SHADOW_COLOR, TEXT_CJUSTIFIED,
          INS_CTRCT_ORDER_GRID1_X + INS_CTRCT_ACCEPT_BTN_X,
          INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_ACCEPT_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
          DEFAULT_MOVE_CALLBACK, BtnInsuranceAcceptClearForm1ButtonCallback);
      SetButtonCursor(guiInsuranceAcceptClearForm1Button, CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(guiInsuranceAcceptClearForm1Button, 0, 0);
    }

    if (gubNumberofDisplayedInsuranceGrids >= 2) {
      // the accept button for form 2
      guiInsuranceAcceptClearForm2Button = CreateIconAndTextButton(
          guiInsuranceAcceptClearForm1ButtonImage, InsContractText[INS_CONTRACT_ACCEPT],
          INS_FONT_MED, INS_FONT_BTN_COLOR, INS_FONT_BTN_SHADOW_COLOR, INS_FONT_BTN_COLOR,
          INS_FONT_BTN_SHADOW_COLOR, TEXT_CJUSTIFIED,
          INS_CTRCT_ORDER_GRID2_X + INS_CTRCT_ACCEPT_BTN_X,
          INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_ACCEPT_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
          DEFAULT_MOVE_CALLBACK, BtnInsuranceAcceptClearForm2ButtonCallback);
      SetButtonCursor(guiInsuranceAcceptClearForm2Button, CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(guiInsuranceAcceptClearForm2Button, 0, 0);
    }

    if (gubNumberofDisplayedInsuranceGrids >= 3) {
      // the accept button for form 3
      guiInsuranceAcceptClearForm3Button = CreateIconAndTextButton(
          guiInsuranceAcceptClearForm1ButtonImage, InsContractText[INS_CONTRACT_ACCEPT],
          INS_FONT_MED, INS_FONT_BTN_COLOR, INS_FONT_BTN_SHADOW_COLOR, INS_FONT_BTN_COLOR,
          INS_FONT_BTN_SHADOW_COLOR, TEXT_CJUSTIFIED,
          INS_CTRCT_ORDER_GRID3_X + INS_CTRCT_ACCEPT_BTN_X,
          INS_CTRCT_ORDER_GRID1_Y + INS_CTRCT_ACCEPT_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
          DEFAULT_MOVE_CALLBACK, BtnInsuranceAcceptClearForm3ButtonCallback);
      SetButtonCursor(guiInsuranceAcceptClearForm3Button, CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(guiInsuranceAcceptClearForm3Button, 0, 0);
    }

    fButtonsCreated = TRUE;
  }

  if (fButtonsCreated && !fCreate) {
    // the accept image
    UnloadButtonImage(guiInsuranceAcceptClearForm1ButtonImage);

    if (gubNumberofDisplayedInsuranceGrids >= 1) {
      // the accept for the first form
      RemoveButton(guiInsuranceAcceptClearForm1Button);
    }

    if (gubNumberofDisplayedInsuranceGrids >= 2) {
      // the accept clear for the second form
      RemoveButton(guiInsuranceAcceptClearForm2Button);
    }

    if (gubNumberofDisplayedInsuranceGrids >= 3) {
      // the accept clear for the third form
      RemoveButton(guiInsuranceAcceptClearForm3Button);
    }

    fButtonsCreated = FALSE;
  }
}

void HandleAcceptButton(uint8_t ubSoldierID, uint8_t ubFormID) {
  // passed in either 1,2,3 should be 0,1,2
  ubFormID--;

  PurchaseOrExtendInsuranceForSoldier(
      GetSoldierByID(ubSoldierID),
      CalculateSoldiersInsuranceContractLength(GetSoldierByID(ubSoldierID)));

  RenderInsuranceContract();
}

// determines if a merc will run out of their insurance contract
void DailyUpdateOfInsuredMercs() {
  int16_t cnt;
  int16_t bLastTeamID;
  struct SOLDIERTYPE *pSoldier;

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

  for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
    // if the soldier is in the team array
    if (IsSolActive(pSoldier)) {
      // if the merc has life insurance
      if (pSoldier->usLifeInsurance) {
        // if the merc wasn't just hired
        if ((int16_t)GetWorldDay() != pSoldier->iStartOfInsuranceContract) {
          // if the contract has run out of time
          if (GetTimeRemainingOnSoldiersInsuranceContract(pSoldier) <= 0) {
            // if the soldier isn't dead
            if (!IsMercDead(GetSolProfile(pSoldier))) {
              pSoldier->usLifeInsurance = 0;
              pSoldier->iTotalLengthOfInsuranceContract = 0;
              pSoldier->iStartOfInsuranceContract = 0;
            }
          }
        }
      }
    }
  }
}

#define MIN_INSURANCE_RATIO 0.1f
#define MAX_INSURANCE_RATIO 10.0f

int32_t CalculateInsuranceContractCost(int32_t iLength, uint8_t ubMercID) {
  MERCPROFILESTRUCT *pProfile;
  int16_t sTotalSkill = 0;
  float flSkillFactor, flFitnessFactor, flExpFactor, flSurvivalFactor;
  float flRiskFactor;
  uint32_t uiDailyInsurancePremium;
  uint32_t uiTotalInsurancePremium;
  struct SOLDIERTYPE *pSoldier;

  pSoldier = GetSoldierByID(GetSoldierIDFromMercID(ubMercID));

  // only mercs with at least 2 days to go on their employment contract are insurable
  // def: 2/5/99.  However if they already have insurance is SHOULD be ok
  if (GetTimeRemainingOnSoldiersContract(pSoldier) < 2 &&
      !(pSoldier->usLifeInsurance != 0 && GetTimeRemainingOnSoldiersContract(pSoldier) >= 1)) {
    return (0);
  }

  // If the merc is currently being held captive, get out
  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return (0);
  }

  /*
          replaced with the above check

          if (iLength < 2)
          {
                  return(0);
          }
          */

  pProfile = &gMercProfiles[ubMercID];

  // calculate the degree of training
  sTotalSkill = (pProfile->bMarksmanship + pProfile->bMedical + pProfile->bMechanical +
                 pProfile->bExplosive + pProfile->bLeadership) /
                5;
  flSkillFactor = DiffFromNormRatio(sTotalSkill, INS_CTRCT_SKILL_BASE);

  // calc relative fitness level
  flFitnessFactor = DiffFromNormRatio(pProfile->bLife, INS_CTRCT_FITNESS_BASE);

  // calc relative experience
  flExpFactor = DiffFromNormRatio(pProfile->bExpLevel, INS_CTRCT_EXP_LEVEL_BASE);

  // calc player's survival rate (death rate subtracted from 100)
  flSurvivalFactor = DiffFromNormRatio((int16_t)(100 - CalcDeathRate()), INS_CTRCT_SURVIVAL_BASE);

  // calculate the overall insurability risk factor for this merc by combining all the subfactors
  flRiskFactor = flSkillFactor * flFitnessFactor * flExpFactor * flSurvivalFactor;

  // restrict the overall factor to within reasonable limits
  if (flRiskFactor < MIN_INSURANCE_RATIO) {
    flRiskFactor = MIN_INSURANCE_RATIO;
  } else if (flRiskFactor > MAX_INSURANCE_RATIO) {
    flRiskFactor = MAX_INSURANCE_RATIO;
  }

  // premium depend on merc's salary, the base insurance rate, and the individual's risk factor at
  // this time
  uiDailyInsurancePremium =
      (uint32_t)(((pProfile->sSalary * INSURANCE_PREMIUM_RATE * flRiskFactor) / 100) + 0.5);
  // multiply by the insurance contract length
  uiTotalInsurancePremium = uiDailyInsurancePremium * iLength;

  return (uiTotalInsurancePremium);
}

// values passed in must be such that exceeding the normal value REDUCES insurance premiums
float DiffFromNormRatio(int16_t sThisValue, int16_t sNormalValue) {
  float flRatio;

  if (sThisValue > 0) {
    flRatio = (float)sNormalValue / sThisValue;

    // restrict each ratio to within a reasonable range
    if (flRatio < MIN_INSURANCE_RATIO) {
      flRatio = MIN_INSURANCE_RATIO;
    } else if (flRatio > MAX_INSURANCE_RATIO) {
      flRatio = MAX_INSURANCE_RATIO;
    }
  } else {
    // use maximum allowable ratio
    flRatio = MAX_INSURANCE_RATIO;
  }

  return (flRatio);
}

void InsContractNoMercsPopupCallBack(uint8_t bExitValue) {
  // yes, so start over, else stay here and do nothing for now
  if (bExitValue == MSG_BOX_RETURN_OK) {
    guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
  }

  return;
}

void BuildInsuranceArray() {
  int16_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int16_t bLastTeamID;

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;
  gsMaxPlayersOnTeam = 0;

  // store profile #s of all insurable mercs in an array
  for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
    if (MercIsInsurable(pSoldier)) {
      gubInsuranceMercArray[gsMaxPlayersOnTeam] = GetSolProfile(pSoldier);
      gsMaxPlayersOnTeam++;
    }
  }
}

BOOLEAN AddLifeInsurancePayout(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubPayoutID;
  uint32_t uiTimeInMinutes;
  MERCPROFILESTRUCT *pProfile;
  uint32_t uiCostPerDay;
  uint32_t uiDaysToPay;

  Assert(pSoldier != NULL);
  Assert(GetSolProfile(pSoldier) != NO_PROFILE);

  pProfile = &(gMercProfiles[GetSolProfile(pSoldier)]);

  // if we need to add more array elements
  if (LaptopSaveInfo.ubNumberLifeInsurancePayouts <=
      LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed) {
    LaptopSaveInfo.ubNumberLifeInsurancePayouts++;
    LaptopSaveInfo.pLifeInsurancePayouts = (LIFE_INSURANCE_PAYOUT *)MemRealloc(
        LaptopSaveInfo.pLifeInsurancePayouts,
        sizeof(LIFE_INSURANCE_PAYOUT) * LaptopSaveInfo.ubNumberLifeInsurancePayouts);
    if (LaptopSaveInfo.pLifeInsurancePayouts == NULL) return (FALSE);

    memset(&LaptopSaveInfo.pLifeInsurancePayouts[LaptopSaveInfo.ubNumberLifeInsurancePayouts - 1],
           0, sizeof(LIFE_INSURANCE_PAYOUT));
  }

  for (ubPayoutID = 0; ubPayoutID < LaptopSaveInfo.ubNumberLifeInsurancePayouts; ubPayoutID++) {
    // get an empty element in the array
    if (!LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].fActive) break;
  }

  LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubSoldierID = GetSolID(pSoldier);
  LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID = GetSolProfile(pSoldier);
  LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].fActive = TRUE;

  // This uses the merc's latest salaries, ignoring that they may be higher than the salaries paid
  // under the current contract if the guy has recently gained a level.  We could store his daily
  // salary when he was last contracted, and use that, but it still doesn't easily account for the
  // fact that renewing a leveled merc early means that the first part of his contract is under his
  // old salary and the second part is under his new one.  Therefore, I chose to ignore this
  // wrinkle, and let the player awlays get paid out using the higher amount.  ARM

  // figure out which of the 3 salary rates the merc has is the cheapest, and use it to calculate
  // the paid amount, to avoid getting back more than the merc cost if he was on a 2-week contract!

  // start with the daily salary
  uiCostPerDay = pProfile->sSalary;

  // consider weekly salary / day
  if ((pProfile->uiWeeklySalary / 7) < uiCostPerDay) {
    uiCostPerDay = (pProfile->uiWeeklySalary / 7);
  }

  // consider biweekly salary / day
  if ((pProfile->uiBiWeeklySalary / 14) < uiCostPerDay) {
    uiCostPerDay = (pProfile->uiBiWeeklySalary / 14);
  }

  // calculate how many full, insured days of work the merc is going to miss
  uiDaysToPay = pSoldier->iTotalLengthOfInsuranceContract -
                (GetWorldDay() + 1 - pSoldier->iStartOfInsuranceContract);

  // calculate & store how much is to be paid out
  LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice = uiDaysToPay * uiCostPerDay;

  // 4pm next day
  uiTimeInMinutes = GetMidnightOfFutureDayInMinutes(1) + 16 * 60;

  // if the death was suspicious, or he's already been investigated twice or more
  if (pProfile->ubSuspiciousDeath || (gStrategicStatus.ubInsuranceInvestigationsCnt >= 2)) {
    // fraud suspected, claim will be investigated first
    AddStrategicEvent(EVENT_INSURANCE_INVESTIGATION_STARTED, uiTimeInMinutes, ubPayoutID);
  } else {
    // is ok, make a prompt payment
    AddStrategicEvent(EVENT_PAY_LIFE_INSURANCE_FOR_DEAD_MERC, uiTimeInMinutes, ubPayoutID);
  }

  LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed++;

  return (TRUE);
}

void StartInsuranceInvestigation(uint8_t ubPayoutID) {
  uint8_t ubDays;

  // send an email telling player an investigation is taking place
  if (gStrategicStatus.ubInsuranceInvestigationsCnt == 0) {
    // first offense
    AddEmailWithSpecialData(INSUR_SUSPIC, INSUR_SUSPIC_LENGTH, INSURANCE_COMPANY,
                            GetWorldTotalMin(),
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice,
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID);
  } else {
    // subsequent offense
    AddEmailWithSpecialData(INSUR_SUSPIC_2, INSUR_SUSPIC_2_LENGTH, INSURANCE_COMPANY,
                            GetWorldTotalMin(),
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice,
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID);
  }

  if (gMercProfiles[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID].ubSuspiciousDeath ==
      VERY_SUSPICIOUS_DEATH) {
    // the fact that you tried to cheat them gets realized very quickly. :-)
    ubDays = 1;
  } else {
    // calculate how many days the investigation will take
    ubDays = (uint8_t)(2 + gStrategicStatus.ubInsuranceInvestigationsCnt +
                       Random(3));  // 2-4 days, +1 for every previous investigation
  }

  // post an event to end the investigation that many days in the future (at 4pm)
  AddStrategicEvent(EVENT_INSURANCE_INVESTIGATION_OVER,
                    GetMidnightOfFutureDayInMinutes(ubDays) + 16 * 60, ubPayoutID);

  // increment counter of all investigations
  gStrategicStatus.ubInsuranceInvestigationsCnt++;
}

void EndInsuranceInvestigation(uint8_t ubPayoutID) {
  // send an email telling player the investigation is over
  if (gMercProfiles[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID].ubSuspiciousDeath ==
      VERY_SUSPICIOUS_DEATH) {
    // fraud, no payout!
    AddEmailWithSpecialData(INSUR_1HOUR_FRAUD, INSUR_1HOUR_FRAUD_LENGTH, INSURANCE_COMPANY,
                            GetWorldTotalMin(),
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice,
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID);
  } else {
    AddEmailWithSpecialData(INSUR_INVEST_OVER, INSUR_INVEST_OVER_LENGTH, INSURANCE_COMPANY,
                            GetWorldTotalMin(),
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice,
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID);

    // only now make a payment (immediately)
    InsuranceContractPayLifeInsuranceForDeadMerc(ubPayoutID);
  }
}

// void InsuranceContractPayLifeInsuranceForDeadMerc( LIFE_INSURANCE_PAYOUT *pPayoutStruct )
void InsuranceContractPayLifeInsuranceForDeadMerc(uint8_t ubPayoutID) {
  // if the mercs id number is the same what is in the soldier array
  if (LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubSoldierID ==
      Menptr[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubSoldierID].ubID) {
    // and if the soldier is still active ( player hasn't removed carcass yet ), reset insurance
    // flag
    if (Menptr[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubSoldierID].bActive)
      Menptr[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubSoldierID].usLifeInsurance = 0;
  }

  // add transaction to players account
  AddTransactionToPlayersBook(INSURANCE_PAYOUT,
                              LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID,
                              LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice);

  // add to the history log the fact that the we paid the insurance claim
  AddHistoryToPlayersLog(HISTORY_INSURANCE_CLAIM_PAYOUT,
                         LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID,
                         GetWorldTotalMin(), -1, -1);

  // if there WASNT an investigation
  if (gMercProfiles[LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID].ubSuspiciousDeath ==
      0) {
    // Add an email telling the user that he received an insurance payment
    AddEmailWithSpecialData(INSUR_PAYMENT, INSUR_PAYMENT_LENGTH, INSURANCE_COMPANY,
                            GetWorldTotalMin(),
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].iPayOutPrice,
                            LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].ubMercID);
  }

  LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed--;
  LaptopSaveInfo.pLifeInsurancePayouts[ubPayoutID].fActive = FALSE;
  //	MemFree( pPayoutStruct );
}

// Gets called at the very end of the game
void InsuranceContractEndGameShutDown() {
  // Free up the memory allocated to the insurance payouts
  if (LaptopSaveInfo.pLifeInsurancePayouts) {
    MemFree(LaptopSaveInfo.pLifeInsurancePayouts);
    LaptopSaveInfo.pLifeInsurancePayouts = NULL;
  }
}

BOOLEAN MercIsInsurable(struct SOLDIERTYPE *pSoldier) {
  // only A.I.M. mercs currently on player's team
  if ((IsSolActive(pSoldier)) && (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)) {
    // with more than one day left on their employment contract are eligible for insurance
    // the second part is because the insurance doesn't pay for any working day already started at
    // time of death
    //		if( ( (pSoldier->iEndofContractTime - GetWorldTotalMin()) > 24 * 60) ||
    // pSoldier->usLifeInsurance )
    if (CanSoldierExtendInsuranceContract(pSoldier) || pSoldier->usLifeInsurance) {
      // who aren't currently being held POW
      // POWs are also uninsurable - if already insured, that insurance IS valid but no new
      // contracts or extension allowed
      if (pSoldier->bAssignment != ASSIGNMENT_POW) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void EnableDisableInsuranceContractAcceptButtons() {
  // If it is the first grid
  if (gubNumberofDisplayedInsuranceGrids >= 1) {
    EnableDisableIndividualInsuranceContractButton(gubMercIDForMercInForm1,
                                                   &guiInsuranceAcceptClearForm1Button);
  }

  // If it is the 2nd grid
  if (gubNumberofDisplayedInsuranceGrids >= 2) {
    EnableDisableIndividualInsuranceContractButton(gubMercIDForMercInForm2,
                                                   &guiInsuranceAcceptClearForm2Button);
  }

  // If it is the 3rd grid
  if (gubNumberofDisplayedInsuranceGrids >= 3) {
    EnableDisableIndividualInsuranceContractButton(gubMercIDForMercInForm3,
                                                   &guiInsuranceAcceptClearForm3Button);
  }
}

void EnableDisableIndividualInsuranceContractButton(uint8_t ubMercIDForMercInForm,
                                                    uint32_t *puiAcceptButton) {
  int16_t sSoldierID = 0;

  sSoldierID = GetSoldierIDFromMercID(ubMercIDForMercInForm);
  if (sSoldierID == -1) return;

  // if the soldiers contract can be extended, enable the button
  if (CanSoldierExtendInsuranceContract(GetSoldierByID(sSoldierID))) EnableButton(*puiAcceptButton);

  // else the soldier cant extend their insurance contract, disable the button
  else
    DisableButton(*puiAcceptButton);

  // if the merc is dead, disable the button
  if (IsMercDead(ubMercIDForMercInForm)) DisableButton(*puiAcceptButton);
}

/*
uint32_t	GetContractLengthForFormNumber( uint8_t ubFormID )
{
        uint8_t	ubMercID;
        struct SOLDIERTYPE	*pSoldier;

        switch( ubFormID )
        {
                case 0:
                        ubMercID = gubMercIDForMercInForm1;
                        break;
                case 1:
                        ubMercID = gubMercIDForMercInForm1;
                        break;
                case 2:
                        ubMercID = gubMercIDForMercInForm1;
                        break;

                default:
                        Assert( 0 );
                        break;
        }

        pSoldier = &Menptr[ GetSoldierIDFromMercID( ubMercID ) ];

        return( pSoldier->iTotalContractLength );
}
*/

uint32_t GetTimeRemainingOnSoldiersInsuranceContract(struct SOLDIERTYPE *pSoldier) {
  // if the soldier has life insurance
  if (pSoldier->usLifeInsurance) {
    // if the insurance contract hasnt started yet
    if ((int32_t)GetWorldDay() < pSoldier->iStartOfInsuranceContract)
      return (pSoldier->iTotalLengthOfInsuranceContract);
    else
      return ((pSoldier->iTotalLengthOfInsuranceContract -
               (GetWorldDay() - pSoldier->iStartOfInsuranceContract)));
  } else
    return (0);
}

uint32_t GetTimeRemainingOnSoldiersContract(struct SOLDIERTYPE *pSoldier) {
  int32_t iDayMercLeaves = (pSoldier->iEndofContractTime / 1440) - 1;

  // Since the merc is leaving in the afternoon, we must adjust since the time left would be
  // different if we did the calc at 11:59 or 12:01 ( noon )
  if (pSoldier->iEndofContractTime % 1440) iDayMercLeaves++;

  // Subtract todays day number
  iDayMercLeaves = iDayMercLeaves - GetWorldDay();

  if (iDayMercLeaves > pSoldier->iTotalContractLength)
    iDayMercLeaves = pSoldier->iTotalContractLength;

  return (iDayMercLeaves);
  //	return( ( pSoldier->iEndofContractTime - (int32_t)GetWorldTotalMin( ) ) / 1440 );
}

void PurchaseOrExtendInsuranceForSoldier(struct SOLDIERTYPE *pSoldier, uint32_t uiInsuranceLength) {
  int32_t iAmountOfMoneyTransfer = -1;

  if (pSoldier == NULL) AssertMsg(0, "Soldier pointer is NULL!");

  // if the user doesnt have insruance already,
  if (!(pSoldier->usLifeInsurance)) {
    // specify the start date of the contract
    pSoldier->iStartOfInsuranceContract = CalcStartDayOfInsurance(pSoldier);
    pSoldier->uiStartTimeOfInsuranceContract = GetWorldTotalMin();
  }

  // transfer money
  iAmountOfMoneyTransfer =
      CalculateInsuranceContractCost(uiInsuranceLength, GetSolProfile(pSoldier));

  // if the user did have insruance already,
  if (pSoldier->usLifeInsurance) {
    // specify the start date of the contract
    pSoldier->iStartOfInsuranceContract = CalcStartDayOfInsurance(pSoldier);
  }

  // add transaction to finaces page
  // if the player has life insurance
  if (pSoldier->usLifeInsurance) {
    // if the player is extending the contract
    if (iAmountOfMoneyTransfer > 0)
      AddTransactionToPlayersBook(EXTENDED_INSURANCE, GetSolProfile(pSoldier),
                                  -(iAmountOfMoneyTransfer));
    else
      Assert(0);
  } else {
    // if the player doesnt have enough money, tell him
    if (MoneyGetBalance() < iAmountOfMoneyTransfer) {
      wchar_t sText[800];

      GetInsuranceText(INS_MLTI_NOT_ENOUGH_FUNDS, sText);
      if (guiCurrentScreen == LAPTOP_SCREEN)
        DoLapTopMessageBox(MSG_BOX_RED_ON_WHITE, sText, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
      else
        DoMapMessageBox(MSG_BOX_RED_ON_WHITE, sText, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
    } else {
      // else if the player has enought to cover the bill, let him

      // the player just purchased life insurance
      AddTransactionToPlayersBook(PURCHASED_INSURANCE, GetSolProfile(pSoldier),
                                  -(iAmountOfMoneyTransfer));

      // add an entry in the history page for the purchasing of life insurance
      AddHistoryToPlayersLog(HISTORY_PURCHASED_INSURANCE, GetSolProfile(pSoldier),
                             GetWorldTotalMin(), -1, -1);

      // Set that we have life insurance
      pSoldier->usLifeInsurance = 1;
    }
  }

  pSoldier->iTotalLengthOfInsuranceContract += uiInsuranceLength;

  // make sure the length doesnt exceed the contract length
  if ((GetTimeRemainingOnSoldiersInsuranceContract(pSoldier)) >
      GetTimeRemainingOnSoldiersContract(pSoldier)) {
    pSoldier->iTotalLengthOfInsuranceContract -=
        GetTimeRemainingOnSoldiersInsuranceContract(pSoldier) -
        GetTimeRemainingOnSoldiersContract(pSoldier);
  }
}

BOOLEAN CanSoldierExtendInsuranceContract(struct SOLDIERTYPE *pSoldier) {
  if (CalculateSoldiersInsuranceContractLength(pSoldier) != 0)
    return (TRUE);
  else
    return (FALSE);
}

int32_t CalculateSoldiersInsuranceContractLength(struct SOLDIERTYPE *pSoldier) {
  int32_t iInsuranceContractLength = 0;
  uint32_t uiTimeRemainingOnSoldiersContract = GetTimeRemainingOnSoldiersContract(pSoldier);

  // if the merc is dead
  if (IsMercDead(GetSolProfile(pSoldier))) return (0);

  // only mercs with at least 2 days to go on their employment contract are insurable
  // def: 2/5/99.  However if they already have insurance is SHOULD be ok
  if (uiTimeRemainingOnSoldiersContract < 2 &&
      !(pSoldier->usLifeInsurance != 0 && uiTimeRemainingOnSoldiersContract >= 1)) {
    return (0);
  }

  //
  // Calculate the insurance contract length
  //

  // if the soldier has an insurance contract, dont deduct a day
  if (pSoldier->usLifeInsurance || DidGameJustStart())
    iInsuranceContractLength =
        uiTimeRemainingOnSoldiersContract - GetTimeRemainingOnSoldiersInsuranceContract(pSoldier);

  // else deduct a day
  else
    iInsuranceContractLength = uiTimeRemainingOnSoldiersContract -
                               GetTimeRemainingOnSoldiersInsuranceContract(pSoldier) - 1;

  // make sure the length doesnt exceed the contract length
  if ((GetTimeRemainingOnSoldiersInsuranceContract(pSoldier) + iInsuranceContractLength) >
      uiTimeRemainingOnSoldiersContract) {
    iInsuranceContractLength =
        uiTimeRemainingOnSoldiersContract - GetTimeRemainingOnSoldiersInsuranceContract(pSoldier);
  }

  // Is the mercs insurace contract is less then a day, set it to 0
  if (iInsuranceContractLength < 0) iInsuranceContractLength = 0;

  if (pSoldier->usLifeInsurance && pSoldier->iStartOfInsuranceContract >= (int32_t)GetWorldDay() &&
      iInsuranceContractLength < 2)
    iInsuranceContractLength = 0;

  return (iInsuranceContractLength);
}

int32_t CalcStartDayOfInsurance(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiDayToStartInsurance = 0;

  // if the soldier was just hired ( in transit ), and the game didnt just start
  if (GetSolAssignment(pSoldier) == IN_TRANSIT && !DidGameJustStart()) {
    uiDayToStartInsurance = GetWorldDay();
  } else {
    // Get tomorows date ( and convert it to days )
    uiDayToStartInsurance = GetMidnightOfFutureDayInMinutes(1) / 1440;
  }

  return (uiDayToStartInsurance);
}

BOOLEAN AreAnyAimMercsOnTeam() {
  int16_t sNextMercID = 0;
  BOOLEAN fIsThereAnyAimMercs = FALSE;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (sNextMercID = 0; sNextMercID <= gTacticalStatus.Team[gbPlayerNum].bLastID; sNextMercID++) {
    pSoldier = GetSoldierByID(GetSoldierIDFromMercID((uint8_t)sNextMercID));

    // check to see if any of the mercs are AIM mercs
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
      fIsThereAnyAimMercs = TRUE;
    }
  }

  return (fIsThereAnyAimMercs);
}
