#include "GameScreen.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "Editor/EditScreen.h"
#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "MessageBoxScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Scheduling.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicTurns.h"
#include "SysGlobals.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AutoBandage.h"
#include "Tactical/Bullets.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/MapInformation.h"
#include "Tactical/MercEntering.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Squads.h"
#include "Tactical/StrategicExitGUI.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#define ARE_IN_FADE_IN() (gfFadeIn || gfFadeInitialized)

BOOLEAN fDirtyRectangleMode = FALSE;
uint16_t *gpFPSBuffer = NULL;
// MarkNote
// extern ScrollStringStPtr pStringS=NULL;
uint32_t counter = 0;
uint32_t count = 0;
BOOLEAN gfTacticalDoHeliRun = FALSE;
BOOLEAN gfPlayAttnAfterMapLoad = FALSE;

// VIDEO OVERLAYS
int32_t giFPSOverlay = 0;
int32_t giCounterPeriodOverlay = 0;

BOOLEAN gfExitToNewSector = FALSE;
// uint8_t		gubNewSectorExitDirection;

BOOLEAN gfGameScreenLocateToSoldier = FALSE;
BOOLEAN gfEnteringMapScreen = FALSE;
uint32_t uiOldMouseCursor;
uint8_t gubPreferredInitialSelectedGuy = NOBODY;

BOOLEAN gfTacticalIsModal = FALSE;
struct MOUSE_REGION gTacticalDisableRegion;
BOOLEAN gfTacticalDisableRegionActive = FALSE;
int8_t gbTacticalDisableMode = FALSE;
MODAL_HOOK gModalDoneCallback;
BOOLEAN gfBeginEndTurn = FALSE;
extern BOOLEAN gfTopMessageDirty;
extern BOOLEAN gfFailedToSaveGameWhenInsideAMessageBox;
extern BOOLEAN gfFirstHeliRun;
extern BOOLEAN gfRenderFullThisFrame;

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game
RENDER_HOOK gRenderOverride = NULL;

#define NOINPUT_DELAY 60000
#define DEMOPLAY_DELAY 40000
#define RESTART_DELAY 6000

void TacticalScreenLocateToSoldier();

uint32_t guiTacticalLeaveScreenID;
BOOLEAN guiTacticalLeaveScreen = FALSE;

void HandleModalTactical();
extern void CheckForDisabledRegionRemove();
extern void InternalLocateGridNo(uint16_t sGridNo, BOOLEAN fForce);

uint32_t MainGameScreenInit(void) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  gpZBuffer = InitZBuffer(1280, 480);
  InitializeBackgroundRects();

  // EnvSetTimeInHours(ENV_TIME_12);

  SetRenderFlags(RENDER_FLAG_FULL);

  // Init Video Overlays
  // FIRST, FRAMERATE
  VideoOverlayDesc.sLeft = 0;
  VideoOverlayDesc.sTop = 0;
  VideoOverlayDesc.uiFontID = SMALLFONT1;
  VideoOverlayDesc.ubFontBack = FONT_MCOLOR_BLACK;
  VideoOverlayDesc.ubFontFore = FONT_MCOLOR_DKGRAY;
  VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
  VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
  swprintf(VideoOverlayDesc.pzText, ARR_SIZE(VideoOverlayDesc.pzText), L"90");
  VideoOverlayDesc.BltCallback = BlitMFont;
  giFPSOverlay =
      RegisterVideoOverlay((VOVERLAY_STARTDISABLED | VOVERLAY_DIRTYBYTEXT), &VideoOverlayDesc);

  // SECOND, PERIOD COUNTER
  VideoOverlayDesc.sLeft = 30;
  VideoOverlayDesc.sTop = 0;
  VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
  VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
  swprintf(VideoOverlayDesc.pzText, ARR_SIZE(VideoOverlayDesc.pzText), L"Levelnodes: 100000");
  VideoOverlayDesc.BltCallback = BlitMFont;
  giCounterPeriodOverlay =
      RegisterVideoOverlay((VOVERLAY_STARTDISABLED | VOVERLAY_DIRTYBYTEXT), &VideoOverlayDesc);

  // register debug topics
  RegisterJA2DebugTopic(TOPIC_JA2, "Reg JA2 Debug");
  // MarkNote

  return TRUE;
}

// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly

uint32_t MainGameScreenShutdown(void) {
  ShutdownZBuffer(gpZBuffer);
  ShutdownBackgroundRects();

  // Remove video Overlays
  RemoveVideoOverlay(giFPSOverlay);

  return TRUE;
}

void FadeInGameScreen() {
  fFirstTimeInGameScreen = TRUE;

  FadeInNextFrame();
}

void FadeOutGameScreen() { FadeOutNextFrame(); }

void EnterTacticalScreen() {
  guiTacticalLeaveScreen = FALSE;

  SetPositionSndsActive();

  // Set pending screen
  SetPendingNewScreen(GAME_SCREEN);

  // Set as active...
  gTacticalStatus.uiFlags |= ACTIVE;

  fInterfacePanelDirty = DIRTYLEVEL2;

  // Disable all faces
  SetAllAutoFacesInactive();

  // CHECK IF OURGUY IS NOW OFF DUTY
  if (gusSelectedSoldier != NOBODY) {
    if (!OK_CONTROLLABLE_MERC(MercPtrs[gusSelectedSoldier])) {
      SelectNextAvailSoldier(MercPtrs[gusSelectedSoldier]);
    }
    // ATE: If the current guy is sleeping, change....
    if (MercPtrs[gusSelectedSoldier]->fMercAsleep) {
      SelectNextAvailSoldier(MercPtrs[gusSelectedSoldier]);
    }
  } else {
    // otherwise, make sure interface is team panel...
    SetCurrentInterfacePanel((uint8_t)TEAM_PANEL);
  }

  if (!gfTacticalPlacementGUIActive) {
    MSYS_EnableRegion(&gRadarRegion);
  }
  MSYS_EnableRegion(&gViewportRegion);

  // set default squad on sector entry
  // ATE: moved these 2 call after initalizing the interface!
  // SetDefaultSquadOnSectorEntry( FALSE );
  // ExamineCurrentSquadLights( );

  // UpdateMercsInSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

  // Init interface ( ALWAYS TO TEAM PANEL.  DEF changed it to go back to the previous panel )
  if (!gfTacticalPlacementGUIActive) {
    // make sure the gsCurInterfacePanel is valid
    if (gsCurInterfacePanel < 0 || gsCurInterfacePanel >= NUM_UI_PANELS)
      gsCurInterfacePanel = TEAM_PANEL;

    SetCurrentInterfacePanel((uint8_t)gsCurInterfacePanel);
  }

  SetTacticalInterfaceFlags(0);

  // set default squad on sector entry
  SetDefaultSquadOnSectorEntry(FALSE);
  ExamineCurrentSquadLights();

  fFirstTimeInGameScreen = FALSE;

  // Make sure it gets re-created....
  DirtyTopMessage();

  // Set compression to normal!
  // SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );

  // Select current guy...
  // gfGameScreenLocateToSoldier = TRUE;

  // Locate if in meanwhile...
  if (AreInMeanwhile()) {
    LocateToMeanwhileCharacter();
  }

  if (gTacticalStatus.uiFlags & IN_DEIDRANNA_ENDGAME) {
    InternalLocateGridNo(4561, TRUE);
  }

  // Clear tactical message q
  ClearTacticalMessageQueue();

  // ATE: Enable messages again...
  EnableScrollMessages();
}

void LeaveTacticalScreen(uint32_t uiNewScreen) {
  guiTacticalLeaveScreenID = uiNewScreen;
  guiTacticalLeaveScreen = TRUE;
}

