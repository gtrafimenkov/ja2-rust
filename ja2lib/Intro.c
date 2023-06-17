#include "Intro.h"

#include <string.h>

#include "GameScreen.h"
#include "Intro.h"
#include "Local.h"
#include "MainMenuScreen.h"
#include "MessageBoxScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Ja2Libs.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Strategic/GameInit.h"
#include "SysGlobals.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/Cinematics.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "rust_fileman.h"

extern wchar_t *gzIntroScreen[];

enum {
  INTRO_TXT__CANT_FIND_INTRO,
};

// ddd

// ggg
BOOLEAN gfIntroScreenEntry;
BOOLEAN gfIntroScreenExit;

uint32_t guiIntroExitScreen = INTRO_SCREEN;

extern BOOLEAN gfDoneWithSplashScreen;

struct SmkFlic *gpSmackFlic = NULL;

#define SMKINTRO_FIRST_VIDEO 255
#define SMKINTRO_NO_VIDEO -1

// enums for the various smacker files
enum {
  SMKINTRO_REBEL_CRDT,
  SMKINTRO_OMERTA,
  SMKINTRO_PRAGUE_CRDT,
  SMKINTRO_PRAGUE,

  // there are no more videos shown for the begining

  SMKINTRO_END_END_SPEECH_MIGUEL,
  SMKINTRO_END_END_SPEECH_NO_MIGUEL,
  SMKINTRO_END_HELI_FLYBY,
  SMKINTRO_END_SKYRIDER_HELICOPTER,
  SMKINTRO_END_NOSKYRIDER_HELICOPTER,

  SMKINTRO_SPLASH_SCREEN,
  SMKINTRO_SPLASH_TALONSOFT,

  // there are no more videos shown for the endgame
  SMKINTRO_LAST_END_GAME,
};

int32_t giCurrentIntroBeingPlayed = SMKINTRO_NO_VIDEO;

char *gpzSmackerFileNames[] = {
    // begining of the game
    "INTRO\\Rebel_cr.smk",
    "INTRO\\Omerta.smk",
    "INTRO\\Prague_cr.smk",
    "INTRO\\Prague.smk",

    // endgame
    "INTRO\\Throne_Mig.smk",
    "INTRO\\Throne_NoMig.smk",
    "INTRO\\Heli_FlyBy.smk",
    "INTRO\\Heli_Sky.smk",
    "INTRO\\Heli_NoSky.smk",

    "INTRO\\SplashScreen.smk",
    "INTRO\\TalonSoftid_endhold.smk",
};

// enums used for when the intro screen can come up, either begining game intro, or end game
// cinematic
int8_t gbIntroScreenMode = -1;

void GetIntroScreenUserInput();
BOOLEAN EnterIntroScreen();
void RenderIntroScreen();
void ExitIntroScreen();
void HandleIntroScreen();
void PrepareToExitIntroScreen();
int32_t GetNextIntroVideo(uint32_t uiCurrentVideo);
void StartPlayingIntroFlic(int32_t iIndexOfFlicToPlay);
void DisplaySirtechSplashScreen();

// ppp

uint32_t IntroScreenInit(void) {
  // Set so next time we come in, we can set up
  gfIntroScreenEntry = TRUE;

  return (1);
}

uint32_t IntroScreenShutdown(void) { return (1); }

uint32_t IntroScreenHandle(void) {
  if (gfIntroScreenEntry) {
    EnterIntroScreen();
    gfIntroScreenEntry = FALSE;
    gfIntroScreenExit = FALSE;

    InvalidateRegion(0, 0, 640, 480);
  }

  RestoreBackgroundRects();

  GetIntroScreenUserInput();

  HandleIntroScreen();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  if (gfIntroScreenExit) {
    ExitIntroScreen();
    gfIntroScreenExit = FALSE;
    gfIntroScreenEntry = TRUE;
  }

  return (guiIntroExitScreen);
}

