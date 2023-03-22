#include "SaveLoadScreen.h"

#include <stdio.h>

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "Local.h"
#include "Money.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "ScreenIDs.h"
#include "Strategic/CampaignInit.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/StrategicMap.h"
#include "SysGlobals.h"
#include "Tactical/MercHiring.h"
#include "Tactical/Overhead.h"
#include "Tactical/TacticalSave.h"
#include "TileEngine/RenderDirty.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "platform.h"
#include "rust_fileman.h"

BOOLEAN gfSchedulesHosed = FALSE;
extern uint32_t guiBrokenSaveGameVersion;

//////////////////////////////////////////////////////
//
//  Defines
//
//////////////////////////////////////////////////////

#define SAVE_LOAD_TITLE_FONT FONT14ARIAL
#define SAVE_LOAD_TITLE_COLOR FONT_MCOLOR_WHITE

#define SAVE_LOAD_NORMAL_FONT FONT12ARIAL
#define SAVE_LOAD_NORMAL_COLOR 2           // FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE
#define SAVE_LOAD_NORMAL_SHADOW_COLOR 118  // 121//118//125
/*#define		SAVE_LOAD_NORMAL_FONT
FONT12ARIAL #define		SAVE_LOAD_NORMAL_COLOR
FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE #define		SAVE_LOAD_NORMAL_SHADOW_COLOR
2//125
*/

#define SAVE_LOAD_QUICKSAVE_FONT FONT12ARIAL
#define SAVE_LOAD_QUICKSAVE_COLOR 2           // FONT_MCOLOR_DKGRAY//FONT_MCOLOR_WHITE
#define SAVE_LOAD_QUICKSAVE_SHADOW_COLOR 189  // 248//2

#define SAVE_LOAD_EMPTYSLOT_FONT FONT12ARIAL
#define SAVE_LOAD_EMPTYSLOT_COLOR 2           // 125//FONT_MCOLOR_WHITE
#define SAVE_LOAD_EMPTYSLOT_SHADOW_COLOR 121  // 118

#define SAVE_LOAD_HIGHLIGHTED_FONT FONT12ARIAL
#define SAVE_LOAD_HIGHLIGHTED_COLOR FONT_MCOLOR_WHITE
#define SAVE_LOAD_HIGHLIGHTED_SHADOW_COLOR 2

#define SAVE_LOAD_SELECTED_FONT FONT12ARIAL
#define SAVE_LOAD_SELECTED_COLOR 2           // 145//FONT_MCOLOR_WHITE
#define SAVE_LOAD_SELECTED_SHADOW_COLOR 130  // 2

#define SAVE_LOAD_NUMBER_FONT FONT12ARIAL
#define SAVE_LOAD_NUMBER_COLOR FONT_MCOLOR_WHITE

#define SLG_SELECTED_COLOR FONT_MCOLOR_WHITE
#define SLG_UNSELECTED_COLOR FONT_MCOLOR_DKWHITE

#define SLG_SAVELOCATION_WIDTH 605
#define SLG_SAVELOCATION_HEIGHT 30  // 46
#define SLG_FIRST_SAVED_SPOT_X 17
#define SLG_FIRST_SAVED_SPOT_Y 49
#define SLG_GAP_BETWEEN_LOCATIONS 35  // 47

#define SLG_DATE_OFFSET_X 13
#define SLG_DATE_OFFSET_Y 11

#define SLG_SECTOR_OFFSET_X 95  // 105//114
#define SLG_SECTOR_OFFSET_Y SLG_DATE_OFFSET_Y
#define SLG_SECTOR_WIDTH 98

#define SLG_NUM_MERCS_OFFSET_X 196                // 190//SLG_DATE_OFFSET_X
#define SLG_NUM_MERCS_OFFSET_Y SLG_DATE_OFFSET_Y  // 26

#define SLG_BALANCE_OFFSET_X 260                // SLG_SECTOR_OFFSET_X
#define SLG_BALANCE_OFFSET_Y SLG_DATE_OFFSET_Y  // SLG_NUM_MERCS_OFFSET_Y

#define SLG_SAVE_GAME_DESC_X 318                // 320//204
#define SLG_SAVE_GAME_DESC_Y SLG_DATE_OFFSET_Y  // SLG_DATE_OFFSET_Y + 7

#define SLG_TITLE_POS_X 0
#define SLG_TITLE_POS_Y 0

#define SLG_SAVE_CANCEL_POS_X 226  // 329
#define SLG_LOAD_CANCEL_POS_X 329
#define SLG_CANCEL_POS_Y 438

#define SLG_SAVE_LOAD_BTN_POS_X 123
#define SLG_SAVE_LOAD_BTN_POS_Y 438

#define SLG_SELECTED_SLOT_GRAPHICS_NUMBER 3
#define SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER 2

#define SLG_DOUBLE_CLICK_DELAY 500

// defines for saved game version status
enum {
  SLS_HEADER_OK,
  SLS_SAVED_GAME_VERSION_OUT_OF_DATE,
  SLS_GAME_VERSION_OUT_OF_DATE,
  SLS_BOTH_SAVE_GAME_AND_GAME_VERSION_OUT_OF_DATE,
};

// ddd

//////////////////////////////////////////////////////
//
//  Global Variables
//
//////////////////////////////////////////////////////

extern void NextLoopCheckForEnoughFreeHardDriveSpace();

BOOLEAN gfSaveLoadScreenEntry = TRUE;
BOOLEAN gfSaveLoadScreenExit = FALSE;
BOOLEAN gfRedrawSaveLoadScreen = TRUE;

BOOLEAN gfExitAfterMessageBox = FALSE;
int32_t giSaveLoadMessageBox = -1;  // SaveLoad pop up messages index value

uint32_t guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;

// Contains the array of valid save game locations
BOOLEAN gbSaveGameArray[NUM_SAVE_GAMES];

BOOLEAN gfDoingQuickLoad = FALSE;

BOOLEAN gfFailedToSaveGameWhenInsideAMessageBox = FALSE;

// This flag is used to diferentiate between loading a game and saveing a game.
// gfSaveGame=TRUE		For saving a game
// gfSaveGame=FALSE		For loading a game
BOOLEAN gfSaveGame = TRUE;

BOOLEAN gfSaveLoadScreenButtonsCreated = FALSE;

int8_t gbSaveGameSelectedLocation[NUM_SAVE_GAMES];
int8_t gbSelectedSaveLocation = -1;
int8_t gbHighLightedLocation = -1;
int8_t gbLastHighLightedLocation = -1;
int8_t gbSetSlotToBeSelected = -1;

uint32_t guiSlgBackGroundImage;
uint32_t guiBackGroundAddOns;

// The string that will contain the game desc text
wchar_t gzGameDescTextField[SIZE_OF_SAVE_GAME_DESC] = {0};

BOOLEAN gfUserInTextInputMode = FALSE;
uint8_t gubSaveGameNextPass = 0;

BOOLEAN gfStartedFadingOut = FALSE;

BOOLEAN gfCameDirectlyFromGame = FALSE;

BOOLEAN gfLoadedGame = FALSE;  // Used to know when a game has been loaded, the flag in
                               // gtacticalstatus might have been reset already

BOOLEAN gfLoadGameUponEntry = FALSE;

BOOLEAN gfHadToMakeBasementLevels = FALSE;

BOOLEAN gfGettingNameFromSaveLoadScreen = FALSE;

#ifdef JA2BETAVERSION
extern BOOLEAN gfDisplaySaveGamesNowInvalidatedMsg;
#endif
// ggg

//
// Buttons
//
int32_t guiSlgButtonImage;

// Cancel Button
void BtnSlgCancelCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiSlgCancelBtn;

// Save game Button
void BtnSlgSaveLoadCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiSlgSaveLoadBtn;
int32_t guiSaveLoadImage;

// Mouse regions for the currently selected save game
struct MOUSE_REGION gSelectedSaveRegion[NUM_SAVE_GAMES];
void SelectedSaveRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectedSaveRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason);

struct MOUSE_REGION gSLSEntireScreenRegion;
void SelectedSLSEntireRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

//////////////////////////////////////////////////////
//
//  Function Prototypes
//
//////////////////////////////////////////////////////

BOOLEAN EnterSaveLoadScreen();
void RenderSaveLoadScreen();
void ExitSaveLoadScreen();
void HandleSaveLoadScreen();
void GetSaveLoadScreenUserInput();
void SaveLoadGameNumber(int8_t bSaveGameID);
BOOLEAN LoadSavedGameHeader(int8_t bEntry, SAVED_GAME_HEADER *pSaveGameHeader);
BOOLEAN DisplaySaveGameEntry(int8_t bEntryID);  //, uint16_t usPosY );
BOOLEAN DisplaySaveGameList();
void InitSaveLoadScreenTextInputBoxes();
void DestroySaveLoadTextInputBoxes();
void SetSelection(uint8_t ubNewSelection);
uint8_t CompareSaveGameVersion(int8_t bSaveGameID);
void LoadSavedGameWarningMessageBoxCallBack(uint8_t bExitValue);
void LoadSavedGameDeleteAllSaveGameMessageBoxCallBack(uint8_t bExitValue);
void DeleteAllSaveGameFile();
void DisplayOnScreenNumber(BOOLEAN fErase);

void DisableSelectedSlot();

void ConfirmSavedGameMessageBoxCallBack(uint8_t bExitValue);
void ConfirmLoadSavedGameMessageBoxCallBack(uint8_t bExitValue);
// void			FailedSavingGameCallBack( uint8_t bExitValue );
void FailedLoadingGameCallBack(uint8_t bExitValue);
void SetSaveLoadExitScreen(uint32_t uiScreen);

void NotEnoughHardDriveSpaceForQuickSaveMessageBoxCallBack(uint8_t bExitValue);
void NotEnoughHardDriveSpaceForNormalSaveMessageBoxCallBack(uint8_t bExitValue);

