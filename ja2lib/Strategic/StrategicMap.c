// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/StrategicMap.h"

#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/History.h"
#include "LoadingScreen.h"
#include "MessageBoxScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SaveLoadGame.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Scheduling.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/StrategicTurns.h"
#include "Strategic/TownMilitia.h"
#include "SysGlobals.h"
#include "Tactical.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Boxing.h"
#include "Tactical/Bullets.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/EnemySoldierSave.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/MercEntering.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "TacticalAI/AI.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/MapEdgepoints.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldMan.h"
#include "Town.h"
#include "UI.h"
#include "Utils/AnimatedProgressBar.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "rust_clock.h"
#include "rust_colors.h"
#include "rust_fileman.h"
#include "rust_militia.h"
#include "rust_sam_sites.h"

// Used by PickGridNoToWalkIn
#define MAX_ATTEMPTS 200

#define QUEST_CHECK_EVENT_TIME (8 * 60)
#define BOBBYRAY_UPDATE_TIME (9 * 60)
#define INSURANCE_UPDATE_TIME 0
#define EARLY_MORNING_TIME (4 * 60)
#define ENRICO_MAIL_TIME (7 * 60)

enum {
  ABOUT_TO_LOAD_NEW_MAP,
  ABOUT_TO_TRASH_WORLD,
};
BOOLEAN HandleDefiniteUnloadingOfWorld(uint8_t ubUnloadCode);

extern int16_t gsRobotGridNo;
extern BOOLEAN gfUndergroundTacticalTraversal;

extern MINE_LOCATION_TYPE gMineLocation[MAX_NUMBER_OF_MINES];

extern void CalculateNonPersistantPBIInfo();

extern void MapScreenDefaultOkBoxCallback(uint8_t bExitValue);

extern BOOLEAN gfGettingNameFromSaveLoadScreen;

int16_t gWorldSectorX = 0;
int16_t gWorldSectorY = 0;
int8_t gbWorldSectorZ = -1;

u8 gsAdjacentSectorX, gsAdjacentSectorY;
int8_t gbAdjacentSectorZ;
struct GROUP *gpAdjacentGroup = NULL;
uint8_t gubAdjacentJumpCode;
uint32_t guiAdjacentTraverseTime;
uint8_t gubTacticalDirection;
int16_t gsAdditionalData;
uint16_t gusDestExitGridNo;

BOOLEAN fUsingEdgePointsForStrategicEntry = FALSE;
BOOLEAN gfInvalidTraversal = FALSE;
BOOLEAN gfLoneEPCAttemptingTraversal = FALSE;
BOOLEAN gfRobotWithoutControllerAttemptingTraversal = FALSE;
BOOLEAN gubLoneMercAttemptingToAbandonEPCs = 0;
int8_t gbPotentiallyAbandonedEPCSlotID = -1;

extern BOOLEAN gfUsePersistantPBI;

BOOLEAN gfUseAlternateMap = FALSE;
// whether or not we have found Orta yet
BOOLEAN fFoundOrta = FALSE;

int8_t gbMercIsNewInThisSector[MAX_NUM_SOLDIERS];

// the amount of time that a soldier will wait to return to desired/old squad
#define DESIRE_SQUAD_RESET_DELAY 12 * 60

int16_t DirXIncrementer[8] = {
    0,   // N
    1,   // NE
    1,   // E
    1,   // SE
    0,   // S
    -1,  // SW
    -1,  // W
    -1   // NW
};

int16_t DirYIncrementer[8] = {
    -1,  // N
    -1,  // NE
    0,   // E
    1,   // SE
    1,   // S
    1,   // SW
    0,   // W
    -1   // NW
};

char* pVertStrings[] = {
    "X", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R",
};

char* pHortStrings[] = {
    "X", "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",
    "9", "10", "11", "12", "13", "14", "15", "16", "17",
};

void DoneFadeOutAdjacentSector(void);
void DoneFadeOutExitGridSector(void);

int16_t PickGridNoNearestEdge(struct SOLDIERTYPE *pSoldier, uint8_t ubTacticalDirection);
int16_t PickGridNoToWalkIn(struct SOLDIERTYPE *pSoldier, uint8_t ubInsertionDirection,
                         uint32_t *puiNumAttempts);

void HandleQuestCodeOnSectorExit(int16_t sOldSectorX, int16_t sOldSectorY, int8_t bOldSectorZ);
void HandlePotentialMoraleHitForSkimmingSectors(struct GROUP *pGroup);

extern void InitializeTacticalStatusAtBattleStart();

extern wchar_t* pBullseyeStrings[];

extern void HandleRPCDescription();

// temp timer stuff -- to measure the time it takes to load a map.
#ifdef JA2TESTVERSION
extern int16_t gsAINumAdmins;
extern int16_t gsAINumTroops;
extern int16_t gsAINumElites;
extern int16_t gsAINumCreatures;
// The wrapper time for EnterSector
BOOLEAN fStartNewFile = TRUE;
uint32_t uiEnterSectorStartTime;
uint32_t uiEnterSectorEndTime;
// The grand total time for loading a map
uint32_t uiLoadWorldStartTime;
uint32_t uiLoadWorldEndTime;
// The time spent in File_Read
uint32_t uiTotalFileReadTime;
uint32_t uiTotalFileReadCalls;
// LoadWorld and parts
uint32_t uiLoadWorldTime;
uint32_t uiTrashWorldTime;
uint32_t uiLoadMapTilesetTime;
uint32_t uiLoadMapLightsTime;
uint32_t uiBuildShadeTableTime;
uint32_t uiNumTablesSaved;
uint32_t uiNumTablesLoaded;
uint32_t uiNumImagesReloaded;

#include "TileEngine/TileDat.h"
#endif

uint32_t UndergroundTacticalTraversalTime(
    int8_t bExitDirection) {  // We are attempting to traverse in an underground environment.  We need
                            // to use a complete different
  // method.  When underground, all sectors are instantly adjacent.
  switch (bExitDirection) {
    case NORTH_STRATEGIC_MOVE:
      if (gMapInformation.sNorthGridNo != -1) return 0;
      break;
    case EAST_STRATEGIC_MOVE:
      if (gMapInformation.sEastGridNo != -1) return 0;
      break;
    case SOUTH_STRATEGIC_MOVE:
      if (gMapInformation.sSouthGridNo != -1) return 0;
      break;
    case WEST_STRATEGIC_MOVE:
      if (gMapInformation.sWestGridNo != -1) return 0;
      break;
  }
  return 0xffffffff;
}

void BeginLoadScreen() {
  struct GRect SrcRect, DstRect;
  uint32_t uiStartTime, uiCurrTime;
  int32_t iPercentage, iFactor;
  uint32_t uiTimeRange;
  uint8_t ubLoadScreenID;

  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  if (IsMapScreen_2() && !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && !AreInMeanwhile()) {
    DstRect.iLeft = 0;
    DstRect.iTop = 0;
    DstRect.iRight = 640;
    DstRect.iBottom = 480;
    uiTimeRange = 2000;
    iPercentage = 0;
    uiStartTime = GetJA2Clock();
    VSurfaceBlitBufToBuf(vsFB, vsSB, 0, 0, 640, 480);
    PlayJA2SampleFromFile("SOUNDS\\Final Psionic Blast 01 (16-44).wav", RATE_11025, HIGHVOLUME, 1,
                          MIDDLEPAN);
    while (iPercentage < 100) {
      uiCurrTime = GetJA2Clock();
      iPercentage = (uiCurrTime - uiStartTime) * 100 / uiTimeRange;
      iPercentage = min(iPercentage, 100);

      // Factor the percentage so that it is modified by a gravity falling acceleration effect.
      iFactor = (iPercentage - 50) * 2;
      if (iPercentage < 50)
        iPercentage = (uint32_t)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
      else
        iPercentage = (uint32_t)(iPercentage + (100 - iPercentage) * iFactor * 0.01 + 0.05);

      if (iPercentage > 50) {
        ShadowVideoSurfaceRectUsingLowPercentTable(vsSB, 0, 0, 640, 480);
      }

      SrcRect.iLeft = 536 * iPercentage / 100;
      SrcRect.iRight = 640 - iPercentage / 20;
      SrcRect.iTop = 367 * iPercentage / 100;
      SrcRect.iBottom = 480 - 39 * iPercentage / 100;
      BltStretchVideoSurface(vsFB, vsSB, 0, 0, 0, &SrcRect, &DstRect);
      InvalidateScreen();
      RefreshScreen();
    }
  }
  VSurfaceColorFill(vsFB, 0, 0, 640, 480, rgb32_to_rgb565(FROMRGB(0, 0, 0)));
  InvalidateScreen();
  RefreshScreen();

  // If we are loading a saved game, use the Loading screen we saved into the SavedGameHeader file
  // ( which gets reloaded into gubLastLoadingScreenID )
  if (!gfGotoSectorTransition) {
    if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) {
      DisplayLoadScreenWithID(gubLastLoadingScreenID);
    } else {
      ubLoadScreenID = GetLoadScreenID((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
      DisplayLoadScreenWithID(ubLoadScreenID);
    }
  }

#ifdef JA2TESTVERSION
  uiEnterSectorStartTime = 0;
  uiEnterSectorEndTime = 0;
  // The grand total time for loading a map
  uiLoadWorldStartTime = 0;
  uiLoadWorldEndTime = 0;
  // The time spent in File_Read
  uiTotalFileReadTime = 0;
  uiTotalFileReadCalls = 0;
  // Sections of LoadWorld
  uiLoadWorldTime = 0;
  uiTrashWorldTime = 0;
  uiLoadMapTilesetTime = 0;
  uiLoadMapLightsTime = 0;
  uiBuildShadeTableTime = 0;
  uiEnterSectorStartTime = GetJA2Clock();
#endif
}

void EndLoadScreen() {
#ifdef JA2TESTVERSION
  // Report the time it took to load the map.  This is temporary until we are satisfied with the
  // time it takes to load the map.
  wchar_t str[60];
  FILE *fp;
  uint32_t uiSeconds;
  uint32_t uiHundreths;
  uint32_t uiUnaccounted;
  uint32_t uiPercentage;
  uiEnterSectorEndTime = GetJA2Clock();
  uiSeconds = (uiEnterSectorEndTime - uiEnterSectorStartTime) / 1000;
  uiHundreths = ((uiEnterSectorEndTime - uiEnterSectorStartTime) / 10) % 100;
  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_YELLOW);
  SetFontBackground(FONT_NEARBLACK);
  if (!gbWorldSectorZ) {
    swprintf(str, ARR_SIZE(str), L"%c%d ENTER GetSectorID8 TIME:  %d.%02d seconds.",
             'A' + gWorldSectorY - 1, gWorldSectorX, uiSeconds, uiHundreths);
  } else {
    swprintf(str, ARR_SIZE(str), L"%c%d_b%d ENTER GetSectorID8 TIME:  %d.%02d seconds.",
             'A' + gWorldSectorY - 1, gWorldSectorX, gbWorldSectorZ, uiSeconds, uiHundreths);
  }
  ScreenMsg(FONT_YELLOW, MSG_TESTVERSION, str);
  if (fStartNewFile) {  // start new file
    fp = fopen("TimeResults.txt", "w");
    ScreenMsg(FONT_YELLOW, MSG_TESTVERSION,
              L"See JA2\\Data\\TimeResults.txt for more detailed timings.");
    fStartNewFile = FALSE;
  } else {  // append to end of file
    fp = fopen("TimeResults.txt", "a");

    if (fp) {
      fprintf(fp, "\n\n--------------------------------------------------------------------\n\n");
    }
  }
  if (fp) {
    // Record all of the timings.
    fprintf(fp, "%S\n", str);
    fprintf(fp, "EnterSector() supersets LoadWorld().  This includes other external sections.\n");
    // File_Read()
    fprintf(
        fp,
        "\n\nVARIOUS FUNCTION TIMINGS (exclusive of actual function timings in second heading)\n");
    uiSeconds = uiTotalFileReadTime / 1000;
    uiHundreths = (uiTotalFileReadTime / 10) % 100;
    fprintf(fp, "File_Read:  %d.%02d (called %d times)\n", uiSeconds, uiHundreths,
            uiTotalFileReadCalls);

    fprintf(fp, "\n\nSECTIONS OF LOADWORLD (all parts should add up to 100%%)\n");
    // TrashWorld()
    uiSeconds = uiTrashWorldTime / 1000;
    uiHundreths = (uiTrashWorldTime / 10) % 100;
    fprintf(fp, "TrashWorld: %d.%02d\n", uiSeconds, uiHundreths);
    // LoadMapTilesets()
    uiSeconds = uiLoadMapTilesetTime / 1000;
    uiHundreths = (uiLoadMapTilesetTime / 10) % 100;
    fprintf(fp, "LoadMapTileset: %d.%02d\n", uiSeconds, uiHundreths);
    // LoadMapLights()
    uiSeconds = uiLoadMapLightsTime / 1000;
    uiHundreths = (uiLoadMapLightsTime / 10) % 100;
    fprintf(fp, "LoadMapLights: %d.%02d\n", uiSeconds, uiHundreths);
    uiSeconds = uiBuildShadeTableTime / 1000;
    uiHundreths = (uiBuildShadeTableTime / 10) % 100;
    fprintf(fp, "  1)  BuildShadeTables: %d.%02d\n", uiSeconds, uiHundreths);

    uiPercentage = uiNumImagesReloaded * 100 / NUMBEROFTILETYPES;
    fprintf(fp, "  2)  %d%% of the tileset images were actually reloaded.\n", uiPercentage);
    if ((uiNumTablesSaved + uiNumTablesLoaded) != 0) {
      uiPercentage = uiNumTablesSaved * 100 / (uiNumTablesSaved + uiNumTablesLoaded);
    } else {
      uiPercentage = 0;
    }
    fprintf(fp, "  3)  Of that, %d%% of the shade tables were generated (not loaded).\n",
            uiPercentage);
    if (gfForceBuildShadeTables)
      fprintf(fp, "  NOTE:  Force building of shadetables enabled on this local computer.\n");

    // Unaccounted
    uiUnaccounted = uiLoadWorldTime - uiTrashWorldTime - uiLoadMapTilesetTime - uiLoadMapLightsTime;
    uiSeconds = uiUnaccounted / 1000;
    uiHundreths = (uiUnaccounted / 10) % 100;
    fprintf(fp, "Unaccounted: %d.%02d\n", uiSeconds, uiHundreths);
    // LoadWorld()
    uiSeconds = uiLoadWorldTime / 1000;
    uiHundreths = (uiLoadWorldTime / 10) % 100;
    fprintf(fp, "\nTotal: %d.%02d\n", uiSeconds, uiHundreths);

    fclose(fp);
  }
#endif
}

BOOLEAN InitStrategicEngine() {
  // this runs every time we start the application, so don't put anything in here that's only
  // supposed to run when a new *game* is started!  Those belong in InitStrategicLayer() instead.

  // town distances are pre-calculated and read in from a data file
  // since it takes quite a while to plot strategic paths between all pairs of town sectors...

// #define RECALC_TOWN_DISTANCES
#ifdef RECALC_TOWN_DISTANCES
  CalcDistancesBetweenTowns();
  WriteOutDistancesBetweenTowns();
  DumpDistancesBetweenTowns();
#endif

  ReadInDistancesBetweenTowns();

  return (TRUE);
}

// return number of sectors this town takes up
uint8_t GetTownSectorSize(TownID bTownId) {
  uint8_t ubSectorSize = 0;
  int32_t iCounterA = 0, iCounterB = 0;

  for (iCounterA = 0; iCounterA < (int32_t)(MAP_WORLD_X - 1); iCounterA++) {
    for (iCounterB = 0; iCounterB < (int32_t)(MAP_WORLD_Y - 1); iCounterB++) {
      if (GetTownIdForSector(iCounterA, iCounterB) == bTownId) {
        ubSectorSize++;
      }
    }
  }

  return (ubSectorSize);
}

// return number of sectors under player control for this town
uint8_t GetTownSectorsUnderControl(TownID bTownId) {
  int8_t ubSectorsControlled = 0;

  for (u8 x = 0; x < (int32_t)(MAP_WORLD_X - 1); x++) {
    for (u8 y = 0; y < (int32_t)(MAP_WORLD_Y - 1); y++) {
      if ((GetTownIdForSector(x, y) == bTownId) && (!IsSectorEnemyControlled(x, y)) &&
          (NumEnemiesInSector(x, y) == 0)) {
        ubSectorsControlled++;
      }
    }
  }

  return (ubSectorsControlled);
}

void InitializeSAMSites(void) {
  // move the landing zone over to Omerta
  gsMercArriveSectorX = 9;
  gsMercArriveSectorY = 1;

  // all SAM sites start game in perfect working condition
  for (int i = 0; i < GetSamSiteCount(); i++) {
    SetSamCondition(i, 100);
  }

  UpdateAirspaceControl();
}

// get short sector name without town name
void GetShortSectorString(u8 sMapX, u8 sMapY, wchar_t* sString, size_t bufSize) {
  // OK, build string id like J11
  swprintf(sString, bufSize, L"%hs%hs", pVertStrings[sMapY], pHortStrings[sMapX]);
}

