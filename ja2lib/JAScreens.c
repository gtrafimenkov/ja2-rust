#include "JAScreens.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "Editor/EditScreen.h"
#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameVersion.h"
#include "Globals.h"
#include "Init.h"
#include "LanguageDefines.h"
#include "MainMenuScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/HImage.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "SysGlobals.h"
#include "Tactical/AnimationCache.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/Environment.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define _UNICODE

#define MAX_DEBUG_PAGES 4

UINT32 guiCurrentScreen;

// GLOBAL FOR PAL EDITOR
UINT8 CurrentPalette = 0;
UINT32 guiBackgroundRect;
BOOLEAN gfExitPalEditScreen = FALSE;
BOOLEAN gfExitDebugScreen = FALSE;
BOOLEAN gfInitRect = TRUE;
static BOOLEAN FirstTime = TRUE;
BOOLEAN gfDoneWithSplashScreen = FALSE;

void PalEditRenderHook();
BOOLEAN PalEditKeyboardHook(InputAtom *pInputEvent);

void DebugRenderHook();
BOOLEAN DebugKeyboardHook(InputAtom *pInputEvent);
INT8 gCurDebugPage = 0;

struct VSurface *hVAnims[7];
INT8 bTitleAnimFrame = 0;
UINT32 uiTitleAnimTime = 0;
UINT32 uiDoTitleAnimTime = 0;
BOOLEAN gfDoTitleAnimation = FALSE;
BOOLEAN gfStartTitleAnimation = FALSE;

void DefaultDebugPage1();
void DefaultDebugPage2();
void DefaultDebugPage3();
void DefaultDebugPage4();
RENDER_HOOK gDebugRenderOverride[MAX_DEBUG_PAGES] = {
    (RENDER_HOOK)DefaultDebugPage1, (RENDER_HOOK)DefaultDebugPage2, (RENDER_HOOK)DefaultDebugPage3,
    (RENDER_HOOK)DefaultDebugPage4};

void DisplayFrameRate() {
  static UINT32 uiFPS = 0;
  static UINT32 uiFrameCount = 0;
  INT16 usMapPos;
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  // Increment frame count
  uiFrameCount++;

  if (COUNTERDONE(FPSCOUNTER)) {
    // Reset counter
    RESETCOUNTER(FPSCOUNTER);

    uiFPS = uiFrameCount;
    uiFrameCount = 0;
  }

  // Create string
  SetFont(SMALLFONT1);

  // DebugMsg(TOPIC_JA2, DBG_ERROR, String( "FPS: %d ", min( uiFPS, 1000 ) ) );

  if (uiFPS < 20) {
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_LTRED);
  } else {
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_DKGRAY);
  }

  if (gbFPSDisplay == SHOW_FULL_FPS) {
    // FRAME RATE
    memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));
    swprintf(VideoOverlayDesc.pzText, ARR_SIZE(VideoOverlayDesc.pzText), L"%ld", min(uiFPS, 1000));
    VideoOverlayDesc.uiFlags = VOVERLAY_DESC_TEXT;
    UpdateVideoOverlay(&VideoOverlayDesc, giFPSOverlay, FALSE);

    // TIMER COUNTER
    swprintf(VideoOverlayDesc.pzText, ARR_SIZE(VideoOverlayDesc.pzText), L"%ld",
             min(giTimerDiag, 1000));
    VideoOverlayDesc.uiFlags = VOVERLAY_DESC_TEXT;
    UpdateVideoOverlay(&VideoOverlayDesc, giCounterPeriodOverlay, FALSE);

    if (GetMouseMapPos(&usMapPos)) {
    } else {
    }
  }

  if ((gTacticalStatus.uiFlags & GODMODE)) {
    SetFont(SMALLFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_DKRED);
    // gprintfdirty( 0, 0, L"GOD MODE" );
    // mprintf( 0, 0, L"GOD MODE" );
  }

  if ((gTacticalStatus.uiFlags & DEMOMODE)) {
    SetFont(SMALLFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_DKGRAY);
    // gprintfdirty( 0, 0, L"DEMO MODE" );
    // mprintf( 0, 0, L"DEMO MODE" );
  }

