#include "Laptop/Insurance.h"

#include "Laptop/InsuranceContract.h"
#include "Laptop/InsuranceText.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Line.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define INSURANCE_BACKGROUND_WIDTH 125
#define INSURANCE_BACKGROUND_HEIGHT 100

#define INSURANCE_BIG_TITLE_X 95 + LAPTOP_SCREEN_UL_X
#define INSURANCE_BIG_TITLE_Y 4 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_RED_BAR_X LAPTOP_SCREEN_UL_X
#define INSURANCE_RED_BAR_Y LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_TOP_RED_BAR_X LAPTOP_SCREEN_UL_X + 66
#define INSURANCE_TOP_RED_BAR_Y 109 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_TOP_RED_BAR_Y1 31 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_RED_BAR_Y 345 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_LINK_RED_BAR_X 77 + LAPTOP_SCREEN_UL_X
#define INSURANCE_BOTTOM_LINK_RED_BAR_Y 392 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH 107
#define INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET 148
#define INSURANCE_BOTTOM_LINK_RED_BAR_X_2 \
  INSURANCE_BOTTOM_LINK_RED_BAR_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_BOTTOM_LINK_RED_BAR_X_3 \
  INSURANCE_BOTTOM_LINK_RED_BAR_X_2 + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET

#define INSURANCE_LINK_TEXT_WIDTH INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH

#define INSURANCE_LINK_TEXT_1_X INSURANCE_BOTTOM_LINK_RED_BAR_X
#define INSURANCE_LINK_TEXT_1_Y INSURANCE_BOTTOM_LINK_RED_BAR_Y - 36

#define INSURANCE_LINK_TEXT_2_X INSURANCE_LINK_TEXT_1_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_LINK_TEXT_2_Y INSURANCE_LINK_TEXT_1_Y

#define INSURANCE_LINK_TEXT_3_X INSURANCE_LINK_TEXT_2_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_LINK_TEXT_3_Y INSURANCE_LINK_TEXT_1_Y

#define INSURANCE_SUBTITLE_X INSURANCE_BOTTOM_LINK_RED_BAR_X + 15
#define INSURANCE_SUBTITLE_Y 150 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BULLET_TEXT_1_Y 188 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BULLET_TEXT_2_Y 215 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BULLET_TEXT_3_Y 242 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_SLOGAN_X INSURANCE_SUBTITLE_X
#define INSURANCE_BOTTOM_SLOGAN_Y 285 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BOTTOM_SLOGAN_WIDTH 370

#define INSURANCE_SMALL_TITLE_X 64 + LAPTOP_SCREEN_UL_X
#define INSURANCE_SMALL_TITLE_Y 5 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_SMALL_TITLE_WIDTH 434 - 170
#define INSURANCE_SMALL_TITLE_HEIGHT 40 - 10

UINT32 guiInsuranceBackGround;
UINT32 guiInsuranceTitleImage;
UINT32 guiInsuranceSmallTitleImage;
UINT32 guiInsuranceRedBarImage;
UINT32 guiInsuranceBigRedLineImage;
UINT32 guiInsuranceBulletImage;

// link to the varios pages
struct MOUSE_REGION gSelectedInsuranceLinkRegion[3];
void SelectInsuranceRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);

// link to the home page by clicking on the small title
struct MOUSE_REGION gSelectedInsuranceTitleLinkRegion;
void SelectInsuranceTitleLinkRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);

void GameInitInsurance() {}

BOOLEAN EnterInsurance() {
  VOBJECT_DESC VObjectDesc;
  UINT16 usPosX, i;

  SetBookMark(INSURANCE_BOOKMARK);

  InitInsuranceDefaults();

  // load the Insurance title graphic and add it
  GetMLGFilename(VObjectDesc.ImageFile, MLG_INSURANCETITLE);
  if (!AddVideoObject(&VObjectDesc, &guiInsuranceTitleImage)) {
    return FALSE;
  }

  // load the red bar on the side of the page and add it
  if (!AddVObjectFromFile("LAPTOP\\Bullet.sti", &guiInsuranceBulletImage)) {
    return FALSE;
  }

  usPosX = INSURANCE_BOTTOM_LINK_RED_BAR_X;
  for (i = 0; i < 3; i++) {
    MSYS_DefineRegion(
        &gSelectedInsuranceLinkRegion[i], usPosX, INSURANCE_BOTTOM_LINK_RED_BAR_Y - 37,
        (UINT16)(usPosX + INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH), INSURANCE_BOTTOM_LINK_RED_BAR_Y + 2,
        MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceRegionCallBack);
    MSYS_AddRegion(&gSelectedInsuranceLinkRegion[i]);
    MSYS_SetRegionUserData(&gSelectedInsuranceLinkRegion[i], 0, i);

    usPosX += INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET;
  }

  RenderInsurance();

  // reset the current merc index on the insurance contract page
  gsCurrentInsuranceMercIndex = 0;

  return (TRUE);
}

void ExitInsurance() {
  UINT8 i;

  RemoveInsuranceDefaults();

  DeleteVideoObjectFromIndex(guiInsuranceTitleImage);
  DeleteVideoObjectFromIndex(guiInsuranceBulletImage);

  for (i = 0; i < 3; i++) MSYS_RemoveRegion(&gSelectedInsuranceLinkRegion[i]);
}

