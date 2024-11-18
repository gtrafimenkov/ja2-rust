#include "SaveLoadGame.h"

#include <stdio.h>
#include <string.h>

#include "Cheats.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "HelpScreen.h"
#include "JAScreens.h"
#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRMailOrder.h"
#include "Laptop/Email.h"
#include "Laptop/Files.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/IMPConfirm.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/Laptop.h"
#include "Laptop/Mercs.h"
#include "LoadingScreen.h"
#include "OptionsScreen.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/MercContract.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Scheduling.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AnimationData.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/Boxing.h"
#include "Tactical/Bullets.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/EnemySoldierSave.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Menptr.h"
#include "Tactical/MercHiring.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/TeamTurns.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/LightEffects.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SmokeEffects.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "UI.h"
#include "Utils/AnimatedProgressBar.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "platform.h"
#include "rust_civ_groups.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_laptop.h"
#include "rust_sam_sites.h"

BOOLEAN fFirstTimeInMapScreen = TRUE;

/////////////////////////////////////////////////////
//
// Local Defines
//
/////////////////////////////////////////////////////

void GetBestPossibleSectorXYZValues(u8 *psSectorX, u8 *psSectorY, int8_t *pbSectorZ);
extern void NextLoopCheckForEnoughFreeHardDriveSpace();
extern void UpdatePersistantGroupsFromOldSave(uint32_t uiSavedGameVersion);
extern void TrashAllSoldiers();

extern void BeginLoadScreen();
extern void EndLoadScreen();

// Global variable used
#ifdef JA2BETAVERSION
uint32_t guiNumberOfMapTempFiles;  // Test purposes
uint32_t guiSizeOfTempFiles;
char gzNameOfMapTempFile[128];
#endif

extern struct SOLDIERTYPE *gpSMCurrentMerc;
extern int32_t giSortStateForMapScreenList;
extern int16_t sDeadMercs[NUMBER_OF_SQUADS][NUMBER_OF_SOLDIERS_PER_SQUAD];
extern int32_t giRTAILastUpdateTime;
extern BOOLEAN gfRedrawSaveLoadScreen;
extern uint8_t gubScreenCount;
extern BOOLEAN gfLoadedGame;
extern HELP_SCREEN_STRUCT gHelpScreen;
extern uint8_t gubDesertTemperature;
extern uint8_t gubGlobalTemperature;
extern BOOLEAN gfCreatureMeanwhileScenePlayed;
#ifdef JA2BETAVERSION
extern uint8_t gubReportMapscreenLock;
#endif

BOOLEAN gMusicModeToPlay = FALSE;

#ifdef JA2BETAVERSION
BOOLEAN gfDisplaySaveGamesNowInvalidatedMsg = FALSE;
#endif

BOOLEAN gfUseConsecutiveQuickSaveSlots = FALSE;
uint32_t guiCurrentQuickSaveNumber = 0;
uint32_t guiLastSaveGameNum;
BOOLEAN DoesAutoSaveFileExist(BOOLEAN fLatestAutoSave);

uint32_t guiJA2EncryptionSet = 0;
uint32_t CalcJA2EncryptionSet(SAVED_GAME_HEADER *pSaveGameHeader);

typedef struct {
  // The screen that the gaem was saved from
  uint32_t uiCurrentScreen;

  uint32_t uiCurrentUniqueSoldierId;

  // The music that was playing when the game was saved
  uint8_t ubMusicMode;

  // Flag indicating that we have purchased something from Tony
  BOOLEAN fHavePurchasedItemsFromTony;

  // The selected soldier in tactical
  uint16_t usSelectedSoldier;

  // The x and y scroll position
  int16_t sRenderCenterX;
  int16_t sRenderCenterY;

  BOOLEAN fAtLeastOneMercWasHired;

  // General Map screen state flags
  BOOLEAN fShowItemsFlag;
  BOOLEAN fShowTownFlag;
  BOOLEAN fShowTeamFlag;
  BOOLEAN fShowMineFlag;
  BOOLEAN fShowAircraftFlag;

  // is the helicopter available to player?
  BOOLEAN fHelicopterAvailable;

  // helicopter vehicle id
  int32_t iHelicopterVehicleId;

  // total distance travelled
  int32_t UNUSEDiTotalHeliDistanceSinceRefuel;

  // total owed to player
  int32_t iTotalAccumulatedCostByPlayer;

  // whether or not skyrider is alive and well? and on our side yet?
  BOOLEAN fSkyRiderAvailable;

  // skyrider engaging in a monologue
  BOOLEAN UNUSEDfSkyriderMonologue;

  // list of sector locations
  int16_t UNUSED[2][2];

  // is the heli in the air?
  BOOLEAN fHelicopterIsAirBorne;

  // is the pilot returning straight to base?
  BOOLEAN fHeliReturnStraightToBase;

  // heli hovering
  BOOLEAN fHoveringHelicopter;

  // time started hovering
  uint32_t uiStartHoverTime;

  // what state is skyrider's dialogue in in?
  uint32_t uiHelicopterSkyriderTalkState;

  // the flags for skyrider events
  BOOLEAN fShowEstoniRefuelHighLight;
  BOOLEAN fShowOtherSAMHighLight;
  BOOLEAN fShowDrassenSAMHighLight;

  uint32_t uiEnvWeather;

  uint8_t ubDefaultButton;

  BOOLEAN fSkyriderEmptyHelpGiven;
  BOOLEAN fEnterMapDueToContract;
  uint8_t ubHelicopterHitsTaken;
  uint8_t ubQuitType;
  BOOLEAN fSkyriderSaidCongratsOnTakingSAM;
  int16_t sContractRehireSoldierID;

  GAME_OPTIONS GameOptions;

  uint32_t uiSeedNumber;

  // The GetJA2Clock() value
  uint32_t uiBaseJA2Clock;

  int16_t sCurInterfacePanel;

  uint8_t ubSMCurrentMercID;

  BOOLEAN fFirstTimeInMapScreen;

  BOOLEAN fDisableDueToBattleRoster;

  BOOLEAN fDisableMapInterfaceDueToBattle;

  int16_t sBoxerGridNo[NUM_BOXERS];
  uint8_t ubBoxerID[NUM_BOXERS];
  BOOLEAN fBoxerFought[NUM_BOXERS];

  BOOLEAN fHelicopterDestroyed;    // if the chopper is destroyed
  BOOLEAN fShowMapScreenHelpText;  // If true, displays help in mapscreen

  int32_t iSortStateForMapScreenList;
  BOOLEAN fFoundTixa;

  uint32_t uiTimeOfLastSkyriderMonologue;
  BOOLEAN fShowCambriaHospitalHighLight;
  BOOLEAN fSkyRiderSetUp;
  BOOLEAN fRefuelingSiteAvailable[NUMBER_OF_REFUEL_SITES];

  // Meanwhile stuff
  MEANWHILE_DEFINITION gCurrentMeanwhileDef;

  BOOLEAN ubPlayerProgressSkyriderLastCommentedOn;

  BOOLEAN gfMeanwhileTryingToStart;
  BOOLEAN gfInMeanwhile;

  // list of dead guys for squads...in id values -> -1 means no one home
  int16_t sDeadMercs[NUMBER_OF_SQUADS][NUMBER_OF_SOLDIERS_PER_SQUAD];

  // levels of publicly known noises
  int8_t gbPublicNoiseLevel[MAXTEAMS];

  uint8_t gubScreenCount;

  uint16_t usOldMeanWhileFlags;

  int32_t iPortraitNumber;

  int16_t sWorldSectorLocationOfFirstBattle;

  BOOLEAN fUnReadMailFlag;
  BOOLEAN fNewMailFlag;
  BOOLEAN fOldUnReadFlag;
  BOOLEAN fOldNewMailFlag;

  BOOLEAN fShowMilitia;

  BOOLEAN fNewFilesInFileViewer;

  BOOLEAN fLastBoxingMatchWonByPlayer;

  uint32_t uiUNUSED;

  BOOLEAN __only_storage_fSamSiteFound[4];

  uint8_t ubNumTerrorists;
  uint8_t ubCambriaMedicalObjects;

  BOOLEAN fDisableTacticalPanelButtons;

  int16_t sSelMapX;
  int16_t sSelMapY;
  int32_t iCurrentMapSectorZ;

  uint16_t usHasPlayerSeenHelpScreenInCurrentScreen;
  BOOLEAN fHideHelpInAllScreens;
  uint8_t ubBoxingMatchesWon;

  uint8_t ubBoxersRests;
  BOOLEAN fBoxersResting;
  uint8_t ubDesertTemperature;
  uint8_t ubGlobalTemperature;

  int16_t sMercArriveSectorX;
  int16_t sMercArriveSectorY;

  BOOLEAN fCreatureMeanwhileScenePlayed;
  uint8_t ubPlayerNum;
  // New stuff for the Prebattle interface / autoresolve
  BOOLEAN fPersistantPBI;
  uint8_t ubEnemyEncounterCode;

  BOOLEAN ubExplicitEnemyEncounterCode;
  BOOLEAN fBlitBattleSectorLocator;
  uint8_t ubPBSectorX;
  uint8_t ubPBSectorY;

  uint8_t ubPBSectorZ;
  BOOLEAN fCantRetreatInPBI;
  BOOLEAN fExplosionQueueActive;
  uint8_t ubUnused[1];

  uint32_t uiMeanWhileFlags;

  int8_t bSelectedInfoChar;
  int8_t bHospitalPriceModifier;
  int8_t bUnused2[2];

  int32_t iHospitalTempBalance;
  int32_t iHospitalRefund;

  int8_t fPlayerTeamSawJoey;
  int8_t fMikeShouldSayHi;

  uint8_t ubFiller[550];  // This structure should be 1024 bytes

} GENERAL_SAVE_INFO;

uint32_t guiSaveGameVersion = 0;

/////////////////////////////////////////////////////
//
// Global Variables
//
/////////////////////////////////////////////////////

// char		gsSaveGameNameWithPath[ 512 ];

uint8_t gubSaveGameLoc = 0;

uint32_t guiScreenToGotoAfterLoadingSavedGame = 0;

extern EmailPtr pEmailList;
extern uint32_t guiCurrentUniqueSoldierId;
extern BOOLEAN gfHavePurchasedItemsFromTony;

/////////////////////////////////////////////////////
//
// Function Prototypes
//
/////////////////////////////////////////////////////

BOOLEAN SaveMercProfiles(FileID hFile);
BOOLEAN LoadSavedMercProfiles(FileID hwFile);

BOOLEAN SaveSoldierStructure(FileID hFile);
BOOLEAN LoadSoldierStructure(FileID hFile);

// BOOLEAN		SavePtrInfo( PTR *pData, uint32_t uiSizeOfObject, FileID hFile );
// BOOLEAN		LoadPtrInfo( PTR *pData, uint32_t uiSizeOfObject, FileID hFile );

BOOLEAN SaveEmailToSavedGame(FileID hFile);
BOOLEAN LoadEmailFromSavedGame(FileID hFile);

BOOLEAN SaveTacticalStatusToSavedGame(FileID hFile);
BOOLEAN LoadTacticalStatusFromSavedGame(FileID hFile);

BOOLEAN SetMercsInsertionGridNo();

BOOLEAN LoadOppListInfoFromSavedGame(FileID hFile);
BOOLEAN SaveOppListInfoToSavedGame(FileID hFile);

BOOLEAN LoadMercPathToSoldierStruct(FileID hFilem, uint8_t ubID);
BOOLEAN SaveMercPathFromSoldierStruct(FileID hFilem, uint8_t ubID);

BOOLEAN LoadGeneralInfo(FileID hFile);
BOOLEAN SaveGeneralInfo(FileID hFile);
BOOLEAN SavePreRandomNumbersToSaveGameFile(FileID hFile);
BOOLEAN LoadPreRandomNumbersFromSaveGameFile(FileID hFile);

BOOLEAN SaveWatchedLocsToSavedGame(FileID hFile);
BOOLEAN LoadWatchedLocsFromSavedGame(FileID hFile);

BOOLEAN LoadMeanwhileDefsFromSaveGameFile(FileID hFile);
BOOLEAN SaveMeanwhileDefsFromSaveGameFile(FileID hFile);

void PauseBeforeSaveGame(void);
void UnPauseAfterSaveGame(void);
void UpdateMercMercContractInfo();
void HandleOldBobbyRMailOrders();

#ifdef JA2BETAVERSION
extern BOOLEAN ValidateSoldierInitLinks(uint8_t ubCode);
#endif

/////////////////////////////////////////////////////
//
// Functions
//
/////////////////////////////////////////////////////

