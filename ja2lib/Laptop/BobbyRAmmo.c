#include "Laptop/BobbyRAmmo.h"

#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Tactical/InterfaceItems.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

UINT32 guiAmmoBackground;
UINT32 guiAmmoGrid;

BOOLEAN DisplayAmmoInfo();

void GameInitBobbyRAmmo() {}

BOOLEAN EnterBobbyRAmmo() {
  // load the background graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\ammobackground.sti", &guiAmmoBackground)) {
    return FALSE;
  }

  // load the gunsgrid graphic and add it
  if (!AddVObjectFromFile("LAPTOP\\ammogrid.sti", &guiAmmoGrid)) {
    return FALSE;
  }

  InitBobbyBrTitle();

  SetFirstLastPagesForNew(IC_AMMO);
  //	CalculateFirstAndLastIndexs();

  // Draw menu bar
  InitBobbyMenuBar();

  RenderBobbyRAmmo();

  return (TRUE);
}

void ExitBobbyRAmmo() {
  DeleteVideoObjectFromIndex(guiAmmoBackground);
  DeleteVideoObjectFromIndex(guiAmmoGrid);
  DeleteBobbyMenuBar();

  DeleteBobbyBrTitle();
  DeleteMouseRegionForBigImage();

  giCurrentSubPage = gusCurWeaponIndex;
  guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_AMMO;
}

void HandleBobbyRAmmo() {}

void RenderBobbyRAmmo() {
  struct VObject* hPixHandle;

  WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
                        BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiAmmoBackground);

  // Display title at top of page
  DisplayBobbyRBrTitle();

  // GunForm
  GetVideoObject(&hPixHandle, guiAmmoGrid);
  BltVObjectSrcTrans(vsFB, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

  DisplayItemInfo(IC_AMMO);

  UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}