void GetMapFileName(u8 sMapX, u8 sMapY, int8_t bSectorZ, char* bString, BOOLEAN fUsePlaceholder,
                    BOOLEAN fAddAlternateMapLetter) {
  char bTestString[150];
  char bExtensionString[15];

  if (bSectorZ != 0) {
    sprintf(bExtensionString, "_b%d", bSectorZ);
  } else {
    strcpy(bExtensionString, "");
  }

  // the gfUseAlternateMap flag is set in the loading saved games.  When starting a new game the
  // underground sector
  // info has not been initialized, so we need the flag to load an alternate sector.
  if (gfUseAlternateMap | GetSectorFlagStatus(sMapX, sMapY, bSectorZ, SF_USE_ALTERNATE_MAP)) {
    gfUseAlternateMap = FALSE;

    // if we ARE to use the a map, or if we are saving AND the save game version is before 80, add
    // the a
    if (fAddAlternateMapLetter) {
      strcat(bExtensionString, "_a");
    }
  }

  // If we are in a meanwhile...
  if (AreInMeanwhile() && sMapX == 3 && sMapY == 16 &&
      !bSectorZ)  // GetMeanwhileID() != INTERROGATION )
  {
    if (fAddAlternateMapLetter) {
      strcat(bExtensionString, "_m");
    }
  }

  // This is the string to return, but...
  sprintf(bString, "%s%s%s.DAT", pVertStrings[sMapY], pHortStrings[sMapX], bExtensionString);

  // We will test against this string
  sprintf(bTestString, "MAPS\\%s", bString);

  if (fUsePlaceholder && !File_Exists(bTestString)) {
    // Debug str
    DebugMsg(TOPIC_JA2, DBG_INFO, String("Map does not exist for %s, using default.", bTestString));
    // Set to a string we know!
    sprintf(bString, "%s", "H10.DAT");
    ScreenMsg(FONT_YELLOW, MSG_DEBUG, L"Using PLACEHOLDER map!");
  }
  return;
}

void GetCurrentWorldSector(u8 *psMapX, u8 *psMapY) {
  *psMapX = (u8)gWorldSectorX;
  *psMapY = (u8)gWorldSectorY;
}

// not in overhead.h!
extern uint8_t NumEnemyInSector();

void HandleRPCDescriptionOfSector(u8 sSectorX, u8 sSectorY, i8 sSectorZ) {
  uint32_t cnt;
  uint8_t ubSectorDescription[33][3] = {
      // row (letter), column, quote #
      {2, 13, 0},  // b13	Drassen
      {3, 13, 1},  // c13	Drassen
      {4, 13, 2},  // d13	Drassen
      {8, 13, 3},  // h13	Alma
      {8, 14, 4},  // h14	Alma
      {9, 13, 5},  // i13	Alma * (extra quote 6 if Sci-fi ) *
      {9, 14, 7},  // i14	Alma
      {6, 8, 8},   // f8	Cambria
      {6, 9, 9},   // f9	Cambria
      {7, 8, 10},  // g8	Cambria

      {7, 9, 11},  // g9	Cambria
      {3, 6, 12},  // c6	San Mona
      {3, 5, 13},  // c5	San Mona
      {4, 5, 14},  // d5	San Mona
      {2, 2, 15},  // b2	Chitzena
      {1, 2, 16},  // a2	Chitzena
      {7, 1, 17},  // g1	Grumm
      {8, 1, 18},  // h1	Grumm
      {7, 2, 19},  // g2 	Grumm
      {8, 2, 20},  // h2	Grumm

      {9, 6, 21},    // i6	Estoni
      {11, 4, 22},   // k4	Orta
      {12, 11, 23},  // l11	Balime
      {12, 12, 24},  // l12	Balime
      {15, 3, 25},   // o3	Meduna
      {16, 3, 26},   // p3	Meduna
      {14, 4, 27},   // n4	Meduna
      {14, 3, 28},   // n3	Meduna
      {15, 4, 30},   // o4	Meduna
      {10, 9, 31},   // j9	Tixa

      {4, 15, 32},  // d15	NE SAM
      {4, 2, 33},   // d2	NW SAM
      {9, 8, 34}    // i8	CENTRAL SAM
  };

  // Default to false
  gTacticalStatus.fCountingDownForGuideDescription = FALSE;

  // OK, if the first time in...
  if (GetSectorFlagStatus(sSectorX, sSectorY, (uint8_t)sSectorZ, SF_HAVE_USED_GUIDE_QUOTE) != TRUE) {
    if (sSectorZ != 0) {
      return;
    }

    // OK, check if we are in a good sector....
    for (cnt = 0; cnt < 33; cnt++) {
      if (sSectorX == ubSectorDescription[cnt][1] && sSectorY == ubSectorDescription[cnt][0]) {
        // If we're not scifi, skip some
        if (!gGameOptions.fSciFi && cnt == 3) {
          continue;
        }

        SetSectorFlag(sSectorX, sSectorY, (uint8_t)sSectorZ, SF_HAVE_USED_GUIDE_QUOTE);

        gTacticalStatus.fCountingDownForGuideDescription = TRUE;
        gTacticalStatus.bGuideDescriptionCountDown =
            (int8_t)(4 + Random(5));  // 4 to 8 tactical turns...
        gTacticalStatus.ubGuideDescriptionToUse = ubSectorDescription[cnt][2];
        gTacticalStatus.bGuideDescriptionSectorX = (int8_t)sSectorX;
        gTacticalStatus.bGuideDescriptionSectorY = (int8_t)sSectorY;
      }
    }
  }

  // Handle guide description ( will be needed if a SAM one )
  HandleRPCDescription();
}

BOOLEAN SetCurrentWorldSector(u8 sMapX, u8 sMapY, i8 bMapZ) {
  BOOLEAN fChangeMusic = TRUE;

  // ATE: Zero out accounting functions
  memset(gbMercIsNewInThisSector, 0, sizeof(gbMercIsNewInThisSector));

  SyncStrategicTurnTimes();

  // is the sector already loaded?
  if ((gWorldSectorX == sMapX) && (sMapY == gWorldSectorY) && (bMapZ == gbWorldSectorZ)) {
    // Inserts the enemies into the newly loaded map based on the strategic information.
    // Note, the flag will return TRUE only if enemies were added.  The game may wish to
    // do something else in a case where no enemies are present.

    SetPendingNewScreen(GAME_SCREEN);
    if (!NumEnemyInSector()) {
      PrepareEnemyForSectorBattle();
    }
    if (gubNumCreaturesAttackingTown && !gbWorldSectorZ &&
        gubSectorIDOfCreatureAttack == GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)) {
      PrepareCreaturesForBattle();
    }
    if (gfGotoSectorTransition) {
      BeginLoadScreen();
      gfGotoSectorTransition = FALSE;
    }

    // Check for helicopter being on the ground in this sector...
    HandleHelicopterOnGroundGraphic();

    ReinitMilitiaTactical();

    AllTeamsLookForAll(TRUE);
    return (TRUE);
  }

  if (gWorldSectorX && gWorldSectorY && gbWorldSectorZ != -1) {
    HandleDefiniteUnloadingOfWorld(ABOUT_TO_LOAD_NEW_MAP);
  }

  // make this the currently loaded sector
  gWorldSectorX = sMapX;
  gWorldSectorY = sMapY;
  gbWorldSectorZ = bMapZ;

  // update currently selected map sector to match
  ChangeSelectedMapSector(sMapX, sMapY, bMapZ);

  // Check to see if the sector we are loading is the cave sector under Tixa.  If so
  // then we will set up the meanwhile scene to start the creature quest.
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    StopAnyCurrentlyTalkingSpeech();

    if (gWorldSectorX == 9 && gWorldSectorY == 10 && gbWorldSectorZ == 2) {
      InitCreatureQuest();  // Ignored if already active.
    }
  }

  // Stop playing any music -- will fade out.
  // SetMusicMode( MUSIC_NONE );

  // ATE: Determine if we should set the default music...

  // Are we already in 'tense' music...

  // ATE: Change music only if not loading....
  /*-
  if ( gubMusicMode == MUSIC_TACTICAL_ENEMYPRESENT  )
  {
          fChangeMusic = FALSE;
  }

  // Did we 'tactically traverse' over....
  if ( gfTacticalTraversal )
  {
          fChangeMusic = FALSE;
  }

  // If we have no music playing at all....
  if ( gubMusicMode == MUSIC_NONE  )
  {
          fChangeMusic = TRUE;
  }
  -*/

  if ((gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    fChangeMusic = TRUE;
  } else {
    fChangeMusic = FALSE;
  }

  if (fChangeMusic) {
    SetMusicMode(MUSIC_MAIN_MENU);
  }

  // ATE: Do this stuff earlier!
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Update the last time we were in tactical...
    gTacticalStatus.uiTimeSinceLastInTactical = GetGameTimeInMin();

    // init some AI stuff
    InitializeTacticalStatusAtBattleStart();

    // CJC: delay this until after entering the sector!
    // InitAI();

    // Check for helicopter being on the ground in this sector...
    HandleHelicopterOnGroundSkyriderProfile();
  }

  // Load and enter the new sector
  if (EnterSector((u8)gWorldSectorX, (u8)gWorldSectorY, bMapZ)) {
    // CJC: moved this here Feb 17
    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      InitAI();
    }

    // If there are any people with schedules, now is the time to process them.
    // CJC: doesn't work here if we're going through the tactical placement GUI; moving
    // this call to PrepareLoadedSector()
    // PostSchedules();

    // ATE: OK, add code here to update the states of doors if they should
    // be closed......
    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      ExamineDoorsOnEnteringSector();
    }

    // Update all the doors in the sector according to the temp file previously
    // loaded, and any changes made by the schedules
    UpdateDoorGraphicsFromStatus(TRUE, FALSE);

    // Set the fact we have visited the  sector
    SetSectorFlag((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ, SF_ALREADY_LOADED);

    // Check for helicopter being on the ground in this sector...
    HandleHelicopterOnGroundGraphic();
  } else
    return (FALSE);

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    if ((gubMusicMode != MUSIC_TACTICAL_ENEMYPRESENT && gubMusicMode != MUSIC_TACTICAL_BATTLE) ||
        (!NumHostilesInSector(sMapX, sMapY, bMapZ) &&
         gubMusicMode == MUSIC_TACTICAL_ENEMYPRESENT)) {
      // ATE; Fade FA.T....
      SetMusicFadeSpeed(5);

      SetMusicMode(MUSIC_TACTICAL_NOTHING);
    }

    // ATE: Check what sector we are in, to show description if we have an RPC.....
    HandleRPCDescriptionOfSector(sMapX, sMapY, bMapZ);

    // ATE: Set Flag for being visited...
    SetSectorFlag(sMapX, sMapY, bMapZ, SF_HAS_ENTERED_TACTICAL);

    // ATE; Reset some flags for creature sayings....
    gTacticalStatus.fSaidCreatureFlavourQuote = FALSE;
    gTacticalStatus.fHaveSeenCreature = FALSE;
    gTacticalStatus.fBeenInCombatOnce = FALSE;
    gTacticalStatus.fSaidCreatureSmellQuote = FALSE;
    ResetMultiSelection();

    // ATE: Decide if we can have crows here....
    gTacticalStatus.fGoodToAllowCrows = FALSE;
    gTacticalStatus.fHasEnteredCombatModeSinceEntering = FALSE;
    gTacticalStatus.fDontAddNewCrows = FALSE;

    // Adjust delay for tense quote
    gTacticalStatus.sCreatureTenseQuoteDelay = (int16_t)(10 + Random(20));

    {
      u8 sWarpWorldX;
      u8 sWarpWorldY;
      int8_t bWarpWorldZ;
      int16_t sWarpGridNo;

      if (GetWarpOutOfMineCodes(&sWarpWorldX, &sWarpWorldY, &bWarpWorldZ, &sWarpGridNo) &&
          gbWorldSectorZ >= 2) {
        gTacticalStatus.uiFlags |= IN_CREATURE_LAIR;
      } else {
        gTacticalStatus.uiFlags &= (~IN_CREATURE_LAIR);
      }
    }

    // Every third turn
    // if ( Random( 3 ) == 0  )
    {
      gTacticalStatus.fGoodToAllowCrows = TRUE;
      gTacticalStatus.ubNumCrowsPossible = (uint8_t)(5 + Random(5));
    }
  }

  return (TRUE);
}

BOOLEAN MapExists(char *szFilename) {
  char str[50];
  FileID fp = FILE_ID_ERR;
  sprintf(str, "MAPS\\%s", szFilename);
  fp = File_OpenForReading(str);
  if (!fp) return FALSE;
  File_Close(fp);
  return TRUE;
}

void RemoveMercsInSector() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // ATE: only for OUR guys.. the rest is taken care of in TrashWorld() when a new sector is
  // added...
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      RemoveSoldierFromGridNo(pSoldier);
    }
  }
}

void PrepareLoadedSector() {
  BOOLEAN fAddCivs = TRUE;
  int8_t bMineIndex = -1;

  if (AreInMeanwhile() == FALSE) {
    if (gbWorldSectorZ == 0) {
      //			MakePlayerPerceptionOfSectorControlCorrect( gWorldSectorX,
      // gWorldSectorY, gbWorldSectorZ );
    } else {
      // we always think we control underground sectors once we've visited them
      SectorInfo[GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)].fPlayer[gbWorldSectorZ] = TRUE;
    }
  }

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    UpdateMercsInSector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
  }

  // Reset ambients!
  HandleNewSectorAmbience(gTilesets[giCurrentTilesetID].ubAmbientID);

  // if we are loading a 'pristine' map ( ie, not loading a saved game )
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    if (!AreReloadingFromMeanwhile()) {
      SetPendingNewScreen(GAME_SCREEN);

      // Make interface the team panel always...
      SetCurrentInterfacePanel((uint8_t)TEAM_PANEL);
    }

    // Check to see if civilians should be added.  Always add civs to maps unless they are
    // in a mine that is shutdown.
    if (gbWorldSectorZ) {
      bMineIndex = GetIdOfMineForSector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ);
      if (bMineIndex != -1) {
        if (!AreThereMinersInsideThisMine((uint8_t)bMineIndex)) {
          fAddCivs = FALSE;
        }
      }
    }
    if (fAddCivs) {
      AddSoldierInitListTeamToWorld(CIV_TEAM, 255);
    }

    AddSoldierInitListTeamToWorld(MILITIA_TEAM, 255);
    AddSoldierInitListBloodcats();
    // Creatures are only added if there are actually some of them.  It has to go through some
    // additional checking.

    PrepareCreaturesForBattle();

    PrepareMilitiaForTactical();

    // OK, set varibles for entring this new sector...
    gTacticalStatus.fVirginSector = TRUE;

    // Inserts the enemies into the newly loaded map based on the strategic information.
    // Note, the flag will return TRUE only if enemies were added.  The game may wish to
    // do something else in a case where no enemies are present.
    if (!gfRestoringEnemySoldiersFromTempFile) {
      // AddSoldierInitListTeamToWorld( CIV_TEAM, 255 );
      //			fEnemyPresenceInThisSector = PrepareEnemyForSectorBattle();
    }
    AddProfilesNotUsingProfileInsertionData();

    if (!AreInMeanwhile() || GetMeanwhileID() == INTERROGATION) {
      PrepareEnemyForSectorBattle();
    }

    // Regardless whether or not this was set, clear it now.
    gfRestoringEnemySoldiersFromTempFile = FALSE;

    if (gbWorldSectorZ > 0) {
      // we always think we control underground sectors once we've visited them
      SectorInfo[GetSectorID8((u8)gWorldSectorX, (u8)gWorldSectorY)].fPlayer[gbWorldSectorZ] = TRUE;
    }

    //@@@Evaluate
    // Add profiles to world using strategic info, not editor placements.
    AddProfilesUsingProfileInsertionData();

    PostSchedules();
  }

  if (gubEnemyEncounterCode == ENEMY_AMBUSH_CODE || gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE) {
    if (gMapInformation.sCenterGridNo != -1) {
      CallAvailableEnemiesTo(gMapInformation.sCenterGridNo);
    } else {
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_RED, MSG_ERROR,
                L"Ambush aborted in sector %c%d -- no center point in map.  LC:1",
                gWorldSectorY + 'A' - 1, gWorldSectorX);
#endif
    }
  }

  EndLoadScreen();

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // unpause game
    UnPauseGame();
  }

  gpBattleGroup = NULL;

  if (gfTacticalTraversal) {
    CalculateNonPersistantPBIInfo();
  }

  ScreenMsg(FONT_YELLOW, MSG_DEBUG, L"Current Time is: %d", GetGameTimeInMin());

  AllTeamsLookForAll(TRUE);
}