BOOLEAN SaveGame(uint8_t ubSaveGameID, wchar_t* pGameDesc, size_t bufSize) {
  uint32_t uiNumBytesWritten = 0;
  FileID hFile = FILE_ID_ERR;
  SAVED_GAME_HEADER SaveGameHeader;
  char zSaveGameName[512];
  char saveDir[100];
  BOOLEAN fPausedStateBeforeSaving = IsGamePaused();
  BOOLEAN fLockPauseStateBeforeSaving = IsPauseLocked();
  int32_t iSaveLoadGameMessageBoxID = -1;
  uint16_t usPosX, usActualWidth, usActualHeight;
  BOOLEAN fWePausedIt = FALSE;

  sprintf(saveDir, "%S", pMessageStrings[MSG_SAVEDIRECTORY]);

  if (ubSaveGameID >= NUM_SAVE_GAMES && ubSaveGameID != SAVE__ERROR_NUM &&
      ubSaveGameID != SAVE__END_TURN_NUM)
    return (FALSE);  // ddd

  // clear out the save game header
  memset(&SaveGameHeader, 0, sizeof(SAVED_GAME_HEADER));

  if (!IsGamePaused()) {
    PauseBeforeSaveGame();
    fWePausedIt = TRUE;
  }

  // Place a message on the screen telling the user that we are saving the game
  iSaveLoadGameMessageBoxID = PrepareMercPopupBox(
      iSaveLoadGameMessageBoxID, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER,
      zSaveLoadText[SLG_SAVING_GAME_MESSAGE], 300, 0, 0, 0, &usActualWidth, &usActualHeight);
  usPosX = (640 - usActualWidth) / 2;

  RenderMercPopUpBoxFromIndex(iSaveLoadGameMessageBoxID, usPosX, 160, FRAME_BUFFER);

  InvalidateRegion(0, 0, 640, 480);

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();
  RefreshScreen();

  if (RemoveMercPopupBoxFromIndex(iSaveLoadGameMessageBoxID)) {
    iSaveLoadGameMessageBoxID = -1;
  }

  //
  // make sure we redraw the screen when we are done
  //

  // if we are in the game screen
  if (IsTacticalMode()) {
    SetRenderFlags(RENDER_FLAG_FULL);
  }

  else if (IsMapScreen_2()) {
    SetMapPanelDirty(true);
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }

  else if (guiCurrentScreen == SAVE_LOAD_SCREEN) {
    gfRedrawSaveLoadScreen = TRUE;
  }

  gubSaveGameLoc = ubSaveGameID;

  // Set the fact that we are saving a game
  gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

  // Save the current sectors open temp files to the disk
  if (!SaveCurrentSectorsInformationToTempItemFile()) {
    ScreenMsg(FONT_MCOLOR_WHITE, MSG_TESTVERSION,
              L"ERROR in SaveCurrentSectorsInformationToTempItemFile()");
    goto FAILED_TO_SAVE;
  }

  // if we are saving the quick save,
  if (ubSaveGameID == 0) {
#ifdef JA2BETAVERSION
    // Increment the quicksave counter
    guiCurrentQuickSaveNumber++;

    if (gfUseConsecutiveQuickSaveSlots)
      swprintf(pGameDesc, bufSize, L"%s%03d", pMessageStrings[MSG_QUICKSAVE_NAME],
               guiCurrentQuickSaveNumber);
    else
#endif
      swprintf(pGameDesc, bufSize, pMessageStrings[MSG_QUICKSAVE_NAME]);
  }

  // If there was no string, add one
  if (pGameDesc[0] == '\0') wcscpy(pGameDesc, pMessageStrings[MSG_NODESC]);

  // Check to see if the save directory exists
  if (!Plat_DirectoryExists(saveDir)) {
    // ok the direcotry doesnt exist, create it
    if (!Plat_CreateDirectory(saveDir)) {
      goto FAILED_TO_SAVE;
    }
  }

  // Create the name of the file
  CreateSavedGameFileNameFromNumber(ubSaveGameID, zSaveGameName);

  // if the file already exists, delete it
  if (File_Exists(zSaveGameName)) {
    if (!Plat_DeleteFile(zSaveGameName)) {
      goto FAILED_TO_SAVE;
    }
  }

  // create the save game file
  hFile = File_OpenForWriting(zSaveGameName);
  if (!hFile) {
    goto FAILED_TO_SAVE;
  }

  //
  // If there are no enemy or civilians to save, we have to check BEFORE savinf the sector info
  // struct because the NewWayOfSavingEnemyAndCivliansToTempFile will RESET the civ or enemy flag
  // AFTER they have been saved.
  //
  NewWayOfSavingEnemyAndCivliansToTempFile((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ,
                                           TRUE, TRUE);
  NewWayOfSavingEnemyAndCivliansToTempFile((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ,
                                           FALSE, TRUE);

  //
  // Setup the save game header
  //

  SaveGameHeader.uiSavedGameVersion = guiSavedGameVersion;
  wcscpy(SaveGameHeader.sSavedGameDesc, pGameDesc);
  strcpy(SaveGameHeader.zGameVersionNumber, czVersionNumber);

  // The following will be used to quickly access info to display in the save/load screen
  SaveGameHeader.uiDay = GetGameTimeInDays();
  SaveGameHeader.ubHour = (uint8_t)GetGameClockHour();
  SaveGameHeader.ubMin = (uint8_t)GetGameClockMinutes();

  // copy over the initial game options
  memcpy(&SaveGameHeader.sInitialGameOptions, &gGameOptions, sizeof(GAME_OPTIONS));

  // Get the sector value to save.
  {
    u8 mapX, mapY;
    GetBestPossibleSectorXYZValues(&mapX, &mapY, &SaveGameHeader.bSectorZ);
    SaveGameHeader.sSectorX = mapX;
    SaveGameHeader.sSectorY = mapY;
  }

  SaveGameHeader.ubNumOfMercsOnPlayersTeam = NumberOfMercsOnPlayerTeam();
  SaveGameHeader.iCurrentBalance = LaptopMoneyGetBalance();

  SaveGameHeader.uiCurrentScreen = guiPreviousOptionScreen;

  SaveGameHeader.fAlternateSector = GetSectorFlagStatus((u8)gWorldSectorX, (u8)gWorldSectorY,
                                                        gbWorldSectorZ, SF_USE_ALTERNATE_MAP);

  if (gfWorldLoaded) {
    SaveGameHeader.fWorldLoaded = TRUE;
    SaveGameHeader.ubLoadScreenID =
        GetLoadScreenID((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
  } else {
    SaveGameHeader.fWorldLoaded = FALSE;
    SaveGameHeader.ubLoadScreenID = 0;
  }

  SaveGameHeader.uiRandom = Random(RAND_MAX);

  //
  // Save the Save Game header file
  //

  File_Write(hFile, &SaveGameHeader, sizeof(SAVED_GAME_HEADER), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(SAVED_GAME_HEADER)) {
    goto FAILED_TO_SAVE;
  }

  guiJA2EncryptionSet = CalcJA2EncryptionSet(&SaveGameHeader);

  //
  // Save the gTactical Status array, plus the curent secotr location
  //
  if (!SaveTacticalStatusToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // save the game clock info
  if (!SaveGameClock(hFile, fPausedStateBeforeSaving, fLockPauseStateBeforeSaving)) {
    goto FAILED_TO_SAVE;
  }

  // save the strategic events
  if (!SaveStrategicEventsToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveLaptopInfoToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  //
  // Save the merc profiles
  //
  if (!SaveMercProfiles(hFile)) {
    goto FAILED_TO_SAVE;
  }

  //
  // Save the soldier structure
  //
  if (!SaveSoldierStructure(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save the Finaces Data file
  if (!SaveFilesToSavedGame(FINANCES_DATA_FILE, hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save the history file
  if (!SaveFilesToSavedGame(HISTORY_DATA_FILE, hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save the Laptop File file
  if (!SaveFilesToSavedGame(FILES_DAT_FILE, hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save email stuff to save file
  if (!SaveEmailToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save the strategic information
  if (!SaveStrategicInfoToSavedFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // save the underground information
  if (!SaveUnderGroundSectorInfoToSaveGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // save the squad info
  if (!SaveSquadInfoToSavedGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveStrategicMovementGroupsToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save all the map temp files from the maps\temp directory into the saved game file
  if (!SaveMapTempFilesToSavedGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveQuestInfoToSavedGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveOppListInfoToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveMapScreenMessagesToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveNPCInfoToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveKeyTableToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveTempNpcQuoteArrayToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SavePreRandomNumbersToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveSmokeEffectsToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveArmsDealerInventoryToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveGeneralInfo(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveMineStatusToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveStrategicTownLoyaltyToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveVehicleInformationToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveBulletStructureToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SavePhysicsTableToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveAirRaidInfoToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveTeamTurnsToTheSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveExplosionTableToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveCreatureDirectives(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveStrategicStatusToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveStrategicAI(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveLightEffectsToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveWatchedLocsToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveItemCursorToSavedGame(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveCivQuotesToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveBackupNPCInfoToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  if (!SaveMeanwhileDefsFromSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // save meanwhiledefs

  if (!SaveSchedules(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save extra vehicle info
  if (!NewSaveVehicleMovementInfoToSavedGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save contract renewal sequence stuff
  if (!SaveContractRenewalDataToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Save leave list stuff
  if (!SaveLeaveItemList(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // do the new way of saving bobbyr mail order items
  if (!NewWayOfSavingBobbyRMailOrdersToSaveGameFile(hFile)) {
    goto FAILED_TO_SAVE;
  }

  // Close the saved game file
  File_Close(hFile);

  // if we succesfully saved the game, mark this entry as the last saved game file
  if (ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM) {
    gGameSettings.bLastSavedGameSlot = ubSaveGameID;
  }

  // Save the save game settings
  SaveGameSettings();

  //
  // Display a screen message that the save was succesful
  //

  // if its the quick save slot
  if (ubSaveGameID == 0) {
    ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[MSG_SAVESUCCESS]);
  }
  // #ifdef JA2BETAVERSION
  else if (ubSaveGameID == SAVE__END_TURN_NUM) {
    //		ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_END_TURN_AUTO_SAVE
    //]
    //);
  }
  // #endif
  else {
    ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[MSG_SAVESLOTSUCCESS]);
  }

  // restore the music mode
  SetMusicMode(gubMusicMode);

  // Unset the fact that we are saving a game
  gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

  UnPauseAfterSaveGame();

#ifdef JA2BETAVERSION
  ValidateSoldierInitLinks(2);
#endif

  // Check for enough free hard drive space
  NextLoopCheckForEnoughFreeHardDriveSpace();

  return (TRUE);

  // if there is an error saving the game
FAILED_TO_SAVE:

  File_Close(hFile);

  if (fWePausedIt) {
    UnPauseAfterSaveGame();
  }

  // Delete the failed attempt at saving
  DeleteSaveGameNumber(ubSaveGameID);

  // Put out an error message
  ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, zSaveLoadText[SLG_SAVE_GAME_ERROR]);

  // Check for enough free hard drive space
  NextLoopCheckForEnoughFreeHardDriveSpace();

#ifdef JA2BETAVERSION
  if (fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle) {
    gubReportMapscreenLock = 2;
  }
#endif

  return (FALSE);
}

BOOLEAN LoadSavedGame(uint8_t ubSavedGameID) {
  FileID hFile = FILE_ID_ERR;
  SAVED_GAME_HEADER SaveGameHeader;
  uint32_t uiNumBytesRead = 0;

  int16_t sLoadSectorX;
  int16_t sLoadSectorY;
  int8_t bLoadSectorZ;
  char zSaveGameName[512];
  uint32_t uiRelStartPerc;
  uint32_t uiRelEndPerc;

#ifdef JA2BETAVERSION
  gfDisplaySaveGamesNowInvalidatedMsg = FALSE;
#endif

  uiRelStartPerc = uiRelEndPerc = 0;

  TrashAllSoldiers();

  // Empty the dialogue Queue cause someone could still have a quote in waiting
  EmptyDialogueQueue();

  // If there is someone talking, stop them
  StopAnyCurrentlyTalkingSpeech();

  ZeroAnimSurfaceCounts();

  ShutdownNPCQuotes();

  // is it a valid save number
  if (ubSavedGameID >= NUM_SAVE_GAMES) {
    if (ubSavedGameID != SAVE__END_TURN_NUM) return (FALSE);
  } else if (!gbSaveGameArray[ubSavedGameID])
    return (FALSE);

  // Used in mapescreen to disable the annoying 'swoosh' transitions
  gfDontStartTransitionFromLaptop = TRUE;

  // Reset timer callbacks
  gpCustomizableTimerCallback = NULL;

  gubSaveGameLoc = ubSavedGameID;

  // Set the fact that we are loading a saved game
  gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

  // Trash the existing world.  This is done to ensure that if we are loading a game that doesn't
  // have a world loaded, that we trash it beforehand -- else the player could theoretically enter
  // that sector where it would be in a pre-load state.
  TrashWorld();

  // Deletes all the Temp files in the Maps\Temp directory
  InitTacticalSave(TRUE);

  // ATE; Added to empry dialogue q
  EmptyDialogueQueue();

  // Create the name of the file
  CreateSavedGameFileNameFromNumber(ubSavedGameID, zSaveGameName);

  // open the save game file
  hFile = File_OpenForReading(zSaveGameName);
  if (!hFile) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  // Load the Save Game header file
  File_Read(hFile, &SaveGameHeader, sizeof(SAVED_GAME_HEADER), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(SAVED_GAME_HEADER)) {
    File_Close(hFile);
    return (FALSE);
  }

  guiJA2EncryptionSet = CalcJA2EncryptionSet(&SaveGameHeader);

  // Store the loading screenID that was saved
  gubLastLoadingScreenID = SaveGameHeader.ubLoadScreenID;

  // HACK
  guiSaveGameVersion = SaveGameHeader.uiSavedGameVersion;

  // Load the gtactical status structure plus the current sector x,y,z
  if (!LoadTacticalStatusFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  // This gets reset by the above function
  gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

  // Load the game clock ingo
  if (!LoadGameClock(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  // if we are suppose to use the alternate sector
  if (SaveGameHeader.fAlternateSector) {
    SetSectorFlag((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ, SF_USE_ALTERNATE_MAP);
    gfUseAlternateMap = TRUE;
  }

  // if the world was loaded when saved, reload it, otherwise dont
  if (SaveGameHeader.fWorldLoaded) {
    // Get the current world sector coordinates
    sLoadSectorX = gWorldSectorX;
    sLoadSectorY = gWorldSectorY;
    bLoadSectorZ = gbWorldSectorZ;

    // This will guarantee that the sector will be loaded
    gbWorldSectorZ = -1;

    // if we should load a sector ( if the person didnt just start the game game )
    if ((gWorldSectorX != 0) && (gWorldSectorY != 0)) {
      // Load the sector
      SetCurrentWorldSector((u8)sLoadSectorX, (u8)sLoadSectorY, bLoadSectorZ);
    }
  } else {  // By clearing these values, we can avoid "in sector" checks -- at least, that's the
            // theory.
    gWorldSectorX = gWorldSectorY = 0;

    // Since there is no
    if (SaveGameHeader.sSectorX == -1 || SaveGameHeader.sSectorY == -1 ||
        SaveGameHeader.bSectorZ == -1)
      gubLastLoadingScreenID = LOADINGSCREEN_HELI;
    else
      gubLastLoadingScreenID = GetLoadScreenID(
          (u8)SaveGameHeader.sSectorX, (u8)SaveGameHeader.sSectorY, SaveGameHeader.bSectorZ);

    BeginLoadScreen();
  }

  CreateLoadingScreenProgressBar();
  SetProgressBarColor(0, 0, 0, 150);

#ifdef JA2BETAVERSION
  // set the font
  SetProgressBarMsgAttributes(0, FONT12ARIAL, FONT_MCOLOR_WHITE, 0);

  //
  // Set the tile so we don see the text come up
  //

  // if the world is unloaded, we must use the save buffer for the text
  if (SaveGameHeader.fWorldLoaded)
    SetProgressBarTextDisplayFlag(0, TRUE, TRUE, FALSE);
  else
    SetProgressBarTextDisplayFlag(0, TRUE, TRUE, TRUE);
#endif

  uiRelStartPerc = 0;

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Events...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // load the game events
  if (!LoadStrategicEventsFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Laptop Info");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadLaptopInfoFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Merc Profiles...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  //
  // Load all the saved Merc profiles
  //
  if (!LoadSavedMercProfiles(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 30;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Soldier Structure...");
  uiRelStartPerc = uiRelEndPerc;

  //
  // Load the soldier structure info
  //
  if (!LoadSoldierStructure(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Finances Data File...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  //
  // Load the Finances Data and write it to a new file
  //
  if (!LoadFilesFromSavedGame(FINANCES_DATA_FILE, hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"History File...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  //
  // Load the History Data and write it to a new file
  //
  if (!LoadFilesFromSavedGame(HISTORY_DATA_FILE, hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"The Laptop FILES file...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  //
  // Load the Files Data and write it to a new file
  //
  if (!LoadFilesFromSavedGame(FILES_DAT_FILE, hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Email...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Load the data for the emails
  if (!LoadEmailFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Information...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Load the strategic Information
  if (!LoadStrategicInfoFromSavedFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"UnderGround Information...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Load the underground information
  if (!LoadUnderGroundSectorInfoFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Squad Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Load all the squad info from the saved game file
  if (!LoadSquadInfoFromSavedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc,
                                   L"Strategic Movement Groups...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Load the group linked list
  if (!LoadStrategicMovementGroupsFromSavedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 30;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"All the Map Temp files...");
  uiRelStartPerc = uiRelEndPerc;

  // Load all the map temp files from the saved game file into the maps\temp directory
  if (!LoadMapTempFilesFromSavedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Quest Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadQuestInfoFromSavedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"OppList Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadOppListInfoFromSavedGame(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"MapScreen Messages...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadMapScreenMessagesFromSaveGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"NPC Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadNPCInfoFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"KeyTable...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadKeyTableFromSaveedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Npc Temp Quote File...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadTempNpcQuoteArrayToSaveGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc,
                                   L"PreGenerated Random Files...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadPreRandomNumbersFromSaveGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Smoke Effect Structures...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadSmokeEffectsFromLoadGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Arms Dealers Inventory...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadArmsDealerInventoryFromSavedGameFile(
          hFile, (BOOLEAN)(SaveGameHeader.uiSavedGameVersion >= 54),
          (BOOLEAN)(SaveGameHeader.uiSavedGameVersion >= 55))) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Misc info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadGeneralInfo(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Mine Status...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (!LoadMineStatusFromSavedGameFile(hFile)) {
    File_Close(hFile);
    guiSaveGameVersion = 0;
    return (FALSE);
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Town Loyalty...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 21) {
    if (!LoadStrategicTownLoyaltyFromSavedGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Vehicle Information...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 22) {
    if (!LoadVehicleInformationFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Bullet Information...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 24) {
    if (!LoadBulletStructureFromSavedGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Physics table...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 24) {
    if (!LoadPhysicsTableFromSavedGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Air Raid Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 24) {
    if (!LoadAirRaidInfoFromSaveGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 0;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Team Turn Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 24) {
    if (!LoadTeamTurnsFromTheSavedGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Explosion Table...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 25) {
    if (!LoadExplosionTableFromSavedGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Creature Spreading...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 27) {
    if (!LoadCreatureDirectives(hFile, SaveGameHeader.uiSavedGameVersion)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Status...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 28) {
    if (!LoadStrategicStatusFromSaveGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic AI...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 31) {
    if (!LoadStrategicAI(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Lighting Effects...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 37) {
    if (!LoadLightEffectsFromLoadGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Watched Locs Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 38) {
    if (!LoadWatchedLocsFromSavedGame(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Item cursor Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 39) {
    if (!LoadItemCursorFromSavedGame(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Civ Quote System...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 51) {
    if (!LoadCivQuotesFromLoadGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return FALSE;
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Backed up NPC Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 53) {
    if (!LoadBackupNPCInfoFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Meanwhile definitions...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 58) {
    if (!LoadMeanwhileDefsFromSaveGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  } else {
    memcpy(&gMeanwhileDef[gCurrentMeanwhileDef.ubMeanwhileID], &gCurrentMeanwhileDef,
           sizeof(MEANWHILE_DEFINITION));
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Schedules...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 59) {
    // trash schedules loaded from map
    DestroyAllSchedulesWithoutDestroyingEvents();
    if (!LoadSchedulesFromSave(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Extra Vehicle Info...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 61) {
    {
      if (!NewLoadVehicleMovementInfoFromSavedGameFile(hFile)) {
        File_Close(hFile);
        guiSaveGameVersion = 0;
        return (FALSE);
      }
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc,
                                   L"Contract renweal sequence stuff...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  if (SaveGameHeader.uiSavedGameVersion >= 67) {
    if (!LoadContractRenewalDataFromSaveGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  if (SaveGameHeader.uiSavedGameVersion >= 70) {
    if (!LoadLeaveItemList(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  if (SaveGameHeader.uiSavedGameVersion >= 85) {
    if (!NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(hFile)) {
      File_Close(hFile);
      guiSaveGameVersion = 0;
      return (FALSE);
    }
  }

  /// lll

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  //
  // Close the saved game file
  //
  File_Close(hFile);

  // ATE: Patch? Patch up groups.....( will only do for old saves.. )
  UpdatePersistantGroupsFromOldSave(SaveGameHeader.uiSavedGameVersion);

  // if the world is loaded, apply the temp files to the loaded map
  if (SaveGameHeader.fWorldLoaded) {
    // Load the current sectors Information From the temporary files
    if (!LoadCurrentSectorsInformationFromTempItemsFile()) {
      InitExitGameDialogBecauseFileHackDetected();
      guiSaveGameVersion = 0;
      return (TRUE);
    }
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  InitAI();

  // Update the mercs in the sector with the new soldier info
  UpdateMercsInSector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);

  // ReconnectSchedules();
  PostSchedules();

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Reset the lighting level if we are outside
  if (gbWorldSectorZ == 0) LightSetBaseLevel(GetTimeOfDayAmbientLightLevel());

  // if we have been to this sector before
  //	if( SectorInfo[ GetSectorID8( gWorldSectorX,gWorldSectorY) ].uiFlags & SF_ALREADY_VISITED )
  {
    // Reset the fact that we are loading a saved game
    gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // if the UI was locked in the saved game file
  if (gTacticalStatus.ubAttackBusyCount > 1) {
    // Lock the ui
    SetUIBusy((uint8_t)gusSelectedSoldier);
  }

  // Reset the shadow
  SetFontShadow(DEFAULT_SHADOW);

  // if we succesfully LOADED! the game, mark this entry as the last saved game file
  gGameSettings.bLastSavedGameSlot = ubSavedGameID;

  // Save the save game settings
  SaveGameSettings();

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // Reset the Ai Timer clock
  giRTAILastUpdateTime = 0;

  // if we are in tactical
  if (guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN) {
    // Initialize the current panel
    InitializeCurrentPanel();

    SelectSoldier(gusSelectedSoldier, FALSE, TRUE);
  }

  uiRelEndPerc += 1;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
  RenderProgressBar(0, 100);
  uiRelStartPerc = uiRelEndPerc;

  // init extern faces
  InitalizeStaticExternalNPCFaces();

  // load portraits
  LoadCarPortraitValues();

  // OK, turn OFF show all enemies....
  gTacticalStatus.uiFlags &= (~SHOW_ALL_MERCS);
  gTacticalStatus.uiFlags &= ~SHOW_ALL_ITEMS;

  if ((gTacticalStatus.uiFlags & INCOMBAT)) {
    DebugMsg(TOPIC_JA2, DBG_INFO, String("Setting attack busy count to 0 from load"));
    gTacticalStatus.ubAttackBusyCount = 0;
  }

  gfLoadedGame = TRUE;

  uiRelEndPerc = 100;
  SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Done!");
  RenderProgressBar(0, 100);

  RemoveLoadingScreenProgressBar();

  SetMusicMode(gMusicModeToPlay);

#ifndef JA2TESTVERSION
  RESET_CHEAT_LEVEL();
#endif

#ifdef JA2BETAVERSION
  if (fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle) {
    gubReportMapscreenLock = 1;
  }
#endif

  // reset to 0
  guiSaveGameVersion = 0;

  // reset once-per-convo records for everyone in the loaded sector
  ResetOncePerConvoRecordsForAllNPCsInLoadedSector();

  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    // fix lingering attack busy count problem on loading saved game by resetting a.b.c
    // if we're not in combat.
    gTacticalStatus.ubAttackBusyCount = 0;
  }

  // fix squads
  CheckSquadMovementGroups();

  // The above function LightSetBaseLevel adjusts ALL the level node light values including the merc
  // node, we must reset the values
  HandlePlayerTogglingLightEffects(FALSE);

  return (TRUE);
}

BOOLEAN SaveMercProfiles(FileID hFile) {
  uint16_t cnt;
  uint32_t uiNumBytesWritten = 0;
  uint32_t uiSaveSize = sizeof(MERCPROFILESTRUCT);

  // Lopp through all the profiles to save
  for (cnt = 0; cnt < NUM_PROFILES; cnt++) {
    gMercProfiles[cnt].uiProfileChecksum = ProfileChecksum(&(gMercProfiles[cnt]));
    NewJA2EncryptedFileWrite(hFile, &gMercProfiles[cnt], uiSaveSize, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiSaveSize) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN LoadSavedMercProfiles(FileID hFile) {
  uint16_t cnt;
  uint32_t uiNumBytesRead = 0;

  // Lopp through all the profiles to Load
  for (cnt = 0; cnt < NUM_PROFILES; cnt++) {
    if (guiSaveGameVersion < 87) {
      JA2EncryptedFileRead(hFile, &gMercProfiles[cnt], sizeof(MERCPROFILESTRUCT), &uiNumBytesRead);
    } else {
      NewJA2EncryptedFileRead(hFile, &gMercProfiles[cnt], sizeof(MERCPROFILESTRUCT),
                              &uiNumBytesRead);
    }
    if (uiNumBytesRead != sizeof(MERCPROFILESTRUCT)) {
      return (FALSE);
    }
    if (gMercProfiles[cnt].uiProfileChecksum != ProfileChecksum(&(gMercProfiles[cnt]))) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN SaveSoldierStructure(FileID hFile) {
  uint16_t cnt;
  uint32_t uiNumBytesWritten = 0;
  uint8_t ubOne = 1;
  uint8_t ubZero = 0;

  uint32_t uiSaveSize = sizeof(struct SOLDIERTYPE);

  // Loop through all the soldier structs to save
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    // if the soldier isnt active, dont add them to the saved game file.
    if (!Menptr[cnt].bActive) {
      // Save the byte specifing to NOT load the soldiers
      File_Write(hFile, &ubZero, 1, &uiNumBytesWritten);
      if (uiNumBytesWritten != 1) {
        return (FALSE);
      }
    }

    else {
      // Save the byte specifing to load the soldiers
      File_Write(hFile, &ubOne, 1, &uiNumBytesWritten);
      if (uiNumBytesWritten != 1) {
        return (FALSE);
      }

      // calculate checksum for soldier
      Menptr[cnt].uiMercChecksum = MercChecksum(&(Menptr[cnt]));
      // Save the soldier structure
      NewJA2EncryptedFileWrite(hFile, GetSoldierByID(cnt), uiSaveSize, &uiNumBytesWritten);
      if (uiNumBytesWritten != uiSaveSize) {
        return (FALSE);
      }

      //
      // Save all the pointer info from the structure
      //

      // Save the pMercPath
      if (!SaveMercPathFromSoldierStruct(hFile, (uint8_t)cnt)) return (FALSE);

      //
      // do we have a 	KEY_ON_RING
      // *pKeyRing;
      //

      if (Menptr[cnt].pKeyRing != NULL) {
        // write to the file saying we have the ....
        File_Write(hFile, &ubOne, 1, &uiNumBytesWritten);
        if (uiNumBytesWritten != 1) {
          return (FALSE);
        }

        // Now save the ....
        File_Write(hFile, Menptr[cnt].pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING), &uiNumBytesWritten);
        if (uiNumBytesWritten != NUM_KEYS * sizeof(KEY_ON_RING)) {
          return (FALSE);
        }
      } else {
        // write to the file saying we DO NOT have the Key ring
        File_Write(hFile, &ubZero, 1, &uiNumBytesWritten);
        if (uiNumBytesWritten != 1) {
          return (FALSE);
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadSoldierStructure(FileID hFile) {
  uint16_t cnt;
  uint32_t uiNumBytesRead = 0;
  struct SOLDIERTYPE SavedSoldierInfo;
  uint32_t uiSaveSize = sizeof(struct SOLDIERTYPE);
  uint8_t ubId;
  uint8_t ubOne = 1;
  uint8_t ubActive = 1;
  uint32_t uiPercentage;

  SOLDIERCREATE_STRUCT CreateStruct;

  // Loop through all the soldier and delete them all
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    TacticalRemoveSoldier(cnt);
  }

  // Loop through all the soldier structs to load
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    // update the progress bar
    uiPercentage = (cnt * 100) / (TOTAL_SOLDIERS - 1);

    RenderProgressBar(0, uiPercentage);

    // Read in a byte to tell us whether or not there is a soldier loaded here.
    File_Read(hFile, &ubActive, 1, &uiNumBytesRead);
    if (uiNumBytesRead != 1) {
      return (FALSE);
    }

    // if the soldier is not active, continue
    if (!ubActive) {
      continue;
    }

    // else if there is a soldier
    else {
      // Read in the saved soldier info into a Temp structure
      if (guiSaveGameVersion < 87) {
        JA2EncryptedFileRead(hFile, &SavedSoldierInfo, uiSaveSize, &uiNumBytesRead);
      } else {
        NewJA2EncryptedFileRead(hFile, &SavedSoldierInfo, uiSaveSize, &uiNumBytesRead);
      }
      if (uiNumBytesRead != uiSaveSize) {
        return (FALSE);
      }
      // check checksum
      if (MercChecksum(&SavedSoldierInfo) != SavedSoldierInfo.uiMercChecksum) {
        return (FALSE);
      }

      // Make sure all the pointer references are NULL'ed out.
      SavedSoldierInfo.pTempObject = NULL;
      SavedSoldierInfo.pKeyRing = NULL;
      SavedSoldierInfo.p8BPPPalette = NULL;
      SavedSoldierInfo.p16BPPPalette = NULL;
      memset(SavedSoldierInfo.pShades, 0, sizeof(uint16_t *) * NUM_SOLDIER_SHADES);
      memset(SavedSoldierInfo.pGlowShades, 0, sizeof(uint16_t *) * 20);
      SavedSoldierInfo.pCurrentShade = NULL;
      SavedSoldierInfo.pThrowParams = NULL;
      SavedSoldierInfo.pLevelNode = NULL;
      SavedSoldierInfo.pExternShadowLevelNode = NULL;
      SavedSoldierInfo.pRoofUILevelNode = NULL;
      SavedSoldierInfo.pBackGround = NULL;
      SavedSoldierInfo.pZBackground = NULL;
      SavedSoldierInfo.pForcedShade = NULL;
      SavedSoldierInfo.pMercPath = NULL;
      memset(SavedSoldierInfo.pEffectShades, 0, sizeof(uint16_t *) * NUM_SOLDIER_EFFECTSHADES);

      // if the soldier wasnt active, dont add them now.  Advance to the next merc
      // if( !SavedSoldierInfo.bActive )
      //	continue;

      // Create the new merc
      memset(&CreateStruct, 0, sizeof(SOLDIERCREATE_STRUCT));
      CreateStruct.bTeam = SavedSoldierInfo.bTeam;
      CreateStruct.ubProfile = SavedSoldierInfo.ubProfile;
      CreateStruct.fUseExistingSoldier = TRUE;
      CreateStruct.pExistingSoldier = &SavedSoldierInfo;

      if (!TacticalCreateSoldier(&CreateStruct, &ubId)) return (FALSE);

      // Load the pMercPath
      if (!LoadMercPathToSoldierStruct(hFile, ubId)) return (FALSE);

      //
      // do we have a 	KEY_ON_RING
      // *pKeyRing;
      //

      // Read the file to see if we have to load the keys
      File_Read(hFile, &ubOne, 1, &uiNumBytesRead);
      if (uiNumBytesRead != 1) {
        return (FALSE);
      }

      if (ubOne) {
        // Now Load the ....
        File_Read(hFile, Menptr[cnt].pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING), &uiNumBytesRead);
        if (uiNumBytesRead != NUM_KEYS * sizeof(KEY_ON_RING)) {
          return (FALSE);
        }

      } else {
        Assert(Menptr[cnt].pKeyRing == NULL);
      }

      // if the soldier is an IMP character
      if (Menptr[cnt].ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER &&
          Menptr[cnt].bTeam == gbPlayerNum) {
        ResetIMPCharactersEyesAndMouthOffsets(Menptr[cnt].ubProfile);
      }

      // if the saved game version is before x, calculate the amount of money paid to mercs
      if (guiSaveGameVersion < 83) {
        // if the soldier is someone
        if (Menptr[cnt].ubProfile != NO_PROFILE) {
          if (Menptr[cnt].ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
            gMercProfiles[Menptr[cnt].ubProfile].uiTotalCostToDate =
                gMercProfiles[Menptr[cnt].ubProfile].sSalary *
                gMercProfiles[Menptr[cnt].ubProfile].iMercMercContractLength;
          } else {
            gMercProfiles[Menptr[cnt].ubProfile].uiTotalCostToDate =
                gMercProfiles[Menptr[cnt].ubProfile].sSalary * Menptr[cnt].iTotalContractLength;
          }
        }
      }

#ifdef GERMAN
      // Fix neutral flags
      if (guiSaveGameVersion < 94) {
        if (Menptr[cnt].bTeam == OUR_TEAM && Menptr[cnt].bNeutral &&
            Menptr[cnt].bAssignment != ASSIGNMENT_POW) {
          // turn off neutral flag
          Menptr[cnt].bNeutral = FALSE;
        }
      }
#endif
      // JA2Gold: fix next-to-previous attacker value
      if (guiSaveGameVersion < 99) {
        Menptr[cnt].ubNextToPreviousAttackerID = NOBODY;
      }
    }
  }

  // Fix robot
  if (guiSaveGameVersion <= 87) {
    struct SOLDIERTYPE *pSoldier;

    if (gMercProfiles[ROBOT].inv[VESTPOS] == SPECTRA_VEST) {
      // update this
      gMercProfiles[ROBOT].inv[VESTPOS] = SPECTRA_VEST_18;
      gMercProfiles[ROBOT].inv[HELMETPOS] = SPECTRA_HELMET_18;
      gMercProfiles[ROBOT].inv[LEGPOS] = SPECTRA_LEGGINGS_18;
      gMercProfiles[ROBOT].bAgility = 50;
      pSoldier = FindSoldierByProfileID(ROBOT, FALSE);
      if (pSoldier) {
        pSoldier->inv[VESTPOS].usItem = SPECTRA_VEST_18;
        pSoldier->inv[HELMETPOS].usItem = SPECTRA_HELMET_18;
        pSoldier->inv[LEGPOS].usItem = SPECTRA_LEGGINGS_18;
        pSoldier->bAgility = 50;
      }
    }
  }

  return (TRUE);
}

/*
BOOLEAN SavePtrInfo( PTR *pData, uint32_t uiSizeOfObject, FileID hFile )
{
        uint8_t		ubOne = 1;
        uint8_t		ubZero = 0;
        uint32_t	uiNumBytesWritten;

        if( pData != NULL )
        {
                // write to the file saying we have the ....
                File_Write( hFile, &ubOne, 1, &uiNumBytesWritten );
                if( uiNumBytesWritten != 1 )
                {
                        DebugMsg( TOPIC_JA2, DBG_INFO, String("FAILED to Write Soldier Structure
to File" ) ); return(FALSE);
                }

                // Now save the ....
                File_Write( hFile, pData, uiSizeOfObject, &uiNumBytesWritten );
                if( uiNumBytesWritten != uiSizeOfObject )
                {
                        DebugMsg( TOPIC_JA2, DBG_INFO, String("FAILED to Write Soldier Structure
to File" ) ); return(FALSE);
                }
        }
        else
        {
                // write to the file saying we DO NOT have the ...
                File_Write( hFile, &ubZero, 1, &uiNumBytesWritten );
                if( uiNumBytesWritten != 1 )
                {
                        DebugMsg( TOPIC_JA2, DBG_INFO, String("FAILED to Write Soldier Structure
to File" ) ); return(FALSE);
                }
        }

        return( TRUE );
}



BOOLEAN LoadPtrInfo( PTR *pData, uint32_t uiSizeOfObject, FileID hFile )
{
        uint8_t		ubOne = 1;
        uint8_t		ubZero = 0;
        uint32_t	uiNumBytesRead;

        // Read the file to see if we have to load the ....
        File_Read( hFile, &ubOne, 1, &uiNumBytesRead );
        if( uiNumBytesRead != 1 )
        {
                DebugMsg( TOPIC_JA2, DBG_INFO, String("FAILED to Read Soldier Structure from
File" ) ); return(FALSE);
        }

        if( ubOne )
        {
                // if there is memory already allocated, free it
                MemFree( pData );

                //Allocate space for the structure data
                *pData = MemAlloc( uiSizeOfObject );
                if( pData == NULL )
                        return( FALSE );

                // Now Load the ....
                File_Read( hFile, pData, uiSizeOfObject, &uiNumBytesRead );
                if( uiNumBytesRead != uiSizeOfObject )
                {
                        DebugMsg( TOPIC_JA2, DBG_INFO, String("FAILED to Write Soldier Structure
to File" ) ); return(FALSE);
                }
        }
        else
        {
                // if there is memory already allocated, free it
                if( pData )
                {
                        MemFree( pData );
                        pData = NULL;
                }
        }


        return( TRUE );
}
*/

BOOLEAN SaveFilesToSavedGame(char* pSrcFileName, FileID hFile) {
  uint32_t uiFileSize;
  uint32_t uiNumBytesWritten = 0;
  FileID hSrcFile = FILE_ID_ERR;
  uint8_t *pData;
  uint32_t uiNumBytesRead;

  // open the file
  hSrcFile = File_OpenForReading(pSrcFileName);
  if (!hSrcFile) {
    return (FALSE);
  }

#ifdef JA2BETAVERSION
  guiNumberOfMapTempFiles++;  // Increment counter:  To determine where the temp files are crashing
#endif

  // Get the file size of the source data file
  uiFileSize = File_GetSize(hSrcFile);
  if (uiFileSize == 0) return (FALSE);

  // Write the the size of the file to the saved game file
  File_Write(hFile, &uiFileSize, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  // Allocate a buffer to read the data into
  pData = (uint8_t *)MemAlloc(uiFileSize);
  if (pData == NULL) return (FALSE);
  memset(pData, 0, uiFileSize);

  // Read the saource file into the buffer
  File_Read(hSrcFile, pData, uiFileSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiFileSize) {
    // Free the buffer
    MemFree(pData);

    return (FALSE);
  }

  // Write the buffer to the saved game file
  File_Write(hFile, pData, uiFileSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiFileSize) {
    // Free the buffer
    MemFree(pData);

    return (FALSE);
  }

  // Free the buffer
  MemFree(pData);

  // Clsoe the source data file
  File_Close(hSrcFile);

  return (TRUE);
}

BOOLEAN LoadFilesFromSavedGame(char* pSrcFileName, FileID hFile) {
  uint32_t uiFileSize;
  uint32_t uiNumBytesWritten = 0;
  FileID hSrcFile = FILE_ID_ERR;
  uint8_t *pData;
  uint32_t uiNumBytesRead;

  // If the source file exists, delete it
  if (File_Exists(pSrcFileName)) {
    if (!Plat_DeleteFile(pSrcFileName)) {
      // unable to delete the original file
      return (FALSE);
    }
  }

#ifdef JA2BETAVERSION
  guiNumberOfMapTempFiles++;  // Increment counter:  To determine where the temp files are crashing
#endif

  // open the destination file to write to
  hSrcFile = File_OpenForWriting(pSrcFileName);
  if (!hSrcFile) {
    // error, we cant open the saved game file
    return (FALSE);
  }

  // Read the size of the data
  File_Read(hFile, &uiFileSize, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    File_Close(hSrcFile);

    return (FALSE);
  }

  // if there is nothing in the file, return;
  if (uiFileSize == 0) {
    File_Close(hSrcFile);
    return (TRUE);
  }

  // Allocate a buffer to read the data into
  pData = (uint8_t *)MemAlloc(uiFileSize);
  if (pData == NULL) {
    File_Close(hSrcFile);
    return (FALSE);
  }

  // Read into the buffer
  File_Read(hFile, pData, uiFileSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiFileSize) {
    File_Close(hSrcFile);

    // Free the buffer
    MemFree(pData);

    return (FALSE);
  }

  // Write the buffer to the new file
  File_Write(hSrcFile, pData, uiFileSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiFileSize) {
    File_Close(hSrcFile);
    DebugMsg(TOPIC_JA2, DBG_INFO, String("FAILED to Write to the %s File", pSrcFileName));
    // Free the buffer
    MemFree(pData);

    return (FALSE);
  }

  // Free the buffer
  MemFree(pData);

  // Close the source data file
  File_Close(hSrcFile);

  return (TRUE);
}

BOOLEAN SaveEmailToSavedGame(FileID hFile) {
  uint32_t uiNumOfEmails = 0;
  EmailPtr pEmail = pEmailList;
  uint32_t cnt;
  uint32_t uiStringLength = 0;
  uint32_t uiNumBytesWritten = 0;

  SavedEmailStruct SavedEmail;

  // loop through all the email to find out the total number
  while (pEmail) {
    pEmail = pEmail->Next;
    uiNumOfEmails++;
  }

  // write the number of email messages
  File_Write(hFile, &uiNumOfEmails, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  // loop trhough all the emails, add each one individually
  pEmail = pEmailList;
  for (cnt = 0; cnt < uiNumOfEmails; cnt++) {
    // Get the strng length of the subject
    uiStringLength = (wcslen(pEmail->pSubject) + 1) * 2;

    // write the length of the current emails subject to the saved game file
    File_Write(hFile, &uiStringLength, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) {
      return (FALSE);
    }

    // write the subject of the current email to the saved game file
    File_Write(hFile, pEmail->pSubject, uiStringLength, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiStringLength) {
      return (FALSE);
    }

    // Get the current emails data and asign it to the 'Saved email' struct
    SavedEmail.usOffset = pEmail->usOffset;
    SavedEmail.usLength = pEmail->usLength;
    SavedEmail.ubSender = pEmail->ubSender;
    SavedEmail.iDate = pEmail->iDate;
    SavedEmail.iId = pEmail->iId;
    SavedEmail.iFirstData = pEmail->iFirstData;
    SavedEmail.uiSecondData = pEmail->uiSecondData;
    SavedEmail.fRead = pEmail->fRead;
    SavedEmail.fNew = pEmail->fNew;
    SavedEmail.iThirdData = pEmail->iThirdData;
    SavedEmail.iFourthData = pEmail->iFourthData;
    SavedEmail.uiFifthData = pEmail->uiFifthData;
    SavedEmail.uiSixData = pEmail->uiSixData;

    // write the email header to the saved game file
    File_Write(hFile, &SavedEmail, sizeof(SavedEmailStruct), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(SavedEmailStruct)) {
      return (FALSE);
    }

    // advance to the next email
    pEmail = pEmail->Next;
  }

  return (TRUE);
}

BOOLEAN LoadEmailFromSavedGame(FileID hFile) {
  uint32_t uiNumOfEmails = 0;
  uint32_t uiSizeOfSubject = 0;
  EmailPtr pEmail = pEmailList;
  EmailPtr pTempEmail = NULL;
  uint8_t *pData = NULL;
  uint32_t cnt;
  SavedEmailStruct SavedEmail;
  uint32_t uiNumBytesRead = 0;

  // Delete the existing list of emails
  ShutDownEmailList();

  pEmailList = NULL;
  // Allocate memory for the header node
  pEmailList = (EmailPtr)MemAlloc(sizeof(Email));
  if (pEmailList == NULL) return (FALSE);

  memset(pEmailList, 0, sizeof(Email));

  // read in the number of email messages
  File_Read(hFile, &uiNumOfEmails, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  // loop through all the emails, add each one individually
  pEmail = pEmailList;
  for (cnt = 0; cnt < uiNumOfEmails; cnt++) {
    // get the length of the email subject
    File_Read(hFile, &uiSizeOfSubject, sizeof(uint32_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint32_t)) {
      return (FALSE);
    }

    // allocate space for the subject
    pData = (uint8_t *)MemAlloc(EMAIL_SUBJECT_LENGTH * sizeof(wchar_t));
    if (pData == NULL) return (FALSE);
    memset(pData, 0, EMAIL_SUBJECT_LENGTH * sizeof(wchar_t));

    // Get the subject
    File_Read(hFile, pData, uiSizeOfSubject, &uiNumBytesRead);
    if (uiNumBytesRead != uiSizeOfSubject) {
      return (FALSE);
    }

    // get the rest of the data from the email
    File_Read(hFile, &SavedEmail, sizeof(SavedEmailStruct), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(SavedEmailStruct)) {
      return (FALSE);
    }

    //
    // add the email
    //

    // if we havent allocated space yet
    pTempEmail = (EmailPtr)MemAlloc(sizeof(Email));
    if (pTempEmail == NULL) return (FALSE);
    memset(pTempEmail, 0, sizeof(Email));

    pTempEmail->usOffset = SavedEmail.usOffset;
    pTempEmail->usLength = SavedEmail.usLength;
    pTempEmail->ubSender = SavedEmail.ubSender;
    pTempEmail->iDate = SavedEmail.iDate;
    pTempEmail->iId = SavedEmail.iId;
    pTempEmail->fRead = SavedEmail.fRead;
    pTempEmail->fNew = SavedEmail.fNew;
    pTempEmail->pSubject = (wchar_t*)pData;
    pTempEmail->iFirstData = SavedEmail.iFirstData;
    pTempEmail->uiSecondData = SavedEmail.uiSecondData;
    pTempEmail->iThirdData = SavedEmail.iThirdData;
    pTempEmail->iFourthData = SavedEmail.iFourthData;
    pTempEmail->uiFifthData = SavedEmail.uiFifthData;
    pTempEmail->uiSixData = SavedEmail.uiSixData;

    // add the current email in
    pEmail->Next = pTempEmail;
    pTempEmail->Prev = pEmail;

    // moved to the next email
    pEmail = pEmail->Next;

    AddMessageToPages(pTempEmail->iId);
  }

  // if there are emails
  if (cnt) {
    // the first node of the LL was a dummy, node,get rid  of it
    pTempEmail = pEmailList;
    pEmailList = pEmailList->Next;
    pEmailList->Prev = NULL;
    MemFree(pTempEmail);
  } else {
    MemFree(pEmailList);
    pEmailList = NULL;
  }

  return (TRUE);
}

BOOLEAN SaveTacticalStatusToSavedGame(FileID hFile) {
  uint32_t uiNumBytesWritten;

  // copy data
  for (int i = FIRST_CIV_GROUP; i < ARR_SIZE(gTacticalStatus.__only_storage_fCivGroupHostile);
       i++) {
    gTacticalStatus.__only_storage_fCivGroupHostile[i] = GetCivGroupHostility(i);
  }

  // write the gTacticalStatus to the saved game file
  File_Write(hFile, &gTacticalStatus, sizeof(TacticalStatusType), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(TacticalStatusType)) {
    return (FALSE);
  }

  //
  // Save the current sector location to the saved game file
  //

  // save gWorldSectorX
  File_Write(hFile, &gWorldSectorX, sizeof(gWorldSectorX), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(gWorldSectorX)) {
    return (FALSE);
  }

  // save gWorldSectorY
  File_Write(hFile, &gWorldSectorY, sizeof(gWorldSectorY), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(gWorldSectorY)) {
    return (FALSE);
  }

  // save gbWorldSectorZ
  File_Write(hFile, &gbWorldSectorZ, sizeof(gbWorldSectorZ), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(gbWorldSectorZ)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadTacticalStatusFromSavedGame(FileID hFile) {
  uint32_t uiNumBytesRead;

  // Read the gTacticalStatus to the saved game file
  File_Read(hFile, &gTacticalStatus, sizeof(TacticalStatusType), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(TacticalStatusType)) {
    return (FALSE);
  }

  // copy data
  for (int i = FIRST_CIV_GROUP; i < ARR_SIZE(gTacticalStatus.__only_storage_fCivGroupHostile);
       i++) {
    SetCivGroupHostility(i, gTacticalStatus.__only_storage_fCivGroupHostile[i]);
  }

  //
  // Load the current sector location to the saved game file
  //

  // Load gWorldSectorX
  File_Read(hFile, &gWorldSectorX, sizeof(gWorldSectorX), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(gWorldSectorX)) {
    return (FALSE);
  }

  // Load gWorldSectorY
  File_Read(hFile, &gWorldSectorY, sizeof(gWorldSectorY), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(gWorldSectorY)) {
    return (FALSE);
  }

  // Load gbWorldSectorZ
  File_Read(hFile, &gbWorldSectorZ, sizeof(gbWorldSectorZ), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(gbWorldSectorZ)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN CopySavedSoldierInfoToNewSoldier(struct SOLDIERTYPE *pDestSourceInfo,
                                         struct SOLDIERTYPE *pSourceInfo) {
  // Copy the old soldier information over to the new structure
  memcpy(pDestSourceInfo, pSourceInfo, sizeof(struct SOLDIERTYPE));

  return (TRUE);
}

BOOLEAN SetMercsInsertionGridNo() {
  uint16_t cnt = 0;

  // loop through all the mercs
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    // if the soldier is active
    if (Menptr[cnt].bActive) {
      if (Menptr[cnt].sGridNo != NOWHERE) {
        // set the insertion type to gridno
        Menptr[cnt].ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;

        // set the insertion gridno
        Menptr[cnt].usStrategicInsertionData = Menptr[cnt].sGridNo;

        // set the gridno
        Menptr[cnt].sGridNo = NOWHERE;
      }
    }
  }

  return (TRUE);
}

BOOLEAN SaveOppListInfoToSavedGame(FileID hFile) {
  uint32_t uiSaveSize = 0;
  uint32_t uiNumBytesWritten = 0;

  // Save the Public Opplist
  uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Write(hFile, gbPublicOpplist, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Seen Oppenents
  uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Write(hFile, gbSeenOpponents, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Last Known Opp Locations
  uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Write(hFile, gsLastKnownOppLoc, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Last Known Opp Level
  uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Write(hFile, gbLastKnownOppLevel, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Public Last Known Opp Locations
  uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Write(hFile, gsPublicLastKnownOppLoc, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Public Last Known Opp Level
  uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Write(hFile, gbPublicLastKnownOppLevel, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Public Noise Volume
  uiSaveSize = MAXTEAMS;
  File_Write(hFile, gubPublicNoiseVolume, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  // Save the Public Last Noise Gridno
  uiSaveSize = MAXTEAMS;
  File_Write(hFile, gsPublicNoiseGridno, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadOppListInfoFromSavedGame(FileID hFile) {
  uint32_t uiLoadSize = 0;
  uint32_t uiNumBytesRead = 0;

  // Load the Public Opplist
  uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Read(hFile, gbPublicOpplist, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Seen Oppenents
  uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Read(hFile, gbSeenOpponents, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Last Known Opp Locations
  uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Read(hFile, gsLastKnownOppLoc, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Last Known Opp Level
  uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
  File_Read(hFile, gbLastKnownOppLevel, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Public Last Known Opp Locations
  uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Read(hFile, gsPublicLastKnownOppLoc, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Public Last Known Opp Level
  uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
  File_Read(hFile, gbPublicLastKnownOppLevel, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Public Noise Volume
  uiLoadSize = MAXTEAMS;
  File_Read(hFile, gubPublicNoiseVolume, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  // Load the Public Last Noise Gridno
  uiLoadSize = MAXTEAMS;
  File_Read(hFile, gsPublicNoiseGridno, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN SaveWatchedLocsToSavedGame(FileID hFile) {
  uint32_t uiArraySize;
  uint32_t uiSaveSize = 0;
  uint32_t uiNumBytesWritten = 0;

  uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

  // save locations of watched points
  uiSaveSize = uiArraySize * sizeof(int16_t);
  File_Write(hFile, gsWatchedLoc, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  uiSaveSize = uiArraySize * sizeof(int8_t);

  File_Write(hFile, gbWatchedLocLevel, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  File_Write(hFile, gubWatchedLocPoints, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  File_Write(hFile, gfWatchedLocReset, uiSaveSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSaveSize) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadWatchedLocsFromSavedGame(FileID hFile) {
  uint32_t uiArraySize;
  uint32_t uiLoadSize = 0;
  uint32_t uiNumBytesRead = 0;

  uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

  uiLoadSize = uiArraySize * sizeof(int16_t);
  File_Read(hFile, gsWatchedLoc, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  uiLoadSize = uiArraySize * sizeof(int8_t);
  File_Read(hFile, gbWatchedLocLevel, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  File_Read(hFile, gubWatchedLocPoints, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  File_Read(hFile, gfWatchedLocReset, uiLoadSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiLoadSize) {
    return (FALSE);
  }

  return (TRUE);
}

void CreateSavedGameFileNameFromNumber(uint8_t ubSaveGameID, char* pzNewFileName) {
  // if we are creating the QuickSave file
  if (ubSaveGameID == 0) {
#ifdef JA2BETAVERSION
    // if the user wants to have consecutive quick saves
    if (gfUseConsecutiveQuickSaveSlots) {
      // if we are loading a game, and the user hasnt saved any consecutinve saves, load the defualt
      // save
      if (guiCurrentQuickSaveNumber == 0)
        sprintf(pzNewFileName, "%S\\%S.%S", pMessageStrings[MSG_SAVEDIRECTORY],
                pMessageStrings[MSG_QUICKSAVE_NAME], pMessageStrings[MSG_SAVEEXTENSION]);
      else
        sprintf(pzNewFileName, "%S\\%S%02d.%S", pMessageStrings[MSG_SAVEDIRECTORY],
                pMessageStrings[MSG_QUICKSAVE_NAME], guiCurrentQuickSaveNumber,
                pMessageStrings[MSG_SAVEEXTENSION]);
    } else
#endif
      sprintf(pzNewFileName, "%S\\%S.%S", pMessageStrings[MSG_SAVEDIRECTORY],
              pMessageStrings[MSG_QUICKSAVE_NAME], pMessageStrings[MSG_SAVEEXTENSION]);
  }
  // #ifdef JA2BETAVERSION
  else if (ubSaveGameID == SAVE__END_TURN_NUM) {
    // The name of the file
    sprintf(pzNewFileName, "%S\\Auto%02d.%S", pMessageStrings[MSG_SAVEDIRECTORY],
            guiLastSaveGameNum, pMessageStrings[MSG_SAVEEXTENSION]);

    // increment end turn number
    guiLastSaveGameNum++;

    // just have 2 saves
    if (guiLastSaveGameNum == 2) {
      guiLastSaveGameNum = 0;
    }
  }
  // #endif

  else
    sprintf(pzNewFileName, "%S\\%S%02d.%S", pMessageStrings[MSG_SAVEDIRECTORY],
            pMessageStrings[MSG_SAVE_NAME], ubSaveGameID, pMessageStrings[MSG_SAVEEXTENSION]);
}

BOOLEAN SaveMercPathFromSoldierStruct(FileID hFile, uint8_t ubID) {
  uint32_t uiNumOfNodes = 0;
  struct path *pTempPath = Menptr[ubID].pMercPath;
  uint32_t uiNumBytesWritten = 0;

  // loop through to get all the nodes
  while (pTempPath) {
    uiNumOfNodes++;
    pTempPath = pTempPath->pNext;
  }

  // Save the number of the nodes
  File_Write(hFile, &uiNumOfNodes, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  // loop through all the nodes and add them
  pTempPath = Menptr[ubID].pMercPath;

  // loop through nodes and save all the nodes
  while (pTempPath) {
    // Save the number of the nodes
    File_Write(hFile, pTempPath, sizeof(struct path), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(struct path)) {
      return (FALSE);
    }

    pTempPath = pTempPath->pNext;
  }

  return (TRUE);
}

BOOLEAN LoadMercPathToSoldierStruct(FileID hFile, uint8_t ubID) {
  uint32_t uiNumOfNodes = 0;
  struct path *pTempPath = NULL;
  struct path *pTemp = NULL;
  uint32_t uiNumBytesRead = 0;
  uint32_t cnt;

  // The list SHOULD be empty at this point
  /*
          //if there is nodes, loop through and delete them
          if( Menptr[ ubID ].pMercPath )
          {
                  pTempPath = Menptr[ ubID ].pMercPath;
                  while( pTempPath )
                  {
                          pTemp = pTempPath;
                          pTempPath = pTempPath->pNext;

                          MemFree( pTemp );
                          pTemp=NULL;
                  }

                  Menptr[ ubID ].pMercPath = NULL;
          }
  */

  // Load the number of the nodes
  File_Read(hFile, &uiNumOfNodes, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  // load all the nodes
  for (cnt = 0; cnt < uiNumOfNodes; cnt++) {
    // Allocate memory for the new node
    pTemp = (struct path *)MemAlloc(sizeof(struct path));
    if (pTemp == NULL) return (FALSE);
    memset(pTemp, 0, sizeof(struct path));

    // Load the node
    File_Read(hFile, pTemp, sizeof(struct path), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(struct path)) {
      return (FALSE);
    }

    // Put the node into the list
    if (cnt == 0) {
      pTempPath = pTemp;
      pTemp->pPrev = NULL;
    } else {
      pTempPath->pNext = pTemp;
      pTemp->pPrev = pTempPath;

      pTempPath = pTempPath->pNext;
    }

    pTemp->pNext = NULL;
  }

  // move to beginning of list
  pTempPath = MoveToBeginningOfPathList(pTempPath);

  Menptr[ubID].pMercPath = pTempPath;
  if (Menptr[ubID].pMercPath) Menptr[ubID].pMercPath->pPrev = NULL;

  return (TRUE);
}

extern struct SectorPoint locationOfFirstBattle;

BOOLEAN SaveGeneralInfo(FileID hFile) {
  uint32_t uiNumBytesWritten;

  GENERAL_SAVE_INFO sGeneralInfo;
  memset(&sGeneralInfo, 0, sizeof(GENERAL_SAVE_INFO));

  sGeneralInfo.ubMusicMode = gubMusicMode;
  sGeneralInfo.uiCurrentUniqueSoldierId = guiCurrentUniqueSoldierId;
  sGeneralInfo.uiCurrentScreen = guiPreviousOptionScreen;

  sGeneralInfo.usSelectedSoldier = gusSelectedSoldier;
  sGeneralInfo.sRenderCenterX = gsRenderCenterX;
  sGeneralInfo.sRenderCenterY = gsRenderCenterY;
  sGeneralInfo.fAtLeastOneMercWasHired = gfAtLeastOneMercWasHired;
  sGeneralInfo.fHavePurchasedItemsFromTony = gfHavePurchasedItemsFromTony;

  sGeneralInfo.fShowItemsFlag = fShowItemsFlag;
  sGeneralInfo.fShowTownFlag = fShowTownFlag;
  sGeneralInfo.fShowMineFlag = fShowMineFlag;
  sGeneralInfo.fShowAircraftFlag = fShowAircraftFlag;
  sGeneralInfo.fShowTeamFlag = fShowTeamFlag;

  sGeneralInfo.fHelicopterAvailable = fHelicopterAvailable;

  // helicopter vehicle id
  sGeneralInfo.iHelicopterVehicleId = iHelicopterVehicleId;

  // total distance travelled
  //	sGeneralInfo.iTotalHeliDistanceSinceRefuel = iTotalHeliDistanceSinceRefuel;

  // total owed by player
  sGeneralInfo.iTotalAccumulatedCostByPlayer = iTotalAccumulatedCostByPlayer;

  // whether or not skyrider is alive and well? and on our side yet?
  sGeneralInfo.fSkyRiderAvailable = fSkyRiderAvailable;

  // is the heli in the air?
  sGeneralInfo.fHelicopterIsAirBorne = fHelicopterIsAirBorne;

  // is the pilot returning straight to base?
  sGeneralInfo.fHeliReturnStraightToBase = fHeliReturnStraightToBase;

  // heli hovering
  sGeneralInfo.fHoveringHelicopter = fHoveringHelicopter;

  // time started hovering
  sGeneralInfo.uiStartHoverTime = uiStartHoverTime;

  // what state is skyrider's dialogue in in?
  sGeneralInfo.uiHelicopterSkyriderTalkState = guiHelicopterSkyriderTalkState;

  // the flags for skyrider events
  sGeneralInfo.fShowEstoniRefuelHighLight = fShowEstoniRefuelHighLight;
  sGeneralInfo.fShowOtherSAMHighLight = fShowOtherSAMHighLight;
  sGeneralInfo.fShowDrassenSAMHighLight = fShowDrassenSAMHighLight;
  sGeneralInfo.fShowCambriaHospitalHighLight = fShowCambriaHospitalHighLight;

  // The current state of the weather
  sGeneralInfo.uiEnvWeather = guiEnvWeather;

  sGeneralInfo.ubDefaultButton = gubDefaultButton;

  sGeneralInfo.fSkyriderEmptyHelpGiven = gfSkyriderEmptyHelpGiven;
  sGeneralInfo.ubHelicopterHitsTaken = gubHelicopterHitsTaken;
  sGeneralInfo.fSkyriderSaidCongratsOnTakingSAM = gfSkyriderSaidCongratsOnTakingSAM;
  sGeneralInfo.ubPlayerProgressSkyriderLastCommentedOn = gubPlayerProgressSkyriderLastCommentedOn;

  sGeneralInfo.fEnterMapDueToContract = fEnterMapDueToContract;
  sGeneralInfo.ubQuitType = ubQuitType;

  if (pContractReHireSoldier != NULL)
    sGeneralInfo.sContractRehireSoldierID = pContractReHireSoldier->ubID;
  else
    sGeneralInfo.sContractRehireSoldierID = -1;

  memcpy(&sGeneralInfo.GameOptions, &gGameOptions, sizeof(GAME_OPTIONS));

#ifdef JA2BETAVERSION
  // Everytime we save get, and set a seed value, when reload, seed again
  sGeneralInfo.uiSeedNumber = GetJA2Clock();
  srand(sGeneralInfo.uiSeedNumber);
#endif

  // Save the Ja2Clock()
  sGeneralInfo.uiBaseJA2Clock = GetJA2Clock();

  // Save the current tactical panel mode
  sGeneralInfo.sCurInterfacePanel = gsCurInterfacePanel;

  // Save the selected merc
  if (gpSMCurrentMerc)
    sGeneralInfo.ubSMCurrentMercID = gpSMCurrentMerc->ubID;
  else
    sGeneralInfo.ubSMCurrentMercID = 255;

  // Save the fact that it is the first time in mapscreen
  sGeneralInfo.fFirstTimeInMapScreen = fFirstTimeInMapScreen;

  // save map screen disabling buttons
  sGeneralInfo.fDisableDueToBattleRoster = fDisableDueToBattleRoster;
  sGeneralInfo.fDisableMapInterfaceDueToBattle = fDisableMapInterfaceDueToBattle;

  // Save boxing info
  memcpy(&sGeneralInfo.sBoxerGridNo, &gsBoxerGridNo, NUM_BOXERS * sizeof(int16_t));
  memcpy(&sGeneralInfo.ubBoxerID, &gubBoxerID, NUM_BOXERS * sizeof(int8_t));
  memcpy(&sGeneralInfo.fBoxerFought, &gfBoxerFought, NUM_BOXERS * sizeof(BOOLEAN));

  // Save the helicopter status
  sGeneralInfo.fHelicopterDestroyed = fHelicopterDestroyed;
  sGeneralInfo.fShowMapScreenHelpText = fShowMapScreenHelpText;

  sGeneralInfo.iSortStateForMapScreenList = giSortStateForMapScreenList;
  sGeneralInfo.fFoundTixa = fFoundTixa;

  sGeneralInfo.uiTimeOfLastSkyriderMonologue = guiTimeOfLastSkyriderMonologue;
  sGeneralInfo.fSkyRiderSetUp = fSkyRiderSetUp;

  memcpy(&sGeneralInfo.fRefuelingSiteAvailable, &fRefuelingSiteAvailable,
         NUMBER_OF_REFUEL_SITES * sizeof(BOOLEAN));

  // Meanwhile stuff
  memcpy(&sGeneralInfo.gCurrentMeanwhileDef, &gCurrentMeanwhileDef, sizeof(MEANWHILE_DEFINITION));
  // sGeneralInfo.gfMeanwhileScheduled = gfMeanwhileScheduled;
  sGeneralInfo.gfMeanwhileTryingToStart = gfMeanwhileTryingToStart;
  sGeneralInfo.gfInMeanwhile = gfInMeanwhile;

  // list of dead guys for squads...in id values -> -1 means no one home
  memcpy(&sGeneralInfo.sDeadMercs, &sDeadMercs,
         sizeof(int16_t) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD);

  // level of public noises
  memcpy(&sGeneralInfo.gbPublicNoiseLevel, &gbPublicNoiseLevel, sizeof(int8_t) * MAXTEAMS);

  // The screen count for the initscreen
  sGeneralInfo.gubScreenCount = gubScreenCount;

  // used for the mean while screen
  sGeneralInfo.uiMeanWhileFlags = uiMeanWhileFlags;

  // Imp portrait number
  sGeneralInfo.iPortraitNumber = iPortraitNumber;

  // location of first enocunter with enemy
  sGeneralInfo.sWorldSectorLocationOfFirstBattle =
      GetSectorID16(locationOfFirstBattle.x, locationOfFirstBattle.y);

  // State of email flags
  sGeneralInfo.fUnReadMailFlag = fUnReadMailFlag;
  sGeneralInfo.fNewMailFlag = fNewMailFlag;
  sGeneralInfo.fOldUnReadFlag = fOldUnreadFlag;
  sGeneralInfo.fOldNewMailFlag = fOldNewMailFlag;

  sGeneralInfo.fShowMilitia = fShowMilitia;

  sGeneralInfo.fNewFilesInFileViewer = fNewFilesInFileViewer;

  sGeneralInfo.fLastBoxingMatchWonByPlayer = gfLastBoxingMatchWonByPlayer;

  for (int i = 0; i < ARR_SIZE(sGeneralInfo.__only_storage_fSamSiteFound); i++) {
    sGeneralInfo.__only_storage_fSamSiteFound[i] = IsSamSiteFound(i);
  }

  sGeneralInfo.ubNumTerrorists = gubNumTerrorists;
  sGeneralInfo.ubCambriaMedicalObjects = gubCambriaMedicalObjects;

  sGeneralInfo.fDisableTacticalPanelButtons = gfDisableTacticalPanelButtons;

  sGeneralInfo.sSelMapX = sSelMapX;
  sGeneralInfo.sSelMapY = sSelMapY;
  sGeneralInfo.iCurrentMapSectorZ = iCurrentMapSectorZ;

  // Save the current status of the help screens flag that say wether or not the user has been there
  // before
  sGeneralInfo.usHasPlayerSeenHelpScreenInCurrentScreen =
      gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen;

  sGeneralInfo.ubBoxingMatchesWon = gubBoxingMatchesWon;
  sGeneralInfo.ubBoxersRests = gubBoxersRests;
  sGeneralInfo.fBoxersResting = gfBoxersResting;

  sGeneralInfo.ubDesertTemperature = gubDesertTemperature;
  sGeneralInfo.ubGlobalTemperature = gubGlobalTemperature;

  sGeneralInfo.sMercArriveSectorX = gsMercArriveSectorX;
  sGeneralInfo.sMercArriveSectorY = gsMercArriveSectorY;

  sGeneralInfo.fCreatureMeanwhileScenePlayed = gfCreatureMeanwhileScenePlayed;

  // save the global player num
  sGeneralInfo.ubPlayerNum = gbPlayerNum;

  // New stuff for the Prebattle interface / autoresolve
  sGeneralInfo.fPersistantPBI = gfPersistantPBI;
  sGeneralInfo.ubEnemyEncounterCode = gubEnemyEncounterCode;
  sGeneralInfo.ubExplicitEnemyEncounterCode = gubExplicitEnemyEncounterCode;
  sGeneralInfo.fBlitBattleSectorLocator = gfBlitBattleSectorLocator;
  sGeneralInfo.ubPBSectorX = gubPBSectorX;
  sGeneralInfo.ubPBSectorY = gubPBSectorY;
  sGeneralInfo.ubPBSectorZ = gubPBSectorZ;
  sGeneralInfo.fCantRetreatInPBI = gfCantRetreatInPBI;
  sGeneralInfo.fExplosionQueueActive = gfExplosionQueueActive;

  sGeneralInfo.bSelectedInfoChar = bSelectedInfoChar;

  sGeneralInfo.iHospitalTempBalance = giHospitalTempBalance;
  sGeneralInfo.iHospitalRefund = giHospitalRefund;
  sGeneralInfo.bHospitalPriceModifier = gbHospitalPriceModifier;
  sGeneralInfo.fPlayerTeamSawJoey = gfPlayerTeamSawJoey;
  sGeneralInfo.fMikeShouldSayHi = gfMikeShouldSayHi;

  // Setup the
  // Save the current music mode
  File_Write(hFile, &sGeneralInfo, sizeof(GENERAL_SAVE_INFO), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(GENERAL_SAVE_INFO)) {
    File_Close(hFile);
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadGeneralInfo(FileID hFile) {
  uint32_t uiNumBytesRead;

  GENERAL_SAVE_INFO sGeneralInfo;
  memset(&sGeneralInfo, 0, sizeof(GENERAL_SAVE_INFO));

  // Load the current music mode
  File_Read(hFile, &sGeneralInfo, sizeof(GENERAL_SAVE_INFO), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(GENERAL_SAVE_INFO)) {
    File_Close(hFile);
    return (FALSE);
  }

  gMusicModeToPlay = sGeneralInfo.ubMusicMode;

  guiCurrentUniqueSoldierId = sGeneralInfo.uiCurrentUniqueSoldierId;

  guiScreenToGotoAfterLoadingSavedGame = sGeneralInfo.uiCurrentScreen;

  //	gusSelectedSoldier = NOBODY;
  gusSelectedSoldier = sGeneralInfo.usSelectedSoldier;

  gsRenderCenterX = sGeneralInfo.sRenderCenterX;
  gsRenderCenterY = sGeneralInfo.sRenderCenterY;

  gfAtLeastOneMercWasHired = sGeneralInfo.fAtLeastOneMercWasHired;

  gfHavePurchasedItemsFromTony = sGeneralInfo.fHavePurchasedItemsFromTony;

  fShowItemsFlag = sGeneralInfo.fShowItemsFlag;
  fShowTownFlag = sGeneralInfo.fShowTownFlag;
  fShowMineFlag = sGeneralInfo.fShowMineFlag;
  fShowAircraftFlag = sGeneralInfo.fShowAircraftFlag;
  fShowTeamFlag = sGeneralInfo.fShowTeamFlag;

  fHelicopterAvailable = sGeneralInfo.fHelicopterAvailable;

  // helicopter vehicle id
  iHelicopterVehicleId = sGeneralInfo.iHelicopterVehicleId;

  // total distance travelled
  //	iTotalHeliDistanceSinceRefuel = sGeneralInfo.iTotalHeliDistanceSinceRefuel;

  // total owed to player
  iTotalAccumulatedCostByPlayer = sGeneralInfo.iTotalAccumulatedCostByPlayer;

  // whether or not skyrider is alive and well? and on our side yet?
  fSkyRiderAvailable = sGeneralInfo.fSkyRiderAvailable;

  // is the heli in the air?
  fHelicopterIsAirBorne = sGeneralInfo.fHelicopterIsAirBorne;

  // is the pilot returning straight to base?
  fHeliReturnStraightToBase = sGeneralInfo.fHeliReturnStraightToBase;

  // heli hovering
  fHoveringHelicopter = sGeneralInfo.fHoveringHelicopter;

  // time started hovering
  uiStartHoverTime = sGeneralInfo.uiStartHoverTime;

  // what state is skyrider's dialogue in in?
  guiHelicopterSkyriderTalkState = sGeneralInfo.uiHelicopterSkyriderTalkState;

  // the flags for skyrider events
  fShowEstoniRefuelHighLight = sGeneralInfo.fShowEstoniRefuelHighLight;
  fShowOtherSAMHighLight = sGeneralInfo.fShowOtherSAMHighLight;
  fShowDrassenSAMHighLight = sGeneralInfo.fShowDrassenSAMHighLight;
  fShowCambriaHospitalHighLight = sGeneralInfo.fShowCambriaHospitalHighLight;

  // The current state of the weather
  guiEnvWeather = sGeneralInfo.uiEnvWeather;

  gubDefaultButton = sGeneralInfo.ubDefaultButton;

  gfSkyriderEmptyHelpGiven = sGeneralInfo.fSkyriderEmptyHelpGiven;
  gubHelicopterHitsTaken = sGeneralInfo.ubHelicopterHitsTaken;
  gfSkyriderSaidCongratsOnTakingSAM = sGeneralInfo.fSkyriderSaidCongratsOnTakingSAM;
  gubPlayerProgressSkyriderLastCommentedOn = sGeneralInfo.ubPlayerProgressSkyriderLastCommentedOn;

  fEnterMapDueToContract = sGeneralInfo.fEnterMapDueToContract;
  ubQuitType = sGeneralInfo.ubQuitType;

  // if the soldier id is valid
  if (sGeneralInfo.sContractRehireSoldierID == -1)
    pContractReHireSoldier = NULL;
  else
    pContractReHireSoldier = GetSoldierByID(sGeneralInfo.sContractRehireSoldierID);

  memcpy(&gGameOptions, &sGeneralInfo.GameOptions, sizeof(GAME_OPTIONS));

#ifdef JA2BETAVERSION
  // Reset the random 'seed' number
  srand(sGeneralInfo.uiSeedNumber);
#endif

  // Restore the JA2 Clock
  SetJA2Clock(sGeneralInfo.uiBaseJA2Clock);

  // Restore the selected merc
  if (sGeneralInfo.ubSMCurrentMercID == 255)
    gpSMCurrentMerc = NULL;
  else
    gpSMCurrentMerc = GetSoldierByID(sGeneralInfo.ubSMCurrentMercID);

  // Set the interface panel to the team panel
  ShutdownCurrentPanel();

  // Restore the current tactical panel mode
  gsCurInterfacePanel = sGeneralInfo.sCurInterfacePanel;

  /*
  //moved to last stage in the LoadSaveGame() function
  //if we are in tactical
  if( guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN )
  {
          //Initialize the current panel
          InitializeCurrentPanel( );

          SelectSoldier( gusSelectedSoldier, FALSE, TRUE );
  }
  */

  // Restore the fact that it is the first time in mapscreen
  fFirstTimeInMapScreen = sGeneralInfo.fFirstTimeInMapScreen;

  // Load map screen disabling buttons
  fDisableDueToBattleRoster = sGeneralInfo.fDisableDueToBattleRoster;
  fDisableMapInterfaceDueToBattle = sGeneralInfo.fDisableMapInterfaceDueToBattle;

  memcpy(&gsBoxerGridNo, &sGeneralInfo.sBoxerGridNo, NUM_BOXERS * sizeof(int16_t));
  memcpy(&gubBoxerID, &sGeneralInfo.ubBoxerID, NUM_BOXERS * sizeof(int8_t));
  memcpy(&gfBoxerFought, &sGeneralInfo.fBoxerFought, NUM_BOXERS * sizeof(BOOLEAN));

  // Load the helicopter status
  fHelicopterDestroyed = sGeneralInfo.fHelicopterDestroyed;
  fShowMapScreenHelpText = sGeneralInfo.fShowMapScreenHelpText;

  giSortStateForMapScreenList = sGeneralInfo.iSortStateForMapScreenList;
  fFoundTixa = sGeneralInfo.fFoundTixa;

  guiTimeOfLastSkyriderMonologue = sGeneralInfo.uiTimeOfLastSkyriderMonologue;
  fSkyRiderSetUp = sGeneralInfo.fSkyRiderSetUp;

  memcpy(&fRefuelingSiteAvailable, &sGeneralInfo.fRefuelingSiteAvailable,
         NUMBER_OF_REFUEL_SITES * sizeof(BOOLEAN));

  // Meanwhile stuff
  memcpy(&gCurrentMeanwhileDef, &sGeneralInfo.gCurrentMeanwhileDef, sizeof(MEANWHILE_DEFINITION));
  //	gfMeanwhileScheduled = sGeneralInfo.gfMeanwhileScheduled;
  gfMeanwhileTryingToStart = sGeneralInfo.gfMeanwhileTryingToStart;
  gfInMeanwhile = sGeneralInfo.gfInMeanwhile;

  // list of dead guys for squads...in id values -> -1 means no one home
  memcpy(&sDeadMercs, &sGeneralInfo.sDeadMercs,
         sizeof(int16_t) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD);

  // level of public noises
  memcpy(&gbPublicNoiseLevel, &sGeneralInfo.gbPublicNoiseLevel, sizeof(int8_t) * MAXTEAMS);

  // the screen count for the init screen
  gubScreenCount = sGeneralInfo.gubScreenCount;

  // used for the mean while screen
  if (guiSaveGameVersion < 71) {
    uiMeanWhileFlags = sGeneralInfo.usOldMeanWhileFlags;
  } else {
    uiMeanWhileFlags = sGeneralInfo.uiMeanWhileFlags;
  }

  // Imp portrait number
  iPortraitNumber = sGeneralInfo.iPortraitNumber;

  // location of first enocunter with enemy
  locationOfFirstBattle.x = SectorID16_X(sGeneralInfo.sWorldSectorLocationOfFirstBattle);
  locationOfFirstBattle.y = SectorID16_Y(sGeneralInfo.sWorldSectorLocationOfFirstBattle);

  fShowMilitia = sGeneralInfo.fShowMilitia;

  fNewFilesInFileViewer = sGeneralInfo.fNewFilesInFileViewer;

  gfLastBoxingMatchWonByPlayer = sGeneralInfo.fLastBoxingMatchWonByPlayer;

  for (int i = 0; i < ARR_SIZE(sGeneralInfo.__only_storage_fSamSiteFound); i++) {
    SetSamSiteFound(i, sGeneralInfo.__only_storage_fSamSiteFound[i]);
  }

  gubNumTerrorists = sGeneralInfo.ubNumTerrorists;
  gubCambriaMedicalObjects = sGeneralInfo.ubCambriaMedicalObjects;

  gfDisableTacticalPanelButtons = sGeneralInfo.fDisableTacticalPanelButtons;

  sSelMapX = (u8)sGeneralInfo.sSelMapX;
  sSelMapY = (u8)sGeneralInfo.sSelMapY;
  iCurrentMapSectorZ = sGeneralInfo.iCurrentMapSectorZ;

  // State of email flags
  fUnReadMailFlag = sGeneralInfo.fUnReadMailFlag;
  fNewMailFlag = sGeneralInfo.fNewMailFlag;
  fOldUnreadFlag = sGeneralInfo.fOldUnReadFlag;
  fOldNewMailFlag = sGeneralInfo.fOldNewMailFlag;

  // Save the current status of the help screens flag that say wether or not the user has been there
  // before
  gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen =
      sGeneralInfo.usHasPlayerSeenHelpScreenInCurrentScreen;

  gubBoxingMatchesWon = sGeneralInfo.ubBoxingMatchesWon;
  gubBoxersRests = sGeneralInfo.ubBoxersRests;
  gfBoxersResting = sGeneralInfo.fBoxersResting;

  gubDesertTemperature = sGeneralInfo.ubDesertTemperature;
  gubGlobalTemperature = sGeneralInfo.ubGlobalTemperature;

  gsMercArriveSectorX = (u8)sGeneralInfo.sMercArriveSectorX;
  gsMercArriveSectorY = (u8)sGeneralInfo.sMercArriveSectorY;

  gfCreatureMeanwhileScenePlayed = sGeneralInfo.fCreatureMeanwhileScenePlayed;

  // load the global player num
  gbPlayerNum = sGeneralInfo.ubPlayerNum;

  // New stuff for the Prebattle interface / autoresolve
  gfPersistantPBI = sGeneralInfo.fPersistantPBI;
  gubEnemyEncounterCode = sGeneralInfo.ubEnemyEncounterCode;
  gubExplicitEnemyEncounterCode = sGeneralInfo.ubExplicitEnemyEncounterCode;
  gfBlitBattleSectorLocator = sGeneralInfo.fBlitBattleSectorLocator;
  gubPBSectorX = sGeneralInfo.ubPBSectorX;
  gubPBSectorY = sGeneralInfo.ubPBSectorY;
  gubPBSectorZ = sGeneralInfo.ubPBSectorZ;
  gfCantRetreatInPBI = sGeneralInfo.fCantRetreatInPBI;
  gfExplosionQueueActive = sGeneralInfo.fExplosionQueueActive;

  bSelectedInfoChar = sGeneralInfo.bSelectedInfoChar;

  giHospitalTempBalance = sGeneralInfo.iHospitalTempBalance;
  giHospitalRefund = sGeneralInfo.iHospitalRefund;
  gbHospitalPriceModifier = sGeneralInfo.bHospitalPriceModifier;
  gfPlayerTeamSawJoey = sGeneralInfo.fPlayerTeamSawJoey;
  gfMikeShouldSayHi = sGeneralInfo.fMikeShouldSayHi;

  return (TRUE);
}

BOOLEAN SavePreRandomNumbersToSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten;

  // Save the Prerandom number index
  File_Write(hFile, &guiPreRandomIndex, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  // Save the Prerandom number index
  File_Write(hFile, guiPreRandomNums, sizeof(uint32_t) * MAX_PREGENERATED_NUMS, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t) * MAX_PREGENERATED_NUMS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadPreRandomNumbersFromSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesRead;

  // Load the Prerandom number index
  File_Read(hFile, &guiPreRandomIndex, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  // Load the Prerandom number index
  File_Read(hFile, guiPreRandomNums, sizeof(uint32_t) * MAX_PREGENERATED_NUMS, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t) * MAX_PREGENERATED_NUMS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadMeanwhileDefsFromSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesRead;

  if (guiSaveGameVersion < 72) {
    // Load the array of meanwhile defs
    File_Read(hFile, gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * (NUM_MEANWHILES - 1),
              &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(MEANWHILE_DEFINITION) * (NUM_MEANWHILES - 1)) {
      return (FALSE);
    }
    // and set the last one
    memset(&(gMeanwhileDef[NUM_MEANWHILES - 1]), 0, sizeof(MEANWHILE_DEFINITION));

  } else {
    // Load the array of meanwhile defs
    File_Read(hFile, gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES, &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN SaveMeanwhileDefsFromSaveGameFile(FileID hFile) {
  uint32_t uiNumBytesWritten;

  // Save the array of meanwhile defs
  File_Write(hFile, &gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES,
             &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN DoesUserHaveEnoughHardDriveSpace() {
  uint32_t uiBytesFree = 0;

  uiBytesFree = Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

  // check to see if there is enough hard drive space
  if (uiBytesFree < REQUIRED_FREE_SPACE) {
    return (FALSE);
  }

  return (TRUE);
}

void GetBestPossibleSectorXYZValues(u8 *psSectorX, u8 *psSectorY, int8_t *pbSectorZ) {
  // if the current sector is valid
  if (gfWorldLoaded) {
    *psSectorX = (u8)gWorldSectorX;
    *psSectorY = (u8)gWorldSectorY;
    *pbSectorZ = gbWorldSectorZ;
  } else if (iCurrentTacticalSquad != NO_CURRENT_SQUAD && Squad[iCurrentTacticalSquad][0]) {
    if (Squad[iCurrentTacticalSquad][0]->bAssignment != IN_TRANSIT) {
      *psSectorX = (u8)Squad[iCurrentTacticalSquad][0]->sSectorX;
      *psSectorY = (u8)Squad[iCurrentTacticalSquad][0]->sSectorY;
      *pbSectorZ = Squad[iCurrentTacticalSquad][0]->bSectorZ;
    }
  } else {
    int16_t sSoldierCnt;
    struct SOLDIERTYPE *pSoldier;
    int16_t bLastTeamID;
    BOOLEAN fFoundAMerc = FALSE;

    // Set locator to first merc
    sSoldierCnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

    // loop through all the mercs on the players team to find the one that is not moving
    for (pSoldier = MercPtrs[sSoldierCnt]; sSoldierCnt <= bLastTeamID; sSoldierCnt++, pSoldier++) {
      if (IsSolActive(pSoldier)) {
        if (pSoldier->bAssignment != IN_TRANSIT && !pSoldier->fBetweenSectors) {
          // we found an alive, merc that is not moving
          *psSectorX = GetSolSectorX(pSoldier);
          *psSectorY = GetSolSectorY(pSoldier);
          *pbSectorZ = GetSolSectorZ(pSoldier);
          fFoundAMerc = TRUE;
          break;
        }
      }
    }

    // if we didnt find a merc
    if (!fFoundAMerc) {
      // Set locator to first merc
      sSoldierCnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
      bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

      // loop through all the mercs and find one that is moving
      for (pSoldier = MercPtrs[sSoldierCnt]; sSoldierCnt <= bLastTeamID;
           sSoldierCnt++, pSoldier++) {
        if (IsSolActive(pSoldier)) {
          // we found an alive, merc that is not moving
          *psSectorX = GetSolSectorX(pSoldier);
          *psSectorY = GetSolSectorY(pSoldier);
          *pbSectorZ = GetSolSectorZ(pSoldier);
          fFoundAMerc = TRUE;
          break;
        }
      }

      // if we STILL havent found a merc, give up and use the -1, -1, -1
      if (!fFoundAMerc) {
        *psSectorX = (u8)gWorldSectorX;
        *psSectorY = (u8)gWorldSectorY;
        *pbSectorZ = gbWorldSectorZ;
      }
    }
  }
}

void PauseBeforeSaveGame(void) {
  // if we are not in the save load screen
  if (guiCurrentScreen != SAVE_LOAD_SCREEN) {
    // Pause the game
    PauseGame();
  }
}

void UnPauseAfterSaveGame(void) {
  // if we are not in the save load screen
  if (guiCurrentScreen != SAVE_LOAD_SCREEN) {
    // UnPause time compression
    UnPauseGame();
  }
}

void UpdateMercMercContractInfo() {
  uint8_t ubCnt;
  struct SOLDIERTYPE *pSoldier;

  for (ubCnt = BIFF; ubCnt <= BUBBA; ubCnt++) {
    pSoldier = FindSoldierByProfileID(ubCnt, TRUE);

    // if the merc is on the team
    if (pSoldier == NULL) continue;

    gMercProfiles[ubCnt].iMercMercContractLength = pSoldier->iTotalContractLength;

    pSoldier->iTotalContractLength = 0;
  }
}

void HandleOldBobbyRMailOrders() {
  int32_t iCnt;
  int32_t iNewListCnt = 0;

  if (LaptopSaveInfo.usNumberOfBobbyRayOrderUsed != 0) {
    // Allocate memory for the list
    gpNewBobbyrShipments = (NewBobbyRayOrderStruct *)MemAlloc(
        sizeof(NewBobbyRayOrderStruct) * LaptopSaveInfo.usNumberOfBobbyRayOrderUsed);
    if (gpNewBobbyrShipments == NULL) {
      Assert(0);
      return;
    }

    giNumberOfNewBobbyRShipment = LaptopSaveInfo.usNumberOfBobbyRayOrderUsed;

    // loop through and add the old items to the new list
    for (iCnt = 0; iCnt < LaptopSaveInfo.usNumberOfBobbyRayOrderItems; iCnt++) {
      // if this slot is used
      if (LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].fActive) {
        // copy over the purchase info
        memcpy(gpNewBobbyrShipments[iNewListCnt].BobbyRayPurchase,
               LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].BobbyRayPurchase,
               sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);

        gpNewBobbyrShipments[iNewListCnt].fActive = TRUE;
        gpNewBobbyrShipments[iNewListCnt].ubDeliveryLoc = BR_DRASSEN;
        gpNewBobbyrShipments[iNewListCnt].ubDeliveryMethod = 0;
        gpNewBobbyrShipments[iNewListCnt].ubNumberPurchases =
            LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].ubNumberPurchases;
        gpNewBobbyrShipments[iNewListCnt].uiPackageWeight = 1;
        gpNewBobbyrShipments[iNewListCnt].uiOrderedOnDayNum = GetGameTimeInDays();
        gpNewBobbyrShipments[iNewListCnt].fDisplayedInShipmentPage = TRUE;

        iNewListCnt++;
      }
    }

    // Clear out the old list
    LaptopSaveInfo.usNumberOfBobbyRayOrderUsed = 0;
    MemFree(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray);
    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
  }
}

uint32_t CalcJA2EncryptionSet(SAVED_GAME_HEADER *pSaveGameHeader) {
  uint32_t uiEncryptionSet = 0;

  uiEncryptionSet = pSaveGameHeader->uiSavedGameVersion;
  uiEncryptionSet *= pSaveGameHeader->uiFlags;
  uiEncryptionSet += pSaveGameHeader->iCurrentBalance;
  uiEncryptionSet *= (pSaveGameHeader->ubNumOfMercsOnPlayersTeam + 1);
  uiEncryptionSet += pSaveGameHeader->bSectorZ * 3;
  uiEncryptionSet += pSaveGameHeader->ubLoadScreenID;

  if (pSaveGameHeader->fAlternateSector) {
    uiEncryptionSet += 7;
  }

  if (pSaveGameHeader->uiRandom % 2 == 0) {
    uiEncryptionSet++;

    if (pSaveGameHeader->uiRandom % 7 == 0) {
      uiEncryptionSet++;
      if (pSaveGameHeader->uiRandom % 23 == 0) {
        uiEncryptionSet++;
      }
      if (pSaveGameHeader->uiRandom % 79 == 0) {
        uiEncryptionSet += 2;
      }
    }
  }

#ifdef GERMAN
  uiEncryptionSet *= 11;
#endif

  uiEncryptionSet = uiEncryptionSet % 10;

  uiEncryptionSet += pSaveGameHeader->uiDay / 10;

  uiEncryptionSet = uiEncryptionSet % 19;

  // now pick a different set of #s depending on what game options we've chosen
  if (pSaveGameHeader->sInitialGameOptions.fGunNut) {
    uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 6;
  }

  if (pSaveGameHeader->sInitialGameOptions.fSciFi) {
    uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 3;
  }

  switch (pSaveGameHeader->sInitialGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:
      uiEncryptionSet += 0;
      break;
    case DIF_LEVEL_MEDIUM:
      uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS;
      break;
    case DIF_LEVEL_HARD:
      uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 2;
      break;
  }

  return (uiEncryptionSet);
}
