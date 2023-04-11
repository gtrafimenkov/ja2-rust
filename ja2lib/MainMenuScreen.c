#include "MainMenuScreen.h"

#include <string.h>

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "Globals.h"
#include "JA2Splash.h"
#include "JAScreens.h"
#include "Local.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameInit.h"
#include "SysGlobals.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define MAINMENU_TEXT_FILE "LoadScreens\\MainMenu.edt"
#define MAINMENU_RECORD_SIZE 80 * 2

// MENU ITEMS
enum {
  //	TITLE,
  NEW_GAME,
  LOAD_GAME,
  PREFERENCES,
  CREDITS,
  QUIT,
  NUM_MENU_ITEMS
};

#define MAINMENU_X ((640 - 214) / 2)
#define MAINMENU_TITLE_Y 75
#define MAINMENU_Y 277  // 200
#define MAINMENU_Y_SPACE 37

INT32 iMenuImages[NUM_MENU_ITEMS];
INT32 iMenuButtons[NUM_MENU_ITEMS];

UINT16 gusMainMenuButtonWidths[NUM_MENU_ITEMS];

static struct VObject *gMainMenuBackGroundImage;
static struct VObject *giJa2LogoImage;

struct MOUSE_REGION gBackRegion;
INT8 gbHandledMainMenu = 0;
BOOLEAN fInitialRender = FALSE;
// BOOLEAN						gfDoHelpScreen = 0;

BOOLEAN gfMainMenuScreenEntry = FALSE;
BOOLEAN gfMainMenuScreenExit = FALSE;

UINT32 guiMainMenuExitScreen = MAINMENU_SCREEN;

extern BOOLEAN gfLoadGameUponEntry;

void ExitMainMenu();
void MenuButtonCallback(GUI_BUTTON *btn, INT32 reason);
void HandleMainMenuInput();
void HandleMainMenuScreen();
void DisplayAssignmentText();
void ClearMainMenu();
void HandleHelpScreenInput();
void SelectMainMenuBackGroundRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason);
void SetMainMenuExitScreen(UINT32 uiNewScreen);
void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate);
BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate);
void RenderMainMenu();
void RestoreButtonBackGrounds();

UINT32 MainMenuScreenInit() {
  DebugMsg(TOPIC_JA2, DBG_INFO, String("Version Label: %S", zBuildInfo));
  DebugMsg(TOPIC_JA2, DBG_INFO, String("Version #:     %s", czVersionNumber));
  DebugMsg(TOPIC_JA2, DBG_INFO, String("Tracking #:    %S", zTrackingNumber));

  return (TRUE);
}

UINT32 MainMenuScreenHandle() {
  UINT32 cnt;

  if (guiSplashStartTime + 4000 > GetJA2Clock()) {
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
    SetMusicMode(MUSIC_NONE);
    return MAINMENU_SCREEN;  // The splash screen hasn't been up long enough yet.
  }
  if (guiSplashFrameFade) {  // Fade the splash screen.
    if (guiSplashFrameFade > 2)
      ShadowVideoSurfaceRectUsingLowPercentTable(vsFB, 0, 0, 640, 480);
    else if (guiSplashFrameFade > 1)
      VSurfaceColorFill(vsFB, 0, 0, 640, 480, 0);
    else {
      SetMusicMode(MUSIC_MAIN_MENU);
    }

    // while( uiTime + 100 > GetJA2Clock() );

    guiSplashFrameFade--;

    InvalidateScreen();
    EndFrameBufferRender();

    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

    return MAINMENU_SCREEN;
  }

  SetCurrentCursorFromDatabase(CURSOR_NORMAL);

  if (gfMainMenuScreenEntry) {
    InitMainMenu();
    gfMainMenuScreenEntry = FALSE;
    gfMainMenuScreenExit = FALSE;
    guiMainMenuExitScreen = MAINMENU_SCREEN;
    SetMusicMode(MUSIC_MAIN_MENU);
  }

  if (fInitialRender) {
    ClearMainMenu();
    RenderMainMenu();

    fInitialRender = FALSE;
  }

  RestoreButtonBackGrounds();

  // Render buttons
  for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
    MarkAButtonDirty(iMenuButtons[cnt]);
  }

  RenderButtons();

  EndFrameBufferRender();

  //	if ( gfDoHelpScreen )
  //		HandleHelpScreenInput();
  //	else
  HandleMainMenuInput();

  HandleMainMenuScreen();

  if (gfMainMenuScreenExit) {
    ExitMainMenu();
    gfMainMenuScreenExit = FALSE;
    gfMainMenuScreenEntry = TRUE;
  }

  if (guiMainMenuExitScreen != MAINMENU_SCREEN) gfMainMenuScreenEntry = TRUE;

  return (guiMainMenuExitScreen);
}

UINT32 MainMenuScreenShutdown() { return (FALSE); }

