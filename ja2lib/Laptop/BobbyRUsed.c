#include "Laptop/BobbyRUsed.h"

#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
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

UINT32 guiUsedBackground;
UINT32 guiUsedGrid;

void GameInitBobbyRUsed() {}

BOOLEAN EnterBobbyRUsed() {
  VOBJECT_DESC VObjectDesc;

  // load the background graphic and add it
  CopyFilename("LAPTOP\\usedbackground.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiUsedBackground)) {
    return FALSE;
  }

  // load the gunsgrid graphic and add it
  CopyFilename("LAPTOP\\usedgrid.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiUsedGrid)) {
    return FALSE;
  }

  InitBobbyBrTitle();

  SetFirstLastPagesForUsed();
  //	CalculateFirstAndLastIndexs();

  // Draw menu bar
  InitBobbyMenuBar();

  RenderBobbyRUsed();

  return (TRUE);
}

void ExitBobbyRUsed() {
  DeleteVideoObjectFromIndex(guiUsedBackground);
  DeleteVideoObjectFromIndex(guiUsedGrid);
  DeleteBobbyMenuBar();
  DeleteBobbyBrTitle();
  DeleteMouseRegionForBigImage();

  giCurrentSubPage = gusCurWeaponIndex;
  guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_USED;
}

void HandleBobbyRUsed() {}

void RenderBobbyRUsed() {
  struct VObject* hPixHandle;

  WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
                        BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiUsedBackground);

  // Display title at top of page
  DisplayBobbyRBrTitle();

  // GunForm
  GetVideoObject(&hPixHandle, guiUsedGrid);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  DisplayItemInfo(BOBBYR_USED_ITEMS);

  UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}