#define RANDOM_HEAD_MINERS 4
void HandleQuestCodeOnSectorEntry(u8 sNewSectorX, u8 sNewSectorY, int8_t bNewSectorZ) {
  uint8_t ubRandomMiner[RANDOM_HEAD_MINERS] = {106, 156, 157, 158};
  uint8_t ubMiner, ubMinersPlaced;
  uint8_t ubMine, ubThisMine;
  uint8_t cnt;
  struct SOLDIERTYPE *pSoldier;

  if (CheckFact(FACT_ALL_TERRORISTS_KILLED, 0)) {
    // end terrorist quest
    EndQuest(QUEST_KILL_TERRORISTS, (u8)gMercProfiles[CARMEN].sSectorX,
             (u8)gMercProfiles[CARMEN].sSectorY);
    // remove Carmen
    gMercProfiles[CARMEN].sSectorX = 0;
    gMercProfiles[CARMEN].sSectorY = 0;
    gMercProfiles[CARMEN].bSectorZ = 0;
  }

  // are we in a mine sector, on the surface?
  if (IsThereAMineInThisSector(sNewSectorX, sNewSectorY) && (bNewSectorZ == 0)) {
    if (CheckFact(FACT_MINERS_PLACED, 0) == FALSE) {
      // SET HEAD MINER LOCATIONS

      ubThisMine = GetMineIndexForSector(sNewSectorX, sNewSectorY);

      if (ubThisMine != MINE_SAN_MONA)  // San Mona is abandoned
      {
        ubMinersPlaced = 0;

        if (ubThisMine != MINE_ALMA) {
          // Fred Morris is always in the first mine sector we enter, unless that's Alma (then he's
          // randomized, too)
          gMercProfiles[106].sSectorX = sNewSectorX;
          gMercProfiles[106].sSectorY = sNewSectorY;
          gMercProfiles[106].bSectorZ = 0;
          gMercProfiles[106].bTown = gMineLocation[ubThisMine].bAssociatedTown;

          // mark miner as placed
          ubRandomMiner[0] = 0;
          ubMinersPlaced++;
        }

        // assign the remaining (3) miners randomly
        for (ubMine = 0; ubMine < MAX_NUMBER_OF_MINES; ubMine++) {
          if (ubMine == ubThisMine || ubMine == MINE_ALMA || ubMine == MINE_SAN_MONA) {
            // Alma always has Matt as a miner, and we have assigned Fred to the current mine
            // and San Mona is abandoned
            continue;
          }

          do {
            ubMiner = (uint8_t)Random(RANDOM_HEAD_MINERS);
          } while (ubRandomMiner[ubMiner] == 0);

          u8 mineX, mineY;
          GetMineSector(ubMine, &mineX, &mineY);
          gMercProfiles[ubRandomMiner[ubMiner]].sSectorX = mineX;
          gMercProfiles[ubRandomMiner[ubMiner]].sSectorY = mineY;
          gMercProfiles[ubRandomMiner[ubMiner]].bSectorZ = 0;
          gMercProfiles[ubRandomMiner[ubMiner]].bTown = gMineLocation[ubMine].bAssociatedTown;

          // mark miner as placed
          ubRandomMiner[ubMiner] = 0;
          ubMinersPlaced++;

          if (ubMinersPlaced == RANDOM_HEAD_MINERS) {
            break;
          }
        }

        SetFactTrue(FACT_MINERS_PLACED);
      }
    }
  }

  if (CheckFact(FACT_ROBOT_RECRUITED_AND_MOVED, 0) == FALSE) {
    struct SOLDIERTYPE *pRobot;
    pRobot = FindSoldierByProfileID(ROBOT, TRUE);
    if (pRobot) {
      // robot is on our team and we have changed sectors, so we can
      // replace the robot-under-construction in Madlab's sector
      RemoveGraphicFromTempFile(gsRobotGridNo, SEVENTHISTRUCT1, (u8)gMercProfiles[MADLAB].sSectorX,
                                (u8)gMercProfiles[MADLAB].sSectorY, gMercProfiles[MADLAB].bSectorZ);
      SetFactTrue(FACT_ROBOT_RECRUITED_AND_MOVED);
    }
  }

  // Check to see if any player merc has the Chalice; if so,
  // note it as stolen
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      if (FindObj(pSoldier, CHALICE) != ITEM_NOT_FOUND) {
        SetFactTrue(FACT_CHALICE_STOLEN);
      }
    }
  }

  if ((gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS) &&
      CheckFact(FACT_KINGPIN_CAN_SEND_ASSASSINS, 0) &&
      (GetTownIdForSector(sNewSectorX, sNewSectorY) != BLANK_SECTOR) &&
      Random(10 + CountMilitiaInSector3D(sNewSectorX, sNewSectorY, bNewSectorZ)) < 3) {
    DecideOnAssassin();
  }

  if (sNewSectorX == 6 && sNewSectorY == MAP_ROW_C && gubQuest[QUEST_RESCUE_MARIA] == QUESTDONE) {
    // make sure Maria and Angel are gone
    gMercProfiles[MARIA].sSectorX = 0;
    gMercProfiles[ANGEL].sSectorY = 0;
    gMercProfiles[MARIA].sSectorX = 0;
    gMercProfiles[ANGEL].sSectorY = 0;
  }

  if (sNewSectorX == 5 && sNewSectorY == MAP_ROW_D) {
    gubBoxerID[0] = NOBODY;
    gubBoxerID[1] = NOBODY;
    gubBoxerID[2] = NOBODY;
  }

  if (sNewSectorX == 3 && sNewSectorY == MAP_ROW_P) {
    // heal up Elliot if he's been hurt
    if (gMercProfiles[ELLIOT].bLife < gMercProfiles[ELLIOT].bLifeMax) {
      gMercProfiles[ELLIOT].bLife = gMercProfiles[ELLIOT].bLifeMax;
    }
  }

  ResetOncePerConvoRecordsForAllNPCsInLoadedSector();
}

void HandleQuestCodeOnSectorExit(int16_t sOldSectorX, int16_t sOldSectorY, int8_t bOldSectorZ) {
  if (sOldSectorX == KINGPIN_MONEY_SECTOR_X && sOldSectorY == KINGPIN_MONEY_SECTOR_Y &&
      bOldSectorZ == KINGPIN_MONEY_SECTOR_Z) {
    CheckForKingpinsMoneyMissing(TRUE);
  }

  if (sOldSectorX == 13 && sOldSectorY == MAP_ROW_H && bOldSectorZ == 0 &&
      CheckFact(FACT_CONRAD_SHOULD_GO, 0)) {
    // remove Conrad from the map
    gMercProfiles[CONRAD].sSectorX = 0;
    gMercProfiles[CONRAD].sSectorY = 0;
  }

  if (sOldSectorX == HOSPITAL_SECTOR_X && sOldSectorY == HOSPITAL_SECTOR_Y &&
      bOldSectorZ == HOSPITAL_SECTOR_Z) {
    CheckForMissingHospitalSupplies();
  }

  // reset the state of the museum alarm for Eldin's quotes
  SetFactFalse(FACT_MUSEUM_ALARM_WENT_OFF);
}

BOOLEAN EnterSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ) {
  int32_t i;
  UNDERGROUND_SECTORINFO *pNode = NULL;
  char bFilename[50];

  // pause game
  PauseGame();

  // stop time for this frame
  InterruptTime();

  // Setup the tactical existance of RPCs and CIVs in the last sector before moving on to a new
  // sector.
  //@@@Evaluate
  if (gfWorldLoaded) {
    for (i = gTacticalStatus.Team[CIV_TEAM].bFirstID; i <= gTacticalStatus.Team[CIV_TEAM].bLastID;
         i++) {
      if (MercPtrs[i]->bActive && MercPtrs[i]->bInSector) {
        SetupProfileInsertionDataForSoldier(MercPtrs[i]);
      }
    }
  }

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Handle NPC stuff related to changing sectors
    HandleQuestCodeOnSectorEntry(sSectorX, sSectorY, bSectorZ);
  }

  // Begin Load
  BeginLoadScreen();

  // This has tobe done before loadworld, as it will remmove old gridnos if present
  RemoveMercsInSector();

  if (AreInMeanwhile() == FALSE) {
    SetSectorFlag(sSectorX, sSectorY, bSectorZ, SF_ALREADY_VISITED);
  }

  GetMapFileName(sSectorX, sSectorY, bSectorZ, bFilename, TRUE, TRUE);

  // Load the placeholder map if the real map doesn't exist.
  if (!MapExists(bFilename)) {
    if (!MapExists("Placeholder1.dat"))
      AssertMsg(0, "Failed to find Placeholder1.dat (placeholder map).");
  }

  CreateLoadingScreenProgressBar();
#ifdef JA2BETAVERSION
  // set the font
  SetProgressBarMsgAttributes(0, FONT12ARIAL, FONT_MCOLOR_WHITE, 0);

  // Set the tile so we don see the text come up
  SetProgressBarTextDisplayFlag(0, TRUE, TRUE, TRUE);
#endif

  // CreateProgressBar( 0, 160, 380, 480, 400 );
  // #ifdef JA2TESTVERSION
  //	//add more detailed progress bar
  //	DefineProgressBarPanel( 0, 65, 79, 94, 130, 350, 510, 430 );
  //	swprintf( str, L"Loading map:  %S", bFilename );
  //	SetProgressBarTitle( 0, str, FONT12POINT1, FONT_BLACK, FONT_BLACK );
  // #endif
  if (!LoadWorld(bFilename)) {
    return (FALSE);
  }

  // underground?
  if (bSectorZ) {
    pNode = FindUnderGroundSector(sSectorX, sSectorY, bSectorZ);

    // is there a sector?..if so set flag
    if (pNode) {
      pNode->fVisited = TRUE;
    }
  }

  // if we arent loading a saved game
  // ATE: Moved this form above, so that we can have the benefit of
  // changing the world BEFORE adding guys to it...
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Load the current sectors Information From the temporary files
    if (!LoadCurrentSectorsInformationFromTempItemsFile()) {  // The integrity of the temp files
                                                              // have been compromised.  Boot out of
                                                              // the game after warning message.
      InitExitGameDialogBecauseFileHackDetected();
      return TRUE;
    }
  }

  RemoveLoadingScreenProgressBar();
  // RemoveProgressBar( 0 );

  if (gfEnterTacticalPlacementGUI) {
    SetPendingNewScreen(GAME_SCREEN);
    InitTacticalPlacementGUI();
  } else {
    PrepareLoadedSector();
  }

  //	UnPauseGame( );

  // This function will either hide or display the tree tops, depending on the game setting
  SetTreeTopStateForMap();

  return TRUE;  // because the map was loaded.
}

void UpdateMercsInSector(u8 sSectorX, u8 sSectorY, int8_t bSectorZ) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fPOWSquadSet = FALSE;
  uint8_t ubPOWSquad = 0;

  // Remove from interface slot
  RemoveAllPlayersFromSlot();

  // Remove tactical interface stuff
  guiPendingOverrideEvent = I_CHANGE_TO_IDLE;

  // If we are in this function during the loading of a sector
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // DONT set these values
    gusSelectedSoldier = NO_SOLDIER;
    gfGameScreenLocateToSoldier = TRUE;
  }

  // Set all faces intactive
  SetAllAutoFacesInactive();

  if (fUsingEdgePointsForStrategicEntry) {
    BeginMapEdgepointSearch();
  }

  // if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
  {
    for (cnt = 0, pSoldier = MercPtrs[cnt]; cnt < MAX_NUM_SOLDIERS; cnt++, pSoldier++) {
      if (gfRestoringEnemySoldiersFromTempFile &&
          cnt >= gTacticalStatus.Team[ENEMY_TEAM].bFirstID &&
          cnt <= gTacticalStatus.Team[CREATURE_TEAM]
                     .bLastID) {  // Don't update enemies/creatures (consec. teams) if they were
        // just restored via the temp map files...
        continue;
      }
      // Remove old merc, if exists
      RemoveMercSlot(pSoldier);

      pSoldier->bInSector = FALSE;

      if (IsSolActive(pSoldier)) {
        if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
          if (gMapInformation.sCenterGridNo != -1 && gfBlitBattleSectorLocator &&
              (gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
               gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE) &&
              pSoldier->bTeam != CIV_TEAM) {
            pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
            pSoldier->usStrategicInsertionData = gMapInformation.sCenterGridNo;
          } else if (gfOverrideInsertionWithExitGrid) {
            pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
            pSoldier->usStrategicInsertionData = gExitGrid.usGridNo;
          }
        }

        if (GetSolSectorX(pSoldier) == sSectorX && GetSolSectorY(pSoldier) == sSectorY &&
            GetSolSectorZ(pSoldier) == bSectorZ && !pSoldier->fBetweenSectors) {
          gbMercIsNewInThisSector[pSoldier->ubID] = 1;

          UpdateMercInSector(pSoldier, sSectorX, sSectorY, bSectorZ);

          if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
            if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
              if (!fPOWSquadSet) {
                fPOWSquadSet = TRUE;

                // ATE: If we are in i13 - pop up message!
                if (sSectorY == MAP_ROW_I && sSectorX == 13) {
                  DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[POW_MERCS_ARE_HERE], GAME_SCREEN,
                               (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
                } else {
                  AddCharacterToUniqueSquad(pSoldier);
                  ubPOWSquad = pSoldier->bAssignment;
                  pSoldier->bNeutral = FALSE;
                }
              } else {
                if (sSectorY != MAP_ROW_I && sSectorX != 13) {
                  AddCharacterToSquad(pSoldier, ubPOWSquad);
                }
              }

              // ATE: Call actions based on what POW we are on...
              if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTINPROGRESS) {
                // Complete quest
                EndQuest(QUEST_HELD_IN_ALMA, sSectorX, sSectorY);

                // Do action
                HandleNPCDoAction(0, NPC_ACTION_GRANT_EXPERIENCE_3, 0);
              }
            }
          }
        } else {
          pSoldier->bInSector = FALSE;
        }
      }
    }
  }

  if (fUsingEdgePointsForStrategicEntry) {
    EndMapEdgepointSearch();

    // Set to false
    fUsingEdgePointsForStrategicEntry = FALSE;
  }
}