void RedrawSaveLoadScreenAfterMessageBox(uint8_t bExitValue);
void ClearSelectedSaveSlot();
void MoveSelectionUpOrDown(BOOLEAN fUp);
void SaveGameToSlotNum();
void StartFadeOutForSaveLoadScreen();

// ppp

//////////////////////////////////////////////////////
//
//  Code
//
//////////////////////////////////////////////////////

uint32_t SaveLoadScreenInit() {
  // Set so next time we come in, we can set up
  gfSaveLoadScreenEntry = TRUE;

  memset(gbSaveGameArray, -1, NUM_SAVE_GAMES);

  ClearSelectedSaveSlot();

  gbHighLightedLocation = -1;

  return (TRUE);
}

uint32_t SaveLoadScreenHandle() {
  StartFrameBufferRender();

  if (gfSaveLoadScreenEntry) {
    EnterSaveLoadScreen();
    gfSaveLoadScreenEntry = FALSE;
    gfSaveLoadScreenExit = FALSE;

    PauseGame();

    // save the new rect
    VSurfaceBlitBufToBuf(vsFB, vsSaveBuffer, 0, 0, 639, 439);
  }

  RestoreBackgroundRects();

  // to guarentee that we do not accept input when we are fading out
  if (!gfStartedFadingOut) {
    GetSaveLoadScreenUserInput();
  } else
    gfRedrawSaveLoadScreen = FALSE;

  // if we have exited the save load screen, exit
  if (!gfSaveLoadScreenButtonsCreated) return (guiSaveLoadExitScreen);

  RenderAllTextFields();

  HandleSaveLoadScreen();

  if (gfRedrawSaveLoadScreen) {
    RenderSaveLoadScreen();
    MarkButtonsDirty();
    RenderButtons();

    gfRedrawSaveLoadScreen = FALSE;
  }

  if (gubSaveGameNextPass != 0) {
    gubSaveGameNextPass++;

    if (gubSaveGameNextPass == 5) {
      gubSaveGameNextPass = 0;
      SaveLoadGameNumber(gbSelectedSaveLocation);
    }
  }

  // If we are not exiting the screen, render the buttons
  if (!gfSaveLoadScreenExit && guiSaveLoadExitScreen == SAVE_LOAD_SCREEN) {
    // render buttons marked dirty
    RenderButtons();
  }

  // ATE: Put here to save RECTS before any fast help being drawn...
  SaveBackgroundRects();
  RenderButtonsFastHelp();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  if (HandleFadeOutCallback()) {
    return (guiSaveLoadExitScreen);
  }

  if (HandleBeginFadeOut(SAVE_LOAD_SCREEN)) {
    return (SAVE_LOAD_SCREEN);
  }

  if (gfSaveLoadScreenExit) {
    ExitSaveLoadScreen();
  }

  if (HandleFadeInCallback()) {
    // Re-render the scene!
    RenderSaveLoadScreen();
  }

  if (HandleBeginFadeIn(SAVE_LOAD_SCREEN)) {
  }

  return (guiSaveLoadExitScreen);
}

uint32_t SaveLoadScreenShutdown() { return (TRUE); }

void SetSaveLoadExitScreen(uint32_t uiScreen) {
  if (uiScreen == GAME_SCREEN) {
    EnterTacticalScreen();
  }

  // If we are currently in the Message box loop
  //	if( gfExitAfterMessageBox )
  //		ExitSaveLoadScreen();

  gfSaveLoadScreenExit = TRUE;

  guiSaveLoadExitScreen = uiScreen;

  SetPendingNewScreen(uiScreen);

  if (gfDoingQuickLoad) {
    fFirstTimeInGameScreen = TRUE;
    SetPendingNewScreen(uiScreen);
  }

  ExitSaveLoadScreen();

  DestroySaveLoadTextInputBoxes();
}

BOOLEAN EnterSaveLoadScreen() {
  int8_t i;
  VOBJECT_DESC VObjectDesc;
  uint16_t usPosX = SLG_FIRST_SAVED_SPOT_X;
  uint16_t usPosY = SLG_FIRST_SAVED_SPOT_Y;

  //	if( guiPreviousOptionScreen != MAINMENU_SCREEN )
  //		gbSetSlotToBeSelected = -1;

  // This is a hack to get sector names , but... if the underground sector is NOT loaded
  if (!gpUndergroundSectorInfoHead) {
    BuildUndergroundSectorInfoList();
    gfHadToMakeBasementLevels = TRUE;
  } else {
    gfHadToMakeBasementLevels = FALSE;
  }

  guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;
  // init the list
  InitSaveGameArray();

  // Clear out all the saved background rects
  EmptyBackgroundRects();

  // if the user has asked to load the selected save
  if (gfLoadGameUponEntry) {
    // make sure the save is valid
    if (gGameSettings.bLastSavedGameSlot != -1 &&
        gbSaveGameArray[gGameSettings.bLastSavedGameSlot]) {
      gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot;

      // load the saved game
      ConfirmLoadSavedGameMessageBoxCallBack(MSG_BOX_RETURN_YES);
    } else {  // else the save isnt valid, so dont load it
      gfLoadGameUponEntry = FALSE;
    }
  }

  // load Main background  graphic and add it
  if (!AddVObjectFromFile("INTERFACE\\LoadScreen.sti", &guiSlgBackGroundImage)) {
    return FALSE;
  }

  // load Load Screen Add ons graphic and add it
  GetMLGFilename(VObjectDesc.ImageFile, MLG_LOADSAVEHEADER);
  if (!AddVideoObject(&VObjectDesc, &guiBackGroundAddOns)) {
    return FALSE;
  }

  guiSlgButtonImage = LoadButtonImage("INTERFACE\\LoadScreenAddOns.sti", -1, 6, -1, 9, -1);
  //	guiSlgButtonImage = UseLoadedButtonImage( guiBackGroundAddOns, -1,9,-1,6,-1 );

  // Cancel button
  //	if( gfSaveGame )
  //		usPosX = SLG_SAVE_CANCEL_POS_X;
  //	else
  usPosX = SLG_LOAD_CANCEL_POS_X;

  guiSlgCancelBtn = CreateIconAndTextButton(
      guiSlgButtonImage, zSaveLoadText[SLG_CANCEL], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
      DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, usPosX,
      SLG_CANCEL_POS_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnSlgCancelCallback);

  // Either the save or load button
  if (gfSaveGame) {
    // If we are saving, dont have the save game button
    guiSaveLoadImage = UseLoadedButtonImage(guiSlgButtonImage, -1, 5, -1, 8, -1);

    guiSlgSaveLoadBtn = CreateIconAndTextButton(
        guiSaveLoadImage, zSaveLoadText[SLG_SAVE_GAME], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
        DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
        SLG_SAVE_LOAD_BTN_POS_X, SLG_SAVE_LOAD_BTN_POS_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
        DEFAULT_MOVE_CALLBACK, BtnSlgSaveLoadCallback);

  } else {
    guiSaveLoadImage = UseLoadedButtonImage(guiSlgButtonImage, -1, 4, -1, 7, -1);

    guiSlgSaveLoadBtn = CreateIconAndTextButton(
        guiSaveLoadImage, zSaveLoadText[SLG_LOAD_GAME], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
        DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
        SLG_SAVE_LOAD_BTN_POS_X, SLG_SAVE_LOAD_BTN_POS_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
        DEFAULT_MOVE_CALLBACK, BtnSlgSaveLoadCallback);
  }

  // if we are loading, disable the load button
  //	if( !gfSaveGame )
  {
    SpecifyDisabledButtonStyle(guiSlgSaveLoadBtn, DISABLED_STYLE_HATCHED);

    if (gbSetSlotToBeSelected == -1) DisableButton(guiSlgSaveLoadBtn);
  }

  usPosX = SLG_FIRST_SAVED_SPOT_X;
  usPosY = SLG_FIRST_SAVED_SPOT_Y;
  for (i = 0; i < NUM_SAVE_GAMES; i++) {
    MSYS_DefineRegion(
        &gSelectedSaveRegion[i], usPosX, usPosY, (uint16_t)(usPosX + SLG_SAVELOCATION_WIDTH),
        (uint16_t)(usPosY + SLG_SAVELOCATION_HEIGHT), MSYS_PRIORITY_HIGH, CURSOR_NORMAL,
        SelectedSaveRegionMovementCallBack, SelectedSaveRegionCallBack);
    MSYS_AddRegion(&gSelectedSaveRegion[i]);
    MSYS_SetRegionUserData(&gSelectedSaveRegion[i], 0, i);

    // if we are to Load a game
    if (!gfSaveGame) {
      // We cannot load a game that hasnt been saved
      if (!gbSaveGameArray[i]) MSYS_DisableRegion(&gSelectedSaveRegion[i]);
    }

    usPosY += SLG_GAP_BETWEEN_LOCATIONS;
  }

  /*
  Removed so that the user can click on it and get displayed a message that the quick save slot is
  for the tactical screen if( gfSaveGame )
          {
                  MSYS_DisableRegion( &gSelectedSaveRegion[0] );
          }
  */

  // Create the screen mask to enable ability to righ click to cancel the sace game
  MSYS_DefineRegion(&gSLSEntireScreenRegion, 0, 0, 639, 479, MSYS_PRIORITY_HIGH - 10, CURSOR_NORMAL,
                    MSYS_NO_CALLBACK, SelectedSLSEntireRegionCallBack);
  MSYS_AddRegion(&gSLSEntireScreenRegion);

  // Reset the regions
  //	for( i=0; i<NUM_SAVE_GAMES; i++)
  //		gbSaveGameSelectedLocation[i] = SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;
  //	gbSelectedSaveLocation=-1;
  ClearSelectedSaveSlot();

  // Remove the mouse region over the clock
  RemoveMouseRegionForPauseOfClock();

  // Draw the screen
  //	gfRedrawSaveLoadScreen = TRUE;	DEF:

  // Reset the highlight
  gbHighLightedLocation = -1;

  gzGameDescTextField[0] = '\0';

  // if we are loading
  //	if( !gfSaveGame )
  {
    SpecifyDisabledButtonStyle(guiSlgSaveLoadBtn, DISABLED_STYLE_HATCHED);

    // if the last saved game slot is ok, set the selected slot to the last saved slot]
    if (gGameSettings.bLastSavedGameSlot != -1) {
      // if the slot is valid
      if (gbSaveGameArray[gGameSettings.bLastSavedGameSlot]) {
        SAVED_GAME_HEADER SaveGameHeader;

        memset(&SaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));

        // if it is not the Quick Save slot, and we are loading
        if (!gfSaveGame || (gfSaveGame && gGameSettings.bLastSavedGameSlot != 0)) {
          gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot;
          gbSaveGameSelectedLocation[gbSelectedSaveLocation] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;

          // load the save gamed header string

          // Get the heade for the saved game
          if (!LoadSavedGameHeader(gbSelectedSaveLocation, &SaveGameHeader)) {
            memset(&SaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));
            gbSaveGameSelectedLocation[gbSelectedSaveLocation] =
                SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;
            gbSaveGameArray[gbSelectedSaveLocation] = FALSE;
            gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot = -1;
          }

          wcscpy(gzGameDescTextField, SaveGameHeader.sSavedGameDesc);
        }
      }
    }

    // if we are loading and the there is no slot selected
    if (gbSelectedSaveLocation == -1)
      DisableButton(guiSlgSaveLoadBtn);
    else
      EnableButton(guiSlgSaveLoadBtn);
  }

  /*
          if( gbSetSlotToBeSelected != -1 )
          {
                  gbSelectedSaveLocation = gbSetSlotToBeSelected;
                  gbSetSlotToBeSelected = -1;

                  //Set the selected slot background graphic
                  gbSaveGameSelectedLocation[ gbSelectedSaveLocation ] =
     SLG_SELECTED_SLOT_GRAPHICS_NUMBER;
          }
  */

  RenderSaveLoadScreen();

  // Save load buttons are created
  gfSaveLoadScreenButtonsCreated = TRUE;

  gfDoingQuickLoad = FALSE;

  // reset
  gfStartedFadingOut = FALSE;

  DisableScrollMessages();

  gfLoadedGame = FALSE;

  if (gfLoadGameUponEntry) {
    uint32_t uiDestPitchBYTES;
    uint8_t *pDestBuf;

    // unmark the 2 buttons from being dirty
    ButtonList[guiSlgCancelBtn]->uiFlags |= BUTTON_FORCE_UNDIRTY;
    ButtonList[guiSlgSaveLoadBtn]->uiFlags |= BUTTON_FORCE_UNDIRTY;

    // CLEAR THE FRAME BUFFER
    pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
    memset(pDestBuf, 0, SCREEN_HEIGHT * uiDestPitchBYTES);
    VSurfaceUnlock(vsFB);

    // CLEAR THE FRAME_BUFFER
    pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
    memset(pDestBuf, 0, SCREEN_HEIGHT * uiDestPitchBYTES);
    VSurfaceUnlock(vsFB);
  }

  gfGettingNameFromSaveLoadScreen = FALSE;

  return (TRUE);
}