void HandleInsurance() {}

void RenderInsurance() {
  wchar_t sText[800];
  struct VObject *hPixHandle;

  DisplayInsuranceDefaults();

  SetFontShadow(INS_FONT_SHADOW);

  // Get and display the insurance title
  GetVideoObject(&hPixHandle, guiInsuranceTitleImage);
  BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_BIG_TITLE_X, INSURANCE_BIG_TITLE_Y,
                  VO_BLT_SRCTRANSPARENCY);

  // Display the title slogan
  GetInsuranceText(INS_SNGL_WERE_LISTENING, sText);
  DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INSURANCE_TOP_RED_BAR_Y - 35,
                   LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display the subtitle slogan
  GetInsuranceText(INS_SNGL_LIFE_INSURANCE_SPECIALISTS, sText);
  DrawTextToScreen(sText, INSURANCE_SUBTITLE_X, INSURANCE_SUBTITLE_Y, 0, INS_FONT_BIG,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the bulleted text 1
  GetVideoObject(&hPixHandle, guiInsuranceBulletImage);
  BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_1_Y,
                  VO_BLT_SRCTRANSPARENCY);
  GetInsuranceText(INS_MLTI_EMPLOY_HIGH_RISK, sText);
  DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                   INSURANCE_BULLET_TEXT_1_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the bulleted text 2
  BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_2_Y,
                  VO_BLT_SRCTRANSPARENCY);
  GetInsuranceText(INS_MLTI_HIGH_FATALITY_RATE, sText);
  DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                   INSURANCE_BULLET_TEXT_2_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the bulleted text 3
  BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_3_Y,
                  VO_BLT_SRCTRANSPARENCY);
  GetInsuranceText(INS_MLTI_DRAIN_SALARY, sText);
  DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                   INSURANCE_BULLET_TEXT_3_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the bottom slogan
  GetInsuranceText(INS_MLTI_IF_ANSWERED_YES, sText);
  DrawTextToScreen(sText, INSURANCE_BOTTOM_SLOGAN_X, INSURANCE_BOTTOM_SLOGAN_Y,
                   INSURANCE_BOTTOM_SLOGAN_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, CENTER_JUSTIFIED);

  // Display the red bar under the link at the bottom.  and the text
  DisplaySmallRedLineWithShadow(
      INSURANCE_BOTTOM_LINK_RED_BAR_X, INSURANCE_BOTTOM_LINK_RED_BAR_Y,
      INSURANCE_BOTTOM_LINK_RED_BAR_X + INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH,
      INSURANCE_BOTTOM_LINK_RED_BAR_Y);

  GetInsuranceText(INS_SNGL_COMMENTSFROM_CLIENTS, sText);
  DisplayWrappedString(INSURANCE_LINK_TEXT_1_X, INSURANCE_LINK_TEXT_1_Y, INSURANCE_LINK_TEXT_WIDTH,
                       2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(
      INSURANCE_BOTTOM_LINK_RED_BAR_X_2, INSURANCE_BOTTOM_LINK_RED_BAR_Y,
      INSURANCE_BOTTOM_LINK_RED_BAR_X_2 + INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH,
      INSURANCE_BOTTOM_LINK_RED_BAR_Y);

  GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK, sText);
  DisplayWrappedString(INSURANCE_LINK_TEXT_2_X, INSURANCE_LINK_TEXT_2_Y + 7,
                       INSURANCE_LINK_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText,
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(
      INSURANCE_BOTTOM_LINK_RED_BAR_X_3, INSURANCE_BOTTOM_LINK_RED_BAR_Y,
      INSURANCE_BOTTOM_LINK_RED_BAR_X_3 + INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH,
      INSURANCE_BOTTOM_LINK_RED_BAR_Y);

  GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW, sText);
  DisplayWrappedString(INSURANCE_LINK_TEXT_3_X, INSURANCE_LINK_TEXT_3_Y + 7,
                       INSURANCE_LINK_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText,
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

BOOLEAN InitInsuranceDefaults() {
  VOBJECT_DESC VObjectDesc;

  // load the Flower Account Box graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\BackGroundTile.sti", &guiInsuranceBackGround)) {
    return FALSE;
  }

  // load the red bar on the side of the page and add it
  if (!AddVObjectFromFile("LAPTOP\\LeftTile.sti", &guiInsuranceRedBarImage)) {
    return FALSE;
  }

  // load the red bar on the side of the page and add it
  if (!AddVObjectFromFile("LAPTOP\\LargeBar.sti", &guiInsuranceBigRedLineImage)) {
    return FALSE;
  }

  // if it is not the first page, display the small title
  if (guiCurrentLaptopMode != LAPTOP_MODE_INSURANCE) {
    // load the small title for the every page other then the first page
    GetMLGFilename(VObjectDesc.ImageFile, MLG_SMALLTITLE);
    if (!AddVideoObject(&VObjectDesc, &guiInsuranceSmallTitleImage)) {
      return FALSE;
    }

    // create the link to the home page on the small titles
    MSYS_DefineRegion(
        &gSelectedInsuranceTitleLinkRegion, INSURANCE_SMALL_TITLE_X + 85, INSURANCE_SMALL_TITLE_Y,
        (UINT16)(INSURANCE_SMALL_TITLE_X + INSURANCE_SMALL_TITLE_WIDTH),
        (UINT16)(INSURANCE_SMALL_TITLE_Y + INSURANCE_SMALL_TITLE_HEIGHT), MSYS_PRIORITY_HIGH,
        CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceTitleLinkRegionCallBack);
    MSYS_AddRegion(&gSelectedInsuranceTitleLinkRegion);
  }

  return (TRUE);
}