#ifdef _DEBUG

  SetFont(SMALLFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_DKGRAY);

  if (gbFPSDisplay == SHOW_FULL_FPS) {
    // Debug
    if (gDebugStr[0] != '\0') {
      // gprintfdirty( 0, 345, L"DEBUG: %S",gDebugStr);
      // mprintf( 0,345,L"DEBUG: %S",gDebugStr);
    }

    if (gSystemDebugStr[0] != '\0') {
      // gprintfdirty( 0, 345, L"%S",gSystemDebugStr);
      // mprintf( 0,345,L"%S",gSystemDebugStr);
      gSystemDebugStr[0] = '\0';
    }

    // Print Num tiles
    // gprintfdirty( 0, 280, L"%d Tiles", gTileDatabaseSize );
    // mprintf( 0, 280, L"%d Tiles", gTileDatabaseSize );
  }
#endif
}

// USELESS!!!!!!!!!!!!!!!!!!
UINT32 SavingScreenInitialize(void) { return (TRUE); }
UINT32 SavingScreenHandle(void) { return SAVING_SCREEN; }
UINT32 SavingScreenShutdown(void) { return TRUE; }

UINT32 LoadingScreenInitialize(void) { return (TRUE); }
UINT32 LoadingScreenHandle(void) { return LOADING_SCREEN; }
UINT32 LoadingScreenShutdown(void) { return (TRUE); }

UINT32 ErrorScreenInitialize(void) { return (TRUE); }

UINT32 ErrorScreenHandle(void) {
  InputAtom InputEvent;
  static BOOLEAN fFirstTime = FALSE;
#ifdef JA2BETAVERSION
  CHAR16 str[256];
#endif

  // For quick setting of new video stuff / to be changed
  StartFrameBufferRender();

  // Create string
  SetFont(LARGEFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_LTGRAY);
  mprintf(50, 200, L"RUNTIME ERROR");

  mprintf(50, 225, L"PRESS <ESC> TO EXIT");

  SetFont(FONT12ARIAL);
  SetFontForeground(FONT_YELLOW);
  SetFontShadow(60);  // 60 is near black
  mprintf(50, 255, L"%S", gubErrorText);
  SetFontForeground(FONT_LTRED);

#ifdef JA2BETAVERSION

  if (gubAssertString[0]) {
    swprintf(str, ARR_SIZE(str), L"%S", gubAssertString);
    DisplayWrappedString(50, 270, 560, 2, FONT12ARIAL, FONT_RED, str, FONT_BLACK, TRUE,
                         LEFT_JUSTIFIED);
  }
#endif

  if (!fFirstTime) {
    DebugMsg(TOPIC_JA2, DBG_ERROR, String("Runtime Error: %s ", gubErrorText));
    fFirstTime = TRUE;
  }

  // For quick setting of new video stuff / to be changed
  InvalidateScreen();
  EndFrameBufferRender();

  // Check for esc
  while (DequeueEvent(&InputEvent) == TRUE) {
    if (InputEvent.usEvent == KEY_DOWN) {
      if (InputEvent.usParam == ESC ||
          (InputEvent.usParam == 'x' && InputEvent.usKeyState & ALT_DOWN)) {  // Exit the program
        DebugMsg(TOPIC_GAME, DBG_ERROR, "GameLoop: User pressed ESCape, TERMINATING");

        // handle shortcut exit
        HandleShortCutExitState();
      }
    }
  }

  return (ERROR_SCREEN);
}

UINT32 ErrorScreenShutdown(void) { return (TRUE); }

UINT32 InitScreenInitialize(void) { return (TRUE); }

UINT32 InitScreenHandle(void) {
  VSURFACE_DESC vs_desc;
  static struct VSurface *hVSurface;
  static UINT8 ubCurrentScreen = 255;

  if (ubCurrentScreen == 255) {
#ifdef ENGLISH
    if (gfDoneWithSplashScreen) {
      ubCurrentScreen = 0;
    } else {
      SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
      return (INTRO_SCREEN);
    }
#else
    ubCurrentScreen = 0;
#endif
  }

  if (ubCurrentScreen == 0) {
    // Load init screen and blit!
    hVSurface = CreateVideoSurfaceFromFile("ja2_logo.STI");
    if (!hVSurface) AssertMsg(0, "Failed to load ja2_logo.sti!");

    ubCurrentScreen = 1;

    // Init screen

    // Set Font
    SetFont(TINYFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_WHITE);

    mprintf(10, 430, L"%s", zBuildInfo);

#ifdef _DEBUG
    mprintf(10, 440, L"struct SOLDIERTYPE: %d bytes", sizeof(struct SOLDIERTYPE));
#endif

    InvalidateScreen();

    // Delete video Surface
    DeleteVideoSurface(hVSurface);
    // ATE: Set to true to reset before going into main screen!

    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

    return (INIT_SCREEN);
  }

  if (ubCurrentScreen == 1) {
    ubCurrentScreen = 2;
    return (InitializeJA2());
  }

  if (ubCurrentScreen == 2) {
    InitMainMenu();
    ubCurrentScreen = 3;
    return (INIT_SCREEN);
  }

  // Let one frame pass....
  if (ubCurrentScreen == 3) {
    ubCurrentScreen = 4;
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
    return (INIT_SCREEN);
  }

  if (ubCurrentScreen == 4) {
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
    InitNewGame(FALSE);
  }
  return (INIT_SCREEN);
}