void HandleMainMenuScreen() {
  if (gbHandledMainMenu != 0) {
    // Exit according to handled value!
    switch (gbHandledMainMenu) {
      case QUIT:
        gfMainMenuScreenExit = TRUE;
        gfProgramIsRunning = FALSE;
        break;

      case NEW_GAME:

        //					gfDoHelpScreen = 1;
        //				gfMainMenuScreenExit = TRUE;
        //				if( !gfDoHelpScreen )
        //					SetMainMenuExitScreen( INIT_SCREEN );
        break;

      case LOAD_GAME:
        // Select the game which is to be restored
        guiPreviousOptionScreen = guiCurrentScreen;
        guiMainMenuExitScreen = SAVE_LOAD_SCREEN;
        gbHandledMainMenu = 0;
        gfSaveGame = FALSE;
        gfMainMenuScreenExit = TRUE;

        break;

      case PREFERENCES:
        guiPreviousOptionScreen = guiCurrentScreen;
        guiMainMenuExitScreen = OPTIONS_SCREEN;
        gbHandledMainMenu = 0;
        gfMainMenuScreenExit = TRUE;
        break;

      case CREDITS:
        guiMainMenuExitScreen = CREDIT_SCREEN;
        gbHandledMainMenu = 0;
        gfMainMenuScreenExit = TRUE;
        break;
    }
  }
}

BOOLEAN InitMainMenu() {
  // Check to see whatr saved game files exist
  InitSaveGameArray();

  // Create the background mouse mask
  CreateDestroyBackGroundMouseMask(TRUE);

  CreateDestroyMainMenuButtons(TRUE);

  // TODO: there is no reason to address loaded images as indexes
  //   just load the image, save the pointer, use the pointer, then remove the pointer when done
  //
  // TODO: create LoadVObjectFromFile, which load the image, returns the pointer, doesn't add that
  //   image to the internal list

  gMainMenuBackGroundImage = LoadVObjectFromFile("LOADSCREENS\\MainMenuBackGround.sti");
  giJa2LogoImage = LoadVObjectFromFile("LOADSCREENS\\Ja2Logo.sti");

  // load background graphic and add it
  if (!gMainMenuBackGroundImage) {
    return FALSE;
  }

  // load ja2 logo graphic and add it
  if (!giJa2LogoImage) {
    return FALSE;
  }

  // if there are no saved games, disable the button
  if (!IsThereAnySavedGameFiles()) DisableButton(iMenuButtons[LOAD_GAME]);

  gbHandledMainMenu = 0;
  fInitialRender = TRUE;

  SetPendingNewScreen(MAINMENU_SCREEN);
  guiMainMenuExitScreen = MAINMENU_SCREEN;

  InitGameOptions();

  DequeueAllKeyBoardEvents();

  return (TRUE);
}

void ExitMainMenu() {
  CreateDestroyBackGroundMouseMask(FALSE);
  CreateDestroyMainMenuButtons(FALSE);

  DeleteVideoObject(gMainMenuBackGroundImage);
  DeleteVideoObject(giJa2LogoImage);

  gMsgBox.uiExitScreen = MAINMENU_SCREEN;
}

void MenuButtonCallback(GUI_BUTTON *btn, INT32 reason) {
  INT8 bID;

  bID = (UINT8)((uintptr_t)btn->UserData[0]);

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // handle menu
    gbHandledMainMenu = bID;
    RenderMainMenu();

    if (gbHandledMainMenu == NEW_GAME) {
      SetMainMenuExitScreen(GAME_INIT_OPTIONS_SCREEN);
    } else if (gbHandledMainMenu == LOAD_GAME) {
      if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE;
    }

    btn->uiFlags &= (~BUTTON_CLICKED_ON);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    RenderMainMenu();
    btn->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void MenuButtonMoveCallback(GUI_BUTTON *btn, INT32 reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );
    RenderMainMenu();
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );
    RenderMainMenu();
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void HandleMainMenuInput() {
  InputAtom InputEvent;

  // Check for esc
  while (DequeueEvent(&InputEvent) == TRUE) {
    if (InputEvent.usEvent == KEY_UP) {
      switch (InputEvent.usParam) {
        /*
                                        case ESC:
                                                gbHandledMainMenu = QUIT;
                                                break;
        */

#ifdef JA2TESTVERSION
        case 'q':
          gbHandledMainMenu = NEW_GAME;
          gfMainMenuScreenExit = TRUE;
          SetMainMenuExitScreen(INIT_SCREEN);

          //						gfDoHelpScreen = 1;
          break;

        case 'i':
          SetPendingNewScreen(INTRO_SCREEN);
          gfMainMenuScreenExit = TRUE;
          break;
#endif

        case 'c':
          gbHandledMainMenu = LOAD_GAME;

          if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE;

          break;

        case 'o':
          gbHandledMainMenu = PREFERENCES;
          break;

        case 's':
          gbHandledMainMenu = CREDITS;
          break;
      }
    }
  }
}

void HandleHelpScreenInput() {
  InputAtom InputEvent;

  // Check for key
  while (DequeueEvent(&InputEvent) == TRUE) {
    switch (InputEvent.usEvent) {
      case KEY_UP:
        SetMainMenuExitScreen(INIT_SCREEN);
        break;
    }
  }
}