void UpdateMercInSector(struct SOLDIERTYPE *pSoldier, u8 sSectorX, u8 sSectorY, int8_t bSectorZ) {
  BOOLEAN fError = FALSE;
  if (pSoldier->uiStatusFlags & SOLDIER_IS_TACTICALLY_VALID) {
    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
  }
  // OK, determine entrence direction and get sweetspot
  // Only if we are an OK guy to control....
  // SOME CHECKS HERE MUST BE FLESHED OUT......
  if (IsSolActive(pSoldier))  // This was in the if, removed by DEF:  pSoldier->bLife >= OKLIFE &&
  {
    // If we are not in transit...
    if (pSoldier->bAssignment != IN_TRANSIT) {
      // CHECK UBINSERTION CODE..
      if (pSoldier->ubStrategicInsertionCode == INSERTION_CODE_PRIMARY_EDGEINDEX ||
          pSoldier->ubStrategicInsertionCode == INSERTION_CODE_SECONDARY_EDGEINDEX) {
        if (!fUsingEdgePointsForStrategicEntry) {
          // If we are not supposed to use this now, pick something better...
          pSoldier->ubStrategicInsertionCode = (uint8_t)pSoldier->usStrategicInsertionData;
        }
      }

    MAPEDGEPOINT_SEARCH_FAILED:

      if (GetSolProfile(pSoldier) != NO_PROFILE &&
          gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags3 &
              PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE) {
        // override orders
        pSoldier->bOrders = STATIONARY;
      }

      // Use insertion direction from loaded map!
      switch (pSoldier->ubStrategicInsertionCode) {
        case INSERTION_CODE_NORTH:
          pSoldier->sInsertionGridNo = gMapInformation.sNorthGridNo;
          if (!gfEditMode && gMapInformation.sNorthGridNo == -1) fError = TRUE;
          break;
        case INSERTION_CODE_SOUTH:
          pSoldier->sInsertionGridNo = gMapInformation.sSouthGridNo;
          if (!gfEditMode && gMapInformation.sSouthGridNo == -1) fError = TRUE;
          break;
        case INSERTION_CODE_EAST:
          pSoldier->sInsertionGridNo = gMapInformation.sEastGridNo;
          if (!gfEditMode && gMapInformation.sEastGridNo == -1) fError = TRUE;
          break;
        case INSERTION_CODE_WEST:
          pSoldier->sInsertionGridNo = gMapInformation.sWestGridNo;
          if (!gfEditMode && gMapInformation.sWestGridNo == -1) fError = TRUE;
          break;
        case INSERTION_CODE_CENTER:
          pSoldier->sInsertionGridNo = gMapInformation.sCenterGridNo;
          if (!gfEditMode && gMapInformation.sCenterGridNo == -1) fError = TRUE;
          break;
        case INSERTION_CODE_GRIDNO:
          pSoldier->sInsertionGridNo = pSoldier->usStrategicInsertionData;
          break;

        case INSERTION_CODE_PRIMARY_EDGEINDEX:
          pSoldier->sInsertionGridNo = SearchForClosestPrimaryMapEdgepoint(
              pSoldier->sPendingActionData2, (uint8_t)pSoldier->usStrategicInsertionData);
#ifdef JA2BETAVERSION
          {
            char str[256];
            sprintf(str,
                    "%S's primary insertion gridno is %d using %d as initial search gridno and %d "
                    "insertion code.",
                    pSoldier->name, pSoldier->sInsertionGridNo, pSoldier->sPendingActionData2,
                    pSoldier->usStrategicInsertionData);
            DebugMsg(TOPIC_JA2, DBG_INFO, str);
          }
#endif
          if (pSoldier->sInsertionGridNo == NOWHERE) {
            ScreenMsg(FONT_RED, MSG_ERROR,
                      L"Main edgepoint search failed for %s -- substituting entrypoint.",
                      pSoldier->name);
            pSoldier->ubStrategicInsertionCode = (uint8_t)pSoldier->usStrategicInsertionData;
            goto MAPEDGEPOINT_SEARCH_FAILED;
          }
          break;
        case INSERTION_CODE_SECONDARY_EDGEINDEX:
          pSoldier->sInsertionGridNo = SearchForClosestSecondaryMapEdgepoint(
              pSoldier->sPendingActionData2, (uint8_t)pSoldier->usStrategicInsertionData);
#ifdef JA2BETAVERSION
          {
            char str[256];
            sprintf(str,
                    "%S's isolated insertion gridno is %d using %d as initial search gridno and %d "
                    "insertion code.",
                    pSoldier->name, pSoldier->sInsertionGridNo, pSoldier->sPendingActionData2,
                    pSoldier->usStrategicInsertionData);
            DebugMsg(TOPIC_JA2, DBG_INFO, str);
          }
#endif
          if (pSoldier->sInsertionGridNo == NOWHERE) {
            ScreenMsg(FONT_RED, MSG_ERROR,
                      L"Isolated edgepont search failed for %s -- substituting entrypoint.",
                      pSoldier->name);
            pSoldier->ubStrategicInsertionCode = (uint8_t)pSoldier->usStrategicInsertionData;
            goto MAPEDGEPOINT_SEARCH_FAILED;
          }
          break;

        case INSERTION_CODE_ARRIVING_GAME:
          // Are we in Omerta!
          if (sSectorX == gWorldSectorX && gWorldSectorX == 9 && sSectorY == gWorldSectorY &&
              gWorldSectorY == 1 && bSectorZ == gbWorldSectorZ && gbWorldSectorZ == 0) {
            // Try another location and walk into map
            pSoldier->sInsertionGridNo = 4379;
          } else {
            pSoldier->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
            pSoldier->sInsertionGridNo = gMapInformation.sNorthGridNo;
          }
          break;
        case INSERTION_CODE_CHOPPER:
          // Try another location and walk into map
          // Add merc to chopper....
          // pSoldier->sInsertionGridNo = 4058;
          AddMercToHeli(pSoldier->ubID);
          return;
          break;
        default:
          pSoldier->sInsertionGridNo = 12880;
          DebugMsg(TOPIC_JA2, DBG_INFO,
                   String("Improper insertion code %d given to UpdateMercsInSector",
                          pSoldier->ubStrategicInsertionCode));
          break;
      }

      if (fError) {  // strategic insertion failed because it expected to find an entry point.  This
                     // is likely
        // a missing part of the map or possible fault in strategic movement costs, traversal logic,
        // etc.
        wchar_t szEntry[10];
        wchar_t szSector[10];
        int16_t sGridNo;
        GetLoadedSectorString(szSector, ARR_SIZE(szSector));
        if (gMapInformation.sNorthGridNo != -1) {
          swprintf(szEntry, ARR_SIZE(szEntry), L"north");
          sGridNo = gMapInformation.sNorthGridNo;
        } else if (gMapInformation.sEastGridNo != -1) {
          swprintf(szEntry, ARR_SIZE(szEntry), L"east");
          sGridNo = gMapInformation.sEastGridNo;
        } else if (gMapInformation.sSouthGridNo != -1) {
          swprintf(szEntry, ARR_SIZE(szEntry), L"south");
          sGridNo = gMapInformation.sSouthGridNo;
        } else if (gMapInformation.sWestGridNo != -1) {
          swprintf(szEntry, ARR_SIZE(szEntry), L"west");
          sGridNo = gMapInformation.sWestGridNo;
        } else if (gMapInformation.sCenterGridNo != -1) {
          swprintf(szEntry, ARR_SIZE(szEntry), L"center");
          sGridNo = gMapInformation.sCenterGridNo;
        } else {
          ScreenMsg(FONT_RED, MSG_BETAVERSION,
                    L"Sector %s has NO entrypoints -- using precise center of map for %s.",
                    szSector, pSoldier->name);
          pSoldier->sInsertionGridNo = 12880;
          AddSoldierToSector(pSoldier->ubID);
          return;
        }
        pSoldier->sInsertionGridNo = sGridNo;
        switch (pSoldier->ubStrategicInsertionCode) {
          case INSERTION_CODE_NORTH:
            ScreenMsg(
                FONT_RED, MSG_BETAVERSION,
                L"Sector %s doesn't have a north entrypoint -- substituting  %s entrypoint for %s.",
                szSector, szEntry, pSoldier->name);
            break;
          case INSERTION_CODE_EAST:
            ScreenMsg(
                FONT_RED, MSG_BETAVERSION,
                L"Sector %s doesn't have a east entrypoint -- substituting  %s entrypoint for %s.",
                szSector, szEntry, pSoldier->name);
            break;
          case INSERTION_CODE_SOUTH:
            ScreenMsg(
                FONT_RED, MSG_BETAVERSION,
                L"Sector %s doesn't have a south entrypoint -- substituting  %s entrypoint for %s.",
                szSector, szEntry, pSoldier->name);
            break;
          case INSERTION_CODE_WEST:
            ScreenMsg(
                FONT_RED, MSG_BETAVERSION,
                L"Sector %s doesn't have a west entrypoint -- substituting  %s entrypoint for %s.",
                szSector, szEntry, pSoldier->name);
            break;
          case INSERTION_CODE_CENTER:
            ScreenMsg(FONT_RED, MSG_BETAVERSION,
                      L"Sector %s doesn't have a center entrypoint -- substituting  %s entrypoint "
                      L"for %s.",
                      szSector, szEntry, pSoldier->name);
            break;
        }
      }
      // If no insertion direction exists, this is bad!
      if (pSoldier->sInsertionGridNo == -1) {
        DebugMsg(TOPIC_JA2, DBG_INFO,
                 String("Insertion gridno for direction %d not added to map sector %d %d",
                        pSoldier->ubStrategicInsertionCode, sSectorX, sSectorY));
        pSoldier->sInsertionGridNo = 12880;
      }

      AddSoldierToSector(pSoldier->ubID);
    }
  }
}

// Get sector ID string makes a string like 'A9 - OMERTA', or just J11 if no town....
void GetSectorIDString(u8 sSectorX, u8 sSectorY, int8_t bSectorZ, wchar_t *zString, size_t bufSize,
                       BOOLEAN fDetailed) {
  SECTORINFO *pSector = NULL;
  UNDERGROUND_SECTORINFO *pUnderground;
  int8_t bTownNameID;
  int8_t bMineIndex;
  uint8_t ubSectorID = 0;
  uint8_t ubLandType = 0;

  if (sSectorX <= 0 || sSectorY <= 0 || bSectorZ < 0) {
    // swprintf( zString, L"%s", pErrorStrings[0] );
  } else if (bSectorZ != 0) {
    pUnderground = FindUnderGroundSector(sSectorX, sSectorY, bSectorZ);
    if (pUnderground && (pUnderground->fVisited || gfGettingNameFromSaveLoadScreen)) {
      bMineIndex = GetIdOfMineForSector(sSectorX, sSectorY, bSectorZ);
      if (bMineIndex != -1) {
        swprintf(zString, bufSize, L"%c%d: %s %s", 'A' + sSectorY - 1, sSectorX,
                 pTownNames[GetTownAssociatedWithMine(bMineIndex)], pwMineStrings[0]);
      } else
        switch (GetSectorID8(sSectorX, sSectorY)) {
          case SEC_A10:
            swprintf(zString, bufSize, L"A10: %s", pLandTypeStrings[REBEL_HIDEOUT]);
            break;
          case SEC_J9:
            swprintf(zString, bufSize, L"J9: %s", pLandTypeStrings[TIXA_DUNGEON]);
            break;
          case SEC_K4:
            swprintf(zString, bufSize, L"K4: %s", pLandTypeStrings[ORTA_BASEMENT]);
            break;
          case SEC_O3:
            swprintf(zString, bufSize, L"O3: %s", pLandTypeStrings[TUNNEL]);
            break;
          case SEC_P3:
            swprintf(zString, bufSize, L"P3: %s", pLandTypeStrings[SHELTER]);
            break;
          default:
            swprintf(zString, bufSize, L"%c%d: %s", 'A' + sSectorY - 1, sSectorX,
                     pLandTypeStrings[CREATURE_LAIR]);
            break;
        }
    } else {  // Display nothing
      wcscpy(zString, L"");
    }
  } else {
    bTownNameID = GetTownIdForSector(sSectorX, sSectorY);
    ubSectorID = (uint8_t)GetSectorID8(sSectorX, sSectorY);
    pSector = &SectorInfo[ubSectorID];
    ubLandType = pSector->ubTraversability[4];
    swprintf(zString, bufSize, L"%c%d: ", 'A' + sSectorY - 1, sSectorX);

    if (bTownNameID == BLANK_SECTOR) {
      // OK, build string id like J11
      // are we dealing with the unfound towns?
      switch (ubSectorID) {
        case SEC_D2:  // Chitzena SAM
          if (!IsSamSiteFound(SamSiteChitzena))
            wcscat(zString, pLandTypeStrings[TROPICS]);
          else if (fDetailed)
            wcscat(zString, pLandTypeStrings[TROPICS_SAM_SITE]);
          else
            wcscat(zString, pLandTypeStrings[SAM_SITE]);
          break;
        case SEC_D15:  // Drassen SAM
          if (!IsSamSiteFound(SamSiteDrassen))
            wcscat(zString, pLandTypeStrings[SPARSE]);
          else if (fDetailed)
            wcscat(zString, pLandTypeStrings[SPARSE_SAM_SITE]);
          else
            wcscat(zString, pLandTypeStrings[SAM_SITE]);
          break;
        case SEC_I8:  // Cambria SAM
          if (!IsSamSiteFound(SamSiteCambria))
            wcscat(zString, pLandTypeStrings[SAND]);
          else if (fDetailed)
            wcscat(zString, pLandTypeStrings[SAND_SAM_SITE]);
          else
            wcscat(zString, pLandTypeStrings[SAM_SITE]);
          break;
        default:
          wcscat(zString, pLandTypeStrings[ubLandType]);
          break;
      }
    } else {
      switch (ubSectorID) {
        case SEC_B13:
          if (fDetailed)
            wcscat(zString, pLandTypeStrings[DRASSEN_AIRPORT_SITE]);
          else
            wcscat(zString, pTownNames[DRASSEN]);
          break;
        case SEC_F8:
          if (fDetailed)
            wcscat(zString, pLandTypeStrings[CAMBRIA_HOSPITAL_SITE]);
          else
            wcscat(zString, pTownNames[CAMBRIA]);
          break;
        case SEC_J9:  // Tixa
          if (!fFoundTixa)
            wcscat(zString, pLandTypeStrings[SAND]);
          else
            wcscat(zString, pTownNames[TIXA]);
          break;
        case SEC_K4:  // Orta
          if (!fFoundOrta)
            wcscat(zString, pLandTypeStrings[SWAMP]);
          else
            wcscat(zString, pTownNames[ORTA]);
          break;
        case SEC_N3:
          if (fDetailed)
            wcscat(zString, pLandTypeStrings[MEDUNA_AIRPORT_SITE]);
          else
            wcscat(zString, pTownNames[MEDUNA]);
          break;
        default:
          if (ubSectorID == SEC_N4 && IsSamSiteFound(SamSiteMeduna)) {  // Meduna's SAM site
            if (fDetailed)
              wcscat(zString, pLandTypeStrings[MEDUNA_SAM_SITE]);
            else
              wcscat(zString, pLandTypeStrings[SAM_SITE]);
          } else {  // All other towns that are known since beginning of the game.
            wcscat(zString, pTownNames[bTownNameID]);
            if (fDetailed) {
              switch (ubSectorID) {  // Append the word, "mine" for town sectors containing a mine.
                case SEC_B2:
                case SEC_D4:
                case SEC_D13:
                case SEC_H3:
                case SEC_H8:
                case SEC_I14:
                  wcscat(zString, L" ");              // space
                  wcscat(zString, pwMineStrings[0]);  // then "Mine"
                  break;
              }
            }
          }
          break;
      }
    }
  }
}

uint8_t SetInsertionDataFromAdjacentMoveDirection(struct SOLDIERTYPE *pSoldier,
                                                uint8_t ubTacticalDirection, int16_t sAdditionalData) {
  uint8_t ubDirection;
  EXITGRID ExitGrid;

  // Set insertion code
  switch (ubTacticalDirection) {
      // OK, we are using an exit grid - set insertion values...

    case 255:
      if (!GetExitGrid(sAdditionalData, &ExitGrid)) {
        AssertMsg(0, String("No valid Exit grid can be found when one was expected: "
                            "SetInsertionDataFromAdjacentMoveDirection."));
      }
      ubDirection = 255;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
      pSoldier->usStrategicInsertionData = ExitGrid.usGridNo;
      pSoldier->bUseExitGridForReentryDirection = TRUE;
      break;

    case NORTH:
      ubDirection = NORTH_STRATEGIC_MOVE;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
      break;
    case SOUTH:
      ubDirection = SOUTH_STRATEGIC_MOVE;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
      break;
    case EAST:
      ubDirection = EAST_STRATEGIC_MOVE;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_WEST;
      break;
    case WEST:
      ubDirection = WEST_STRATEGIC_MOVE;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_EAST;
      break;
    default:
// Wrong direction given!
#ifdef JA2BETAVERSION
      DebugMsg(
          TOPIC_JA2, DBG_INFO,
          String(
              "Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection",
              ubTacticalDirection));
      ScreenMsg(
          FONT_RED, MSG_ERROR,
          L"Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection",
          ubTacticalDirection);
#endif
      ubDirection = EAST_STRATEGIC_MOVE;
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_WEST;
  }

  return (ubDirection);
}

uint8_t GetInsertionDataFromAdjacentMoveDirection(uint8_t ubTacticalDirection, int16_t sAdditionalData) {
  uint8_t ubDirection;

  // Set insertion code
  switch (ubTacticalDirection) {
      // OK, we are using an exit grid - set insertion values...

    case 255:

      ubDirection = 255;
      break;

    case NORTH:
      ubDirection = NORTH_STRATEGIC_MOVE;
      break;
    case SOUTH:
      ubDirection = SOUTH_STRATEGIC_MOVE;
      break;
    case EAST:
      ubDirection = EAST_STRATEGIC_MOVE;
      break;
    case WEST:
      ubDirection = WEST_STRATEGIC_MOVE;
      break;
    default:
// Wrong direction given!
#ifdef JA2BETAVERSION
      DebugMsg(
          TOPIC_JA2, DBG_INFO,
          String(
              "Improper insertion direction %d given to GetInsertionDataFromAdjacentMoveDirection",
              ubTacticalDirection));
      ScreenMsg(
          FONT_RED, MSG_ERROR,
          L"Improper insertion direction %d given to GetInsertionDataFromAdjacentMoveDirection",
          ubTacticalDirection);
#endif
      ubDirection = EAST_STRATEGIC_MOVE;
  }

  return (ubDirection);
}

uint8_t GetStrategicInsertionDataFromAdjacentMoveDirection(uint8_t ubTacticalDirection,
                                                         int16_t sAdditionalData) {
  uint8_t ubDirection;

  // Set insertion code
  switch (ubTacticalDirection) {
      // OK, we are using an exit grid - set insertion values...

    case 255:

      ubDirection = 255;
      break;

    case NORTH:
      ubDirection = INSERTION_CODE_SOUTH;
      break;
    case SOUTH:
      ubDirection = INSERTION_CODE_NORTH;
      break;
    case EAST:
      ubDirection = INSERTION_CODE_WEST;
      break;
    case WEST:
      ubDirection = INSERTION_CODE_EAST;
      break;
    default:
// Wrong direction given!
#ifdef JA2BETAVERSION
      DebugMsg(
          TOPIC_JA2, DBG_INFO,
          String(
              "Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection",
              ubTacticalDirection));
      ScreenMsg(FONT_RED, MSG_ERROR,
                L"Improper insertion direction %d given to "
                L"GetStrategicInsertionDataFromAdjacentMoveDirection",
                ubTacticalDirection);
#endif
      ubDirection = EAST_STRATEGIC_MOVE;
  }

  return (ubDirection);
}