BOOLEAN EnterIntroScreen() {
  int32_t iFirstVideoID = -1;

  ClearMainMenu();

  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  // Don't play music....
  SetMusicMode(MUSIC_NONE);

#ifdef JA2BETAVERSION
  if (File_Exists("..\\NoIntro.txt")) {
    PrepareToExitIntroScreen();
    return (TRUE);
  }
#endif

  // initialize smacker
  SmkInitialize(640, 480);

  // get the index opf the first video to watch
  iFirstVideoID = GetNextIntroVideo(SMKINTRO_FIRST_VIDEO);

  if (iFirstVideoID != -1) {
    StartPlayingIntroFlic(iFirstVideoID);

    guiIntroExitScreen = INTRO_SCREEN;
  }

  // Got no intro video, exit
  else {
    PrepareToExitIntroScreen();
  }

  return (TRUE);
}

void RenderIntroScreen() {}

void ExitIntroScreen() {
  // shutdown smaker
  SmkShutdown();
}

void HandleIntroScreen() {
  BOOLEAN fFlicStillPlaying = FALSE;

  // if we are exiting this screen, this frame, dont update the screen
  if (gfIntroScreenExit) return;

  // handle smaker each frame
  fFlicStillPlaying = SmkPollFlics();

  // if the flic is not playing
  if (!fFlicStillPlaying) {
    int32_t iNextVideoToPlay = -1;

    iNextVideoToPlay = GetNextIntroVideo(giCurrentIntroBeingPlayed);

    if (iNextVideoToPlay != -1) {
      StartPlayingIntroFlic(iNextVideoToPlay);
    } else {
      PrepareToExitIntroScreen();
      giCurrentIntroBeingPlayed = -1;
    }
  }

  InvalidateScreen();
}

void GetIntroScreenUserInput() {
  InputAtom Event;
  struct Point MousePos = GetMousePoint();

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

    if (Event.usEvent == KEY_UP) {
      switch (Event.usParam) {
        case ESC:
          PrepareToExitIntroScreen();
          break;
        case SPACE:
          SmkCloseFlic(gpSmackFlic);
          break;

#ifdef JA2TESTVERSION

        case 'r':
          break;

        case 'i':
          InvalidateRegion(0, 0, 640, 480);
          break;

#endif
      }
    }
  }

  // if the user presses either mouse button
  if (gfLeftButtonState || gfRightButtonState) {
    // advance to the next flic
    SmkCloseFlic(gpSmackFlic);
  }
}

void PrepareToExitIntroScreen() {
  // if its the intro at the begining of the game
  if (gbIntroScreenMode == INTRO_BEGINING) {
    // go to the init screen
    guiIntroExitScreen = INIT_SCREEN;
  } else if (gbIntroScreenMode == INTRO_SPLASH) {
    // display a logo when exiting
    DisplaySirtechSplashScreen();

    gfDoneWithSplashScreen = TRUE;
    guiIntroExitScreen = INIT_SCREEN;
  } else {
    // We want to reinitialize the game
    ReStartingGame();

    //		guiIntroExitScreen = MAINMENU_SCREEN;
    guiIntroExitScreen = CREDIT_SCREEN;
  }

  gfIntroScreenExit = TRUE;
}