void ClearMainMenu() {
  UINT32 uiDestPitchBYTES;
  UINT8 *pDestBuf;

  // CLEAR THE FRAME BUFFER
  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  memset(pDestBuf, 0, SCREEN_HEIGHT * uiDestPitchBYTES);
  VSurfaceUnlock(vsFB);
  InvalidateScreen();
}

void SelectMainMenuBackGroundRegionCallBack(struct MOUSE_REGION *pRegion, INT32 iReason) {}

void SetMainMenuExitScreen(UINT32 uiNewScreen) {
  guiMainMenuExitScreen = uiNewScreen;

  // REmove the background region
  CreateDestroyBackGroundMouseMask(FALSE);

  gfMainMenuScreenExit = TRUE;
}

void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate) {
  static BOOLEAN fRegionCreated = FALSE;

  if (fCreate) {
    if (fRegionCreated) return;

    // Make a mouse region
    MSYS_DefineRegion(&(gBackRegion), 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                      MSYS_NO_CALLBACK, SelectMainMenuBackGroundRegionCallBack);
    // Add region
    MSYS_AddRegion(&(gBackRegion));

    fRegionCreated = TRUE;
  } else {
    if (!fRegionCreated) return;

    MSYS_RemoveRegion(&gBackRegion);
    fRegionCreated = FALSE;
  }
}

BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate) {
  static BOOLEAN fButtonsCreated = FALSE;
  INT32 cnt;
  SGPFILENAME filename;
  INT16 sSlot;
  INT32 iStartLoc = 0;
#ifndef _DEBUG
  CHAR16 zText[512];
#endif

  if (fCreate) {
    if (fButtonsCreated) return (TRUE);

    // reset the variable that allows the user to ALT click on the continue save btn to load the
    // save instantly
    gfLoadGameUponEntry = FALSE;

    // Load button images
    GetMLGFilename(filename, MLG_TITLETEXT);

    iMenuImages[NEW_GAME] = LoadButtonImage(filename, 0, 0, 1, 2, -1);
    sSlot = 0;
    iMenuImages[LOAD_GAME] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 6, 3, 4, 5, -1);
    iMenuImages[PREFERENCES] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 7, 7, 8, 9, -1);
    iMenuImages[CREDITS] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 13, 10, 11, 12, -1);
    iMenuImages[QUIT] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 14, 14, 15, 16, -1);

    for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
      switch (cnt) {
        case NEW_GAME:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((UINT16)iMenuImages[cnt], sSlot);
          break;
        case LOAD_GAME:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((UINT16)iMenuImages[cnt], 3);
          break;
        case PREFERENCES:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((UINT16)iMenuImages[cnt], 7);
          break;
        case CREDITS:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((UINT16)iMenuImages[cnt], 10);
          break;
        case QUIT:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((UINT16)iMenuImages[cnt], 15);
          break;
      }
      iMenuButtons[cnt] =
          QuickCreateButton(iMenuImages[cnt], (INT16)(320 - gusMainMenuButtonWidths[cnt] / 2),
                            (INT16)(MAINMENU_Y + (cnt * MAINMENU_Y_SPACE)), BUTTON_TOGGLE,
                            MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, MenuButtonCallback);
      if (iMenuButtons[cnt] == -1) {
        return (FALSE);
      }
      ButtonList[iMenuButtons[cnt]]->UserData[0] = cnt;

#ifndef _DEBUG
      // load up some info from the 'mainmenu.edt' file.  This makes sure the file is present. The
      // file is
      // 'marked' with a code that identifies the testers
      iStartLoc = MAINMENU_RECORD_SIZE * cnt;
      if (!LoadEncryptedDataFromFile(MAINMENU_TEXT_FILE, zText, iStartLoc, MAINMENU_RECORD_SIZE)) {
        // the file was not able to be loaded properly
        struct SOLDIERTYPE *pSoldier = NULL;

        if (IsSolActive(pSoldier) != TRUE) {
          // something is very wrong
          pSoldier->bActive = pSoldier->bLife;
        }
      }
#endif
    }

    fButtonsCreated = TRUE;
  } else {
    if (!fButtonsCreated) return (TRUE);

    // Delete images/buttons
    for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
      RemoveButton(iMenuButtons[cnt]);
      UnloadButtonImage(iMenuImages[cnt]);
    }
    fButtonsCreated = FALSE;
  }

  return (TRUE);
}

void RenderMainMenu() {
  BltVObject(vsSB, gMainMenuBackGroundImage, 0, 0, 0);
  BltVObject(vsFB, gMainMenuBackGroundImage, 0, 0, 0);

  BltVObject(vsFB, giJa2LogoImage, 0, 188, 15);
  BltVObject(vsSB, giJa2LogoImage, 0, 188, 15);

  DrawTextToScreen(gzCopyrightText[0], 0, 465, 640, FONT10ARIAL, FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  InvalidateRegion(0, 0, 640, 480);
}

void RestoreButtonBackGrounds() {}
