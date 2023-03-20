#include "Laptop/BobbyRMisc.h"

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

UINT32 guiMiscBackground;
UINT32 guiMiscGrid;

void GameInitBobbyRMisc() {}

BOOLEAN EnterBobbyRMisc() {
  VOBJECT_DESC VObjectDesc;

  // load the background graphic and add it
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\miscbackground.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMiscBackground));

  // load the gunsgrid graphic and add it
  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\miscgrid.sti", VObjectDesc.ImageFile);
  CHECKF(AddVideoObject(&VObjectDesc, &guiMiscGrid));

  InitBobbyBrTitle();
  // Draw menu bar
  InitBobbyMenuBar();

  SetFirstLastPagesForNew(IC_BOBBY_MISC);
  //	CalculateFirstAndLastIndexs();

  RenderBobbyRMisc();

  return (TRUE);
}

void ExitBobbyRMisc() {
  DeleteVideoObjectFromIndex(guiMiscBackground);
  DeleteVideoObjectFromIndex(guiMiscGrid);
  DeleteBobbyBrTitle();
  DeleteMouseRegionForBigImage();
  DeleteBobbyMenuBar();

  guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_MISC;
}

void HandleBobbyRMisc() {}

void RenderBobbyRMisc() {
  struct VObject* hPixHandle;

  WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
                        BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiMiscBackground);

  // Display title at top of page
  DisplayBobbyRBrTitle();

  // GunForm
  GetVideoObject(&hPixHandle, guiMiscGrid);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  DisplayItemInfo(IC_BOBBY_MISC);

  UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}