UINT32 InitScreenShutdown(void) { return (TRUE); }

UINT32 PalEditScreenInit(void) { return (TRUE); }

UINT32 PalEditScreenHandle(void) {
  static BOOLEAN FirstTime = TRUE;

  if (gfExitPalEditScreen) {
    gfExitPalEditScreen = FALSE;
    FirstTime = TRUE;
    FreeBackgroundRect(guiBackgroundRect);
    SetRenderHook((RENDER_HOOK)NULL);
    SetUIKeyboardHook((UIKEYBOARD_HOOK)NULL);
    return (GAME_SCREEN);
  }

  if (FirstTime) {
    FirstTime = FALSE;

    SetRenderHook((RENDER_HOOK)PalEditRenderHook);
    SetUIKeyboardHook((UIKEYBOARD_HOOK)PalEditKeyboardHook);

    guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, 50, 10, 600, 400);

  } else {
    (*(GameScreens[GAME_SCREEN].HandleScreen))();
  }

  return (PALEDIT_SCREEN);
}

UINT32 PalEditScreenShutdown(void) { return (TRUE); }

void PalEditRenderHook() {
  struct SOLDIERTYPE *pSoldier;

  if (gusSelectedSoldier != NO_SOLDIER) {
    // Set to current
    GetSoldier(&pSoldier, gusSelectedSoldier);

    DisplayPaletteRep(pSoldier->HeadPal, 50, 10, FRAME_BUFFER);
    DisplayPaletteRep(pSoldier->PantsPal, 50, 50, FRAME_BUFFER);
    DisplayPaletteRep(pSoldier->VestPal, 50, 90, FRAME_BUFFER);
    DisplayPaletteRep(pSoldier->SkinPal, 50, 130, FRAME_BUFFER);
  }
}