void ExitSaveLoadScreen() {
  int8_t i;

  gfLoadGameUponEntry = FALSE;

  if (!gfSaveLoadScreenButtonsCreated) return;

  gfSaveLoadScreenExit = FALSE;
  gfSaveLoadScreenEntry = TRUE;
  gfExitAfterMessageBox = FALSE;

  UnloadButtonImage(guiSlgButtonImage);

  RemoveButton(guiSlgCancelBtn);

  // Remove the save / load button
  //	if( !gfSaveGame )
  {
    RemoveButton(guiSlgSaveLoadBtn);
    UnloadButtonImage(guiSaveLoadImage);
  }

  for (i = 0; i < NUM_SAVE_GAMES; i++) {
    MSYS_RemoveRegion(&gSelectedSaveRegion[i]);
  }

  DeleteVideoObjectFromIndex(guiSlgBackGroundImage);
  DeleteVideoObjectFromIndex(guiBackGroundAddOns);

  // Destroy the text fields ( if created )
  DestroySaveLoadTextInputBoxes();

  MSYS_RemoveRegion(&gSLSEntireScreenRegion);

  gfSaveLoadScreenEntry = TRUE;
  gfSaveLoadScreenExit = FALSE;

  if (!gfLoadedGame) {
    UnLockPauseState();
    UnPauseGame();
  }

  gfSaveLoadScreenButtonsCreated = FALSE;

  gfCameDirectlyFromGame = FALSE;

  // unload the basement sectors
  if (gfHadToMakeBasementLevels) TrashUndergroundSectorInfo();

  gfGettingNameFromSaveLoadScreen = FALSE;
}

void RenderSaveLoadScreen() {
  struct VObject *hPixHandle;

  // if we are going to be instantly leaving the screen, dont draw the numbers
  if (gfLoadGameUponEntry) {
    return;
  }

  GetVideoObject(&hPixHandle, guiSlgBackGroundImage);
  BltVideoObject2(vsFB, hPixHandle, 0, 0, 0, VO_BLT_SRCTRANSPARENCY, NULL);

  if (gfSaveGame) {
    // If we are saving a game

    // Display the Title
    //		DrawTextToScreen( zSaveLoadText[SLG_SAVE_GAME], 0, 10, 639, SAVE_LOAD_TITLE_FONT,
    // SAVE_LOAD_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
    GetVideoObject(&hPixHandle, guiBackGroundAddOns);
    BltVideoObject2(vsFB, hPixHandle, 1, SLG_TITLE_POS_X, SLG_TITLE_POS_Y, VO_BLT_SRCTRANSPARENCY,
                    NULL);
  } else {
    // If we are Loading a game

    // Display the Title
    GetVideoObject(&hPixHandle, guiBackGroundAddOns);
    BltVideoObject2(vsFB, hPixHandle, 0, SLG_TITLE_POS_X, SLG_TITLE_POS_Y, VO_BLT_SRCTRANSPARENCY,
                    NULL);
  }

  DisplaySaveGameList();

  InvalidateRegion(0, 0, 639, 479);
}

void HandleSaveLoadScreen() {
  // If the game failed when in a message box, pop up a message box stating this
  if (gfFailedToSaveGameWhenInsideAMessageBox) {
    gfFailedToSaveGameWhenInsideAMessageBox = FALSE;

    DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], SAVE_LOAD_SCREEN,
                         MSG_BOX_FLAG_OK, RedrawSaveLoadScreenAfterMessageBox);

    //		gbSelectedSaveLocation = -1;
    gbHighLightedLocation = -1;

    //		for( i=0; i<NUM_SAVE_GAMES; i++)
    //			gbSaveGameSelectedLocation[i] = SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;
    ClearSelectedSaveSlot();
  }
}

