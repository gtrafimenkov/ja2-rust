#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <zmouse.h>

#include "BuildDefines.h"
#include "GameLoop.h"
#include "Globals.h"
#include "Intro.h"
#include "JA2Splash.h"
#include "Laptop/Laptop.h"
#include "Local.h"
#include "Res/Resource.h"
#include "SGP/ButtonSystem.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/Random.h"
#include "SGP/SGP.h"
#include "SGP/SoundMan.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "StrUtils.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Tactical/InterfacePanels.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "platform.h"
#include "platform_strings.h"
#include "platform_win.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

extern BOOLEAN gfPauseDueToPlayerGamePause;

extern void QueueEvent(uint16_t ubInputEvent, uint32_t usParam, uint32_t uiParam);

// Prototype Declarations

INT32 FAR PASCAL WindowProcedure(HWND hWindow, uint16_t Message, WPARAM wParam, LPARAM lParam);

int PASCAL HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                          int sCommandShow);

HINSTANCE ghInstance;

void ProcessJa2CommandLineBeforeInitialization(CHAR8 *pCommandLine);

// Global Variable Declarations
#ifdef WINDOWED_MODE
RECT rcWindow;
#endif

uint32_t guiMouseWheelMsg;  // For mouse wheel messages

BOOLEAN gfApplicationActive;
BOOLEAN gfProgramIsRunning;
BOOLEAN gfGameInitialized = FALSE;

// There were TWO of them??!?! -- DB
// CHAR8		gzCommandLine[ 100 ];
CHAR8 gzCommandLine[100];  // Command line given

BOOLEAN gfIgnoreMessages = FALSE;

INT32 FAR PASCAL WindowProcedure(HWND hWindow, uint16_t Message, WPARAM wParam, LPARAM lParam) {
  static BOOLEAN fRestore = FALSE;

  if (gfIgnoreMessages) return (DefWindowProc(hWindow, Message, wParam, lParam));

  // ATE: This is for older win95 or NT 3.51 to get MOUSE_WHEEL Messages
  if (Message == guiMouseWheelMsg) {
    QueueEvent(MOUSE_WHEEL, wParam, lParam);
    return (0L);
  }

  switch (Message) {
    case WM_MOUSEWHEEL: {
      QueueEvent(MOUSE_WHEEL, wParam, lParam);
      break;
    }

#ifdef WINDOWED_MODE
    case WM_MOVE:

      GetClientRect(hWindow, &rcWindow);
      ClientToScreen(hWindow, (LPPOINT)&rcWindow);
      ClientToScreen(hWindow, (LPPOINT)&rcWindow + 1);
      break;
#endif

    case WM_ACTIVATEAPP:
      switch (wParam) {
        case TRUE:  // We are restarting DirectDraw
          if (fRestore == TRUE) {
            RestoreVideoManager();
            RestoreVideoSurfaces();  // Restore any video surfaces

            // unpause the JA2 Global clock
            if (!gfPauseDueToPlayerGamePause) {
              PauseTime(FALSE);
            }

            gfApplicationActive = TRUE;
          }
          break;
        case FALSE:  // We are suspending direct draw
                     // pause the JA2 Global clock
          PauseTime(TRUE);
          SuspendVideoManager();

          gfApplicationActive = FALSE;
          fRestore = TRUE;
          break;
      }
      break;

    case WM_CREATE:
      break;

    case WM_DESTROY:
      ShutdownStandardGamingPlatform();
      ShowCursor(TRUE);
      PostQuitMessage(0);
      break;

    case WM_SETFOCUS:
      ReapplyCursorClipRect();
      break;

    case WM_KILLFOCUS:
      // Set a flag to restore surfaces once a WM_ACTIVEATEAPP is received
      fRestore = TRUE;
      break;

    case WM_DEVICECHANGE: {
    } break;

    default:
      return DefWindowProc(hWindow, Message, wParam, lParam);
  }
  return 0L;
}