BOOLEAN PalEditKeyboardHook(InputAtom *pInputEvent) {
  UINT8 ubType;
  struct SOLDIERTYPE *pSoldier;
  UINT8 ubPaletteRep;
  UINT32 cnt;
  UINT8 ubStartRep = 0;
  UINT8 ubEndRep = 0;

  if (gusSelectedSoldier == NO_SOLDIER) {
    return (FALSE);
  }

  if ((pInputEvent->usEvent == KEY_DOWN) && (pInputEvent->usParam == ESC)) {
    gfExitPalEditScreen = TRUE;
    return (TRUE);
  }

  if ((pInputEvent->usEvent == KEY_DOWN) && (pInputEvent->usParam == 'h')) {
    // Get Soldier
    GetSoldier(&pSoldier, gusSelectedSoldier);

    // Get index of current
    if (!(GetPaletteRepIndexFromID(pSoldier->HeadPal, &ubPaletteRep))) {
      return FALSE;
    }
    ubType = gpPalRep[ubPaletteRep].ubType;

    ubPaletteRep++;

    // Count start and end index
    for (cnt = 0; cnt < ubType; cnt++) {
      ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[cnt]);
    }

    ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ubType]);

    if (ubPaletteRep == ubEndRep) {
      ubPaletteRep = ubStartRep;
    }
    SET_PALETTEREP_ID(pSoldier->HeadPal, gpPalRep[ubPaletteRep].ID);

    CreateSoldierPalettes(pSoldier);

    return (TRUE);
  }

  if ((pInputEvent->usEvent == KEY_DOWN) && (pInputEvent->usParam == 'v')) {
    // Get Soldier
    GetSoldier(&pSoldier, gusSelectedSoldier);

    // Get index of current
    if (!(GetPaletteRepIndexFromID(pSoldier->VestPal, &ubPaletteRep))) {
      return FALSE;
    }
    ubType = gpPalRep[ubPaletteRep].ubType;

    ubPaletteRep++;

    // Count start and end index
    for (cnt = 0; cnt < ubType; cnt++) {
      ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[cnt]);
    }

    ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ubType]);

    if (ubPaletteRep == ubEndRep) {
      ubPaletteRep = ubStartRep;
    }
    SET_PALETTEREP_ID(pSoldier->VestPal, gpPalRep[ubPaletteRep].ID);

    CreateSoldierPalettes(pSoldier);

    return (TRUE);
  }

  if ((pInputEvent->usEvent == KEY_DOWN) && (pInputEvent->usParam == 'p')) {
    // Get Soldier
    GetSoldier(&pSoldier, gusSelectedSoldier);

    // Get index of current
    if (!(GetPaletteRepIndexFromID(pSoldier->PantsPal, &ubPaletteRep))) {
      return FALSE;
    }
    ubType = gpPalRep[ubPaletteRep].ubType;

    ubPaletteRep++;

    // Count start and end index
    for (cnt = 0; cnt < ubType; cnt++) {
      ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[cnt]);
    }

    ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ubType]);

    if (ubPaletteRep == ubEndRep) {
      ubPaletteRep = ubStartRep;
    }
    SET_PALETTEREP_ID(pSoldier->PantsPal, gpPalRep[ubPaletteRep].ID);

    CreateSoldierPalettes(pSoldier);

    return (TRUE);
  }

  if ((pInputEvent->usEvent == KEY_DOWN) && (pInputEvent->usParam == 's')) {
    // Get Soldier
    GetSoldier(&pSoldier, gusSelectedSoldier);

    // Get index of current
    if (!(GetPaletteRepIndexFromID(pSoldier->SkinPal, &ubPaletteRep))) {
      return FALSE;
    }
    ubType = gpPalRep[ubPaletteRep].ubType;

    ubPaletteRep++;

    // Count start and end index
    for (cnt = 0; cnt < ubType; cnt++) {
      ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[cnt]);
    }

    ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ubType]);

    if (ubPaletteRep == ubEndRep) {
      ubPaletteRep = ubStartRep;
    }
    SET_PALETTEREP_ID(pSoldier->SkinPal, gpPalRep[ubPaletteRep].ID);

    CreateSoldierPalettes(pSoldier);

    return (TRUE);
  }

  return (FALSE);
}

UINT32 DebugScreenInit(void) { return (TRUE); }

BOOLEAN CheckForAndExitTacticalDebug() {
  if (gfExitDebugScreen) {
    FirstTime = TRUE;
    gfInitRect = TRUE;
    gfExitDebugScreen = FALSE;
    FreeBackgroundRect(guiBackgroundRect);
    SetRenderHook((RENDER_HOOK)NULL);
    SetUIKeyboardHook((UIKEYBOARD_HOOK)NULL);

    return (TRUE);
  }

  return (FALSE);
}

void ExitDebugScreen() {
  if (guiCurrentScreen == DEBUG_SCREEN) {
    gfExitDebugScreen = TRUE;
  }

  CheckForAndExitTacticalDebug();
}

UINT32 DebugScreenHandle(void) {
  if (CheckForAndExitTacticalDebug()) {
    return (GAME_SCREEN);
  }

  if (gfInitRect) {
    guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, 0, 0, 600, 360);
    gfInitRect = FALSE;
  }

  if (FirstTime) {
    FirstTime = FALSE;

    SetRenderHook((RENDER_HOOK)DebugRenderHook);
    SetUIKeyboardHook((UIKEYBOARD_HOOK)DebugKeyboardHook);

  } else {
    (*(GameScreens[GAME_SCREEN].HandleScreen))();
  }

  return (DEBUG_SCREEN);
}

UINT32 DebugScreenShutdown(void) { return (TRUE); }

void DebugRenderHook() { gDebugRenderOverride[gCurDebugPage](); }

BOOLEAN DebugKeyboardHook(InputAtom *pInputEvent) {
  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == 'q')) {
    gfExitDebugScreen = TRUE;
    return (TRUE);
  }

  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == PGUP)) {
    // Page down
    gCurDebugPage++;

    if (gCurDebugPage == MAX_DEBUG_PAGES) {
      gCurDebugPage = 0;
    }

    FreeBackgroundRect(guiBackgroundRect);
    gfInitRect = TRUE;
  }

  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == PGDN)) {
    // Page down
    gCurDebugPage--;

    if (gCurDebugPage < 0) {
      gCurDebugPage = MAX_DEBUG_PAGES - 1;
    }

    FreeBackgroundRect(guiBackgroundRect);
    gfInitRect = TRUE;
  }

  return (FALSE);
}