void InternalLeaveTacticalScreen(uint32_t uiNewScreen) {
  gpCustomizableTimerCallback = NULL;

  // unload the sector they teleported out of
  if (!gfAutomaticallyStartAutoResolve) {
    CheckAndHandleUnloadingOfCurrentWorld();
  }

  SetPositionSndsInActive();

  // Turn off active flag
  gTacticalStatus.uiFlags &= (~ACTIVE);

  fFirstTimeInGameScreen = TRUE;

  SetPendingNewScreen(uiNewScreen);

  // Disable all faces
  SetAllAutoFacesInactive();

  ResetInterfaceAndUI();

  // Remove cursor and reset height....
  gsGlobalCursorYOffset = 0;
  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  // Shutdown panel
  ShutdownCurrentPanel();

  // disable the radar map
  MSYS_DisableRegion(&gRadarRegion);
  // MSYS_DisableRegion( &gViewportRegion );

  // We are leaving... turn off pedning autobadage...
  SetAutoBandagePending(FALSE);

  // ATE: Disable messages....
  DisableScrollMessages();

  if (uiNewScreen == MAINMENU_SCREEN) {
    // We want to reinitialize the game
    ReStartingGame();
  }

  if (uiNewScreen != MAP_SCREEN) {
    StopAnyCurrentlyTalkingSpeech();
  }

  // If we have some disabled screens up.....remove...
  CheckForDisabledRegionRemove();

  // ATE: Record last time we were in tactical....
  gTacticalStatus.uiTimeSinceLastInTactical = GetWorldTotalMin();

  FinishAnySkullPanelAnimations();
}

extern int32_t iInterfaceDialogueBox;

#ifdef JA2BETAVERSION
extern BOOLEAN ValidateSoldierInitLinks(uint8_t ubCode);
extern BOOLEAN gfDoDialogOnceGameScreenFadesIn;
#endif

uint32_t MainGameScreenHandle(void) {
  uint32_t uiNewScreen = GAME_SCREEN;

  // DO NOT MOVE THIS FUNCTION CALL!!!
  // This determines if the help screen should be active
  //	if( ( !gfTacticalDoHeliRun && !gfFirstHeliRun ) && ShouldTheHelpScreenComeUp(
  // HELP_SCREEN_TACTICAL, FALSE ) )
  if (!gfPreBattleInterfaceActive && ShouldTheHelpScreenComeUp(HELP_SCREEN_TACTICAL, FALSE)) {
    // handle the help screen
    HelpScreenHandler();
    return (GAME_SCREEN);
  }

#ifdef JA2BETAVERSION
  DebugValidateSoldierData();
#endif

  if (HandleAutoBandage()) {
#ifndef VISIBLE_AUTO_BANDAGE
    return (GAME_SCREEN);
#endif
  }

  if (gfBeginEndTurn) {
    UIHandleEndTurn(NULL);
    gfBeginEndTurn = FALSE;
  }

  // The gfFailedToSaveGameWhenInsideAMessageBox flag will only be set at this point if the game
  // fails to save during a quick save and when the game was already in a message box.
  // If the game failed to save when in a message box, pop up a message box stating an error occured
  if (gfFailedToSaveGameWhenInsideAMessageBox) {
    gfFailedToSaveGameWhenInsideAMessageBox = FALSE;

    DoMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], GAME_SCREEN,
                 MSG_BOX_FLAG_OK, NULL, NULL);

    return (GAME_SCREEN);
  }

  // Check if we are in bar animation...
  if (InTopMessageBarAnimation()) {
    ExecuteBaseDirtyRectQueue();

    EndFrameBufferRender();

    return (GAME_SCREEN);
  }

  if (gfTacticalIsModal) {
    if (gfTacticalIsModal == 1) {
      gfTacticalIsModal++;
    } else {
      HandleModalTactical();

      return (GAME_SCREEN);
    }
  }

  // OK, this is the pause system for when we see a guy...
  if (!ARE_IN_FADE_IN()) {
    if (gTacticalStatus.fEnemySightingOnTheirTurn) {
      if ((GetJA2Clock() - gTacticalStatus.uiTimeSinceDemoOn) > 3000) {
        if (gTacticalStatus.ubCurrentTeam != gbPlayerNum) {
          AdjustNoAPToFinishMove(MercPtrs[gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID],
                                 FALSE);
        }
        MercPtrs[gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID]->fPauseAllAnimation = FALSE;

        gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
      }
    }
  }

  // see if the helicopter is coming in this time for the initial entrance by the mercs
  InitHelicopterEntranceByMercs();

  // Handle Environment controller here
  EnvironmentController(TRUE);

  if (!ARE_IN_FADE_IN()) {
    HandleWaitTimerForNPCTrigger();

    // Check timer that could have been set to do anything
    CheckCustomizableTimer();

    // HAndle physics engine
    SimulateWorld();

    // Handle strategic engine
    HandleStrategicTurn();
  }

  if (gfTacticalDoHeliRun) {
    gfGameScreenLocateToSoldier = FALSE;
    InternalLocateGridNo(gMapInformation.sNorthGridNo, TRUE);

    // Start heli Run...
    StartHelicopterRun(gMapInformation.sNorthGridNo);

    // Update clock by one so that our DidGameJustStatrt() returns now false for things like LAPTOP,
    // etc...
    SetGameTimeCompressionLevel(TIME_COMPRESS_X1);
    // UpdateClock( 1 );

    gfTacticalDoHeliRun = FALSE;
    // SetMusicMode( MUSIC_TACTICAL_NOTHING );
  }

  if (InOverheadMap()) {
    HandleOverheadMap();
    return (GAME_SCREEN);
  }

  if (!ARE_IN_FADE_IN()) {
    HandleAirRaid();
  }

  if (gfGameScreenLocateToSoldier) {
    TacticalScreenLocateToSoldier();
    gfGameScreenLocateToSoldier = FALSE;
  }

  if (fFirstTimeInGameScreen) {
    EnterTacticalScreen();

    // Select a guy if he hasn;'
    if (!gfTacticalPlacementGUIActive) {
      if (gusSelectedSoldier != NOBODY && OK_INTERRUPT_MERC(MercPtrs[gusSelectedSoldier])) {
        SelectSoldier(gusSelectedSoldier, FALSE, TRUE);
      }
    }
  }

  // ATE: check that this flag is not set.... display message if so
  if (IsMapScreen()) {
    // Unset
    guiTacticalInterfaceFlags &= (~INTERFACE_MAPSCREEN);

#ifdef JA2BETAVERSION
    ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
              L"MAPSCREEN_INTERFACE flag set: Please remember how you entered Tactical.");