void ShutdownStandardGamingPlatform(void) {
  SoundServiceStreams();

  if (gfGameInitialized) {
    ShutdownGame();
  }

  ShutdownButtonSystem();
  MSYS_Shutdown();

  ShutdownSoundManager();

  DestroyEnglishTransTable();  // has to go before ShutdownFontManager()
  ShutdownFontManager();

  ShutdownClockManager();  // must shutdown before VideoManager, 'cause it uses ghWindow

#ifdef SGP_VIDEO_DEBUGGING
  PerformVideoInfoDumpIntoFile("SGPVideoShutdownDump.txt", FALSE);
#endif

  ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

  ShutdownInputManager();
  ShutdownContainers();
  FileMan_Shutdown();

  ShutdownMemoryManager();  // must go last (except for Debug), for MemDebugCounter to work right...

  //
  // Make sure we unregister the last remaining debug topic before shutting
  // down the debugging layer
  UnRegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

#ifdef SGP_DEBUG
  ShutdownDebugManager();
#endif
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                   int sCommandShow) {
  MSG Message;
  HWND hPrevInstanceWindow;

  // Make sure that only one instance of this application is running at once
  // // Look for prev instance by searching for the window
  hPrevInstanceWindow = FindWindowEx(NULL, NULL, APPLICATION_NAME, APPLICATION_NAME);

  // One is found, bring it up!
  if (hPrevInstanceWindow != NULL) {
    SetForegroundWindow(hPrevInstanceWindow);
    ShowWindow(hPrevInstanceWindow, SW_RESTORE);
    return (0);
  }

  ghInstance = hInstance;

  // Copy commandline!
  strcopy(gzCommandLine, ARR_SIZE(gzCommandLine), pCommandLine);

  // Process the command line BEFORE initialization
  ProcessJa2CommandLineBeforeInitialization(pCommandLine);

  ShowCursor(FALSE);

  // Inititialize the SGP
  struct PlatformInitParams params = {hInstance, (uint16_t)sCommandShow, (void *)WindowProcedure,
                                      IDI_ICON1};
  if (InitializeStandardGamingPlatform(&params) == FALSE) {
    return 0;
  }

  // Register mouse wheel message
  guiMouseWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

  gfGameInitialized = TRUE;

#ifdef ENGLISH
  SetIntroType(INTRO_SPLASH);
#endif

  gfApplicationActive = TRUE;
  gfProgramIsRunning = TRUE;

  FastDebugMsg("Running Game");

  // At this point the SGP is set up, which means all I/O, Memory, tools, etc... are available. All
  // we need to do is attend to the gaming mechanics themselves
  while (gfProgramIsRunning) {
    if (PeekMessage(&Message, NULL, 0, 0,
                    PM_NOREMOVE)) {  // We have a message on the WIN95 queue, let's get it
      if (!GetMessage(&Message, NULL, 0, 0)) {  // It's quitting time
        return Message.wParam;
      }
      // Ok, now that we have the message, let's handle it
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    } else {  // Windows hasn't processed any messages, therefore we handle the rest
      if (gfApplicationActive ==
          FALSE) {  // Well we got nothing to do but to wait for a message to activate
        WaitMessage();
      } else {  // Well, the game is active, so we handle the game stuff
        GameLoop();

        // After this frame, reset input given flag
        gfSGPInputReceived = FALSE;
      }
    }
  }

  // This is the normal exit point
  FastDebugMsg("Exiting Game");
  PostQuitMessage(0);

  // SGPExit() will be called next through the atexit() mechanism...  This way we correctly process
  // both normal exits and emergency aborts (such as those caused by a failed assertion).

  // return wParam of the last message received
  return Message.wParam;
}

void ProcessJa2CommandLineBeforeInitialization(CHAR8 *pCommandLine) {
  CHAR8 cSeparators[] = "\t =";
  CHAR8 *pCopy = NULL, *pToken;

  pCopy = (CHAR8 *)MemAlloc(strlen(pCommandLine) + 1);

  Assert(pCopy);
  if (!pCopy) return;

  memcpy(pCopy, pCommandLine, strlen(pCommandLine) + 1);

  pToken = strtok(pCopy, cSeparators);
  while (pToken) {
    // if its the NO SOUND option
    if (!strncasecmp(pToken, "/NOSOUND", 8)) {
      // disable the sound
      SoundEnableSound(FALSE);
    }

    // get the next token
    pToken = strtok(NULL, cSeparators);
  }

  MemFree(pCopy);
}