void SetDebugRenderHook(RENDER_HOOK pDebugRenderOverride, INT8 ubPage) {
  gDebugRenderOverride[ubPage] = pDebugRenderOverride;
}

void DefaultDebugPage1() {
  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG PAGE ONE");
}

void DefaultDebugPage2() {
  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG PAGE TWO");
}

void DefaultDebugPage3() {
  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG PAGE THREE");
}

void DefaultDebugPage4() {
  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG PAGE FOUR");
}

UINT32 SexScreenInit(void) { return (TRUE); }

#define SMILY_DELAY 100
#define SMILY_END_DELAY 1000

UINT32 SexScreenHandle(void) {
  static UINT8 ubCurrentScreen = 0;
  static UINT32 guiSMILY;
  static INT8 bCurFrame = 0;
  static UINT32 uiTimeOfLastUpdate = 0, uiTime;
  ETRLEObject *pTrav;
  struct VObject *hVObject;
  INT16 sX, sY;

  // OK, Clear screen and show smily face....
  VSurfaceColorFill(vsFB, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  InvalidateScreen();
  // Remove cursor....
  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  if (ubCurrentScreen == 0) {
    // Load face....
    if (!AddVObjectFromFile("INTERFACE\\luckysmile.sti", &guiSMILY))
      AssertMsg(0, "Missing INTERFACE\\luckysmile.sti");

    // Init screen
    bCurFrame = 0;

    ubCurrentScreen = 1;

    uiTimeOfLastUpdate = GetJA2Clock();

    return (SEX_SCREEN);
  }

  // Update frame
  uiTime = GetJA2Clock();

  // if we are animation smile...
  if (ubCurrentScreen == 1) {
    PlayJA2StreamingSampleFromFile("Sounds\\Sex.wav", RATE_11025, HIGHVOLUME, 1, MIDDLEPAN, NULL);
    if ((uiTime - uiTimeOfLastUpdate) > SMILY_DELAY) {
      uiTimeOfLastUpdate = uiTime;

      bCurFrame++;

      if (bCurFrame == 32) {
        // Start end delay
        ubCurrentScreen = 2;
      }
    }
  }

  if (ubCurrentScreen == 2) {
    if ((uiTime - uiTimeOfLastUpdate) > SMILY_END_DELAY) {
      uiTimeOfLastUpdate = uiTime;

      ubCurrentScreen = 0;

      // Remove video object...
      DeleteVideoObjectFromIndex(guiSMILY);

      FadeInGameScreen();

      // Advance time...
      // Chris.... do this based on stats?
      WarpGameTime(((5 + Random(20)) * NUM_SEC_IN_MIN), WARPTIME_NO_PROCESSING_OF_EVENTS);

      return (GAME_SCREEN);
    }
  }

  // Calculate smily face positions...
  GetVideoObject(&hVObject, guiSMILY);
  pTrav = &(hVObject->pETRLEObject[0]);

  sX = (INT16)((640 - pTrav->usWidth) / 2);
  sY = (INT16)((480 - pTrav->usHeight) / 2);

  if (bCurFrame < 24) {
    BltVideoObjectFromIndex(FRAME_BUFFER, guiSMILY, 0, sX, sY, VO_BLT_SRCTRANSPARENCY, NULL);
  } else {
    BltVideoObjectFromIndex(FRAME_BUFFER, guiSMILY, (INT8)(bCurFrame % 8), sX, sY,
                            VO_BLT_SRCTRANSPARENCY, NULL);
  }

  InvalidateRegion(sX, sY, (INT16)(sX + pTrav->usWidth), (INT16)(sY + pTrav->usHeight));

  return (SEX_SCREEN);
}

UINT32 SexScreenShutdown(void) { return (TRUE); }

UINT32 DemoExitScreenInit(void) { return (TRUE); }

void DoneFadeOutForDemoExitScreen(void) { gfProgramIsRunning = FALSE; }

extern INT8 gbFadeSpeed;

UINT32 DemoExitScreenHandle(void) {
  gfProgramIsRunning = FALSE;
  return (DEMO_EXIT_SCREEN);
}

UINT32 DemoExitScreenShutdown(void) { return (TRUE); }
