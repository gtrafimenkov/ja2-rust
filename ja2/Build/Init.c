#include <stdio.h>
#include <windows.h>

#include "Editor/EditScreen.h"
#include "Editor/SummaryInfo.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/MouseSystem.h"
#include "SGP/SGP.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Screens.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreen.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "SysGlobals.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/WorldDef.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/MapUtility.h"
#include "Utils/MercTextBox.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "platfrom_strings.h"

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game

#ifdef JA2BETAVERSION
extern BOOLEAN gfUseConsecutiveQuickSaveSlots;
#endif

extern HINSTANCE ghInstance;

UINT32 InitializeJA2(void) {
  HandleJA2CDCheck();

  gfWorldLoaded = FALSE;

  // Load external text
  LoadAllExternalText();

  // Init JA2 sounds
  InitJA2Sound();

  gsRenderCenterX = 805;
  gsRenderCenterY = 805;

  // Init data
  InitializeSystemVideoObjects();

  // Init animation system
  if (!InitAnimationSystem()) {
    return (ERROR_SCREEN);
  }

  // Init lighting system
  InitLightingSystem();

  // Init dialog queue system
  InitalizeDialogueControl();

  if (!InitStrategicEngine()) {
    return (ERROR_SCREEN);
  }

  // needs to be called here to init the SectorInfo struct
  InitStrategicMovementCosts();

  // Init tactical engine
  if (!InitTacticalEngine()) {
    return (ERROR_SCREEN);
  }

  // Init timer system
  // Moved to the splash screen code.
  // InitializeJA2Clock( );

  // INit shade tables
  BuildShadeTable();

  // INit intensity tables
  BuildIntensityTable();

  // Init Event Manager
  if (!InitializeEventManager()) {
    return (ERROR_SCREEN);
  }

  // Initailize World
  if (!InitializeWorld()) {
    return (ERROR_SCREEN);
  }

  InitTileCache();

  InitMercPopupBox();

  // Set global volume
  MusicSetVolume(gGameSettings.ubMusicVolumeSetting);

  DetermineRGBDistributionSettings();

#ifdef JA2BETAVERSION
#ifdef JA2EDITOR

  // UNCOMMENT NEXT LINE TO ALLOW FORCE UPDATES...
  // LoadGlobalSummary();
  if (gfMustForceUpdateAllMaps) {
    ApologizeOverrideAndForceUpdateEverything();
  }
#endif
#endif

#ifdef JA2BETAVERSION
  // CHECK COMMANDLINE FOR SPECIAL UTILITY
  if (strcmp(gzCommandLine, "-DOMAPS") == 0) {
    return (MAPUTILITY_SCREEN);
  }
#endif

#ifdef JA2BETAVERSION
  // This allows the QuickSave Slots to be autoincremented, ie everytime the user saves, there will
  // be a new quick save file
  if (strcasecmp(gzCommandLine, "-quicksave") == 0) {
    gfUseConsecutiveQuickSaveSlots = TRUE;
  }
#endif

#ifdef JA2BETAVERSION
#ifdef JA2EDITOR
  // CHECK COMMANDLINE FOR SPECIAL UTILITY
  if (!strcmp(gzCommandLine, "-EDITORAUTO")) {
    DebugPrint("Beginning JA2 using -EDITORAUTO commandline argument...\n");
    // For editor purposes, need to know the default map file.
    sprintf(gubFilename, "none");
    // also set the sector
    gWorldSectorX = 0;
    gWorldSectorY = 0;
    gfAutoLoadA9 = TRUE;
    gfIntendOnEnteringEditor = TRUE;
    gGameOptions.fGunNut = TRUE;
    return (GAME_SCREEN);
  }
  if (strcmp(gzCommandLine, "-EDITOR") == 0) {
    DebugPrint("Beginning JA2 using -EDITOR commandline argument...\n");
    // For editor purposes, need to know the default map file.
    sprintf(gubFilename, "none");
    // also set the sector
    gWorldSectorX = 0;
    gWorldSectorY = 0;
    gfAutoLoadA9 = FALSE;
    gfIntendOnEnteringEditor = TRUE;
    gGameOptions.fGunNut = TRUE;
    return (GAME_SCREEN);
  }
#endif
#endif

  return (INIT_SCREEN);
}

void ShutdownJA2(void) {
  UINT32 uiIndex;

  // Clear screen....
  ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  InvalidateScreen();
  // Remove cursor....
  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  RefreshScreen(NULL);

  ShutdownStrategicLayer();

  // remove temp files built by laptop
  ClearOutTempLaptopFiles();

  // Shutdown queue system
  ShutdownDialogueControl();

  // Shutdown Screens
  for (uiIndex = 0; uiIndex < MAX_SCREENS; uiIndex++) {
    (*(GameScreens[uiIndex].ShutdownScreen))();
  }

  // Shutdown animation system
  DeInitAnimationSystem();

  ShutdownLightingSystem();

  CursorDatabaseClear();

  ShutdownTacticalEngine();

  // Shutdown Overhead
  ShutdownOverhead();

  DeinitializeWorld();

  DeleteTileCache();

  ShutdownJA2Clock();

  ShutdownFonts();

  ShutdownJA2Sound();

  ShutdownEventManager();

  ShutdownBaseDirtyRectQueue();

  // Unload any text box images!
  RemoveTextMercPopupImages();

  ClearOutVehicleList();
}