#endif
  }

  if (HandleFadeOutCallback()) {
    return (GAME_SCREEN);
  }

  if (guiCurrentScreen != MSG_BOX_SCREEN) {
    if (HandleBeginFadeOut(GAME_SCREEN)) {
      return (GAME_SCREEN);
    }
  }

#ifdef JA2BETAVERSION
  if (gfDoDialogOnceGameScreenFadesIn) {
    ValidateSoldierInitLinks(4);
  }
#endif

  HandleHeliDrop();

  if (!ARE_IN_FADE_IN()) {
    HandleAutoBandagePending();
  }

  // ATE: CHRIS_C LOOK HERE FOR GETTING AI CONSTANTLY GOING
  // if ( gTacticalStatus.uiFlags & TURNBASED )
  //{
  //	if ( !(gTacticalStatus.uiFlags & ENEMYS_TURN) )
  //	{
  //		EndTurn( );
  //	}
  //}

  //	if ( gfScrollInertia == FALSE )
  {
    if (!ARE_IN_FADE_IN()) {
      UpdateBullets();

      // Execute Tactical Overhead
      ExecuteOverhead();
    }

    // Handle animated cursors
    if (gfWorldLoaded) {
      HandleAnimatedCursors();

      // Handle Interface
      uiNewScreen = HandleTacticalUI();

      // called to handle things like face panels changeing due to team panel, squad changes, etc
      // To be done AFTER HandleUI and before ExecuteOverlays( )
      HandleDialogueUIAdjustments();

      HandleTalkingAutoFaces();
    }
#ifdef JA2EDITOR
    else if (gfIntendOnEnteringEditor) {
      DebugPrint("Aborting normal game mode and entering editor mode...\n");
      SetPendingNewScreen(0xffff);  // NO_SCREEN
      return EDIT_SCREEN;
    }
#endif
    else if (!gfEnteringMapScreen) {
      gfEnteringMapScreen = TRUE;
    }

    if (uiNewScreen != GAME_SCREEN) {
      return (uiNewScreen);
    }

    // Deque all game events
    if (!ARE_IN_FADE_IN()) {
      DequeAllGameEvents(TRUE);
    }
  }

  /////////////////////////////////////////////////////
  StartFrameBufferRender();

  HandleTopMessages();

  if (gfScrollPending || gfScrollInertia) {
  } else {
    // Handle Interface Stuff
    SetUpInterface();
    HandleTacticalPanelSwitch();
  }

  // Handle Scroll Of World
  ScrollWorld();

  // SetRenderFlags( RENDER_FLAG_FULL );

  RenderWorld();

  if (gRenderOverride != NULL) {
    gRenderOverride();
  }

  if (gfScrollPending || gfScrollInertia) {
    RenderTacticalInterfaceWhileScrolling();
  } else {
    // Handle Interface Stuff
    // RenderTacticalInterface( );
  }