void DisplayInsuranceDefaults() {
  struct VObject *hPixHandle;
  UINT8 i;
  UINT16 usPosY;

  WebPageTileBackground(4, 4, INSURANCE_BACKGROUND_WIDTH, INSURANCE_BACKGROUND_HEIGHT,
                        guiInsuranceBackGround);

  usPosY = INSURANCE_RED_BAR_Y;

  GetVideoObject(&hPixHandle, guiInsuranceRedBarImage);
  for (i = 0; i < 4; i++) {
    BltVObjectSrcTrans(vsFB, hPixHandle, 0, INSURANCE_RED_BAR_X, usPosY);
    usPosY += INSURANCE_BACKGROUND_HEIGHT;
  }

  // display the top red bar
  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_INSURANCE:
      usPosY = INSURANCE_TOP_RED_BAR_Y;

      // display the top red bar
      GetVideoObject(&hPixHandle, guiInsuranceBigRedLineImage);
      BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_TOP_RED_BAR_X, usPosY, VO_BLT_SRCTRANSPARENCY);

      break;

    case LAPTOP_MODE_INSURANCE_INFO:
    case LAPTOP_MODE_INSURANCE_CONTRACT:
      usPosY = INSURANCE_TOP_RED_BAR_Y1;
      break;
  }

  // display the Bottom red bar
  GetVideoObject(&hPixHandle, guiInsuranceBigRedLineImage);
  BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_TOP_RED_BAR_X, INSURANCE_BOTTOM_RED_BAR_Y,
                  VO_BLT_SRCTRANSPARENCY);

  // if it is not the first page, display the small title
  if (guiCurrentLaptopMode != LAPTOP_MODE_INSURANCE) {
    // display the small title bar
    GetVideoObject(&hPixHandle, guiInsuranceSmallTitleImage);
    BltVideoObject2(vsFB, hPixHandle, 0, INSURANCE_SMALL_TITLE_X, INSURANCE_SMALL_TITLE_Y,
                    VO_BLT_SRCTRANSPARENCY);
  }
}

void RemoveInsuranceDefaults() {
  DeleteVideoObjectFromIndex(guiInsuranceBackGround);
  DeleteVideoObjectFromIndex(guiInsuranceRedBarImage);
  DeleteVideoObjectFromIndex(guiInsuranceBigRedLineImage);

  // if it is not the first page, display the small title
  if (guiPreviousLaptopMode != LAPTOP_MODE_INSURANCE) {
    DeleteVideoObjectFromIndex(guiInsuranceSmallTitleImage);
    MSYS_RemoveRegion(&gSelectedInsuranceTitleLinkRegion);
  }
}

void DisplaySmallRedLineWithShadow(UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY) {
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);

  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw the red line
  LineDraw(FALSE, usStartX, usStartY, EndX, EndY, Get16BPPColor(FROMRGB(255, 0, 0)), pDestBuf);

  // draw the black shadow line
  LineDraw(FALSE, usStartX + 1, usStartY + 1, EndX + 1, EndY + 1, Get16BPPColor(FROMRGB(0, 0, 0)),
           pDestBuf);

  // unlock frame buffer
  VSurfaceUnlock(vsFB);
}

void GetInsuranceText(UINT8 ubNumber, STR16 pString) {
  UINT32 uiStartLoc = 0;

  if (ubNumber < INS_MULTI_LINE_BEGINS) {
    // Get and display the card saying
    uiStartLoc = INSURANCE_TEXT_SINGLE_LINE_SIZE * ubNumber;
    LoadEncryptedDataFromFile(INSURANCE_TEXT_SINGLE_FILE, pString, uiStartLoc,
                              INSURANCE_TEXT_SINGLE_LINE_SIZE);
  } else {
    // Get and display the card saying
    uiStartLoc = INSURANCE_TEXT_MULTI_LINE_SIZE * (ubNumber - INS_MULTI_LINE_BEGINS - 1);
    LoadEncryptedDataFromFile(INSURANCE_TEXT_MULTI_FILE, pString, uiStartLoc,
                              INSURANCE_TEXT_MULTI_LINE_SIZE);
  }
}

void SelectInsuranceRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    UINT32 uiInsuranceLink = MSYS_GetRegionUserData(pRegion, 0);

    if (uiInsuranceLink == 0)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_COMMENTS;
    else if (uiInsuranceLink == 1)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
    else if (uiInsuranceLink == 2)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectInsuranceTitleLinkRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}
