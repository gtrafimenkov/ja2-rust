#include "SGP/SGP.h"

#include "BuildDefines.h"
#include "GameLoop.h"
#include "Globals.h"
#include "JA2Splash.h"
#include "Laptop/Laptop.h"
#include "Local.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "platform.h"
#include "rust_fileman.h"

void SGPExit(void);

BOOLEAN InitializeStandardGamingPlatform(struct PlatformInitParams *params) {
  FontTranslationTable *pFontTable;

  // now required by all (even JA2) in order to call ShutdownSGP
  atexit(SGPExit);

  // Initialize the Debug Manager - success doesn't matter
#ifdef SGP_DEBUG
  InitializeDebugManager();
#endif

  // Now start up everything else.
  RegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

  // this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
  FastDebugMsg("Initializing Memory Manager");
  // Initialize the Memory Manager
  if (InitializeMemoryManager() == FALSE) {  // We were unable to initialize the memory manager
    FastDebugMsg("FAILED : Initializing Memory Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Containers Manager");
  InitializeContainers();

  FastDebugMsg("Initializing Input Manager");
  // Initialize the Input Manager
  if (InitializeInputManager() == FALSE) {  // We were unable to initialize the input manager
    FastDebugMsg("FAILED : Initializing Input Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Video Manager");
  // Initialize DirectDraw (DirectX 2)
  if (InitializeVideoManager(params) == FALSE) {
    FastDebugMsg("FAILED : Initializing Video Manager");
    return FALSE;
  }

  // Initialize Video Object Manager
  FastDebugMsg("Initializing Video Object Manager");
  if (!InitializeVideoObjectManager()) {
    FastDebugMsg("FAILED : Initializing Video Object Manager");
    return FALSE;
  }

  // Initialize Video Surface Manager
  FastDebugMsg("Initializing Video Surface Manager");
  if (!InitializeVideoSurfaceManager()) {
    FastDebugMsg("FAILED : Initializing Video Surface Manager");
    return FALSE;
  }

  InitJA2SplashScreen();

  // Make sure we start up our local clock (in milliseconds)
  // We don't need to check for a return value here since so far its always TRUE
  InitializeClockManager();  // must initialize after VideoManager, 'cause it uses ghWindow

  // Create font translation table (store in temp structure)
  pFontTable = CreateEnglishTransTable();
  if (pFontTable == NULL) {
    return (FALSE);
  }

  // Initialize Font Manager
  FastDebugMsg("Initializing the Font Manager");
  // Init the manager and copy the TransTable stuff into it.
  if (!InitializeFontManager(8, pFontTable)) {
    FastDebugMsg("FAILED : Initializing Font Manager");
    return FALSE;
  }
  // Don't need this thing anymore, so get rid of it (but don't de-alloc the contents)
  MemFree(pFontTable);

  FastDebugMsg("Initializing Sound Manager");
  // Initialize the Sound Manager (DirectSound)
  if (InitializeSoundManager() == FALSE) {  // We were unable to initialize the sound manager
    FastDebugMsg("FAILED : Initializing Sound Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Random");
  // Initialize random number generator
  InitializeRandom();  // no Shutdown

  FastDebugMsg("Initializing Game Manager");
  // Initialize the Game
  if (InitializeGame() == FALSE) {  // We were unable to initialize the game
    FastDebugMsg("FAILED : Initializing Game Manager");
    return FALSE;
  }

  return TRUE;
}

void SGPExit(void) {
  static BOOLEAN fAlreadyExiting = FALSE;

  // helps prevent heap crashes when multiple assertions occur and call us
  if (fAlreadyExiting) {
    return;
  }

  fAlreadyExiting = TRUE;
  gfProgramIsRunning = FALSE;

  ShutdownStandardGamingPlatform();
  Plat_OnSGPExit();
}
