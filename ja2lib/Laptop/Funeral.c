#include "Laptop/Funeral.h"

#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define FUNERAL_SENTENCE_FONT FONT12ARIAL
#define FUNERAL_SENTENCE_COLOR 2                         // FONT_MCOLOR_WHITE
#define FUNERAL_SENTENCE_SHADOW_COLOR FONT_MCOLOR_WHITE  // FONT_MCOLOR_DKWHITE

#define FUNERAL_SMALL_FONT FONT10ARIAL

#define FUNERAL_TITLE_FONT FONT14ARIAL
#define FUNERAL_TITLE_COLOR FONT_MCOLOR_WHITE
#define FUNERAL_TITLE_SHADOW_COLOR FONT_MCOLOR_DKWHITE

#define FUNERAL_RIP_SHADOW_COLOR FONT_MCOLOR_DKWHITE

#define FUNERAL_MCGILICUTTYS_SIGN_X LAPTOP_SCREEN_UL_X + 92
#define FUNERAL_MCGILICUTTYS_SIGN_Y LAPTOP_SCREEN_WEB_UL_Y + 0

#define FUNERAL_MORTUARY_SIGN_X LAPTOP_SCREEN_UL_X + 58
#define FUNERAL_MORTUARY_SIGN_Y LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_LEFT_COLUMN_X LAPTOP_SCREEN_UL_X + 0
#define FUNERAL_LEFT_COLUMN_Y LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_RIGHT_COLUMN_X LAPTOP_SCREEN_UL_X + 442
#define FUNERAL_RIGHT_COLUMN_Y LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_LINK_1_X LAPTOP_SCREEN_UL_X + 37
#define FUNERAL_LINK_1_Y LAPTOP_SCREEN_WEB_UL_Y + 329
#define FUNERAL_LINK_1_WIDTH 85
#define FUNERAL_LINK_1_HEIGHT 60

#define FUNERAL_LINK_OFFSET_X 85
#define FUNERAL_NUMBER_OF_LINKS 5

#define FUNERAL_LINK_TEXT_OFFSET_X 4
#define FUNERAL_LINK_TEXT_OFFSET_Y 17
#define FUNERAL_LINK_TEXT_WIDTH 76

#define FUNERAL_MARBLE_WIDTH 125
#define FUNERAL_MARBLE_HEIGHT 100

#define FUNERAL_SENTENCE_WIDTH 380

#define FUNERAL_SENTENCE_1_X LAPTOP_SCREEN_UL_X + 60
#define FUNERAL_SENTENCE_1_Y LAPTOP_SCREEN_WEB_UL_Y + 164

#define FUNERAL_SENTENCE_2_X FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_2_Y LAPTOP_SCREEN_WEB_UL_Y + 198

#define FUNERAL_SENTENCE_3_X FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_3_Y LAPTOP_SCREEN_WEB_UL_Y + 227

#define FUNERAL_SENTENCE_4_X FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_4_Y LAPTOP_SCREEN_WEB_UL_Y + 261

#define FUNERAL_SENTENCE_5_X FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_5_Y LAPTOP_SCREEN_WEB_UL_Y + 303

#define FUNERAL_CLOSED_RIP_SIGN_X LAPTOP_SCREEN_UL_X + 72
#define FUNERAL_CLOSED_RIP_SIGN_Y LAPTOP_SCREEN_WEB_UL_Y + 151
#define FUNERAL_CLOSED_WIDTH 364
#define FUNERAL_CLOSED_HEIGHT 204

#define FUNERAL_RIP_SENTENCE_WIDTH 260

#define FUNERAL_RIP_SENTENCE_1_X FUNERAL_CLOSED_RIP_SIGN_X + 55
#define FUNERAL_RIP_SENTENCE_1_Y FUNERAL_CLOSED_RIP_SIGN_Y + 98

#define FUNERAL_RIP_SENTENCE_2_X FUNERAL_RIP_SENTENCE_1_X
#define FUNERAL_RIP_SENTENCE_2_Y FUNERAL_CLOSED_RIP_SIGN_Y + 162

// Image Identifiers
uint32_t guiClosedSign;
uint32_t guiLeftColumn;
uint32_t guiLinkCarving;
uint32_t guiMarbleBackground;
uint32_t guiMcGillicuttys;
uint32_t guiMortuary;
uint32_t guiRightColumn;

// Clicking on Funeral link
struct MOUSE_REGION gSelectedFuneralLinkRegion[FUNERAL_NUMBER_OF_LINKS];
void SelectFuneralLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

// Clicking on rip sign to make it disappear
struct MOUSE_REGION gSelectedRipSignRegion;
void SelectRipSignRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

void DisplayFuneralRipTombStone();

void GameInitFuneral() {}