#ifdef NETWORKED
  // DEF:  Test Code
  PrintNetworkInfo();
#endif

  // Render Interface
  RenderTopmostTacticalInterface();

#ifdef JA2TESTVERSION
  if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) {
    SetFont(MILITARYFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_LTGREEN);

    mprintf(0, 0, L"IN CONVERSATION %d", giNPCReferenceCount);
    gprintfdirty(0, 0, L"IN CONVERSATION %d", giNPCReferenceCount);
  }

#ifdef JA2BETAVERSION

  if (GamePaused() == TRUE) {
    SetFont(MILITARYFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_LTGREEN);

    mprintf(0, 10, L"Game Clock Paused");
    gprintfdirty(0, 10, L"Game Clock Paused");
  }

#endif

  if (gTacticalStatus.uiFlags & SHOW_ALL_MERCS) {
    int32_t iSchedules;
    SCHEDULENODE *curr;

    SetFont(MILITARYFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_LTGREEN);

    mprintf(0, 15, L"Attacker Busy Count: %d", gTacticalStatus.ubAttackBusyCount);
    gprintfdirty(0, 15, L"Attacker Busy Count: %d", gTacticalStatus.ubAttackBusyCount);

    curr = gpScheduleList;
    iSchedules = 0;
    while (curr) {
      iSchedules++;
      curr = curr->next;
    }

    mprintf(0, 25, L"Schedules: %d", iSchedules);
    gprintfdirty(0, 25, L"Schedules: %d", iSchedules);
  }
#endif

  // Render view window
  RenderRadarScreen();

  ResetInterface();

  if (gfScrollPending) {
    AllocateVideoOverlaysArea();
    SaveVideoOverlaysArea(FRAME_BUFFER);
    ExecuteVideoOverlays();
  } else {
    ExecuteVideoOverlays();
  }

  // Adding/deleting of video overlays needs to be done below
  // ExecuteVideoOverlays( )....

  // Handle dialogue queue system
  if (!ARE_IN_FADE_IN()) {
    if (gfPlayAttnAfterMapLoad) {
      gfPlayAttnAfterMapLoad = FALSE;

      if (gusSelectedSoldier != NOBODY) {
        if (!gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS])
          DoMercBattleSound(MercPtrs[gusSelectedSoldier], BATTLE_SOUND_ATTN1);
      }
    }

    HandleDialogue();
  }

  // Don't render if we have a scroll pending!
  if (!gfScrollPending && !gfScrollInertia && !gfRenderFullThisFrame) {
    RenderButtonsFastHelp();
  }

  // Display Framerate
  DisplayFrameRate();

  CheckForMeanwhileOKStart();

  ScrollString();

  ExecuteBaseDirtyRectQueue();

  // KillBackgroundRects( );

  /////////////////////////////////////////////////////
  EndFrameBufferRender();

  if (HandleFadeInCallback()) {
    // Re-render the scene!
    SetRenderFlags(RENDER_FLAG_FULL);
    fInterfacePanelDirty = DIRTYLEVEL2;
  }

  if (HandleBeginFadeIn(GAME_SCREEN)) {
    guiTacticalLeaveScreenID = FADE_SCREEN;
  }

  if (guiTacticalLeaveScreen) {
    guiTacticalLeaveScreen = FALSE;

    InternalLeaveTacticalScreen(guiTacticalLeaveScreenID);
  }

  // Check if we are to enter map screen
  if (gfEnteringMapScreen == 2) {
    gfEnteringMapScreen = FALSE;
    EnterMapScreen();
  }

  // Are we entering map screen? if so, wait a frame!
  if (gfEnteringMapScreen > 0) {
    gfEnteringMapScreen++;
  }

  return (GAME_SCREEN);
}

void SetRenderHook(RENDER_HOOK pRenderOverride) { gRenderOverride = pRenderOverride; }