void GetSaveLoadScreenUserInput() {
  InputAtom Event;
  int8_t bActiveTextField;
  static BOOLEAN fWasCtrlHeldDownLastFrame = FALSE;

  struct Point MousePos = GetMousePoint();

  // if we are going to be instantly leaving the screen, dont draw the numbers
  if (gfLoadGameUponEntry) {
    return;
  }

  if (gfKeyState[ALT]) {
    DisplayOnScreenNumber(FALSE);
  } else {
    DisplayOnScreenNumber(TRUE);
  }

  if (gfKeyState[CTRL] || fWasCtrlHeldDownLastFrame) {
    DisplaySaveGameEntry(gbSelectedSaveLocation);
  }

  fWasCtrlHeldDownLastFrame = gfKeyState[CTRL];

  while (DequeueEvent(&Event)) {
    // HOOK INTO MOUSE HOOKS
    switch (Event.usEvent) {
      case LEFT_BUTTON_DOWN:
        MouseSystemHook(LEFT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case LEFT_BUTTON_UP:
        MouseSystemHook(LEFT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_DOWN:
        MouseSystemHook(RIGHT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case RIGHT_BUTTON_UP:
        MouseSystemHook(RIGHT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_REPEAT:
        MouseSystemHook(RIGHT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case LEFT_BUTTON_REPEAT:
        MouseSystemHook(LEFT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
    }

    if (!HandleTextInput(&Event) && Event.usEvent == KEY_DOWN) {
      switch (Event.usParam) {
        case '1':
          SetSelection(1);
          break;
        case '2':
          SetSelection(2);
          break;
        case '3':
          SetSelection(3);
          break;
        case '4':
          SetSelection(4);
          break;
        case '5':
          SetSelection(5);
          break;
        case '6':
          SetSelection(6);
          break;
        case '7':
          SetSelection(7);
          break;
        case '8':
          SetSelection(8);
          break;
        case '9':
          SetSelection(9);
          break;
        case '0':
          SetSelection(10);
          break;
      }
    }

    if (Event.usEvent == KEY_UP) {
      switch (Event.usParam) {
        case UPARROW:
          MoveSelectionUpOrDown(TRUE);
          break;

        case DNARROW:
          MoveSelectionUpOrDown(FALSE);
          break;

        case ESC:
          if (gbSelectedSaveLocation == -1) {
            if (gfCameDirectlyFromGame)
              SetSaveLoadExitScreen(guiPreviousOptionScreen);

            else if (guiPreviousOptionScreen == MAINMENU_SCREEN)
              SetSaveLoadExitScreen(MAINMENU_SCREEN);
            else
              SetSaveLoadExitScreen(OPTIONS_SCREEN);
          } else {
            // Reset the selected slot background graphic
            gbSaveGameSelectedLocation[gbSelectedSaveLocation] =
                SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;

            // reset selected slot
            gbSelectedSaveLocation = -1;
            gfRedrawSaveLoadScreen = TRUE;
            DestroySaveLoadTextInputBoxes();

            //						if( !gfSaveGame )
            DisableButton(guiSlgSaveLoadBtn);
          }
          break;

        case ENTER:

          if (gfSaveGame) {
            bActiveTextField = (int8_t)GetActiveFieldID();
            if (bActiveTextField && bActiveTextField != -1) {
              Get16BitStringFromField((uint8_t)bActiveTextField, gzGameDescTextField,
                                      ARR_SIZE(gzGameDescTextField));
              SetActiveField(0);

              DestroySaveLoadTextInputBoxes();

              SaveLoadGameNumber(gbSelectedSaveLocation);
              return;
            } else {
              if (gbSelectedSaveLocation != -1) {
                SaveLoadGameNumber(gbSelectedSaveLocation);
                return;
              }
            }
            // Enable the save/load button
            if (gbSelectedSaveLocation != -1)
              if (!gfSaveGame) EnableButton(guiSlgSaveLoadBtn);

            gfRedrawSaveLoadScreen = TRUE;
          } else
            SaveLoadGameNumber(gbSelectedSaveLocation);

          break;
      }
    }
  }
}

void SaveLoadGameNumber(int8_t bSaveGameID) {
  //	wchar_t	zTemp[128];
  uint8_t ubRetVal = 0;

  if (bSaveGameID >= NUM_SAVE_GAMES || bSaveGameID < 0) {
    return;
  }

  if (gfSaveGame) {
    int8_t bActiveTextField;

    bActiveTextField = (int8_t)GetActiveFieldID();
    if (bActiveTextField && bActiveTextField != -1) {
      Get16BitStringFromField((uint8_t)bActiveTextField, gzGameDescTextField,
                              ARR_SIZE(gzGameDescTextField));
    }

    // if there is save game in the slot, ask for confirmation before overwriting
    if (gbSaveGameArray[bSaveGameID]) {
      wchar_t sText[512];

      swprintf(sText, ARR_SIZE(sText), zSaveLoadText[SLG_CONFIRM_SAVE], bSaveGameID);

      DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, sText, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO,
                           ConfirmSavedGameMessageBoxCallBack);
    } else {
      // else do NOT put up a confirmation

      // Save the game
      SaveGameToSlotNum();
    }
  } else {
    // Check to see if the save game headers are the same
    ubRetVal = CompareSaveGameVersion(bSaveGameID);
    if (ubRetVal != SLS_HEADER_OK) {
      if (ubRetVal == SLS_GAME_VERSION_OUT_OF_DATE) {
        DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_GAME_VERSION_DIF],
                             SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO,
                             LoadSavedGameWarningMessageBoxCallBack);
      } else if (ubRetVal == SLS_SAVED_GAME_VERSION_OUT_OF_DATE) {
        DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVED_GAME_VERSION_DIF],
                             SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO,
                             LoadSavedGameWarningMessageBoxCallBack);
      } else {
        DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_BOTH_GAME_AND_SAVED_GAME_DIF],
                             SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO,
                             LoadSavedGameWarningMessageBoxCallBack);
      }
    } else {
      /*
                              IF YOU UNCOMMENT THIS -- LOCALIZE IT!!!
                              wchar_t	sText[512];

                              swprintf( sText, L"%s%d?", zSaveLoadText[SLG_CONFIRM_LOAD],
         bSaveGameID );

                              DoSaveLoadMessageBox( MSG_BOX_BASIC_STYLE, sText, SAVE_LOAD_SCREEN,
         MSG_BOX_FLAG_YESNO, ConfirmLoadSavedGameMessageBoxCallBack );
      */
      // Setup up the fade routines
      StartFadeOutForSaveLoadScreen();
    }
  }
}

BOOLEAN DoSaveLoadMessageBoxWithRect(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                                     uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                     const SGPRect *pCenteringRect) {
  // do message box and return
  giSaveLoadMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                      (uint8_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                      ReturnCallback, pCenteringRect);

  // send back return state
  return ((giSaveLoadMessageBox != -1));
}

BOOLEAN DoSaveLoadMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                             uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback) {
  SGPRect CenteringRect = {0, 0, 639, 479};

  // do message box and return
  giSaveLoadMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                      (uint8_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                      ReturnCallback, &CenteringRect);

  // send back return state
  return ((giSaveLoadMessageBox != -1));
}

BOOLEAN InitSaveGameArray() {
  int8_t cnt;
  char zSaveGameName[512];
  SAVED_GAME_HEADER SaveGameHeader;

  for (cnt = 0; cnt < NUM_SAVE_GAMES; cnt++) {
    CreateSavedGameFileNameFromNumber(cnt, zSaveGameName);

    if (File_Exists(zSaveGameName)) {
      // Get the header for the saved game
      if (!LoadSavedGameHeader(cnt, &SaveGameHeader))
        gbSaveGameArray[cnt] = FALSE;
      else
        gbSaveGameArray[cnt] = TRUE;
    } else
      gbSaveGameArray[cnt] = FALSE;
  }

  return (TRUE);
}

BOOLEAN DisplaySaveGameList() {
  int8_t bLoop1;
  //	uint16_t usPosX = SLG_FIRST_SAVED_SPOT_X;
  uint16_t usPosY = SLG_FIRST_SAVED_SPOT_Y;

  for (bLoop1 = 0; bLoop1 < NUM_SAVE_GAMES; bLoop1++) {
    // display all the information from the header
    DisplaySaveGameEntry(bLoop1);  // usPosY );

    usPosY += SLG_GAP_BETWEEN_LOCATIONS;
  }

  return (TRUE);
}

