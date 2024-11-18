#include "Laptop/AIMLinks.h"

#include "Laptop/AIM.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define AIM_LINK_TITLE_FONT FONT14ARIAL
#define AIM_LINK_TITLE_COLOR AIM_GREEN

#define AIM_LINK_FONT FONT12ARIAL
#define AIM_LINK_COLOR AIM_FONT_GOLD

#define AIM_LINK_NUM_LINKS 3

#define AIM_LINK_LINK_OFFSET_Y 94  // 90

#define AIM_LINK_LINK_WIDTH 420
#define AIM_LINK_LINK_HEIGHT 70

#define AIM_LINK_BOBBY_LINK_X LAPTOP_SCREEN_UL_X + 40
#define AIM_LINK_BOBBY_LINK_Y LAPTOP_SCREEN_WEB_UL_Y + 91

#define AIM_LINK_FUNERAL_LINK_X AIM_LINK_BOBBY_LINK_X
#define AIM_LINK_FUNERAL_LINK_Y AIM_LINK_BOBBY_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define AIM_LINK_INSURANCE_LINK_X AIM_LINK_BOBBY_LINK_X
#define AIM_LINK_INSURANCE_LINK_Y AIM_LINK_FUNERAL_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define AIM_LINK_TITLE_X IMAGE_OFFSET_X + 149
#define AIM_LINK_TITLE_Y AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 10
#define AIM_LINK_TITLE_WIDTH AIM_SYMBOL_WIDTH

#define AIM_LINK_LINK_TEXT_1_Y AIM_LINK_BOBBY_LINK_Y + 71
#define AIM_LINK_LINK_TEXT_2_Y AIM_LINK_FUNERAL_LINK_Y + 36
#define AIM_LINK_LINK_TEXT_3_Y AIM_LINK_INSURANCE_LINK_Y + 45

uint32_t guiBobbyLink;
uint32_t guiFuneralLink;
uint32_t guiInsuranceLink;
uint8_t gubLinkPages[] = {BOBBYR_BOOKMARK, FUNERAL_BOOKMARK, INSURANCE_BOOKMARK};

// Clicking on guys Face
struct MOUSE_REGION gSelectedLinkRegion[AIM_LINK_NUM_LINKS];
void SelectLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

void GameInitAimLinks() {}

BOOLEAN EnterAimLinks() {
  uint16_t usPosY;
  int16_t i;

  InitAimDefaults();
  InitAimMenuBar();

  // load the Bobby link graphic and add it
  SGPFILENAME ImageFile;
  GetMLGFilename(ImageFile, MLG_BOBBYRAYLINK);
  if (!AddVObjectFromFile(ImageFile, &guiBobbyLink)) {
    return FALSE;
  }

  // load the Funeral graphic and add it
  GetMLGFilename(ImageFile, MLG_MORTUARYLINK);
  if (!AddVObjectFromFile(ImageFile, &guiFuneralLink)) {
    return FALSE;
  }

  // load the Insurance graphic and add it
  GetMLGFilename(ImageFile, MLG_INSURANCELINK);
  if (!AddVObjectFromFile(ImageFile, &guiInsuranceLink)) {
    return FALSE;
  }

  usPosY = AIM_LINK_BOBBY_LINK_Y;
  for (i = 0; i < AIM_LINK_NUM_LINKS; i++) {
    MSYS_DefineRegion(&gSelectedLinkRegion[i], AIM_LINK_BOBBY_LINK_X, usPosY,
                      AIM_LINK_BOBBY_LINK_X + AIM_LINK_LINK_WIDTH,
                      (uint16_t)(usPosY + AIM_LINK_LINK_HEIGHT), MSYS_PRIORITY_HIGH, CURSOR_WWW,
                      MSYS_NO_CALLBACK, SelectLinkRegionCallBack);
    MSYS_AddRegion(&gSelectedLinkRegion[i]);
    MSYS_SetRegionUserData(&gSelectedLinkRegion[i], 0, gubLinkPages[i]);
    usPosY += AIM_LINK_LINK_OFFSET_Y;
  }

  RenderAimLinks();
  return (TRUE);
}

void ExitAimLinks() {
  int16_t i;

  RemoveAimDefaults();

  DeleteVideoObjectFromIndex(guiBobbyLink);
  DeleteVideoObjectFromIndex(guiFuneralLink);
  DeleteVideoObjectFromIndex(guiInsuranceLink);

  for (i = 0; i < AIM_LINK_NUM_LINKS; i++) MSYS_RemoveRegion(&gSelectedLinkRegion[i]);

  ExitAimMenuBar();
}

void HandleAimLinks() {}

void RenderAimLinks() {
  struct VObject* hPixHandle;

  DrawAimDefaults();
  DisableAimButton();

  GetVideoObject(&hPixHandle, guiBobbyLink);
  BltVObject(vsFB, hPixHandle, 0, AIM_LINK_BOBBY_LINK_X, AIM_LINK_BOBBY_LINK_Y);

  GetVideoObject(&hPixHandle, guiFuneralLink);
  BltVObject(vsFB, hPixHandle, 0, AIM_LINK_FUNERAL_LINK_X, AIM_LINK_FUNERAL_LINK_Y);
  //	DrawTextToScreen(AimLinkText[AIM_LINK_FUNERAL], AIM_LINK_BOBBY_LINK_X,
  // AIM_LINK_LINK_TEXT_2_Y, AIM_LINK_LINK_WIDTH, AIM_LINK_FONT, AIM_LINK_COLOR, FONT_MCOLOR_BLACK,
  // FALSE, CENTER_JUSTIFIED);

  GetVideoObject(&hPixHandle, guiInsuranceLink);
  BltVObject(vsFB, hPixHandle, 0, AIM_LINK_INSURANCE_LINK_X, AIM_LINK_INSURANCE_LINK_Y);
  //	DrawTextToScreen(AimLinkText[AIM_LINK_LISTENING], AIM_LINK_BOBBY_LINK_X,
  // AIM_LINK_LINK_TEXT_3_Y, AIM_LINK_LINK_WIDTH, AIM_LINK_FONT, AIM_LINK_COLOR, FONT_MCOLOR_BLACK,
  // FALSE, CENTER_JUSTIFIED);

  // Draw Link Title
  DrawTextToScreen(AimLinkText[AIM_LINK_TITLE], AIM_LINK_TITLE_X, AIM_LINK_TITLE_Y,
                   AIM_LINK_TITLE_WIDTH, AIM_LINK_TITLE_FONT, AIM_LINK_TITLE_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint32_t gNextLaptopPage;

    gNextLaptopPage = MSYS_GetRegionUserData(pRegion, 0);

    GoToWebPage(gNextLaptopPage);
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}