void JumpIntoAdjacentSector(uint8_t ubTacticalDirection, uint8_t ubJumpCode, int16_t sAdditionalData) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  struct SOLDIERTYPE *pValidSoldier = NULL;
  struct GROUP *pGroup;
  uint32_t uiTraverseTime = 0;
  uint8_t ubDirection;
  EXITGRID ExitGrid;
  int8_t bPrevAssignment;
  uint8_t ubPrevGroupID;

  // Set initial selected
  // ATE: moved this towards top...
  gubPreferredInitialSelectedGuy = (uint8_t)gusSelectedSoldier;

  if (ubJumpCode == JUMP_ALL_LOAD_NEW || ubJumpCode == JUMP_ALL_NO_LOAD) {
    // TODO: Check flags to see if we can jump!
    // Move controllable mercs!
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

    // look for all mercs on the same team,
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      // If we are controllable
      if (OK_CONTROLLABLE_MERC(pSoldier) && GetSolAssignment(pSoldier) == CurrentSquad()) {
        pValidSoldier = pSoldier;
        // This now gets handled by strategic movement.  It is possible that the
        // group won't move instantaneously.
        // pSoldier->sSectorX = sNewX;
        // pSoldier->sSectorY = sNewY;

        ubDirection =
            GetInsertionDataFromAdjacentMoveDirection(ubTacticalDirection, sAdditionalData);
        break;
      }
    }
  } else if ((ubJumpCode == JUMP_SINGLE_LOAD_NEW || ubJumpCode == JUMP_SINGLE_NO_LOAD)) {
    // Use selected soldier...
    // This guy should always be 1 ) selected and 2 ) close enough to exit sector to leave
    if (gusSelectedSoldier != NOBODY) {
      pValidSoldier = MercPtrs[gusSelectedSoldier];
      ubDirection = GetInsertionDataFromAdjacentMoveDirection(ubTacticalDirection, sAdditionalData);
    }

    // save info for desired squad and and time for all single mercs leaving their squad.
    bPrevAssignment = pValidSoldier->bAssignment;
    ubPrevGroupID = pValidSoldier->ubGroupID;

    if (ubJumpCode == JUMP_SINGLE_NO_LOAD) {  // handle soldier moving by themselves
      HandleSoldierLeavingSectorByThemSelf(pValidSoldier);
    } else {  // now add char to a squad all their own
      AddCharacterToUniqueSquad(pValidSoldier);
    }
    if (!pValidSoldier->ubNumTraversalsAllowedToMerge && bPrevAssignment < ON_DUTY) {
      PLAYERGROUP *pPlayer;
      pValidSoldier->ubDesiredSquadAssignment = bPrevAssignment;
      pValidSoldier->ubNumTraversalsAllowedToMerge = 2;
      pGroup = GetGroup(ubPrevGroupID);
      Assert(pGroup);
      Assert(pGroup->fPlayer);
      // Assert( pGroup->ubGroupSize );
      pPlayer = pGroup->pPlayerList;
      while (pPlayer) {
        if (pPlayer->pSoldier != pValidSoldier) {
          pPlayer->pSoldier->ubNumTraversalsAllowedToMerge = 100;
          pPlayer->pSoldier->ubDesiredSquadAssignment = NO_ASSIGNMENT;
        }
        pPlayer = pPlayer->next;
      }
    }
  } else {
    // OK, no jump code here given...
    DebugMsg(TOPIC_JA2, DBG_INFO,
             String("Improper jump code %d given to JumpIntoAdjacentSector", ubJumpCode));
  }

  Assert(pValidSoldier);

  // Now, determine the traversal time.
  pGroup = GetGroup(pValidSoldier->ubGroupID);
  AssertMsg(pGroup, String("%S is not in a valid group (pSoldier->ubGroupID is %d)",
                           pValidSoldier->name, pValidSoldier->ubGroupID));

  // If we are going through an exit grid, don't get traversal direction!
  if (ubTacticalDirection != 255) {
    if (!gbWorldSectorZ) {
      uiTraverseTime = GetSectorMvtTimeForGroup(
          (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY), ubDirection, pGroup);
    } else if (gbWorldSectorZ > 0) {  // We are attempting to traverse in an underground
                                      // environment.  We need to use a complete different
      // method.  When underground, all sectors are instantly adjacent.
      uiTraverseTime = UndergroundTacticalTraversalTime(ubDirection);
    }
    if (uiTraverseTime == 0xffffffff)
      AssertMsg(
          0,
          "Attempting to tactically traverse to adjacent sector, despite being unable to do so.");
  }

  // Alrighty, we want to do whatever our omnipotent player asked us to do
  // this is what the ubJumpCode is for.
  // Regardless of that we were asked to do, we MUST walk OFF ( Ian loves this... )
  // So..... let's setup our people to walk off...
  // We deal with a pGroup here... if an all move or a group...

  // Setup some globals so our callback that deals when guys go off screen is handled....
  // Look in the handler function AllMercsHaveWalkedOffSector() below...
  gpAdjacentGroup = pGroup;
  gubAdjacentJumpCode = ubJumpCode;
  guiAdjacentTraverseTime = uiTraverseTime;
  gubTacticalDirection = ubTacticalDirection;
  gsAdditionalData = sAdditionalData;

  // If normal direction, use it!
  if (ubTacticalDirection != 255) {
    gsAdjacentSectorX = (u8)(gWorldSectorX + DirXIncrementer[ubTacticalDirection]);
    gsAdjacentSectorY = (u8)(gWorldSectorY + DirYIncrementer[ubTacticalDirection]);
    gbAdjacentSectorZ = pValidSoldier->bSectorZ;
  } else {
    // Take directions from exit grid info!
    if (!GetExitGrid(sAdditionalData, &ExitGrid)) {
      AssertMsg(0, String("Told to use exit grid at %d but one does not exist", sAdditionalData));
    }

    gsAdjacentSectorX = ExitGrid.ubGotoSectorX;
    gsAdjacentSectorY = ExitGrid.ubGotoSectorY;
    gbAdjacentSectorZ = ExitGrid.ubGotoSectorZ;
    gusDestExitGridNo = ExitGrid.usGridNo;
  }

  // Give guy(s) orders to walk off sector...
  if (pGroup->fPlayer) {  // For player groups, update the soldier information
    PLAYERGROUP *curr;
    int16_t sGridNo;
    uint8_t ubNum = 0;

    curr = pGroup->pPlayerList;
    while (curr) {
      if (OK_CONTROLLABLE_MERC(curr->pSoldier)) {
        if (ubTacticalDirection != 255) {
          sGridNo = PickGridNoNearestEdge(curr->pSoldier, ubTacticalDirection);

          curr->pSoldier->sPreTraversalGridNo = curr->pSoldier->sGridNo;

          if (sGridNo != NOWHERE) {
            // Save wait code - this will make buddy walk off screen into oblivion
            curr->pSoldier->ubWaitActionToDo = 2;
            // This will set the direction so we know now to move into oblivion
            curr->pSoldier->uiPendingActionData1 = ubTacticalDirection;
          } else {
            AssertMsg(0, String("Failed to get good exit location for adjacentmove"));
          }

          EVENT_GetNewSoldierPath(curr->pSoldier, sGridNo, WALKING);

        } else {
          // Here, get closest location for exit grid....
          sGridNo = FindGridNoFromSweetSpotCloseToExitGrid(curr->pSoldier, sAdditionalData, 10,
                                                           &ubDirection);

          // curr->pSoldier->
          if (sGridNo != NOWHERE) {
            // Save wait code - this will make buddy walk off screen into oblivion
            //	curr->pSoldier->ubWaitActionToDo = 2;
          } else {
            AssertMsg(0, String("Failed to get good exit location for adjacentmove"));
          }

          // Don't worry about walk off screen, just stay at gridno...
          curr->pSoldier->ubWaitActionToDo = 1;

          // Set buddy go!
          gfPlotPathToExitGrid = TRUE;
          EVENT_GetNewSoldierPath(curr->pSoldier, sGridNo, WALKING);
          gfPlotPathToExitGrid = FALSE;
        }
        ubNum++;
      } else {
        // We will remove them later....
      }
      curr = curr->next;
    }

    // ATE: Do another round, removing guys from group that can't go on...
  BEGINNING_LOOP:

    curr = pGroup->pPlayerList;
    while (curr) {
      if (!OK_CONTROLLABLE_MERC(curr->pSoldier)) {
        RemoveCharacterFromSquads(curr->pSoldier);
        goto BEGINNING_LOOP;
      }
      curr = curr->next;
    }

    // OK, setup TacticalOverhead polling system that will notify us once everybody
    // has made it to our destination.
    if (ubTacticalDirection != 255) {
      SetActionToDoOnceMercsGetToLocation(WAIT_FOR_MERCS_TO_WALKOFF_SCREEN, ubNum, ubJumpCode, 0,
                                          0);
    } else {
      // Add new wait action here...
      SetActionToDoOnceMercsGetToLocation(WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO, ubNum, ubJumpCode, 0,
                                          0);
    }

    // Lock UI!
    guiPendingOverrideEvent = LU_BEGINUILOCK;
    HandleTacticalUI();
  }
}

void HandleSoldierLeavingSectorByThemSelf(struct SOLDIERTYPE *pSoldier) {
  // soldier leaving thier squad behind, will rejoin later
  // if soldier in a squad, set the fact they want to return here
  uint8_t ubGroupId;

  if (pSoldier->bAssignment < ON_DUTY) {
    RemoveCharacterFromSquads(pSoldier);

    // are they in a group?..remove from group
    if (pSoldier->ubGroupID != 0) {
      // remove from group
      RemovePlayerFromGroup(pSoldier->ubGroupID, pSoldier);
      pSoldier->ubGroupID = 0;
    }
  } else {
    // otherwise, they are on thier own, not in a squad, simply remove mvt group
    if (pSoldier->ubGroupID &&
        pSoldier->bAssignment != VEHICLE) {  // Can only remove groups if they aren't persistant
                                             // (not in a squad or vehicle)
      // delete group
      RemoveGroup(pSoldier->ubGroupID);
      pSoldier->ubGroupID = 0;
    }
  }

  // set to guard
  AddCharacterToUniqueSquad(pSoldier);

  if (pSoldier->ubGroupID == 0) {
    // create independant group
    ubGroupId = CreateNewPlayerGroupDepartingFromSector((uint8_t)GetSolSectorX(pSoldier),
                                                        (uint8_t)GetSolSectorY(pSoldier));
    AddPlayerToGroup(ubGroupId, pSoldier);
  }

  return;
}

void AllMercsWalkedToExitGrid() {
  PLAYERGROUP *pPlayer;
  BOOLEAN fDone;

  HandlePotentialMoraleHitForSkimmingSectors(gpAdjacentGroup);

  if (gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD) {
    Assert(gpAdjacentGroup);
    pPlayer = gpAdjacentGroup->pPlayerList;
    pPlayer = gpAdjacentGroup->pPlayerList;
    while (pPlayer) {
      SetInsertionDataFromAdjacentMoveDirection(pPlayer->pSoldier, gubTacticalDirection,
                                                gsAdditionalData);

      RemoveSoldierFromTacticalSector(pPlayer->pSoldier, TRUE);

      pPlayer = pPlayer->next;
    }

    SetGroupSectorValue((uint8_t)gsAdjacentSectorX, (uint8_t)gsAdjacentSectorY, gbAdjacentSectorZ,
                        gpAdjacentGroup->ubGroupID);
    AttemptToMergeSeparatedGroups(gpAdjacentGroup, FALSE);

    SetDefaultSquadOnSectorEntry(TRUE);

  } else {
    // Because we are actually loading the new map, and we are physically traversing, we don't want
    // to bring up the prebattle interface when we arrive if there are enemies there.  This flag
    // ignores the initialization of the prebattle interface and clears the flag.
    gfTacticalTraversal = TRUE;
    gpTacticalTraversalGroup = gpAdjacentGroup;

    // Check for any unconcious and/or dead merc and remove them from the current squad, so that
    // they don't get moved to the new sector.
    fDone = FALSE;
    while (!fDone) {
      fDone = FALSE;
      pPlayer = gpAdjacentGroup->pPlayerList;
      while (pPlayer) {
        if (pPlayer->pSoldier->bLife < OKLIFE) {
          AddCharacterToUniqueSquad(pPlayer->pSoldier);
          break;
        }
        pPlayer = pPlayer->next;
      }
      if (!pPlayer) {
        fDone = TRUE;
      }
    }

    // OK, Set insertion direction for all these guys....
    Assert(gpAdjacentGroup);
    pPlayer = gpAdjacentGroup->pPlayerList;
    while (pPlayer) {
      SetInsertionDataFromAdjacentMoveDirection(pPlayer->pSoldier, gubTacticalDirection,
                                                gsAdditionalData);

      pPlayer = pPlayer->next;
    }
    SetGroupSectorValue(gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ,
                        gpAdjacentGroup->ubGroupID);
    AttemptToMergeSeparatedGroups(gpAdjacentGroup, FALSE);

    gFadeOutDoneCallback = DoneFadeOutExitGridSector;
    FadeOutGameScreen();
  }
  if (!PlayerMercsInSector((uint8_t)gsAdjacentSectorX, (uint8_t)gsAdjacentSectorY,
                           (uint8_t)gbAdjacentSectorZ)) {
    HandleLoyaltyImplicationsOfMercRetreat(RETREAT_TACTICAL_TRAVERSAL, gsAdjacentSectorX,
                                           gsAdjacentSectorY, gbAdjacentSectorZ);
  }
  if (gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD) {
    gfTacticalTraversal = FALSE;
    gpTacticalTraversalGroup = NULL;
    gpTacticalTraversalChosenSoldier = NULL;
  }
}

void SetupTacticalTraversalInformation() {
  struct SOLDIERTYPE *pSoldier;
  PLAYERGROUP *pPlayer;
  int32_t sWorldX, sWorldY;
  int16_t sScreenX, sScreenY, sNewGridNo;

  Assert(gpAdjacentGroup);
  pPlayer = gpAdjacentGroup->pPlayerList;
  while (pPlayer) {
    pSoldier = pPlayer->pSoldier;

    SetInsertionDataFromAdjacentMoveDirection(pSoldier, gubTacticalDirection, gsAdditionalData);

    // pass flag that this is a tactical traversal, the path built MUST go in the traversed
    // direction even if longer!
    PlotPathForCharacter(pSoldier, gsAdjacentSectorX, gsAdjacentSectorY, TRUE);

    if (guiAdjacentTraverseTime <= 5) {
      // Determine 'mirror' gridno...
      // Convert to absolute xy
      GetWorldXYAbsoluteScreenXY((int16_t)(pSoldier->sX / CELL_X_SIZE),
                                 (int16_t)(pSoldier->sY / CELL_Y_SIZE), &sScreenX, &sScreenY);

      // Get 'mirror', depending on what direction...
      switch (gubTacticalDirection) {
        case NORTH:
          sScreenY = 1520;
          break;
        case SOUTH:
          sScreenY = 0;
          break;
        case EAST:
          sScreenX = 0;
          break;
        case WEST:
          sScreenX = 3160;
          break;
      }

      // Convert into a gridno again.....
      GetFromAbsoluteScreenXYWorldXY(&sWorldX, &sWorldY, sScreenX, sScreenY);
      sNewGridNo = (int16_t)GETWORLDINDEXFROMWORLDCOORDS(sWorldY, sWorldX);

      // Save this gridNo....
      pSoldier->sPendingActionData2 = sNewGridNo;
      // Copy CODe computed earlier into data
      pSoldier->usStrategicInsertionData = pSoldier->ubStrategicInsertionCode;
      // Now use NEW code....

      pSoldier->ubStrategicInsertionCode =
          CalcMapEdgepointClassInsertionCode(pSoldier->sPreTraversalGridNo);

      if (gubAdjacentJumpCode == JUMP_SINGLE_LOAD_NEW || gubAdjacentJumpCode == JUMP_ALL_LOAD_NEW) {
        fUsingEdgePointsForStrategicEntry = TRUE;
      }
    }

    pPlayer = pPlayer->next;
  }
  if (gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD) {
    gfTacticalTraversal = FALSE;
    gpTacticalTraversalGroup = NULL;
    gpTacticalTraversalChosenSoldier = NULL;
  }
}