BOOLEAN DisplaySaveGameEntry(int8_t bEntryID)  //, uint16_t usPosY )
{
  wchar_t zDateString[128];
  wchar_t zLocationString[128];
  wchar_t zNumMercsString[128];
  wchar_t zBalanceString[128];
  SAVED_GAME_HEADER SaveGameHeader;
  struct VObject *hPixHandle;
  uint16_t usPosX = SLG_FIRST_SAVED_SPOT_X;
  uint32_t uiFont = SAVE_LOAD_TITLE_FONT;
  uint8_t ubFontColor = SAVE_LOAD_TITLE_COLOR;
  uint16_t usPosY = SLG_FIRST_SAVED_SPOT_Y + (SLG_GAP_BETWEEN_LOCATIONS * bEntryID);

  // if we are going to be instantly leaving the screen, dont draw the numbers
  if (gfLoadGameUponEntry) {
    return (TRUE);
  }

  if (bEntryID == -1) return (TRUE);

  // if we are currently fading out, leave
  if (gfStartedFadingOut) return (TRUE);

  // background
  GetVideoObject(&hPixHandle, guiBackGroundAddOns);
  BltVideoObject2(vsFB, hPixHandle, gbSaveGameSelectedLocation[bEntryID], usPosX, usPosY,
                  VO_BLT_SRCTRANSPARENCY, NULL);

  //
  // Set the shadow color
  //

  // if its the QuickSave slot
  if (bEntryID == 0 && gfSaveGame) {
    SetFontShadow(SAVE_LOAD_QUICKSAVE_SHADOW_COLOR);
    ubFontColor = SAVE_LOAD_QUICKSAVE_COLOR;
    uiFont = SAVE_LOAD_QUICKSAVE_FONT;

    // Shadow the slot
    //		if( !gbSaveGameArray[ bEntryID ] )
    ShadowVideoSurfaceRect(vsFB, usPosX, usPosY, usPosX + SLG_SAVELOCATION_WIDTH,
                           usPosY + SLG_SAVELOCATION_HEIGHT);
  }

  // else if its the currently selected location
  else if (bEntryID == gbSelectedSaveLocation) {
    SetFontShadow(SAVE_LOAD_SELECTED_SHADOW_COLOR);  // 130
    ubFontColor = SAVE_LOAD_SELECTED_COLOR;          // SAVE_LOAD_NORMAL_COLOR;
    uiFont = SAVE_LOAD_SELECTED_FONT;
  }

  // else it is the highlighted slot
  else if (bEntryID == gbHighLightedLocation) {
    SetFontShadow(SAVE_LOAD_HIGHLIGHTED_SHADOW_COLOR);
    ubFontColor = SAVE_LOAD_HIGHLIGHTED_COLOR;
    uiFont = SAVE_LOAD_HIGHLIGHTED_FONT;
  }

  // if the file doesnt exists
  else if (!gbSaveGameArray[bEntryID]) {
    // if we are loading a game
    if (!gfSaveGame) {
      SetFontShadow(SAVE_LOAD_QUICKSAVE_SHADOW_COLOR);
      ubFontColor = SAVE_LOAD_QUICKSAVE_COLOR;
      uiFont = SAVE_LOAD_QUICKSAVE_FONT;

      // Shadow the surface
      ShadowVideoSurfaceRect(vsFB, usPosX, usPosY, usPosX + SLG_SAVELOCATION_WIDTH,
                             usPosY + SLG_SAVELOCATION_HEIGHT);
    } else {
      SetFontShadow(SAVE_LOAD_EMPTYSLOT_SHADOW_COLOR);
      ubFontColor = SAVE_LOAD_EMPTYSLOT_COLOR;
      uiFont = SAVE_LOAD_EMPTYSLOT_FONT;
    }
  } else {
    SetFontShadow(SAVE_LOAD_NORMAL_SHADOW_COLOR);
    ubFontColor = SAVE_LOAD_NORMAL_COLOR;
    uiFont = SAVE_LOAD_NORMAL_FONT;
  }

  // if the file exists
  if (gbSaveGameArray[bEntryID] || gbSelectedSaveLocation == bEntryID) {
    //
    // Setup the strings to be displayed
    //

    // if we are saving AND it is the currently selected slot
    if (gfSaveGame && gbSelectedSaveLocation == bEntryID) {
      // the user has selected a spot to save.  Fill out all the required information
      SaveGameHeader.uiDay = GetWorldDay();
      SaveGameHeader.ubHour = (uint8_t)GetWorldHour();
      SaveGameHeader.ubMin = (uint8_t)guiMin;

      // Get the sector value to save.
      {
        uint8_t mapX, mapY;
        GetBestPossibleSectorXYZValues(&mapX, &mapY, &SaveGameHeader.bSectorZ);
        SaveGameHeader.sSectorX = mapX;
        SaveGameHeader.sSectorY = mapY;
      }

      //			SaveGameHeader.sSectorX = gWorldSectorX;
      //			SaveGameHeader.sSectorY = gWorldSectorY;
      //			SaveGameHeader.bSectorZ = gbWorldSectorZ;
      SaveGameHeader.ubNumOfMercsOnPlayersTeam = NumberOfMercsOnPlayerTeam();
      SaveGameHeader.iCurrentBalance = MoneyGetBalance();
      wcscpy(SaveGameHeader.sSavedGameDesc, gzGameDescTextField);

      // copy over the initial game options
      memcpy(&SaveGameHeader.sInitialGameOptions, &gGameOptions, sizeof(GAME_OPTIONS));

    } else {
      // Get the header for the specified saved game
      if (!LoadSavedGameHeader(bEntryID, &SaveGameHeader)) {
        memset(&SaveGameHeader, 0, sizeof(SaveGameHeader));
        return (FALSE);
      }
    }

    // if this is the currently selected location, move the text down a bit
    if (gbSelectedSaveLocation == bEntryID) {
      usPosX++;
      usPosY--;
    }

    // if the user is LOADING and holding down the CTRL key, display the additional info
    if (!gfSaveGame && gfKeyState[CTRL] && gbSelectedSaveLocation == bEntryID) {
      wchar_t zMouseHelpTextString[256];
      wchar_t zDifString[256];

      // Create a string for difficulty level
      swprintf(
          zDifString, ARR_SIZE(zDifString), L"%s %s",
          gzGIOScreenText[GIO_EASY_TEXT + SaveGameHeader.sInitialGameOptions.ubDifficultyLevel - 1],
          zSaveLoadText[SLG_DIFF]);

      // make a string containing the extended options
      swprintf(zMouseHelpTextString, ARR_SIZE(zMouseHelpTextString),
               L"%20s     %22s     %22s     %22s", zDifString,
               /*gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT +
                  SaveGameHeader.sInitialGameOptions.fTurnTimeLimit + 1],*/

               SaveGameHeader.sInitialGameOptions.fIronManMode
                   ? gzGIOScreenText[GIO_IRON_MAN_TEXT]
                   : gzGIOScreenText[GIO_SAVE_ANYWHERE_TEXT],

               SaveGameHeader.sInitialGameOptions.fGunNut ? zSaveLoadText[SLG_ADDITIONAL_GUNS]
                                                          : zSaveLoadText[SLG_NORMAL_GUNS],

               SaveGameHeader.sInitialGameOptions.fSciFi ? zSaveLoadText[SLG_SCIFI]
                                                         : zSaveLoadText[SLG_REALISTIC]);

      // The date
      DrawTextToScreen(zMouseHelpTextString, (uint16_t)(usPosX + SLG_DATE_OFFSET_X),
                       (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 0, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    } else {
      // Create the string for the Data
      swprintf(zDateString, ARR_SIZE(zDateString), L"%s %d, %02d:%02d", pMessageStrings[MSG_DAY],
               SaveGameHeader.uiDay, SaveGameHeader.ubHour, SaveGameHeader.ubMin);

      // Create the string for the current location
      if ((SaveGameHeader.sSectorX == -1 && SaveGameHeader.sSectorY == -1) ||
          SaveGameHeader.bSectorZ < 0) {
        if ((SaveGameHeader.uiDay * NUM_SEC_IN_DAY + SaveGameHeader.ubHour * NUM_SEC_IN_HOUR +
             SaveGameHeader.ubMin * NUM_SEC_IN_MIN) <= STARTING_TIME)
          swprintf(zLocationString, ARR_SIZE(zLocationString),
                   gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
        else
          swprintf(zLocationString, ARR_SIZE(zLocationString), gzLateLocalizedString[14]);
      } else {
        gfGettingNameFromSaveLoadScreen = TRUE;

        GetSectorIDString((uint8_t)SaveGameHeader.sSectorX, (uint8_t)SaveGameHeader.sSectorY,
                          SaveGameHeader.bSectorZ, zLocationString, ARR_SIZE(zLocationString),
                          FALSE);

        gfGettingNameFromSaveLoadScreen = FALSE;
      }

      //
      // Number of mercs on the team
      //

      // if only 1 merc is on the team
      if (SaveGameHeader.ubNumOfMercsOnPlayersTeam == 1) {
        // use "merc"
        swprintf(zNumMercsString, ARR_SIZE(zNumMercsString), L"%d %s",
                 SaveGameHeader.ubNumOfMercsOnPlayersTeam, MercAccountText[MERC_ACCOUNT_MERC]);
      } else {
        // use "mercs"
        swprintf(zNumMercsString, ARR_SIZE(zNumMercsString), L"%d %s",
                 SaveGameHeader.ubNumOfMercsOnPlayersTeam, pMessageStrings[MSG_MERCS]);
      }

      // Get the current balance
      swprintf(zBalanceString, ARR_SIZE(zBalanceString), L"%d", SaveGameHeader.iCurrentBalance);
      InsertCommasForDollarFigure(zBalanceString);
      InsertDollarSignInToString(zBalanceString);

      //
      // Display the Saved game information
      //

      // The date
      DrawTextToScreen(zDateString, (uint16_t)(usPosX + SLG_DATE_OFFSET_X),
                       (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 0, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // if the sector string exceeds the width, and the ...
      ReduceStringLength(zLocationString, ARR_SIZE(zLocationString), SLG_SECTOR_WIDTH, uiFont);

      // The Sector
      DrawTextToScreen(zLocationString, (uint16_t)(usPosX + SLG_SECTOR_OFFSET_X),
                       (uint16_t)(usPosY + SLG_SECTOR_OFFSET_Y), 0, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // The Num of mercs
      DrawTextToScreen(zNumMercsString, (uint16_t)(usPosX + SLG_NUM_MERCS_OFFSET_X),
                       (uint16_t)(usPosY + SLG_NUM_MERCS_OFFSET_Y), 0, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // The balance
      DrawTextToScreen(zBalanceString, (uint16_t)(usPosX + SLG_BALANCE_OFFSET_X),
                       (uint16_t)(usPosY + SLG_BALANCE_OFFSET_Y), 0, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      if (gbSaveGameArray[bEntryID] ||
          (gfSaveGame && !gfUserInTextInputMode && (gbSelectedSaveLocation == bEntryID))) {
        // The Saved Game description
        DrawTextToScreen(SaveGameHeader.sSavedGameDesc, (uint16_t)(usPosX + SLG_SAVE_GAME_DESC_X),
                         (uint16_t)(usPosY + SLG_SAVE_GAME_DESC_Y), 0, uiFont, ubFontColor,
                         FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
      }
    }
  } else {
    // if this is the quick save slot
    if (bEntryID == 0) {
      // display the empty spot
      DrawTextToScreen(pMessageStrings[MSG_EMPTY_QUICK_SAVE_SLOT], usPosX,
                       (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 609, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
    } else {
      // display the empty spot
      DrawTextToScreen(pMessageStrings[MSG_EMPTYSLOT], usPosX,
                       (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 609, uiFont, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
    }
  }

  // REset the shadow color
  SetFontShadow(DEFAULT_SHADOW);

  usPosX = SLG_FIRST_SAVED_SPOT_X;
  usPosY = SLG_FIRST_SAVED_SPOT_Y + (SLG_GAP_BETWEEN_LOCATIONS * bEntryID);

  InvalidateRegion(usPosX, usPosY, usPosX + SLG_SAVELOCATION_WIDTH,
                   usPosY + SLG_SAVELOCATION_HEIGHT);

  return (TRUE);
}

BOOLEAN LoadSavedGameHeader(int8_t bEntry, SAVED_GAME_HEADER *pSaveGameHeader) {
  FileID hFile = FILE_ID_ERR;
  char zSavedGameName[512];
  uint32_t uiNumBytesRead;

  // make sure the entry is valid
  if (bEntry < 0 || bEntry > NUM_SAVE_GAMES) {
    memset(pSaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));
    return (FALSE);
  }

  // Get the name of the file
  CreateSavedGameFileNameFromNumber(bEntry, zSavedGameName);

  if (File_Exists(zSavedGameName)) {
    // create the save game file
    hFile = File_OpenForReading(zSavedGameName);
    if (!hFile) {
      File_Close(hFile);
      gbSaveGameArray[bEntry] = FALSE;
      return (FALSE);
    }

    // Load the Save Game header file
    File_Read(hFile, pSaveGameHeader, sizeof(SAVED_GAME_HEADER), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(SAVED_GAME_HEADER)) {
      File_Close(hFile);
      gbSaveGameArray[bEntry] = FALSE;
      return (FALSE);
    }

    File_Close(hFile);

    //
    // Do some Tests on the header to make sure it is valid
    //

    // Check to see if the desc field is bigger then it should be, ie no null char
    if (wcslen(pSaveGameHeader->sSavedGameDesc) >= SIZE_OF_SAVE_GAME_DESC) {
      memset(pSaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));
      gbSaveGameArray[bEntry] = FALSE;
      return (FALSE);
    }

    // Check to see if the version # field is bigger then it should be, ie no null char
    if (strlen(pSaveGameHeader->zGameVersionNumber) >= GAME_VERSION_LENGTH) {
      memset(pSaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));
      gbSaveGameArray[bEntry] = FALSE;
      return (FALSE);
    }
  } else {
    memset(pSaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));
#ifdef JA2BETAVERSION
    wcscpy(pSaveGameHeader->sSavedGameDesc,
           L"ERROR loading saved game header, file doesn't exist!!");
#endif
  }

  return (TRUE);
}

void BtnSlgCancelCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // Exit back
    if (gfCameDirectlyFromGame)
      SetSaveLoadExitScreen(guiPreviousOptionScreen);

    else if (guiPreviousOptionScreen == MAINMENU_SCREEN)
      SetSaveLoadExitScreen(MAINMENU_SCREEN);

    else
      SetSaveLoadExitScreen(OPTIONS_SCREEN);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnSlgSaveLoadCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    SaveLoadGameNumber(gbSelectedSaveLocation);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

/*
void BtnSlgLoadCallback(GUI_BUTTON *btn,int32_t reason)
{
        if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                btn->uiFlags |= BUTTON_CLICKED_ON;
                InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
        }
        if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
        {
                btn->uiFlags &= (~BUTTON_CLICKED_ON );

                SaveLoadGameNumber( gbSelectedSaveLocation );

                InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
        }
        if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
        {
                btn->uiFlags &= (~BUTTON_CLICKED_ON );
                InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
        }
}
*/

void SelectedSaveRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int8_t bActiveTextField;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t bSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    static uint32_t uiLastTime = 0;
    uint32_t uiCurTime = GetJA2Clock();
    int32_t i;

    /*
                    //If we are saving and this is the quick save slot
                    if( gfSaveGame && bSelected == 0 )
                    {
                            //Display a pop up telling user what the quick save slot is
                            DoSaveLoadMessageBox( MSG_BOX_BASIC_STYLE, pMessageStrings[
       MSG_QUICK_SAVE_RESERVED_FOR_TACTICAL ], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK,
       RedrawSaveLoadScreenAfterMessageBox ); return;
                    }

                    SetSelection( bSelected );
    */

    // If we are saving and this is the quick save slot
    if (gfSaveGame && bSelected == 0) {
      // Display a pop up telling user what the quick save slot is
      DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE,
                           pMessageStrings[MSG_QUICK_SAVE_RESERVED_FOR_TACTICAL], SAVE_LOAD_SCREEN,
                           MSG_BOX_FLAG_OK, RedrawSaveLoadScreenAfterMessageBox);
      return;
    }

    // Reset the regions
    for (i = 0; i < NUM_SAVE_GAMES; i++)
      gbSaveGameSelectedLocation[i] = SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;

    // if the user is selecting an unselected saved game slot
    if (gbSelectedSaveLocation != bSelected) {
      // Destroy the previous region
      DestroySaveLoadTextInputBoxes();

      gbSelectedSaveLocation = bSelected;

      // Reset the global string
      gzGameDescTextField[0] = '\0';

      // Init the text field for the game desc
      InitSaveLoadScreenTextInputBoxes();

      // If we are Loading the game
      //			if( !gfSaveGame )
      {
        // Enable the save/load button
        EnableButton(guiSlgSaveLoadBtn);
      }

      // If we are saving the game, disbale the button
      //			if( gfSaveGame )
      //					DisableButton( guiSlgSaveLoadBtn );
      //			else
      {
        // Set the time in which the button was first pressed
        uiLastTime = GetJA2Clock();
      }

      // Set the selected region to be highlighted
      gbSaveGameSelectedLocation[bSelected] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;

      gfRedrawSaveLoadScreen = TRUE;

      uiLastTime = GetJA2Clock();
    }

    // the user is selecting the selected save game slot
    else {
      // if we are saving a game
      if (gfSaveGame) {
        // if the user is not currently editing the game desc
        if (!gfUserInTextInputMode) {
          //					SaveLoadGameNumber( gbSelectedSaveLocation );

          if ((uiCurTime - uiLastTime) < SLG_DOUBLE_CLICK_DELAY) {
            // Load the saved game
            SaveLoadGameNumber(gbSelectedSaveLocation);
          } else {
            uiLastTime = GetJA2Clock();
          }

          InitSaveLoadScreenTextInputBoxes();

          // Set the selected region to be highlighted
          gbSaveGameSelectedLocation[bSelected] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;

          gfRedrawSaveLoadScreen = TRUE;

        } else {
          bActiveTextField = (int8_t)GetActiveFieldID();
          if (bActiveTextField && bActiveTextField != -1) {
            Get16BitStringFromField((uint8_t)bActiveTextField, gzGameDescTextField,
                                    ARR_SIZE(gzGameDescTextField));
            SetActiveField(0);

            DestroySaveLoadTextInputBoxes();

            //						gfRedrawSaveLoadScreen = TRUE;

            //						EnableButton( guiSlgSaveLoadBtn );

            DisplaySaveGameEntry(gbLastHighLightedLocation);

            gfRedrawSaveLoadScreen = TRUE;

            if ((uiCurTime - uiLastTime) < SLG_DOUBLE_CLICK_DELAY) {
              gubSaveGameNextPass = 1;
            } else {
              uiLastTime = GetJA2Clock();
            }
          }
        }
        // Set the selected region to be highlighted
        gbSaveGameSelectedLocation[bSelected] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;
      }
      // else we are loading
      else {
        if ((uiCurTime - uiLastTime) < SLG_DOUBLE_CLICK_DELAY) {
          // Load the saved game
          SaveLoadGameNumber(bSelected);
        } else {
          uiLastTime = GetJA2Clock();
        }
      }
    }
    // Set the selected region to be highlighted
    gbSaveGameSelectedLocation[bSelected] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    DisableSelectedSlot();
  }
}

void SelectedSaveRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    int8_t bTemp;
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);

    bTemp = gbHighLightedLocation;
    gbHighLightedLocation = -1;
    //		DisplaySaveGameList();
    DisplaySaveGameEntry(bTemp);

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    pRegion->uiFlags |= BUTTON_CLICKED_ON;

    // If we are saving and this is the quick save slot, leave
    if (gfSaveGame && MSYS_GetRegionUserData(pRegion, 0) != 0) {
      return;
    }

    gbLastHighLightedLocation = gbHighLightedLocation;
    gbHighLightedLocation = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    DisplaySaveGameEntry(gbLastHighLightedLocation);
    DisplaySaveGameEntry(gbHighLightedLocation);  //, usPosY );

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void InitSaveLoadScreenTextInputBoxes() {
  uint16_t usPosY;
  SAVED_GAME_HEADER SaveGameHeader;

  if (gbSelectedSaveLocation == -1) return;

  if (!gfSaveGame) return;

  // if we are exiting, dont create the fields
  if (gfSaveLoadScreenExit || guiSaveLoadExitScreen != SAVE_LOAD_SCREEN) return;

  InitTextInputMode();

  SetTextInputCursor(CUROSR_IBEAM_WHITE);
  SetTextInputFont((uint16_t)FONT12ARIALFIXEDWIDTH);  // FONT12ARIAL //FONT12ARIALFIXEDWIDTH
  Set16BPPTextFieldColor(Get16BPPColor(FROMRGB(0, 0, 0)));
  SetBevelColors(Get16BPPColor(FROMRGB(136, 138, 135)), Get16BPPColor(FROMRGB(24, 61, 81)));
  SetTextInputRegularColors(FONT_WHITE, 2);
  SetTextInputHilitedColors(2, FONT_WHITE, FONT_WHITE);
  SetCursorColor(Get16BPPColor(FROMRGB(255, 255, 255)));

  AddUserInputField(NULL);

  usPosY = SLG_FIRST_SAVED_SPOT_Y + SLG_GAP_BETWEEN_LOCATIONS * gbSelectedSaveLocation;

  // if there is already a string here, use its string
  if (gbSaveGameArray[gbSelectedSaveLocation]) {
    // if we are modifying a previously modifed string, use it
    if (gzGameDescTextField[0] != '\0') {
    } else {
      // Get the header for the specified saved game
      LoadSavedGameHeader(gbSelectedSaveLocation, &SaveGameHeader);
      wcscpy(gzGameDescTextField, SaveGameHeader.sSavedGameDesc);
    }
  } else
    gzGameDescTextField[0] = '\0';

  // Game Desc Field
  AddTextInputField(SLG_FIRST_SAVED_SPOT_X + SLG_SAVE_GAME_DESC_X,
                    (int16_t)(usPosY + SLG_SAVE_GAME_DESC_Y - 5),
                    SLG_SAVELOCATION_WIDTH - SLG_SAVE_GAME_DESC_X - 7, 17, MSYS_PRIORITY_HIGH + 2,
                    gzGameDescTextField, 46, INPUTTYPE_ASCII);  // 23

  SetActiveField(1);

  gfUserInTextInputMode = TRUE;
}

void DestroySaveLoadTextInputBoxes() {
  gfUserInTextInputMode = FALSE;
  KillAllTextInputModes();
  SetTextInputCursor(CURSOR_IBEAM);
}

void SetSelection(uint8_t ubNewSelection) {
  //	wchar_t		zMouseHelpTextString[256];
  //	SAVED_GAME_HEADER SaveGameHeader;

  // if we are loading and there is no entry, return
  if (!gfSaveGame) {
    if (!gbSaveGameArray[ubNewSelection]) return;
  }

  // Reset the selected slot background graphic
  if (gbSelectedSaveLocation != -1) {
    gbSaveGameSelectedLocation[gbSelectedSaveLocation] = SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;

    // reset the slots help text
    SetRegionFastHelpText(&gSelectedSaveRegion[gbSelectedSaveLocation], L"\0");
  }

  gfRedrawSaveLoadScreen = TRUE;
  DestroySaveLoadTextInputBoxes();

  // if we are loading,
  if (!gfSaveGame) {
    // Enable the save/load button
    EnableButton(guiSlgSaveLoadBtn);
  }

  // if we are to save
  if (gfSaveGame) {
    if (gbSelectedSaveLocation != ubNewSelection) {
      // Destroy the previous region
      DestroySaveLoadTextInputBoxes();

      // reset selected slot
      gbSelectedSaveLocation = ubNewSelection;

      // Null out the currently selected save game
      gzGameDescTextField[0] = '\0';

      // Init the text field for the game desc
      InitSaveLoadScreenTextInputBoxes();
    }

    // Enable the save/load button
    EnableButton(guiSlgSaveLoadBtn);
  }

  // reset selected slot
  gbSelectedSaveLocation = ubNewSelection;

  // Set the selected slot background graphic
  gbSaveGameSelectedLocation[gbSelectedSaveLocation] = SLG_SELECTED_SLOT_GRAPHICS_NUMBER;

  /*
          //if we are saving AND it is the currently selected slot
          if( gfSaveGame && gbSelectedSaveLocation == ubNewSelection )
          {
                  //copy over the initial game options
                  memcpy( &SaveGameHeader.sInitialGameOptions, &gGameOptions, sizeof( GAME_OPTIONS )
     );
          }
          else
          {
                  //Get the header for the specified saved game
                  if( !LoadSavedGameHeader( ubNewSelection, &SaveGameHeader ) )
                  {
                          memset( &SaveGameHeader, 0, sizeof( SaveGameHeader ) );
                  }
          }

          swprintf( zMouseHelpTextString, L"%s: %s\n%s: %s\n%s: %s\n%s: %s", gzGIOScreenText[
     GIO_DIF_LEVEL_TEXT ], gzGIOScreenText[ GIO_DIF_LEVEL_TEXT +
     SaveGameHeader.sInitialGameOptions.ubDifficultyLevel + 1 ],

                  gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT ],
                  gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT +
     SaveGameHeader.sInitialGameOptions.fTurnTimeLimit + 1],

                  gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ],
                  gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT + 2 -
     SaveGameHeader.sInitialGameOptions.fGunNut ],

                  gzGIOScreenText[ GIO_GAME_STYLE_TEXT ],
                  gzGIOScreenText[ GIO_GAME_STYLE_TEXT + SaveGameHeader.sInitialGameOptions.fSciFi +
     1 ] );

          //set the slots help text
          SetRegionFastHelpText( &gSelectedSaveRegion[ gbSelectedSaveLocation ],
     zMouseHelpTextString );
          */
}

uint8_t CompareSaveGameVersion(int8_t bSaveGameID) {
  uint8_t ubRetVal = SLS_HEADER_OK;

  SAVED_GAME_HEADER SaveGameHeader;

  // Get the heade for the saved game
  LoadSavedGameHeader(bSaveGameID, &SaveGameHeader);

  // check to see if the saved game version in the header is the same as the current version
  if (SaveGameHeader.uiSavedGameVersion != guiSavedGameVersion) {
    ubRetVal = SLS_SAVED_GAME_VERSION_OUT_OF_DATE;
  }

  if (strcmp(SaveGameHeader.zGameVersionNumber, czVersionNumber) != 0) {
    if (ubRetVal == SLS_SAVED_GAME_VERSION_OUT_OF_DATE)
      ubRetVal = SLS_BOTH_SAVE_GAME_AND_GAME_VERSION_OUT_OF_DATE;
    else
      ubRetVal = SLS_GAME_VERSION_OUT_OF_DATE;
  }

  return (ubRetVal);
}

void LoadSavedGameWarningMessageBoxCallBack(uint8_t bExitValue) {
  // yes, load the game
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // Setup up the fade routines
    StartFadeOutForSaveLoadScreen();
  }

  // The user does NOT want to continue..
  else {
    // ask if the user wants to delete all the saved game files
    DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_DELETE_ALL_SAVE_GAMES],
                         SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO,
                         LoadSavedGameDeleteAllSaveGameMessageBoxCallBack);
  }
}

void LoadSavedGameDeleteAllSaveGameMessageBoxCallBack(uint8_t bExitValue) {
  // yes, Delete all the save game files
  if (bExitValue == MSG_BOX_RETURN_YES) {
    DeleteAllSaveGameFile();
    gfSaveLoadScreenExit = TRUE;
  }

  gfExitAfterMessageBox = TRUE;

  SetSaveLoadExitScreen(OPTIONS_SCREEN);

  gbSelectedSaveLocation = -1;
}

void DeleteAllSaveGameFile() {
  uint8_t cnt;

  for (cnt = 0; cnt < NUM_SAVE_GAMES; cnt++) {
    DeleteSaveGameNumber(cnt);
  }

  gGameSettings.bLastSavedGameSlot = -1;

  InitSaveGameArray();
}

void DeleteSaveGameNumber(uint8_t ubSaveGameSlotID) {
  char zSaveGameName[512];

  // Create the name of the file
  CreateSavedGameFileNameFromNumber(ubSaveGameSlotID, zSaveGameName);

  // Delete the saved game file
  Plat_DeleteFile(zSaveGameName);
}

void DisplayOnScreenNumber(BOOLEAN fErase) {
  wchar_t zTempString[16];
  uint16_t usPosX = 6;
  uint16_t usPosY;
  int8_t bLoopNum;
  int8_t bNum = 0;

  usPosY = SLG_FIRST_SAVED_SPOT_Y;

  for (bLoopNum = 0; bLoopNum < NUM_SAVE_GAMES; bLoopNum++) {
    // Dont diplay it for the quicksave
    if (bLoopNum == 0) {
      usPosY += SLG_GAP_BETWEEN_LOCATIONS;
      continue;
    }

    VSurfaceBlitBufToBuf(vsSaveBuffer, vsFB, usPosX, (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 10,
                         10);

    if (bLoopNum != 10) {
      bNum = bLoopNum;
      swprintf(zTempString, ARR_SIZE(zTempString), L"%2d", bNum);
    } else {
      bNum = 0;
      swprintf(zTempString, ARR_SIZE(zTempString), L"%2d", bNum);
    }

    if (!fErase)
      DrawTextToScreen(zTempString, usPosX, (uint16_t)(usPosY + SLG_DATE_OFFSET_Y), 0,
                       SAVE_LOAD_NUMBER_FONT, SAVE_LOAD_NUMBER_COLOR, FONT_MCOLOR_BLACK, FALSE,
                       LEFT_JUSTIFIED);

    InvalidateRegion(usPosX, usPosY + SLG_DATE_OFFSET_Y, usPosX + 10,
                     usPosY + SLG_DATE_OFFSET_Y + 10);

    usPosY += SLG_GAP_BETWEEN_LOCATIONS;
  }
}

#ifdef JA2BETAVERSION
extern BOOLEAN ValidateSoldierInitLinks(uint8_t ubCode);
#endif

void DoneFadeOutForSaveLoadScreen(void) {
  // Make sure we DONT reset the levels if we are loading a game
  gfHadToMakeBasementLevels = FALSE;

  if (!LoadSavedGame(gbSelectedSaveLocation)) {
    if (guiBrokenSaveGameVersion < 95 &&
        !gfSchedulesHosed) {  // Hack problem with schedules getting misaligned.
      gfSchedulesHosed = TRUE;
      if (!LoadSavedGame(gbSelectedSaveLocation)) {
        DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_LOAD_GAME_ERROR],
                             SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, FailedLoadingGameCallBack);
        NextLoopCheckForEnoughFreeHardDriveSpace();
      } else {
        gfSchedulesHosed = FALSE;
        goto SUCCESSFULLY_CORRECTED_SAVE;
      }
      gfSchedulesHosed = FALSE;
    } else {
      DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_LOAD_GAME_ERROR],
                           SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, FailedLoadingGameCallBack);
      NextLoopCheckForEnoughFreeHardDriveSpace();
    }
  } else {
  SUCCESSFULLY_CORRECTED_SAVE:
#ifdef JA2BETAVERSION
    //		if( ValidateSoldierInitLinks( 1 ) )
    ValidateSoldierInitLinks(1);
    {
#endif
      // If we are to go to map screen after loading the game
      if (guiScreenToGotoAfterLoadingSavedGame == MAP_SCREEN) {
        gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;

        SetSaveLoadExitScreen(guiScreenToGotoAfterLoadingSavedGame);

        //			LeaveTacticalScreen( MAP_SCREEN );

        FadeInNextFrame();
      }

      else {
        // if we are to go to the Tactical screen after loading
        gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;

        SetSaveLoadExitScreen(guiScreenToGotoAfterLoadingSavedGame);

        PauseTime(FALSE);

        //			EnterTacticalScreen( );
        FadeInGameScreen();
      }
#ifdef JA2BETAVERSION
    }
#endif
  }
  gfStartedFadingOut = FALSE;
}