void DisableFPSOverlay(BOOLEAN fEnable) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  VideoOverlayDesc.fDisabled = fEnable;
  VideoOverlayDesc.uiFlags = VOVERLAY_DESC_DISABLED;

  UpdateVideoOverlay(&VideoOverlayDesc, giFPSOverlay, FALSE);
  UpdateVideoOverlay(&VideoOverlayDesc, giCounterPeriodOverlay, FALSE);
}

void TacticalScreenLocateToSoldier() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int16_t bLastTeamID;
  BOOLEAN fPreferedGuyUsed = FALSE;

  if (gubPreferredInitialSelectedGuy != NOBODY) {
    // ATE: Put condition here...
    if (OK_CONTROLLABLE_MERC(MercPtrs[gubPreferredInitialSelectedGuy]) &&
        OK_INTERRUPT_MERC(MercPtrs[gubPreferredInitialSelectedGuy])) {
      LocateSoldier(gubPreferredInitialSelectedGuy, 10);
      SelectSoldier(gubPreferredInitialSelectedGuy, FALSE, TRUE);
      fPreferedGuyUsed = TRUE;
    }
    gubPreferredInitialSelectedGuy = NOBODY;
  }

  if (!fPreferedGuyUsed) {
    // Set locator to first merc
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;
    for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
      if (OK_CONTROLLABLE_MERC(pSoldier) && OK_INTERRUPT_MERC(pSoldier)) {
        LocateSoldier(pSoldier->ubID, 10);
        SelectSoldier(pSoldier->ubID, FALSE, TRUE);
        break;
      }
    }
  }
}

void EnterMapScreen() {
  // ATE: These flags well get set later on in mapscreen....
  // SetTacticalInterfaceFlags( INTERFACE_MAPSCREEN );
  // fInterfacePanelDirty = DIRTYLEVEL2;
  LeaveTacticalScreen(MAP_SCREEN);
}

void UpdateTeamPanelAssignments() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int16_t bLastTeamID;

  // Remove all players
  RemoveAllPlayersFromSlot();

  // Set locator to first merc
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;
  for (pSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pSoldier++) {
    // Setup team interface
    CheckForAndAddMercToTeamPanel(pSoldier);
  }
}

void EnterModalTactical(int8_t bMode) {
  gbTacticalDisableMode = bMode;
  gfTacticalIsModal = TRUE;

  if (gbTacticalDisableMode == TACTICAL_MODAL_NOMOUSE) {
    if (!gfTacticalDisableRegionActive) {
      gfTacticalDisableRegionActive = TRUE;

      MSYS_DefineRegion(&gTacticalDisableRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGH,
                        VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
      // Add region
      MSYS_AddRegion(&gTacticalDisableRegion);
    }
  }

  UpdateSaveBuffer();
}

void EndModalTactical() {
  if (gfTacticalDisableRegionActive) {
    MSYS_RemoveRegion(&gTacticalDisableRegion);

    gfTacticalDisableRegionActive = FALSE;
  }

  if (gModalDoneCallback != NULL) {
    gModalDoneCallback();

    gModalDoneCallback = NULL;
  }

  gfTacticalIsModal = FALSE;

  SetRenderFlags(RENDER_FLAG_FULL);
}

void HandleModalTactical() {
  StartFrameBufferRender();

  RestoreBackgroundRects();

  RenderWorld();
  RenderRadarScreen();
  ExecuteVideoOverlays();

  // Handle dialogue queue system
  HandleDialogue();

  HandleTalkingAutoFaces();

  // Handle faces
  HandleAutoFaces();

  if (gfInSectorExitMenu) {
    RenderSectorExitMenu();
  }
  RenderButtons();

  SaveBackgroundRects();
  RenderButtonsFastHelp();
  RenderPausedGameBox();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();
}

void InitHelicopterEntranceByMercs(void) {
  if (DidGameJustStart()) {
    // Update clock ahead from STARTING_TIME to make mercs arrive!
    WarpGameTime(FIRST_ARRIVAL_DELAY, WARPTIME_PROCESS_EVENTS_NORMALLY);

    gfTacticalDoHeliRun = TRUE;
    gfFirstHeliRun = TRUE;

    gTacticalStatus.fDidGameJustStart = FALSE;
  }
}