void AllMercsHaveWalkedOffSector() {
  PLAYERGROUP *pPlayer;
  BOOLEAN fEnemiesInLoadedSector = FALSE;

  if (NumEnemiesInAnySector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ)) {
    fEnemiesInLoadedSector = TRUE;
  }

  HandleLoyaltyImplicationsOfMercRetreat(RETREAT_TACTICAL_TRAVERSAL, (u8)gWorldSectorX,
                                         (u8)gWorldSectorY, gbWorldSectorZ);

  // Setup strategic traversal information
  if (guiAdjacentTraverseTime <= 5) {
    gfTacticalTraversal = TRUE;
    gpTacticalTraversalGroup = gpAdjacentGroup;

    if (gbAdjacentSectorZ > 0 &&
        guiAdjacentTraverseTime <=
            5) {  // Nasty strategic movement logic doesn't like underground sectors!
      gfUndergroundTacticalTraversal = TRUE;
    }
  }
  ClearMercPathsAndWaypointsForAllInGroup(gpAdjacentGroup);
  AddWaypointToPGroup(gpAdjacentGroup, (uint8_t)gsAdjacentSectorX, (uint8_t)gsAdjacentSectorY);
  if (gbAdjacentSectorZ > 0 &&
      guiAdjacentTraverseTime <=
          5) {  // Nasty strategic movement logic doesn't like underground sectors!
    gfUndergroundTacticalTraversal = TRUE;
  }

  SetupTacticalTraversalInformation();

  // ATE: Added here: donot load another screen if we were told not to....
  if ((gubAdjacentJumpCode == JUMP_ALL_NO_LOAD ||
       gubAdjacentJumpCode ==
           JUMP_SINGLE_NO_LOAD)) {  // Case 1:  Group is leaving sector, but there are other mercs
                                    // in sector and player wants to stay, or
    //         there are other mercs in sector while a battle is in progress.
    pPlayer = gpAdjacentGroup->pPlayerList;
    while (pPlayer) {
      RemoveSoldierFromTacticalSector(pPlayer->pSoldier, TRUE);
      pPlayer = pPlayer->next;
    }
    SetDefaultSquadOnSectorEntry(TRUE);
  } else {
    if (fEnemiesInLoadedSector) {  // We are retreating from a sector with enemies in it and there
                                   // are no mercs left  so
      // warp the game time by 5 minutes to simulate the actual retreat.  This restricts the
      // player from immediately coming back to the same sector they left to perhaps take advantage
      // of the tactical placement gui to get into better position.  Additionally, if there are any
      // enemies in this sector that are part of a movement group, reset that movement group so that
      // they are "in" the sector rather than 75% of the way to the next sector if that is the case.
      ResetMovementForEnemyGroupsInLocation((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);

      if (guiAdjacentTraverseTime > 5) {
        // Because this final group is retreating, simulate extra time to retreat, so they can't
        // immediately come back.
        WarpGameTime(300, WARPTIME_NO_PROCESSING_OF_EVENTS);
      }
    }
    if (guiAdjacentTraverseTime <= 5) {
      // Case 2:  Immediatly loading the next sector
      if (!gbAdjacentSectorZ) {
        uint32_t uiWarpTime;
        uiWarpTime = (GetGameTimeInMin() + 5) * 60 - GetGameTimeInSec();
        WarpGameTime(uiWarpTime, WARPTIME_PROCESS_TARGET_TIME_FIRST);
      } else if (gbAdjacentSectorZ > 0) {
        uint32_t uiWarpTime;
        uiWarpTime = (GetGameTimeInMin() + 1) * 60 - GetGameTimeInSec();
        WarpGameTime(uiWarpTime, WARPTIME_PROCESS_TARGET_TIME_FIRST);
      }

      // Because we are actually loading the new map, and we are physically traversing, we don't
      // want to bring up the prebattle interface when we arrive if there are enemies there.  This
      // flag ignores the initialization of the prebattle interface and clears the flag.
      gFadeOutDoneCallback = DoneFadeOutAdjacentSector;
      FadeOutGameScreen();
    } else {  // Case 3:  Going directly to mapscreen

      // Lock game into mapscreen mode, but after the fade is done.
      gfEnteringMapScreen = TRUE;

      // ATE; Fade FAST....
      SetMusicFadeSpeed(5);
      SetMusicMode(MUSIC_TACTICAL_NOTHING);
    }
  }
}

void DoneFadeOutExitGridSector() {
  SetCurrentWorldSector(gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ);
  if (gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier) {
    if (gTacticalStatus.fEnemyInSector) {
      TacticalCharacterDialogueWithSpecialEvent(gpTacticalTraversalChosenSoldier,
                                                QUOTE_ENEMY_PRESENCE, 0, 0, 0);
    }
  }
  gfTacticalTraversal = FALSE;
  gpTacticalTraversalGroup = NULL;
  gpTacticalTraversalChosenSoldier = NULL;
  FadeInGameScreen();
}

void DoneFadeOutAdjacentSector() {
  uint8_t ubDirection;
  SetCurrentWorldSector(gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ);

  ubDirection =
      GetStrategicInsertionDataFromAdjacentMoveDirection(gubTacticalDirection, gsAdditionalData);
  if (gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier) {
    if (gTacticalStatus.fEnemyInSector) {
      TacticalCharacterDialogueWithSpecialEvent(gpTacticalTraversalChosenSoldier,
                                                QUOTE_ENEMY_PRESENCE, 0, 0, 0);
    }
  }
  gfTacticalTraversal = FALSE;
  gpTacticalTraversalGroup = NULL;
  gpTacticalTraversalChosenSoldier = NULL;

  if (gfCaves) {
    // ATE; Set tactical status flag...
    gTacticalStatus.uiFlags |= IGNORE_ALL_OBSTACLES;
    // Set pathing flag to path through anything....
    gfPathAroundObstacles = FALSE;
  }

  // OK, give our guys new orders...
  if (gpAdjacentGroup->fPlayer) {
    // For player groups, update the soldier information
    PLAYERGROUP *curr;
    uint32_t uiAttempts;
    int16_t sGridNo, sOldGridNo;
    uint8_t ubNum = 0;
    int16_t sWorldX, sWorldY;
    curr = gpAdjacentGroup->pPlayerList;
    while (curr) {
      if (!(curr->pSoldier->uiStatusFlags & SOLDIER_IS_TACTICALLY_VALID)) {
        if (curr->pSoldier->sGridNo != NOWHERE) {
          sGridNo = PickGridNoToWalkIn(curr->pSoldier, ubDirection, &uiAttempts);

          // If the search algorithm failed due to too many attempts, simply reset the
          // the gridno as the destination is a reserved gridno and we will place the
          // merc there without walking into the sector.
          if (sGridNo == NOWHERE && uiAttempts == MAX_ATTEMPTS) {
            sGridNo = curr->pSoldier->sGridNo;
          }

          if (sGridNo != NOWHERE) {
            curr->pSoldier->ubWaitActionToDo = 1;
            // OK, here we have been given a position, a gridno has been given to use as well....
            sOldGridNo = curr->pSoldier->sGridNo;
            sWorldX = CenterX(sGridNo);
            sWorldY = CenterY(sGridNo);
            EVENT_SetSoldierPosition(curr->pSoldier, sWorldX, sWorldY);
            if (sGridNo != sOldGridNo) {
              EVENT_GetNewSoldierPath(curr->pSoldier, sOldGridNo, WALKING);
            }
            ubNum++;
          }
        } else {
#ifdef JA2BETAVERSION
          char str[256];
          sprintf(str, "%S's gridno is NOWHERE, and is attempting to walk into sector.",
                  curr->pSoldier->name);
          DebugMsg(TOPIC_JA2, DBG_INFO, str);
#endif
        }
      }
      curr = curr->next;
    }
    SetActionToDoOnceMercsGetToLocation(WAIT_FOR_MERCS_TO_WALKON_SCREEN, ubNum, 0, 0, 0);
    guiPendingOverrideEvent = LU_BEGINUILOCK;
    HandleTacticalUI();

    // Unset flag here.....
    gfPathAroundObstacles = TRUE;
  }
  FadeInGameScreen();
}

BOOLEAN SoldierOKForSectorExit(struct SOLDIERTYPE *pSoldier, int8_t bExitDirection,
                               uint16_t usAdditionalData) {
  int16_t sXMapPos;
  int16_t sYMapPos;
  int16_t sWorldX;
  int16_t sWorldY;
  uint8_t ubDirection;
  int16_t sGridNo;
  int16_t sAPs;

  // if the soldiers gridno is not NOWHERE
  if (pSoldier->sGridNo == NOWHERE) return (FALSE);

  // OK, anyone on roofs cannot!
  if (pSoldier->bLevel > 0) return (FALSE);

  // get world absolute XY
  ConvertGridNoToXY(pSoldier->sGridNo, &sXMapPos, &sYMapPos);

  // Get screen coordinates for current position of soldier
  GetWorldXYAbsoluteScreenXY(sXMapPos, sYMapPos, &sWorldX, &sWorldY);

  // Check direction
  switch (bExitDirection) {
    case EAST_STRATEGIC_MOVE:

      if (sWorldX < ((gsTRX - gsTLX) - CHECK_DIR_X_DELTA)) {
        // NOT OK, return FALSE
        return (FALSE);
      }
      break;

    case WEST_STRATEGIC_MOVE:

      if (sWorldX > CHECK_DIR_X_DELTA) {
        // NOT OK, return FALSE
        return (FALSE);
      }
      break;

    case SOUTH_STRATEGIC_MOVE:

      if (sWorldY < ((gsBLY - gsTRY) - CHECK_DIR_Y_DELTA)) {
        // NOT OK, return FALSE
        return (FALSE);
      }
      break;

    case NORTH_STRATEGIC_MOVE:

      if (sWorldY > CHECK_DIR_Y_DELTA) {
        // NOT OK, return FALSE
        return (FALSE);
      }
      break;

      // This case is for an exit grid....
      // check if we are close enough.....

    case -1:

      // FOR REALTIME - DO MOVEMENT BASED ON STANCE!
      if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
        pSoldier->usUIMovementMode =
            GetMoveStateBasedOnStance(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight);
      }

      sGridNo =
          FindGridNoFromSweetSpotCloseToExitGrid(pSoldier, usAdditionalData, 10, &ubDirection);

      if (sGridNo == NOWHERE) {
        return (FALSE);
      }

      // ATE: if we are in combat, get cost to move here....
      if (gTacticalStatus.uiFlags & INCOMBAT) {
        // Turn off at end of function...
        sAPs = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                        (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                        pSoldier->bActionPoints);

        if (!EnoughPoints(pSoldier, sAPs, 0, FALSE)) {
          return (FALSE);
        }
      }
      break;
  }
  return (TRUE);
}

// ATE: Returns FALSE if NOBODY is close enough, 1 if ONLY selected guy is and 2 if all on squad
// are...
BOOLEAN OKForSectorExit(int8_t bExitDirection, uint16_t usAdditionalData,
                        uint32_t *puiTraverseTimeInMinutes) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fAtLeastOneMercControllable = FALSE;
  BOOLEAN fOnlySelectedGuy = FALSE;
  struct SOLDIERTYPE *pValidSoldier = NULL;
  uint8_t ubReturnVal = FALSE;
  uint8_t ubNumControllableMercs = 0;
  uint8_t ubNumMercs = 0, ubNumEPCs = 0;
  uint8_t ubPlayerControllableMercsInSquad = 0;

  if (gusSelectedSoldier ==
      NOBODY) {  // must have a selected soldier to be allowed to tactically traverse.
    return FALSE;
  }

  /*
  //Exception code for the two sectors in San Mona that are separated by a cliff.  We want to allow
  strategic
  //traversal, but NOT tactical traversal.  The only way to tactically go from D4 to D5 (or
  viceversa) is to enter
  //the cave entrance.
  if( gWorldSectorX == 4 && gWorldSectorY == 4 && !gbWorldSectorZ && bExitDirection ==
  EAST_STRATEGIC_MOVE )
  {
          gfInvalidTraversal = TRUE;
          return FALSE;
  }
  if( gWorldSectorX == 5 && gWorldSectorY == 4 && !gbWorldSectorZ && bExitDirection ==
  WEST_STRATEGIC_MOVE )
  {
          gfInvalidTraversal = TRUE;
          return FALSE;
  }
  */

  gfInvalidTraversal = FALSE;
  gfLoneEPCAttemptingTraversal = FALSE;
  gubLoneMercAttemptingToAbandonEPCs = 0;
  gbPotentiallyAbandonedEPCSlotID = -1;

  // Look through all mercs and check if they are within range of east end....
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // If we are controllable
    if (OK_CONTROLLABLE_MERC(pSoldier) && GetSolAssignment(pSoldier) == CurrentSquad()) {
      // Need to keep a copy of a good soldier, so we can access it later, and
      // not more than once.
      pValidSoldier = pSoldier;

      ubNumControllableMercs++;

      // We need to keep track of the number of EPCs and mercs in this squad.  If we have
      // only one merc and one or more EPCs, then we can't allow the merc to tactically traverse,
      // if he is the only merc near enough to traverse.
      if (AM_AN_EPC(pSoldier)) {
        ubNumEPCs++;
        // Also record the EPC's slot ID incase we later build a string using the EPC's name.
        gbPotentiallyAbandonedEPCSlotID = (int8_t)cnt;
        if (AM_A_ROBOT(pSoldier) && !CanRobotBeControlled(pSoldier)) {
          gfRobotWithoutControllerAttemptingTraversal = TRUE;
          ubNumControllableMercs--;
          continue;
        }
      } else {
        ubNumMercs++;
      }

      if (SoldierOKForSectorExit(pSoldier, bExitDirection, usAdditionalData)) {
        fAtLeastOneMercControllable++;

        if (cnt == gusSelectedSoldier) {
          fOnlySelectedGuy = TRUE;
        }
      } else {
        struct GROUP *pGroup;

        // ATE: Dont's assume exit grids here...
        if (bExitDirection != -1) {
          // Now, determine if this is a valid path.
          pGroup = GetGroup(pValidSoldier->ubGroupID);
          AssertMsg(pGroup, String("%S is not in a valid group (pSoldier->ubGroupID is %d)",
                                   pValidSoldier->name, pValidSoldier->ubGroupID));
          if (!gbWorldSectorZ) {
            *puiTraverseTimeInMinutes = GetSectorMvtTimeForGroup(
                (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY), bExitDirection, pGroup);
          } else if (gbWorldSectorZ > 1) {  // We are attempting to traverse in an underground
                                            // environment.  We need to use a complete different
            // method.  When underground, all sectors are instantly adjacent.
            *puiTraverseTimeInMinutes = UndergroundTacticalTraversalTime(bExitDirection);
          }
          if (*puiTraverseTimeInMinutes == 0xffffffff) {
            gfInvalidTraversal = TRUE;
            return FALSE;
          }
        } else {
          *puiTraverseTimeInMinutes = 0;  // exit grid travel is instantaneous
        }
      }
    }
  }

  // If we are here, at least one guy is controllable in this sector, at least he can go!
  if (fAtLeastOneMercControllable) {
    ubPlayerControllableMercsInSquad =
        (uint8_t)NumberOfPlayerControllableMercsInSquad(MercPtrs[gusSelectedSoldier]->bAssignment);
    if (fAtLeastOneMercControllable <=
        ubPlayerControllableMercsInSquad) {  // if the selected merc is an EPC and we can only leave
                                             // with that merc, then prevent it
      // as EPCs aren't allowed to leave by themselves.  Instead of restricting this in the
      // exiting sector gui, we restrict it by explaining it with a message box.
      if (AM_AN_EPC(MercPtrs[gusSelectedSoldier])) {
        if (AM_A_ROBOT(pSoldier) && !CanRobotBeControlled(pSoldier)) {
          // gfRobotWithoutControllerAttemptingTraversal = TRUE;
          return FALSE;
        } else if (fAtLeastOneMercControllable < ubPlayerControllableMercsInSquad ||
                   fAtLeastOneMercControllable == 1) {
          gfLoneEPCAttemptingTraversal = TRUE;
          return FALSE;
        }
      } else {  // We previously counted the number of EPCs and mercs, and if the selected merc is
                // not an EPC and there are no
        // other mercs in the squad able to escort the EPCs, we will prohibit this merc from
        // tactically traversing.
        if (ubNumEPCs && ubNumMercs == 1 &&
            fAtLeastOneMercControllable < ubPlayerControllableMercsInSquad) {
          gubLoneMercAttemptingToAbandonEPCs = ubNumEPCs;
          return FALSE;
        }
      }
    }
    if (bExitDirection != -1) {
      struct GROUP *pGroup;
      // Now, determine if this is a valid path.
      pGroup = GetGroup(pValidSoldier->ubGroupID);
      AssertMsg(pGroup, String("%S is not in a valid group (pSoldier->ubGroupID is %d)",
                               pValidSoldier->name, pValidSoldier->ubGroupID));
      if (!gbWorldSectorZ) {
        *puiTraverseTimeInMinutes = GetSectorMvtTimeForGroup(
            (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY), bExitDirection, pGroup);
      } else if (gbWorldSectorZ > 0) {  // We are attempting to traverse in an underground
                                        // environment.  We need to use a complete different
        // method.  When underground, all sectors are instantly adjacent.
        *puiTraverseTimeInMinutes = UndergroundTacticalTraversalTime(bExitDirection);
      }
      if (*puiTraverseTimeInMinutes == 0xffffffff) {
        gfInvalidTraversal = TRUE;
        ubReturnVal = FALSE;
      } else {
        ubReturnVal = TRUE;
      }
    } else {
      ubReturnVal = TRUE;
      *puiTraverseTimeInMinutes = 0;  // exit grid travel is instantaneous
    }
  }

  if (ubReturnVal) {
    // Default to FALSE again, until we see that we have
    ubReturnVal = FALSE;

    if (fAtLeastOneMercControllable) {
      // Do we contain the selected guy?
      if (fOnlySelectedGuy) {
        ubReturnVal = 1;
      }
      // Is the whole squad able to go here?
      if (fAtLeastOneMercControllable == ubPlayerControllableMercsInSquad) {
        ubReturnVal = 2;
      }
    }
  }

  return (ubReturnVal);
}