void DoneFadeInForSaveLoadScreen(void) {
  // Leave the screen
  // if we are supposed to stay in tactical, due nothing,
  // if we are supposed to goto mapscreen, leave tactical and go to mapscreen

  if (guiScreenToGotoAfterLoadingSavedGame == MAP_SCREEN) {
    if (!gfPauseDueToPlayerGamePause) {
      UnLockPauseState();
      UnPauseGame();
    }
  }

  else {
    // if the game is currently paused
    if (GamePaused()) {
      // need to call it twice
      HandlePlayerPauseUnPauseOfGame();
      HandlePlayerPauseUnPauseOfGame();
    }

    //		UnLockPauseState( );
    //		UnPauseGame( );
  }
}

void SelectedSLSEntireRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    DisableSelectedSlot();
  }
}

void DisableSelectedSlot() {
  // reset selected slot
  gbSelectedSaveLocation = -1;
  gfRedrawSaveLoadScreen = TRUE;
  DestroySaveLoadTextInputBoxes();

  if (!gfSaveGame) DisableButton(guiSlgSaveLoadBtn);

  // reset the selected graphic
  ClearSelectedSaveSlot();
}

void ConfirmSavedGameMessageBoxCallBack(uint8_t bExitValue) {
  Assert(gbSelectedSaveLocation != -1);

  // yes, load the game
  if (bExitValue == MSG_BOX_RETURN_YES) {
    SaveGameToSlotNum();
  }
}