BOOLEAN EnterFuneral() {
  VOBJECT_DESC VObjectDesc;
  uint16_t usPosX, i;

  // load the Closed graphic and add it
  GetMLGFilename(VObjectDesc.ImageFile, MLG_CLOSED);
  if (!AddVideoObject(&VObjectDesc, &guiClosedSign)) {
    return FALSE;
  }

  // load the Left column graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\LeftColumn.sti", &guiLeftColumn)) {
    return FALSE;
  }

  // load the Link carving graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\LinkCarving.sti", &guiLinkCarving)) {
    return FALSE;
  }

  // load the Marble graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\Marble.sti", &guiMarbleBackground)) {
    return FALSE;
  }

  // load the McGillicuttys sign graphic and add it
  GetMLGFilename(VObjectDesc.ImageFile, MLG_MCGILLICUTTYS);
  if (!AddVideoObject(&VObjectDesc, &guiMcGillicuttys)) {
    return FALSE;
  }

  // load the Mortuary  graphic and add it
  GetMLGFilename(VObjectDesc.ImageFile, MLG_MORTUARY);
  if (!AddVideoObject(&VObjectDesc, &guiMortuary)) {
    return FALSE;
  }

  // load the right column graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\RightColumn.sti", &guiRightColumn)) {
    return FALSE;
  }

  usPosX = FUNERAL_LINK_1_X;
  for (i = 0; i < FUNERAL_NUMBER_OF_LINKS; i++) {
    // Mouse region for the bottom links

    MSYS_DefineRegion(&gSelectedFuneralLinkRegion[i], usPosX, FUNERAL_LINK_1_Y,
                      (uint16_t)(usPosX + FUNERAL_LINK_1_WIDTH),
                      (uint16_t)(FUNERAL_LINK_1_Y + FUNERAL_LINK_1_HEIGHT), MSYS_PRIORITY_HIGH,
                      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFuneralLinkRegionCallBack);
    MSYS_AddRegion(&gSelectedFuneralLinkRegion[i]);
    MSYS_SetRegionUserData(&gSelectedFuneralLinkRegion[i], 0, i);

    usPosX += FUNERAL_LINK_OFFSET_X;
  }

  MSYS_DefineRegion(&gSelectedRipSignRegion, FUNERAL_CLOSED_RIP_SIGN_X, FUNERAL_CLOSED_RIP_SIGN_Y,
                    (uint16_t)(FUNERAL_CLOSED_RIP_SIGN_X + FUNERAL_CLOSED_WIDTH),
                    (uint16_t)(FUNERAL_CLOSED_RIP_SIGN_Y + FUNERAL_CLOSED_HEIGHT),
                    MSYS_PRIORITY_HIGH + 1, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                    SelectRipSignRegionCallBack);
  MSYS_AddRegion(&gSelectedRipSignRegion);
  MSYS_DisableRegion(&gSelectedRipSignRegion);

  SetBookMark(FUNERAL_BOOKMARK);

  return (TRUE);
}

void ExitFuneral() {
  uint8_t i;

  DeleteVideoObjectFromIndex(guiClosedSign);
  DeleteVideoObjectFromIndex(guiLeftColumn);
  DeleteVideoObjectFromIndex(guiLinkCarving);
  DeleteVideoObjectFromIndex(guiMarbleBackground);
  DeleteVideoObjectFromIndex(guiMcGillicuttys);
  DeleteVideoObjectFromIndex(guiMortuary);
  DeleteVideoObjectFromIndex(guiRightColumn);

  for (i = 0; i < FUNERAL_NUMBER_OF_LINKS; i++) {
    MSYS_RemoveRegion(&gSelectedFuneralLinkRegion[i]);
  }

  MSYS_RemoveRegion(&gSelectedRipSignRegion);
}

void HandleFuneral() {}