int32_t GetNextIntroVideo(uint32_t uiCurrentVideo) {
  int32_t iStringToUse = -1;

  // switch on whether it is the beginging or the end game video
  switch (gbIntroScreenMode) {
    // the video at the begining of the game
    case INTRO_BEGINING: {
      switch (uiCurrentVideo) {
        case SMKINTRO_FIRST_VIDEO:
          iStringToUse = SMKINTRO_REBEL_CRDT;
          break;
        case SMKINTRO_REBEL_CRDT:
          iStringToUse = SMKINTRO_OMERTA;
          break;
        case SMKINTRO_OMERTA:
          iStringToUse = SMKINTRO_PRAGUE_CRDT;
          break;
        case SMKINTRO_PRAGUE_CRDT:
          iStringToUse = SMKINTRO_PRAGUE;
          break;
        case SMKINTRO_PRAGUE:
          iStringToUse = -1;
          break;
          //				case SMKINTRO_LAST_INTRO:
          //					iStringToUse = -1;
          //					break;
      }
    } break;

    // end game
    case INTRO_ENDING: {
      switch (uiCurrentVideo) {
        case SMKINTRO_FIRST_VIDEO:
          // if Miguel is dead, play the flic with out him in it
          if (gMercProfiles[MIGUEL].bMercStatus == MERC_IS_DEAD)
            iStringToUse = SMKINTRO_END_END_SPEECH_NO_MIGUEL;
          else
            iStringToUse = SMKINTRO_END_END_SPEECH_MIGUEL;
          break;

        case SMKINTRO_END_END_SPEECH_MIGUEL:
        case SMKINTRO_END_END_SPEECH_NO_MIGUEL:
          iStringToUse = SMKINTRO_END_HELI_FLYBY;
          break;

        // if SkyRider is dead, play the flic without him
        case SMKINTRO_END_HELI_FLYBY:
          if (gMercProfiles[SKYRIDER].bMercStatus == MERC_IS_DEAD)
            iStringToUse = SMKINTRO_END_NOSKYRIDER_HELICOPTER;
          else
            iStringToUse = SMKINTRO_END_SKYRIDER_HELICOPTER;
          break;
      }
    } break;

    case INTRO_SPLASH:
      switch (uiCurrentVideo) {
        case SMKINTRO_FIRST_VIDEO:
          iStringToUse = SMKINTRO_SPLASH_SCREEN;
          break;
        case SMKINTRO_SPLASH_SCREEN:
          // iStringToUse = SMKINTRO_SPLASH_TALONSOFT;
          break;
      }
      break;
  }

  return (iStringToUse);
}

void StartPlayingIntroFlic(int32_t iIndexOfFlicToPlay) {
  if (iIndexOfFlicToPlay != -1) {
    // start playing a flic
    gpSmackFlic = SmkPlayFlic(gpzSmackerFileNames[iIndexOfFlicToPlay], 0, 0, TRUE);

    if (gpSmackFlic != NULL) {
      giCurrentIntroBeingPlayed = iIndexOfFlicToPlay;
    } else {
      PrepareToExitIntroScreen();
    }
  }
}

void SetIntroType(int8_t bIntroType) {
  if (bIntroType == INTRO_BEGINING) {
    gbIntroScreenMode = INTRO_BEGINING;
  } else if (bIntroType == INTRO_ENDING) {
    gbIntroScreenMode = INTRO_ENDING;
  } else if (bIntroType == INTRO_SPLASH) {
    gbIntroScreenMode = INTRO_SPLASH;
  }
}

void DisplaySirtechSplashScreen() {
  struct VObject *hPixHandle;
  VOBJECT_DESC VObjectDesc;
  uint32_t uiLogoID;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // JA3Gold: do nothing until we have a graphic to replace Talonsoft's
  // return;

  // CLEAR THE FRAME BUFFER
  pDestBuf = VSurfaceLockOld(vsFB, &uiDestPitchBYTES);
  memset(pDestBuf, 0, SCREEN_HEIGHT * uiDestPitchBYTES);
  VSurfaceUnlock(vsFB);

  if (!AddVObjectFromFile("INTERFACE\\SirtechSplash.sti", &uiLogoID)) {
    AssertMsg(0, String("Failed to load %s", VObjectDesc.ImageFile));
    return;
  }

  GetVideoObject(&hPixHandle, uiLogoID);
  BltVObject(vsFB, hPixHandle, 0, 0, 0);
  DeleteVideoObjectFromIndex(uiLogoID);

  InvalidateScreen();
  RefreshScreen();
}