void ConfirmLoadSavedGameMessageBoxCallBack(uint8_t bExitValue) {
  Assert(gbSelectedSaveLocation != -1);

  // yes, load the game
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // Setup up the fade routines
    StartFadeOutForSaveLoadScreen();
  } else {
    gfExitAfterMessageBox = FALSE;
  }
}

#ifdef JA2BETAVERSION
void ErrorDetectedInSaveCallback(uint8_t bValue) {
  // If we are to go to map screen after loading the game
  if (guiScreenToGotoAfterLoadingSavedGame == MAP_SCREEN) {
    gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;
    SetSaveLoadExitScreen(guiScreenToGotoAfterLoadingSavedGame);
    FadeInNextFrame();
  } else {
    // if we are to go to the Tactical screen after loading
    gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;
    SetSaveLoadExitScreen(guiScreenToGotoAfterLoadingSavedGame);
    FadeInGameScreen();
  }
  gfStartedFadingOut = TRUE;
}
#endif

void FailedLoadingGameCallBack(uint8_t bExitValue) {
  // yes
  if (bExitValue == MSG_BOX_RETURN_OK) {
    // if the current screen is tactical
    if (guiPreviousOptionScreen == MAP_SCREEN) {
      SetPendingNewScreen(MAINMENU_SCREEN);
    } else {
      LeaveTacticalScreen(MAINMENU_SCREEN);
    }

    SetSaveLoadExitScreen(MAINMENU_SCREEN);

    // We want to reinitialize the game
    ReStartingGame();
  }
}