void SetupNewStrategicGame() {
  u8 sSectorX, sSectorY;

  // Set all sectors as enemy controlled
  for (sSectorX = 0; sSectorX < MAP_WORLD_X; sSectorX++) {
    for (sSectorY = 0; sSectorY < MAP_WORLD_Y; sSectorY++) {
      SetSectorEnemyControlled(sSectorX, sSectorY, true);
    }
  }

  // Initialize the game time
  InitNewGameClock();
  // Remove all events
  DeleteAllStrategicEvents();

  // Set up all events that get processed daily...
  //.............................................
  BuildDayLightLevels();
  // Check for quests each morning
  AddEveryDayStrategicEvent(EVENT_CHECKFORQUESTS, QUEST_CHECK_EVENT_TIME, 0);
  // Some things get updated in the very early morning
  AddEveryDayStrategicEvent(EVENT_DAILY_EARLY_MORNING_EVENTS, EARLY_MORNING_TIME, 0);
  // Daily Update BobbyRay Inventory
  AddEveryDayStrategicEvent(EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY, BOBBYRAY_UPDATE_TIME, 0);
  // Daily Update of the M.E.R.C. site.
  AddEveryDayStrategicEvent(EVENT_DAILY_UPDATE_OF_MERC_SITE, 0, 0);
  // Daily update of insured mercs
  AddEveryDayStrategicEvent(EVENT_HANDLE_INSURED_MERCS, INSURANCE_UPDATE_TIME, 0);
  // Daily update of mercs
  AddEveryDayStrategicEvent(EVENT_MERC_DAILY_UPDATE, 0, 0);
  // Daily mine production processing events
  AddEveryDayStrategicEvent(EVENT_SETUP_MINE_INCOME, 0, 0);
  // Daily merc reputation processing events
  AddEveryDayStrategicEvent(EVENT_SETUP_TOWN_OPINION, 0, 0);
  // Daily checks for E-mail from Enrico
  AddEveryDayStrategicEvent(EVENT_ENRICO_MAIL, ENRICO_MAIL_TIME, 0);

  // Hourly update of all sorts of things
  AddPeriodStrategicEvent(EVENT_HOURLY_UPDATE, 60, 0);
  AddPeriodStrategicEvent(EVENT_QUARTER_HOUR_UPDATE, 15, 0);

  // Clear any possible battle locator
  gfBlitBattleSectorLocator = FALSE;

  StrategicTurnsNewGame();
}

BOOLEAN CanGoToTacticalInSector(int16_t sX, int16_t sY, uint8_t ubZ) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // if not a valid sector
  if ((sX < 1) || (sX > 16) || (sY < 1) || (sY > 16) || (ubZ > 3)) {
    return (FALSE);
  }

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all living, fighting mercs on player's team.  Robot and EPCs qualify!
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // ARM: now allows loading of sector with all mercs below OKLIFE as long as they're alive
    if ((IsSolActive(pSoldier) && pSoldier->bLife) &&
        !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (pSoldier->bAssignment != IN_TRANSIT) &&
        (pSoldier->bAssignment != ASSIGNMENT_POW) && (pSoldier->bAssignment != ASSIGNMENT_DEAD) &&
        !SoldierAboardAirborneHeli(pSoldier)) {
      if (!pSoldier->fBetweenSectors && GetSolSectorX(pSoldier) == sX &&
          GetSolSectorY(pSoldier) == sY && GetSolSectorZ(pSoldier) == ubZ) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void UpdateAirspaceControl(void) {
  UpdateAirspaceControlMap();

  // check if currently selected arrival sector still has secure airspace

  // if it's not enemy air controlled
  if (IsSectorEnemyAirControlled(gsMercArriveSectorX, gsMercArriveSectorY)) {
    // NOPE!
    wchar_t sMsgString[256], sMsgSubString1[64], sMsgSubString2[64];

    // get the name of the old sector
    GetSectorIDString(gsMercArriveSectorX, gsMercArriveSectorY, 0, sMsgSubString1,
                      ARR_SIZE(sMsgSubString1), FALSE);

    // move the landing zone over to Omerta
    gsMercArriveSectorX = 9;
    gsMercArriveSectorY = 1;

    // get the name of the new sector
    GetSectorIDString(gsMercArriveSectorX, gsMercArriveSectorY, 0, sMsgSubString2,
                      ARR_SIZE(sMsgSubString2), FALSE);

    // now build the string
    swprintf(sMsgString, ARR_SIZE(sMsgString), pBullseyeStrings[4], sMsgSubString1, sMsgSubString2);

    // confirm the change with overlay message
    DoScreenIndependantMessageBox(sMsgString, MSG_BOX_FLAG_OK, NULL);

    // update position of bullseye
    SetMapPanelDirty(true);

    // update destination column for any mercs in transit
    fTeamPanelDirty = TRUE;
  }

  // ARM: airspace control now affects refueling site availability, so update that too with every
  // change!
  UpdateRefuelSiteAvailability();
}

typedef struct strategicmapelement StrategicMapElement;

BOOLEAN SaveStrategicInfoToSavedFile(FileID hFile) {
  uint32_t uiNumBytesWritten = 0;
  StrategicMapElement strategicMap[MAP_WORLD_X * MAP_WORLD_Y];
  uint32_t uiSize = sizeof(strategicMap);

  // copy data
  for (int i = 0; i < GetSamSiteCount(); i++) {
    u8 sX = GetSamSiteX(i);
    u8 sY = GetSamSiteY(i);
    SectorID16 sector = GetSectorID16(sX, sY);
    strategicMap[sector].__only_storage_bSAMCondition = GetSamCondition(i);
  }

  for (int y = 1; y < 17; y++) {
    for (int x = 1; x < 17; x++) {
      SectorID16 sector = GetSectorID16(x, y);
      strategicMap[sector].__only_storage_fEnemyControlled = IsSectorEnemyControlled(x, y);
    }
  }

  // Save the strategic map information
  File_Write(hFile, strategicMap, uiSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiSize) {
    return (FALSE);
  }

  // Save the Sector Info
  {
    // copying actual data about militia count
    for (u8 y = 1; y < 17; y++) {
      for (u8 x = 1; x < 17; x++) {
        SectorID8 sectorID = GetSectorID8(x, y);
        struct MilitiaCount milCount = GetMilitiaInSector(x, y);
        SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[0] = milCount.green;
        SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[1] = milCount.regular;
        SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[2] = milCount.elite;
        SectorInfo[sectorID]._only_savedgame_fMilitiaTrainingPaid =
            IsMilitiaTrainingPayedForSector(x, y);
      }
    }

    uiSize = sizeof(SECTORINFO) * 256;
    File_Write(hFile, SectorInfo, uiSize, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiSize) {
      return (FALSE);
    }
  }

  // Save the SAM Controlled Sector Information
  uiSize = MAP_WORLD_X * MAP_WORLD_Y;
  File_Seek(hFile, uiSize, FILE_SEEK_CURRENT);

  // Save the fFoundOrta
  File_Write(hFile, &fFoundOrta, sizeof(BOOLEAN), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(BOOLEAN)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadStrategicInfoFromSavedFile(FileID hFile) {
  uint32_t uiNumBytesRead = 0;
  StrategicMapElement strategicMap[MAP_WORLD_X * MAP_WORLD_Y];
  uint32_t uiSize = sizeof(strategicMap);

  // Load the strategic map information
  File_Read(hFile, strategicMap, uiSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiSize) {
    return (FALSE);
  }

  // copy data
  for (int i = 0; i < GetSamSiteCount(); i++) {
    u8 sX = GetSamSiteX(i);
    u8 sY = GetSamSiteY(i);
    SectorID16 sector = GetSectorID16(sX, sY);
    SetSamCondition(i, strategicMap[sector].__only_storage_bSAMCondition);
  }
  for (int y = 1; y < 17; y++) {
    for (int x = 1; x < 17; x++) {
      SectorID16 sector = GetSectorID16(x, y);
      SetSectorEnemyControlled(x, y, strategicMap[sector].__only_storage_fEnemyControlled);
    }
  }

  // Load the Sector Info
  {
    uiSize = sizeof(SECTORINFO) * 256;
    File_Read(hFile, SectorInfo, uiSize, &uiNumBytesRead);
    if (uiNumBytesRead != uiSize) {
      return (FALSE);
    }

    // copying actual data about militia
    for (u8 y = 1; y < 17; y++) {
      for (u8 x = 1; x < 17; x++) {
        SectorID8 sectorID = GetSectorID8(x, y);
        struct MilitiaCount milCount = {
            SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[0],
            SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[1],
            SectorInfo[sectorID]._only_savedgame_ubNumberOfCivsAtLevel[2],
        };
        SetMilitiaInSector(x, y, milCount);
        SetMilitiaTrainingPayedForSector(
            x, y, SectorInfo[sectorID]._only_savedgame_fMilitiaTrainingPaid != 0);
      }
    }
  }

  // Load the SAM Controlled Sector Information
  uiSize = MAP_WORLD_X * MAP_WORLD_Y;
  File_Seek(hFile, uiSize, FILE_SEEK_CURRENT);

  // Load the fFoundOrta
  File_Read(hFile, &fFoundOrta, sizeof(BOOLEAN), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(BOOLEAN)) {
    return (FALSE);
  }

  return (TRUE);
}

int16_t PickGridNoNearestEdge(struct SOLDIERTYPE *pSoldier, uint8_t ubTacticalDirection) {
  int16_t sGridNo, sStartGridNo, sOldGridNo;
  int8_t bOdd = 1, bOdd2 = 1;
  uint8_t bAdjustedDist = 0;
  uint32_t cnt;

  switch (ubTacticalDirection) {
    case EAST:

      sGridNo = pSoldier->sGridNo;
      sStartGridNo = pSoldier->sGridNo;
      sOldGridNo = pSoldier->sGridNo;

      // Move directly to the right!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo -= WORLD_COLS;
        } else {
          sGridNo++;
        }

        bOdd = (int8_t)!bOdd;
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      do {
        // OK, here we go back one, check for OK destination...
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
            FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                         PATH_THROUGH_PEOPLE)) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate up/down )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);

      } while (TRUE);

      break;

    case WEST:

      sGridNo = pSoldier->sGridNo;
      sStartGridNo = pSoldier->sGridNo;
      sOldGridNo = pSoldier->sGridNo;

      // Move directly to the left!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo += WORLD_COLS;
        } else {
          sGridNo--;
        }

        bOdd = (int8_t)!bOdd;
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      do {
        // OK, here we go back one, check for OK destination...
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
            FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                         PATH_THROUGH_PEOPLE)) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate up/down )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);

      } while (TRUE);

      break;

    case NORTH:

      sGridNo = pSoldier->sGridNo;
      sStartGridNo = pSoldier->sGridNo;
      sOldGridNo = pSoldier->sGridNo;

      // Move directly to the left!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo -= WORLD_COLS;
        } else {
          sGridNo--;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      do {
        // OK, here we go back one, check for OK destination...
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
            FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                         PATH_THROUGH_PEOPLE)) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate left/right )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);

      } while (TRUE);

      break;

    case SOUTH:

      sGridNo = pSoldier->sGridNo;
      sStartGridNo = pSoldier->sGridNo;
      sOldGridNo = pSoldier->sGridNo;

      // Move directly to the left!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo += WORLD_COLS;
        } else {
          sGridNo++;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      do {
        // OK, here we go back one, check for OK destination...
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
            FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                         PATH_THROUGH_PEOPLE)) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate left/right )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);

      } while (TRUE);

      break;
  }

  return (NOWHERE);
}

void AdjustSoldierPathToGoOffEdge(struct SOLDIERTYPE *pSoldier, int16_t sEndGridNo,
                                  uint8_t ubTacticalDirection) {
  int16_t sNewGridNo, sTempGridNo;
  int32_t iLoop;

  // will this path segment actually take us to our desired destination in the first place?
  if (pSoldier->usPathDataSize + 2 > MAX_PATH_LIST_SIZE) {
    sTempGridNo = pSoldier->sGridNo;

    for (iLoop = 0; iLoop < pSoldier->usPathDataSize; iLoop++) {
      sTempGridNo += (int16_t)DirectionInc(pSoldier->usPathingData[iLoop]);
    }

    if (sTempGridNo == sEndGridNo) {
      // we can make it, but there isn't enough path room for the two steps required.
      // truncate our path so there's guaranteed the merc will have to generate another
      // path later on...
      pSoldier->usPathDataSize -= 4;
      return;
    } else {
      // can't even make it there with these 30 tiles of path, abort...
      return;
    }
  }

  switch (ubTacticalDirection) {
    case EAST:

      sNewGridNo = NewGridNo((uint16_t)sEndGridNo, (uint16_t)DirectionInc((uint8_t)NORTHEAST));

      if (OutOfBounds(sEndGridNo, sNewGridNo)) {
        return;
      }

      pSoldier->usPathingData[pSoldier->usPathDataSize] = NORTHEAST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      sTempGridNo = NewGridNo((uint16_t)sNewGridNo, (uint16_t)DirectionInc((uint8_t)NORTHEAST));

      if (OutOfBounds(sNewGridNo, sTempGridNo)) {
        return;
      }
      sNewGridNo = sTempGridNo;

      pSoldier->usPathingData[pSoldier->usPathDataSize] = NORTHEAST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      break;

    case WEST:

      sNewGridNo = NewGridNo((uint16_t)sEndGridNo, (uint16_t)DirectionInc((uint8_t)SOUTHWEST));

      if (OutOfBounds(sEndGridNo, sNewGridNo)) {
        return;
      }

      pSoldier->usPathingData[pSoldier->usPathDataSize] = SOUTHWEST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      sTempGridNo = NewGridNo((uint16_t)sNewGridNo, (uint16_t)DirectionInc((uint8_t)SOUTHWEST));

      if (OutOfBounds(sNewGridNo, sTempGridNo)) {
        return;
      }
      sNewGridNo = sTempGridNo;

      pSoldier->usPathingData[pSoldier->usPathDataSize] = SOUTHWEST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;
      break;

    case NORTH:

      sNewGridNo = NewGridNo((uint16_t)sEndGridNo, (uint16_t)DirectionInc((uint8_t)NORTHWEST));

      if (OutOfBounds(sEndGridNo, sNewGridNo)) {
        return;
      }

      pSoldier->usPathingData[pSoldier->usPathDataSize] = NORTHWEST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      sTempGridNo = NewGridNo((uint16_t)sNewGridNo, (uint16_t)DirectionInc((uint8_t)NORTHWEST));

      if (OutOfBounds(sNewGridNo, sTempGridNo)) {
        return;
      }
      sNewGridNo = sTempGridNo;

      pSoldier->usPathingData[pSoldier->usPathDataSize] = NORTHWEST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      break;

    case SOUTH:

      sNewGridNo = NewGridNo((uint16_t)sEndGridNo, (uint16_t)DirectionInc((uint8_t)SOUTHEAST));

      if (OutOfBounds(sEndGridNo, sNewGridNo)) {
        return;
      }

      pSoldier->usPathingData[pSoldier->usPathDataSize] = SOUTHEAST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;

      sTempGridNo = NewGridNo((uint16_t)sNewGridNo, (uint16_t)DirectionInc((uint8_t)SOUTHEAST));

      if (OutOfBounds(sNewGridNo, sTempGridNo)) {
        return;
      }
      sNewGridNo = sTempGridNo;

      pSoldier->usPathingData[pSoldier->usPathDataSize] = SOUTHEAST;
      pSoldier->usPathDataSize++;
      pSoldier->sFinalDestination = sNewGridNo;
      pSoldier->usActionData = sNewGridNo;
      break;
  }
}

int16_t PickGridNoToWalkIn(struct SOLDIERTYPE *pSoldier, uint8_t ubInsertionDirection,
                         uint32_t *puiNumAttempts) {
  int16_t sGridNo, sStartGridNo, sOldGridNo;
  int8_t bOdd = 1, bOdd2 = 1;
  uint8_t bAdjustedDist = 0;
  uint32_t cnt;

  *puiNumAttempts = 0;

  switch (ubInsertionDirection) {
    // OK, we're given a direction on visible map, let's look for the first oone
    // we find that is just on the start of visible map...
    case INSERTION_CODE_WEST:

      sGridNo = (int16_t)pSoldier->sGridNo;
      sStartGridNo = (int16_t)pSoldier->sGridNo;
      sOldGridNo = (int16_t)pSoldier->sGridNo;

      // Move directly to the left!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo += WORLD_COLS;
        } else {
          sGridNo--;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      while (*puiNumAttempts < MAX_ATTEMPTS) {
        (*puiNumAttempts)++;
        // OK, here we go back one, check for OK destination...
        if ((gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES) ||
            (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
             FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                          PATH_THROUGH_PEOPLE))) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate up/down )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);
      }
      return NOWHERE;

    case INSERTION_CODE_EAST:

      sGridNo = (int16_t)pSoldier->sGridNo;
      sStartGridNo = (int16_t)pSoldier->sGridNo;
      sOldGridNo = (int16_t)pSoldier->sGridNo;

      // Move directly to the right!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo -= WORLD_COLS;
        } else {
          sGridNo++;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      while (*puiNumAttempts < MAX_ATTEMPTS) {
        (*puiNumAttempts)++;
        // OK, here we go back one, check for OK destination...
        if ((gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES) ||
            (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
             FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                          PATH_THROUGH_PEOPLE))) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate up/down )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS - 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS + 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);
      }
      return NOWHERE;

    case INSERTION_CODE_NORTH:

      sGridNo = (int16_t)pSoldier->sGridNo;
      sStartGridNo = (int16_t)pSoldier->sGridNo;
      sOldGridNo = (int16_t)pSoldier->sGridNo;

      // Move directly to the up!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo -= WORLD_COLS;
        } else {
          sGridNo--;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      while (*puiNumAttempts < MAX_ATTEMPTS) {
        (*puiNumAttempts)++;
        // OK, here we go back one, check for OK destination...
        if ((gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES) ||
            (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
             FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                          PATH_THROUGH_PEOPLE))) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate left/right )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS + 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS - 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);
      }
      return NOWHERE;

    case INSERTION_CODE_SOUTH:

      sGridNo = (int16_t)pSoldier->sGridNo;
      sStartGridNo = (int16_t)pSoldier->sGridNo;
      sOldGridNo = (int16_t)pSoldier->sGridNo;

      // Move directly to the down!
      while (GridNoOnVisibleWorldTile(sGridNo)) {
        sOldGridNo = sGridNo;

        if (bOdd) {
          sGridNo += WORLD_COLS;
        } else {
          sGridNo++;
        }

        bOdd = (int8_t)(!bOdd);
      }

      sGridNo = sOldGridNo;
      sStartGridNo = sOldGridNo;

      while (*puiNumAttempts < MAX_ATTEMPTS) {
        (*puiNumAttempts)++;
        // OK, here we go back one, check for OK destination...
        if ((gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES) ||
            (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel) &&
             FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE,
                          PATH_THROUGH_PEOPLE))) {
          return (sGridNo);
        }

        // If here, try another place!
        // ( alternate left/right )
        if (bOdd2) {
          bAdjustedDist++;

          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo - WORLD_COLS + 1);
          }
        } else {
          sGridNo = sStartGridNo;

          for (cnt = 0; cnt < bAdjustedDist; cnt++) {
            sGridNo = (int16_t)(sGridNo + WORLD_COLS - 1);
          }
        }

        bOdd2 = (int8_t)(!bOdd2);
      }
      return NOWHERE;
  }

  // Unhandled exit
  *puiNumAttempts = 0;

  return (NOWHERE);
}