void RenderFuneral() {
  struct VObject* hPixHandle;
  uint16_t i, usPosX, usStringHeight;

  WebPageTileBackground(4, 4, FUNERAL_MARBLE_WIDTH, FUNERAL_MARBLE_HEIGHT, guiMarbleBackground);

  // LeftColumn
  GetVideoObject(&hPixHandle, guiLeftColumn);
  BltVObject(vsFB, hPixHandle, 0, FUNERAL_LEFT_COLUMN_X, FUNERAL_LEFT_COLUMN_Y);

  // Mcgillicuttys
  GetVideoObject(&hPixHandle, guiMcGillicuttys);
  BltVObject(vsFB, hPixHandle, 0, FUNERAL_MCGILICUTTYS_SIGN_X, FUNERAL_MCGILICUTTYS_SIGN_Y);

  // Mortuary
  GetVideoObject(&hPixHandle, guiMortuary);
  BltVObject(vsFB, hPixHandle, 0, FUNERAL_MORTUARY_SIGN_X, FUNERAL_MORTUARY_SIGN_Y);

  // right column
  GetVideoObject(&hPixHandle, guiRightColumn);
  BltVObject(vsFB, hPixHandle, 0, FUNERAL_RIGHT_COLUMN_X, FUNERAL_RIGHT_COLUMN_Y);

  // LinkCarving
  GetVideoObject(&hPixHandle, guiLinkCarving);

  usPosX = FUNERAL_LINK_1_X;
  for (i = 0; i < FUNERAL_NUMBER_OF_LINKS; i++) {
    BltVObject(vsFB, hPixHandle, 0, usPosX, FUNERAL_LINK_1_Y);

    // Calculate the height of the string, as it needs to be vertically centered.
    usStringHeight = IanWrappedStringHeight(0, 0, FUNERAL_LINK_TEXT_WIDTH, 2, FUNERAL_SENTENCE_FONT,
                                            0, sFuneralString[i + FUNERAL_SEND_FLOWERS], 0, 0, 0);
    DisplayWrappedString(
        (uint16_t)(usPosX + FUNERAL_LINK_TEXT_OFFSET_X),
        (uint16_t)(FUNERAL_LINK_1_Y + (FUNERAL_LINK_1_HEIGHT - usStringHeight) / 2),
        FUNERAL_LINK_TEXT_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_TITLE_COLOR,
        sFuneralString[i + FUNERAL_SEND_FLOWERS], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

    usPosX += FUNERAL_LINK_OFFSET_X;
  }

  // display all the sentences

  // sentence 1
  DisplayWrappedString(FUNERAL_SENTENCE_1_X, FUNERAL_SENTENCE_1_Y, FUNERAL_SENTENCE_WIDTH, 2,
                       FUNERAL_TITLE_FONT, FUNERAL_TITLE_COLOR, sFuneralString[FUNERAL_INTRO_1],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  SetFontShadow(FUNERAL_SENTENCE_SHADOW_COLOR);

  // sentence 2
  DisplayWrappedString(FUNERAL_SENTENCE_2_X, FUNERAL_SENTENCE_2_Y, FUNERAL_SENTENCE_WIDTH, 2,
                       FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_INTRO_2], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // sentence 3
  DisplayWrappedString(FUNERAL_SENTENCE_3_X, FUNERAL_SENTENCE_3_Y, FUNERAL_SENTENCE_WIDTH, 2,
                       FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_INTRO_3], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // sentence 4
  DisplayWrappedString(FUNERAL_SENTENCE_4_X, FUNERAL_SENTENCE_4_Y, FUNERAL_SENTENCE_WIDTH, 2,
                       FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_INTRO_4], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // sentence 5
  DisplayWrappedString(FUNERAL_SENTENCE_5_X, FUNERAL_SENTENCE_5_Y, FUNERAL_SENTENCE_WIDTH, 2,
                       FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_INTRO_5], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void DisplayFuneralRipTombStone() {
  struct VObject* hPixHandle;

  // rip tombstone
  GetVideoObject(&hPixHandle, guiClosedSign);
  BltVideoObjectOutlineShadowFromIndex(vsFB, guiClosedSign, 0, FUNERAL_CLOSED_RIP_SIGN_X + 5,
                                       FUNERAL_CLOSED_RIP_SIGN_Y + 5);
  BltVObject(vsFB, hPixHandle, 0, FUNERAL_CLOSED_RIP_SIGN_X, FUNERAL_CLOSED_RIP_SIGN_Y);

  SetFontShadow(FUNERAL_RIP_SHADOW_COLOR);

  // sentence 10
  DisplayWrappedString(FUNERAL_RIP_SENTENCE_1_X, FUNERAL_RIP_SENTENCE_1_Y,
                       FUNERAL_RIP_SENTENCE_WIDTH, 2, FUNERAL_SMALL_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_OUR_CONDOLENCES], FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);  // FUNERAL_TITLE_FONT

  // sentence 11
  DisplayWrappedString(FUNERAL_RIP_SENTENCE_2_X, FUNERAL_RIP_SENTENCE_2_Y,
                       FUNERAL_RIP_SENTENCE_WIDTH, 2, FUNERAL_SMALL_FONT, FUNERAL_SENTENCE_COLOR,
                       sFuneralString[FUNERAL_OUR_SYMPATHIES], FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  InvalidateRegion(FUNERAL_CLOSED_RIP_SIGN_X, FUNERAL_CLOSED_RIP_SIGN_Y,
                   FUNERAL_CLOSED_RIP_SIGN_X + FUNERAL_CLOSED_WIDTH + 5,
                   FUNERAL_CLOSED_RIP_SIGN_Y + FUNERAL_CLOSED_HEIGHT + 5);

  // enable the region to make the sign disappear
  MSYS_EnableRegion(&gSelectedRipSignRegion);
}

void SelectFuneralLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint32_t uiUserData;

    uiUserData = MSYS_GetRegionUserData(pRegion, 0);

    if (uiUserData == 0)
      GoToWebPage(FLORIST_BOOKMARK);
    else {
      RenderFuneral();
      DisplayFuneralRipTombStone();
    }

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectRipSignRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    MSYS_DisableRegion(&gSelectedRipSignRegion);
    fPausedReDrawScreenFlag = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}