BOOLEAN DoQuickSave() {
  gzGameDescTextField[0] = '\0';

  /*
          // Make sure the user has enough hard drive space
          if( !DoesUserHaveEnoughHardDriveSpace() )
          {
                  wchar_t	zText[512];
                  wchar_t	zSpaceOnDrive[512];
                  uint32_t	uiSpaceOnDrive;
                  wchar_t	zSizeNeeded[512];

                  swprintf( zSizeNeeded, L"%d", REQUIRED_FREE_SPACE / BYTESINMEGABYTE );
                  InsertCommasForDollarFigure( zSizeNeeded );

                  uiSpaceOnDrive = Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom( );

                  swprintf( zSpaceOnDrive, L"%.2f", uiSpaceOnDrive / (float)BYTESINMEGABYTE );

                  swprintf( zText, pMessageStrings[ MSG_LOWDISKSPACE_WARNING ], zSpaceOnDrive,
     zSizeNeeded );

                  if( guiPreviousOptionScreen == MAP_SCREEN )
                          DoMapMessageBox( MSG_BOX_BASIC_STYLE, zText, MAP_SCREEN, MSG_BOX_FLAG_OK,
     NotEnoughHardDriveSpaceForQuickSaveMessageBoxCallBack ); else DoMessageBox(
     MSG_BOX_BASIC_STYLE, zText, GAME_SCREEN, MSG_BOX_FLAG_OK,
     NotEnoughHardDriveSpaceForQuickSaveMessageBoxCallBack, NULL );

                  return( FALSE );
          }
  */

  if (!SaveGame(0, gzGameDescTextField, ARR_SIZE(gzGameDescTextField))) {
    // Unset the fact that we are saving a game
    gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

    if (guiPreviousOptionScreen == MAP_SCREEN)
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], MAP_SCREEN,
                      MSG_BOX_FLAG_OK, NULL);
    else
      DoMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], GAME_SCREEN,
                   MSG_BOX_FLAG_OK, NULL, NULL);
  }

  return (TRUE);
}

BOOLEAN DoQuickLoad() {
  // Build the save game array
  InitSaveGameArray();

  // if there is no save in the quick save slot
  if (!gbSaveGameArray[0]) return (FALSE);

  // Set the selection to be the quick save slot
  gbSelectedSaveLocation = 0;

  // if the game is paused, and we are in tactical, unpause
  if (IsTacticalMode()) {
    PauseTime(FALSE);
  }

  // Do a fade out before we load the game
  gFadeOutDoneCallback = DoneFadeOutForSaveLoadScreen;

  FadeOutNextFrame();
  gfStartedFadingOut = TRUE;
  gfDoingQuickLoad = TRUE;

  return (TRUE);
}

BOOLEAN IsThereAnySavedGameFiles() {
  int8_t cnt;
  char zSaveGameName[512];

  for (cnt = 0; cnt < NUM_SAVE_GAMES; cnt++) {
    CreateSavedGameFileNameFromNumber(cnt, zSaveGameName);

    if (File_Exists(zSaveGameName)) return (TRUE);
  }

  return (FALSE);
}

void NotEnoughHardDriveSpaceForQuickSaveMessageBoxCallBack(uint8_t bExitValue) {
  if (!SaveGame(0, gzGameDescTextField, ARR_SIZE(gzGameDescTextField))) {
    // Unset the fact that we are saving a game
    gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

    // Set a flag indicating that the save failed ( cant initiate a message box from within a mb
    // callback )
    gfFailedToSaveGameWhenInsideAMessageBox = TRUE;
  }
}

void NotEnoughHardDriveSpaceForNormalSaveMessageBoxCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_OK) {
    // If the game failed to save
    if (!SaveGame(gbSelectedSaveLocation, gzGameDescTextField, ARR_SIZE(gzGameDescTextField))) {
      // Unset the fact that we are saving a game
      gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

      // Set a flag indicating that the save failed ( cant initiate a message box from within a mb
      // callback )
      gfFailedToSaveGameWhenInsideAMessageBox = TRUE;
    } else {
      SetSaveLoadExitScreen(guiPreviousOptionScreen);
    }
  }
}

void RedrawSaveLoadScreenAfterMessageBox(uint8_t bExitValue) { gfRedrawSaveLoadScreen = TRUE; }

void MoveSelectionUpOrDown(BOOLEAN fUp) {
  int32_t i;

  // if we are saving, any slot otgher then the quick save slot is valid
  if (gfSaveGame) {
    if (fUp) {
      // if there is no selected slot, get out
      if (gbSelectedSaveLocation == -1) return;

      // if the selected slot is above the first slot
      if (gbSelectedSaveLocation > 1) {
        SetSelection((uint8_t)(gbSelectedSaveLocation - 1));
      }
    } else {
      // if the selected slot is invalid
      if (gbSelectedSaveLocation == -1) {
        SetSelection(1);
      } else {
        if (gbSelectedSaveLocation >= 1 && gbSelectedSaveLocation < NUM_SAVE_GAMES - 1) {
          SetSelection((uint8_t)(gbSelectedSaveLocation + 1));
        }
      }
    }
  }

  else {
    if (fUp) {
      for (i = gbSelectedSaveLocation - 1; i >= 0; i--) {
        if (gbSaveGameArray[i]) {
          ClearSelectedSaveSlot();

          SetSelection((uint8_t)i);
          break;
        }
      }
    } else {
      // if there is no selected slot, move the selected slot to the first slot
      if (gbSelectedSaveLocation == -1) {
        ClearSelectedSaveSlot();

        SetSelection(0);
      } else {
        for (i = gbSelectedSaveLocation + 1; i < NUM_SAVE_GAMES; i++) {
          if (gbSaveGameArray[i]) {
            ClearSelectedSaveSlot();

            SetSelection((uint8_t)i);
            break;
          }
        }
      }
    }
  }
}

void ClearSelectedSaveSlot() {
  int32_t i;
  for (i = 0; i < NUM_SAVE_GAMES; i++)
    gbSaveGameSelectedLocation[i] = SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;

  gbSelectedSaveLocation = -1;
}

void SaveGameToSlotNum() {
  /*
          // Make sure the user has enough hard drive space
          if( !DoesUserHaveEnoughHardDriveSpace() )
          {
                  wchar_t	zText[512];
                  wchar_t	zSizeNeeded[512];
                  wchar_t	zSpaceOnDrive[512];
                  uint32_t	uiSpaceOnDrive;

                  swprintf( zSizeNeeded, L"%d", REQUIRED_FREE_SPACE / BYTESINMEGABYTE );
                  InsertCommasForDollarFigure( zSizeNeeded );

                  uiSpaceOnDrive = Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom( );

                  swprintf( zSpaceOnDrive, L"%.2f", uiSpaceOnDrive / (float)BYTESINMEGABYTE );

                  swprintf( zText, pMessageStrings[ MSG_LOWDISKSPACE_WARNING ], zSpaceOnDrive,
     zSizeNeeded );

                  DoSaveLoadMessageBox( MSG_BOX_BASIC_STYLE, zText, SAVE_LOAD_SCREEN,
     MSG_BOX_FLAG_OK, NotEnoughHardDriveSpaceForNormalSaveMessageBoxCallBack );

                  return;
          }
  */

  // Redraw the save load screen
  RenderSaveLoadScreen();

  // render the buttons
  MarkButtonsDirty();
  RenderButtons();

  if (!SaveGame(gbSelectedSaveLocation, gzGameDescTextField, ARR_SIZE(gzGameDescTextField))) {
    // Unset the fact that we are saving a game
    gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

    DoSaveLoadMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], SAVE_LOAD_SCREEN,
                         MSG_BOX_FLAG_OK, NULL);
  }

  //			gfExitAfterMessageBox = TRUE;

  SetSaveLoadExitScreen(guiPreviousOptionScreen);
}

void StartFadeOutForSaveLoadScreen() {
  // if the game is paused, and we are in tactical, unpause
  if (guiPreviousOptionScreen == GAME_SCREEN) {
    PauseTime(FALSE);
  }

  gFadeOutDoneCallback = DoneFadeOutForSaveLoadScreen;

  FadeOutNextFrame();
  gfStartedFadingOut = TRUE;
  gfExitAfterMessageBox = TRUE;
}