void GetLoadedSectorString(wchar_t* pString, size_t bufSize) {
  if (!gfWorldLoaded) {
    swprintf(pString, bufSize, L"");
    return;
  }
  if (gbWorldSectorZ) {
    swprintf(pString, bufSize, L"%c%d_b%d", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ);
  } else if (!gbWorldSectorZ) {
    swprintf(pString, bufSize, L"%c%d", gWorldSectorY + 'A' - 1, gWorldSectorX);
  }
}

void HandleSlayDailyEvent(void) {
  struct SOLDIERTYPE *pSoldier = NULL;

  // grab slay
  pSoldier = FindSoldierByProfileID(64, TRUE);

  if (pSoldier == NULL) {
    return;
  }

  // valid soldier?
  if ((IsSolActive(pSoldier) == FALSE) || (pSoldier->bLife == 0) ||
      (GetSolAssignment(pSoldier) == IN_TRANSIT) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
    // no
    return;
  }

  // ATE: This function is used to check for the ultimate last day SLAY can stay for
  // he may decide to leave randomly while asleep...
  // if the user hasnt renewed yet, and is still leaving today
  if ((pSoldier->iEndofContractTime / 1440) <= (int32_t)GetGameTimeInDays()) {
    pSoldier->ubLeaveHistoryCode = HISTORY_SLAY_MYSTERIOUSLY_LEFT;
    TacticalCharacterDialogueWithSpecialEvent(
        pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP, 0, 0);
  }
}

BOOLEAN IsSectorDesert(u8 sSectorX, u8 sSectorY) {
  if (SectorInfo[GetSectorID8(sSectorX, sSectorY)].ubTraversability[THROUGH_STRATEGIC_MOVE] ==
      SAND) {
    // desert
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN HandleDefiniteUnloadingOfWorld(uint8_t ubUnloadCode) {
  int32_t i;

  // clear tactical queue
  ClearEventQueue();

  // ATE: End all bullets....
  DeleteAllBullets();

  // End all physics objects...
  RemoveAllPhysicsObjects();

  RemoveAllActiveTimedBombs();

  // handle any quest stuff here so world items can be affected
  HandleQuestCodeOnSectorExit(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

  // if we arent loading a saved game
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Clear any potential battle flags.  They will be set if necessary.
    gTacticalStatus.fEnemyInSector = FALSE;
    gTacticalStatus.uiFlags &= ~INCOMBAT;
  }

  if (ubUnloadCode == ABOUT_TO_LOAD_NEW_MAP) {
    // if we arent loading a saved game
    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      // Save the current sectors Item list to a temporary file, if its not the first time in
      SaveCurrentSectorsInformationToTempItemFile();

      // Update any mercs currently in sector, their profile info...
      UpdateSoldierPointerDataIntoProfile(FALSE);
    }
  } else if (ubUnloadCode == ABOUT_TO_TRASH_WORLD) {
    // Save the current sectors open temp files to the disk
    if (!SaveCurrentSectorsInformationToTempItemFile()) {
      ScreenMsg(FONT_MCOLOR_WHITE, MSG_TESTVERSION,
                L"ERROR in SaveCurrentSectorsInformationToTempItemFile()");
      return FALSE;
    }

    // Setup the tactical existance of the current soldier.
    //@@@Evaluate
    for (i = gTacticalStatus.Team[CIV_TEAM].bFirstID; i <= gTacticalStatus.Team[CIV_TEAM].bLastID;
         i++) {
      if (MercPtrs[i]->bActive && MercPtrs[i]->bInSector) {
        SetupProfileInsertionDataForSoldier(MercPtrs[i]);
      }
    }

    gfBlitBattleSectorLocator = FALSE;
  }

  // Handle cases for both types of unloading
  if (GetTeamSide(MILITIA_TEAM) != 0) {
    // handle militia defections and reset team to friendly
    HandleMilitiaDefections(GetLoadedSectorX(), GetLoadedSectorY());
    SetTeamSide(MILITIA_TEAM, 0);
  } else if (!IsGoingToAutoresolve()) {
    // Don't promote militia if we are going directly
    // to autoresolve to finish the current battle.
    HandleMilitiaPromotions(GetLoadedSectorX(), GetLoadedSectorY());
  }
  return TRUE;
}

BOOLEAN HandlePotentialBringUpAutoresolveToFinishBattle() {
  int32_t i;

  // We don't have mercs in the sector.  Now, we check to see if there are BOTH enemies and
  // militia. If both co-exist in the sector, then make them fight for control of the sector via
  // autoresolve.
  for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
       i <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife) {
      if (MercPtrs[i]->sSectorX == gWorldSectorX && MercPtrs[i]->sSectorY == gWorldSectorY &&
          MercPtrs[i]->bSectorZ == gbWorldSectorZ) {  // We have enemies, now look for militia!
        for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
             i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
          if (MercPtrs[i]->bActive && MercPtrs[i]->bLife && MercPtrs[i]->bSide == OUR_TEAM) {
            if (MercPtrs[i]->sSectorX == gWorldSectorX && MercPtrs[i]->sSectorY == gWorldSectorY &&
                MercPtrs[i]->bSectorZ ==
                    gbWorldSectorZ) {  // We have militia and enemies and no mercs!  Let's finish
                                       // this battle in autoresolve.
              gfEnteringMapScreen = TRUE;
              gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
              gfAutomaticallyStartAutoResolve = TRUE;
              gfUsePersistantPBI = FALSE;
              gubPBSectorX = (uint8_t)gWorldSectorX;
              gubPBSectorY = (uint8_t)gWorldSectorY;
              gubPBSectorZ = (uint8_t)gbWorldSectorZ;
              gfBlitBattleSectorLocator = TRUE;
              gfTransferTacticalOppositionToAutoResolve = TRUE;
              if (gubEnemyEncounterCode != CREATURE_ATTACK_CODE) {
                gubEnemyEncounterCode = ENEMY_INVASION_CODE;  // has to be, if militia are here.
              } else {
                // DoScreenIndependantMessageBox( gzLateLocalizedString[ 39 ], MSG_BOX_FLAG_OK,
                // MapScreenDefaultOkBoxCallback );
              }

              return (TRUE);
            }
          }
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN CheckAndHandleUnloadingOfCurrentWorld() {
  int32_t i;
  u8 sBattleSectorX, sBattleSectorY;
  i8 sBattleSectorZ;

  // Don't bother checking this if we don't have a world loaded.
  if (!gfWorldLoaded) {
    return FALSE;
  }

  if (gTacticalStatus.fDidGameJustStart && gWorldSectorX == 9 && gWorldSectorY == 1 &&
      !gbWorldSectorZ) {
    return FALSE;
  }

  GetCurrentBattleSectorXYZ(&sBattleSectorX, &sBattleSectorY, &sBattleSectorZ);

  if (guiCurrentScreen == AUTORESOLVE_SCREEN) {  // The user has decided to let the game
                                                 // autoresolve the current battle.
    if (gWorldSectorX == sBattleSectorX && gWorldSectorY == sBattleSectorY &&
        gbWorldSectorZ == sBattleSectorZ) {
      for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
           i++) {  // If we have a live and valid soldier
        if (MercPtrs[i]->bActive && MercPtrs[i]->bLife && !MercPtrs[i]->fBetweenSectors &&
            !(MercPtrs[i]->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(MercPtrs[i]) &&
            !AM_AN_EPC(MercPtrs[i])) {
          if (MercPtrs[i]->sSectorX == gWorldSectorX && MercPtrs[i]->sSectorY == gWorldSectorY &&
              MercPtrs[i]->bSectorZ == gbWorldSectorZ) {
            RemoveSoldierFromGridNo(MercPtrs[i]);
            InitSoldierOppList(MercPtrs[i]);
          }
        }
      }
    }
  } else {  // Check and see if we have any live mercs in the sector.
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {  // If we have a live and valid soldier
      if (MercPtrs[i]->bActive && MercPtrs[i]->bLife && !MercPtrs[i]->fBetweenSectors &&
          !(MercPtrs[i]->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(MercPtrs[i]) &&
          !AM_AN_EPC(MercPtrs[i])) {
        if (MercPtrs[i]->sSectorX == gWorldSectorX && MercPtrs[i]->sSectorY == gWorldSectorY &&
            MercPtrs[i]->bSectorZ == gbWorldSectorZ) {
          return FALSE;
        }
      }
    }
    // KM : August 6, 1999 Patch fix
    //     Added logic to prevent a crash when player mercs would retreat from a battle involving
    //     militia and enemies.
    //		 Without the return here, it would proceed to trash the world, and then when
    // autoresolve would come up to
    //     finish the tactical battle, it would fail to find the existing soldier information
    //     (because it was trashed).
    if (HandlePotentialBringUpAutoresolveToFinishBattle()) {
      return FALSE;
    }
    // end

    // HandlePotentialBringUpAutoresolveToFinishBattle( ); //prior patch logic
  }

  CheckForEndOfCombatMode(FALSE);
  EndTacticalBattleForEnemy();

  // ATE: Change cursor to wait cursor for duration of frame.....
  // save old cursor ID....
  SetCurrentCursorFromDatabase(CURSOR_WAIT_NODELAY);
  RefreshScreen();

  // JA2Gold: Leaving sector, so get rid of ambients!
  DeleteAllAmbients();

  if (IsTacticalMode()) {
    if (!gfTacticalTraversal) {  // if we are in tactical and don't intend on going to another
                                 // sector immediately, then
      gfEnteringMapScreen = TRUE;
    } else {  // The trashing of the world will be handled automatically.
      return FALSE;
    }
  }

  // We have passed all the checks and can Trash the world.
  if (!HandleDefiniteUnloadingOfWorld(ABOUT_TO_TRASH_WORLD)) {
    return FALSE;
  }

  if (guiCurrentScreen == AUTORESOLVE_SCREEN) {
    if (gWorldSectorX == sBattleSectorX && gWorldSectorY == sBattleSectorY &&
        gbWorldSectorZ == sBattleSectorZ) {  // Yes, this is and looks like a hack.  The conditions
                                             // of this if statement doesn't work inside
      // TrashWorld() or more specifically, TacticalRemoveSoldier() from within TrashWorld().
      // Because we are in the autoresolve screen, soldiers are internally created different (from
      // pointers instead of the MercPtrs[]).  It keys on the fact that we are in the autoresolve
      // screen.  So, by switching the screen, it'll delete the soldiers in the loaded world
      // properly, then later on, once autoresolve is complete, it'll delete the autoresolve
      // soldiers properly.  As you can now see, the above if conditions don't change throughout
      // this whole process which makes it necessary to do it this way.
      guiCurrentScreen = MAP_SCREEN;
      TrashWorld();
      guiCurrentScreen = AUTORESOLVE_SCREEN;
    }
  } else {
    TrashWorld();
  }

  // Clear all combat related flags.
  gTacticalStatus.fEnemyInSector = FALSE;
  gTacticalStatus.uiFlags &= ~INCOMBAT;
  EndTopMessage();

  // Clear the world sector values.
  gWorldSectorX = gWorldSectorY = 0;
  gbWorldSectorZ = -1;

  // Clear the flags regarding.
  gfCaves = FALSE;
  gfBasement = FALSE;

  return TRUE;
}

// This is called just before the world is unloaded to preserve location information for RPCs and
// NPCs either in the sector or strategically in the sector (such as firing an NPC in a sector
// that isn't yet loaded.)  When loading that sector, the RPC would be added.
//@@@Evaluate
void SetupProfileInsertionDataForSoldier(struct SOLDIERTYPE *pSoldier) {
  if (!pSoldier || GetSolProfile(pSoldier) == NO_PROFILE) {  // Doesn't have profile information.
    return;
  }

  if (gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags3 &
      PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE)
  // if ( gMercProfiles[ GetSolProfile(pSoldier) ].ubStrategicInsertionCode ==
  // INSERTION_CODE_PERMANENT_GRIDNO )
  {
    // can't be changed!
    return;
  }

  if (gfWorldLoaded && IsSolActive(pSoldier) &&
      pSoldier->bInSector) {  // This soldier is currently in the sector

    //@@@Evaluate -- insert code here
    // SAMPLE CODE:  There are multiple situations that I didn't code.  The gridno should be the
    // final destination or reset???

    if (pSoldier->ubQuoteRecord && pSoldier->ubQuoteActionID) {
      // if moving to traverse
      if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST &&
          pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH) {
        // Handle traversal.  This NPC's sector will NOT already be set correctly, so we have to
        // call for that too
        HandleNPCChangesForTacticalTraversal(pSoldier);
        gMercProfiles[GetSolProfile(pSoldier)].fUseProfileInsertionInfo = FALSE;
        if (GetSolProfile(pSoldier) != NO_PROFILE &&
            NPCHasUnusedRecordWithGivenApproach(GetSolProfile(pSoldier), APPROACH_DONE_TRAVERSAL)) {
          gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags3 |=
              PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL;
        }

      } else {
        if (pSoldier->sFinalDestination == pSoldier->sGridNo) {
          gMercProfiles[GetSolProfile(pSoldier)].usStrategicInsertionData = pSoldier->sGridNo;
        } else if (pSoldier->sAbsoluteFinalDestination != NOWHERE) {
          gMercProfiles[GetSolProfile(pSoldier)].usStrategicInsertionData =
              pSoldier->sAbsoluteFinalDestination;
        } else {
          gMercProfiles[GetSolProfile(pSoldier)].usStrategicInsertionData =
              pSoldier->sFinalDestination;
        }

        gMercProfiles[GetSolProfile(pSoldier)].fUseProfileInsertionInfo = TRUE;
        gMercProfiles[GetSolProfile(pSoldier)].ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
        gMercProfiles[GetSolProfile(pSoldier)].ubQuoteActionID = pSoldier->ubQuoteActionID;
        gMercProfiles[GetSolProfile(pSoldier)].ubQuoteRecord = pSoldier->ubQuoteActionID;
      }
    } else {
      gMercProfiles[GetSolProfile(pSoldier)].fUseProfileInsertionInfo = FALSE;
    }

  } else {  // use strategic information

    // It appears to set the soldier's strategic insertion code everytime a group arrives in a new
    // sector.  The insertion data isn't needed for these cases as the code is a direction only.
    gMercProfiles[GetSolProfile(pSoldier)].ubStrategicInsertionCode =
        pSoldier->ubStrategicInsertionCode;
    gMercProfiles[GetSolProfile(pSoldier)].usStrategicInsertionData = 0;

    // Strategic system should now work.
    gMercProfiles[GetSolProfile(pSoldier)].fUseProfileInsertionInfo = TRUE;
  }
}

void HandlePotentialMoraleHitForSkimmingSectors(struct GROUP *pGroup) {
  PLAYERGROUP *pPlayer;

  if (!gTacticalStatus.fHasEnteredCombatModeSinceEntering && gTacticalStatus.fEnemyInSector) {
    // Flag is set so if "wilderness" enemies are in the adjacent sector of this group, the group
    // has a 90% chance of ambush.  Because this typically doesn't happen very often, the chance
    // is high. This reflects the enemies radioing ahead to other enemies of the group's arrival,
    // so they have time to setup a good ambush!
    pGroup->uiFlags |= GROUPFLAG_HIGH_POTENTIAL_FOR_AMBUSH;

    pPlayer = pGroup->pPlayerList;

    while (pPlayer) {
      // Do morale hit...
      // CC look here!
      // pPlayer->pSoldier

      pPlayer = pPlayer->next;
    }
  }
}
