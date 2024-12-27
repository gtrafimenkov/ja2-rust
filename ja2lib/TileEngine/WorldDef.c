// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/WorldDef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Editor/EditorBuildings.h"
#include "Editor/EditorMapInfo.h"
#include "Editor/Smooth.h"
#include "Editor/SummaryInfo.h"
#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/Shading.h"
#include "SGP/VObject.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Scheduling.h"
#include "Strategic/StrategicMap.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Buildings.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/LightEffects.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/MapEdgepoints.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/Pits.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/SmokeEffects.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/TileSurface.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldMan.h"
#include "Utils/AnimatedProgressBar.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/MusicControl.h"
#include "Utils/Utilities.h"
#include "platform.h"
#include "rust_fileman.h"
#include "rust_images.h"

#define SET_MOVEMENTCOST(a, b, c, d) \
  ((gubWorldMovementCosts[a][b][c] < d) ? (gubWorldMovementCosts[a][b][c] = d) : 0);
#define FORCE_SET_MOVEMENTCOST(a, b, c, d) (gubWorldMovementCosts[a][b][c] = d)
#define SET_CURRMOVEMENTCOST(a, b) SET_MOVEMENTCOST(usGridNo, a, 0, b)

#define TEMP_FILE_FOR_TILESET_CHANGE "jatileS34.dat"

#define MAP_FULLSOLDIER_SAVED 0x00000001
#define MAP_WORLDONLY_SAVED 0x00000002
#define MAP_WORLDLIGHTS_SAVED 0x00000004
#define MAP_WORLDITEMS_SAVED 0x00000008
#define MAP_EXITGRIDS_SAVED 0x00000010
#define MAP_DOORTABLE_SAVED 0x00000020
#define MAP_EDGEPOINTS_SAVED 0x00000040
#define MAP_AMBIENTLIGHTLEVEL_SAVED 0x00000080
#define MAP_NPCSCHEDULES_SAVED 0x00000100

#ifdef JA2EDITOR
extern BOOLEAN gfErrorCatch;
extern wchar_t gzErrorCatchString[256];
#endif

// TEMP
BOOLEAN gfForceLoadPlayers = FALSE;
char gzForceLoadFile[100];
BOOLEAN gfForceLoad = FALSE;

uint8_t gubCurrentLevel;
int32_t giCurrentTilesetID = 0;
char gzLastLoadedFile[260];

uint32_t gCurrentBackground = FIRSTTEXTURE;

int8_t gbNewTileSurfaceLoaded[NUMBEROFTILETYPES];

void SetAllNewTileSurfacesLoaded(BOOLEAN fNew) {
  memset(gbNewTileSurfaceLoaded, fNew, sizeof(gbNewTileSurfaceLoaded));
}

BOOLEAN gfInitAnimateLoading = FALSE;

// Local Functions
BOOLEAN LoadTileSurfaces(char pTileSurfaceFilenames[][32], uint8_t ubTilesetID);
BOOLEAN AddTileSurface(SGPFILENAME cFilename, uint32_t ubType, uint8_t ubTilesetID,
                       BOOLEAN fGetFromRoot);
void DestroyTileSurfaces(void);
void ProcessTilesetNamesForBPP(void);
BOOLEAN IsRoofVisibleForWireframe(int16_t sMapPos);

int8_t IsHiddenTileMarkerThere(int16_t sGridNo);
extern void SetInterfaceHeightLevel();

void SaveMapLights(FileID hfile);
void LoadMapLights(int8_t **hBuffer);

// Global Variables
MAP_ELEMENT *gpWorldLevelData;
int32_t *gpDirtyData;
uint32_t gSurfaceMemUsage;
uint8_t gubWorldMovementCosts[WORLD_MAX][MAXDIR][2];

// set to nonzero (locs of base gridno of structure are good) to have it defined by structure code
int16_t gsRecompileAreaTop = 0;
int16_t gsRecompileAreaLeft = 0;
int16_t gsRecompileAreaRight = 0;
int16_t gsRecompileAreaBottom = 0;

// TIMER TESTING STUFF
#ifdef JA2TESTVERSION
extern uint32_t uiLoadWorldTime;
extern uint32_t uiTrashWorldTime;
extern uint32_t uiLoadMapTilesetTime;
extern uint32_t uiLoadMapLightsTime;
extern uint32_t uiBuildShadeTableTime;
extern uint32_t uiNumTablesLoaded;
extern uint32_t uiNumTablesSaved;
extern uint32_t uiNumImagesReloaded;
#endif

BOOLEAN DoorAtGridNo(uint32_t iMapIndex) {
  struct STRUCTURE *pStruct;
  pStruct = gpWorldLevelData[iMapIndex].pStructureHead;
  while (pStruct) {
    if (pStruct->fFlags & STRUCTURE_ANYDOOR) return TRUE;
    pStruct = pStruct->pNext;
  }
  return FALSE;
}

BOOLEAN OpenableAtGridNo(uint32_t iMapIndex) {
  struct STRUCTURE *pStruct;
  pStruct = gpWorldLevelData[iMapIndex].pStructureHead;
  while (pStruct) {
    if (pStruct->fFlags & STRUCTURE_OPENABLE) return TRUE;
    pStruct = pStruct->pNext;
  }
  return FALSE;
}

BOOLEAN FloorAtGridNo(uint32_t iMapIndex) {
  struct LEVELNODE *pLand;
  uint32_t uiTileType;
  pLand = gpWorldLevelData[iMapIndex].pLandHead;
  // Look through all objects and Search for type
  while (pLand) {
    if (pLand->usIndex != NO_TILE) {
      GetTileType(pLand->usIndex, &uiTileType);
      if (uiTileType >= FIRSTFLOOR && uiTileType <= LASTFLOOR) {
        return TRUE;
      }
      pLand = pLand->pNext;
    }
  }
  return FALSE;
}

BOOLEAN GridNoIndoors(uint32_t iMapIndex) {
  if (gfBasement || gfCaves) return TRUE;
  if (FloorAtGridNo(iMapIndex)) return TRUE;
  return FALSE;
}

void DOIT() {
  //	struct LEVELNODE *			pLand;
  // struct LEVELNODE *			pObject;
  struct LEVELNODE *pStruct, *pNewStruct;
  // struct LEVELNODE	*			pShadow;
  uint32_t uiLoop;

  // first level
  for (uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++) {
    pStruct = gpWorldLevelData[uiLoop].pStructHead;

    while (pStruct != NULL) {
      pNewStruct = pStruct->pNext;

      if (pStruct->usIndex >= DEBRISWOOD1 && pStruct->usIndex <= DEBRISWEEDS10) {
        AddObjectToHead(uiLoop, pStruct->usIndex);

        RemoveStruct(uiLoop, pStruct->usIndex);
      }

      pStruct = pNewStruct;
    }
  }
}

BOOLEAN InitializeWorld() {
  gTileDatabaseSize = 0;
  gSurfaceMemUsage = 0;
  giCurrentTilesetID = -1;

  // DB Adds the _8 to the names if we're in 8 bit mode.
  // ProcessTilesetNamesForBPP();

  // Memset tileset list
  memset(TileSurfaceFilenames, '\0', sizeof(TileSurfaceFilenames));

  // ATE: MEMSET LOG HEIGHT VALUES
  memset(gTileTypeLogicalHeight, 1, sizeof(gTileTypeLogicalHeight));

  // Memset tile database
  memset(gTileDatabase, 0, sizeof(gTileDatabase));

  // Init surface list
  memset(gTileSurfaceArray, 0, sizeof(gTileSurfaceArray));

  // Init default surface list
  memset(gbDefaultSurfaceUsed, 0, sizeof(gbDefaultSurfaceUsed));

  // Init same surface list
  memset(gbSameAsDefaultSurfaceUsed, 0, sizeof(gbSameAsDefaultSurfaceUsed));

  // Initialize world data

  gpWorldLevelData = (MAP_ELEMENT *)MemAlloc(WORLD_MAX * sizeof(MAP_ELEMENT));
  if (!(gpWorldLevelData)) {
    return FALSE;
  }

  // Zero world
  memset(gpWorldLevelData, 0, WORLD_MAX * sizeof(MAP_ELEMENT));

  // Init room database
  InitRoomDatabase();

  // INit tilesets
  InitEngineTilesets();

  return (TRUE);
}

void DeinitializeWorld() {
  TrashWorld();

  if (gpWorldLevelData != NULL) {
    MemFree(gpWorldLevelData);
  }

  if (gpDirtyData != NULL) {
    MemFree(gpDirtyData);
  }

  DestroyTileSurfaces();
  FreeAllStructureFiles();

  // Shutdown tile database data
  DeallocateTileDatabase();

  ShutdownRoomDatabase();
}

BOOLEAN ReloadTilesetSlot(int32_t iSlot) { return (TRUE); }

BOOLEAN LoadTileSurfaces(char ppTileSurfaceFilenames[][32], uint8_t ubTilesetID) {
  uint32_t uiLoop;
  uint32_t uiPercentage;

  // If no Tileset filenames are given, return error
  if (ppTileSurfaceFilenames == NULL) {
    return (FALSE);
  } else {
    for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
      strcpy(TileSurfaceFilenames[uiLoop], ppTileSurfaceFilenames[uiLoop]);
  }

  // load the tile surfaces
  SetRelativeStartAndEndPercentage(0, 1, 35, L"Tile Surfaces");
  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    uiPercentage = (uiLoop * 100) / (NUMBEROFTILETYPES - 1);
    RenderProgressBar(0, uiPercentage);

    // The cost of having to do this check each time through the loop,
    // thus about 20 times, seems better than having to maintain two
    // almost completely identical functions
    if (ppTileSurfaceFilenames == NULL) {
      if (AddTileSurface(TileSurfaceFilenames[uiLoop], uiLoop, ubTilesetID, FALSE) == FALSE) {
        DestroyTileSurfaces();
        return (FALSE);
      }
    } else {
      if (*(ppTileSurfaceFilenames[uiLoop]) != '\0') {
        if (AddTileSurface(ppTileSurfaceFilenames[uiLoop], uiLoop, ubTilesetID, FALSE) == FALSE) {
          DestroyTileSurfaces();
          return (FALSE);
        }
      } else {
        // USE FIRST TILESET VALUE!

        // ATE: If here, don't load default surface if already loaded...
        if (!gbDefaultSurfaceUsed[uiLoop]) {
          strcpy(TileSurfaceFilenames[uiLoop], gTilesets[GENERIC_1].TileSurfaceFilenames[uiLoop]);
          if (AddTileSurface(gTilesets[GENERIC_1].TileSurfaceFilenames[uiLoop], uiLoop, GENERIC_1,
                             FALSE) == FALSE) {
            DestroyTileSurfaces();
            return (FALSE);
          }
        } else {
          gbSameAsDefaultSurfaceUsed[uiLoop] = TRUE;
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN AddTileSurface(SGPFILENAME cFilename, uint32_t ubType, uint8_t ubTilesetID,
                       BOOLEAN fGetFromRoot) {
  // Add tile surface
  struct TILE_IMAGERY *TileSurf;
  char cFileBPP[128];
  char cAdjustedFile[200];

  // Delete the surface first!
  if (gTileSurfaceArray[ubType] != NULL) {
    DeleteTileSurface(gTileSurfaceArray[ubType]);
    gTileSurfaceArray[ubType] = NULL;
  }

  // Adjust flag for same as default used...
  gbSameAsDefaultSurfaceUsed[ubType] = FALSE;

  // Adjust for BPP
  CopyFilename(cFilename, cFileBPP);

  if (!fGetFromRoot) {
    // Adjust for tileset position
    sprintf(cAdjustedFile, "TILESETS\\%d\\%s", ubTilesetID, cFileBPP);
  } else {
    sprintf(cAdjustedFile, "%s", cFileBPP);
  }

  TileSurf = LoadTileSurface(cAdjustedFile);

  if (TileSurf == NULL) return (FALSE);

  TileSurf->fType = ubType;

  SetRaisedObjectFlag(cAdjustedFile, TileSurf);

  gTileSurfaceArray[ubType] = TileSurf;

  // OK, if we were not the default tileset, set value indicating that!
  if (ubTilesetID != GENERIC_1) {
    gbDefaultSurfaceUsed[ubType] = FALSE;
  } else {
    gbDefaultSurfaceUsed[ubType] = TRUE;
  }

  gbNewTileSurfaceLoaded[ubType] = TRUE;

  return (TRUE);
}

extern BOOLEAN gfLoadShadeTablesFromTextFile;

void BuildTileShadeTables() {
  FileID hfile = FILE_ID_ERR;
  struct Str512 DataDir;
  char ShadeTableDir[600];
  uint32_t uiLoop;
  char cRootFile[128];
  BOOLEAN fForceRebuildForSlot = FALSE;

#ifdef JA2TESTVERSION
  uint32_t uiStartTime;
#endif
  static uint8_t ubLastRed = 255, ubLastGreen = 255, ubLastBlue = 255;

#ifdef JA2TESTVERSION
  uiNumTablesLoaded = 0;
  uiNumTablesSaved = 0;
  uiNumImagesReloaded = 0;
  uiStartTime = GetJA2Clock();
#endif

  // Set the directory to the shadetable directory
  if (!Plat_GetCurrentDirectory(&DataDir)) {
    return;
  }
  snprintf(ShadeTableDir, ARR_SIZE(ShadeTableDir), "%s\\ShadeTables", DataDir.buf);
  if (!Plat_SetCurrentDirectory(ShadeTableDir)) {
    AssertMsg(0, "Can't set the directory to Data\\ShadeTable.  Kris' big problem!");
  }
  hfile = File_OpenForReading("IgnoreShadeTables.txt");
  if (hfile) {
    File_Close(hfile);
    gfForceBuildShadeTables = TRUE;
  } else {
    gfForceBuildShadeTables = FALSE;
  }
  // now, determine if we are using specialized colors.
  if (gpLightColors[0].red || gpLightColors[0].green ||
      gpLightColors[0].blue) {  // we are, which basically means we force build the shadetables.
                                // However, the one
    // exception is if we are loading another map and the colors are the same.
    if (gpLightColors[0].red != ubLastRed || gpLightColors[0].green != ubLastGreen ||
        gpLightColors[0].blue != ubLastBlue) {  // Same tileset, but colors are different, so set
                                                // things up to regenerate the shadetables.
      gfForceBuildShadeTables = TRUE;
    } else {  // same colors, same tileset, so don't rebuild shadetables -- much faster!
      gfForceBuildShadeTables = FALSE;
    }
  }

  if (gfLoadShadeTablesFromTextFile) {  // Because we're tweaking the RGB values in the text file,
                                        // always force rebuild the shadetables
    // so that the user can tweak them in the same exe session.
    memset(gbNewTileSurfaceLoaded, 1, sizeof(gbNewTileSurfaceLoaded));
  }

  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    if (gTileSurfaceArray[uiLoop] != NULL) {
// Don't Create shade tables if default were already used once!
#ifdef JA2EDITOR
      if (gbNewTileSurfaceLoaded[uiLoop] || gfEditorForceShadeTableRebuild)
#else
      if (gbNewTileSurfaceLoaded[uiLoop])
#endif
      {
        fForceRebuildForSlot = FALSE;

        GetRootName(cRootFile, TileSurfaceFilenames[uiLoop]);

        if (strcmp(cRootFile, "grass2") == 0) {
          fForceRebuildForSlot = TRUE;
        }

#ifdef JA2TESTVERSION
        uiNumImagesReloaded++;
#endif
        RenderProgressBar(0, uiLoop * 100 / NUMBEROFTILETYPES);
        CreateTilePaletteTables(gTileSurfaceArray[uiLoop]->vo, uiLoop, fForceRebuildForSlot);
      }
    }
  }

  // Restore the data directory once we are finished.
  Plat_SetCurrentDirectory(DataDir.buf);

  ubLastRed = gpLightColors[0].red;
  ubLastGreen = gpLightColors[0].green;
  ubLastBlue = gpLightColors[0].blue;

#ifdef JA2TESTVERSION
  uiBuildShadeTableTime = GetJA2Clock() - uiStartTime;
#endif
}

void DestroyTileShadeTables() {
  uint32_t uiLoop;

  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    if (gTileSurfaceArray[uiLoop] != NULL) {
// Don't Delete shade tables if default are still being used...
#ifdef JA2EDITOR
      if (gbNewTileSurfaceLoaded[uiLoop] || gfEditorForceShadeTableRebuild) {
        DestroyObjectPaletteTables(gTileSurfaceArray[uiLoop]->vo);
      }
#else
      if (gbNewTileSurfaceLoaded[uiLoop]) {
        DestroyObjectPaletteTables(gTileSurfaceArray[uiLoop]->vo);
      }
#endif
    }
  }
}

void DestroyTileSurfaces() {
  uint32_t uiLoop;

  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++) {
    if (gTileSurfaceArray[uiLoop] != NULL) {
      DeleteTileSurface(gTileSurfaceArray[uiLoop]);
      gTileSurfaceArray[uiLoop] = NULL;
    }
  }
}

void CompileWorldTerrainIDs(void) {
  int16_t sGridNo;
  int16_t sTempGridNo;
  struct LEVELNODE *pNode;
  TILE_ELEMENT *pTileElement;
  uint8_t ubLoop;

  for (sGridNo = 0; sGridNo < WORLD_MAX; sGridNo++) {
    if (GridNoOnVisibleWorldTile(sGridNo)) {
      // Check if we have anything in object layer which has a terrain modifier
      pNode = gpWorldLevelData[sGridNo].pObjectHead;

      // ATE: CRAPOLA! Special case stuff here for the friggen pool since art was fu*ked up
      if (giCurrentTilesetID == TEMP_19) {
        // Get ID
        if (pNode != NULL) {
          if (pNode->usIndex == ANOTHERDEBRIS4 || pNode->usIndex == ANOTHERDEBRIS6 ||
              pNode->usIndex == ANOTHERDEBRIS7) {
            gpWorldLevelData[sGridNo].ubTerrainID = LOW_WATER;
            continue;
          }
        }
      }

      if (pNode == NULL || pNode->usIndex >= NUMBEROFTILES ||
          gTileDatabase[pNode->usIndex].ubTerrainID == NO_TERRAIN) {  // Try terrain instead!
        pNode = gpWorldLevelData[sGridNo].pLandHead;
      }
      pTileElement = &(gTileDatabase[pNode->usIndex]);
      if (pTileElement->ubNumberOfTiles > 1) {
        for (ubLoop = 0; ubLoop < pTileElement->ubNumberOfTiles; ubLoop++) {
          sTempGridNo = sGridNo + pTileElement->pTileLocData[ubLoop].bTileOffsetX +
                        pTileElement->pTileLocData[ubLoop].bTileOffsetY * WORLD_COLS;
          gpWorldLevelData[sTempGridNo].ubTerrainID = pTileElement->ubTerrainID;
        }
      } else {
        gpWorldLevelData[sGridNo].ubTerrainID = pTileElement->ubTerrainID;
      }
    }
  }
}

void CompileTileMovementCosts(uint16_t usGridNo) {
  uint8_t ubTerrainID;
  struct LEVELNODE *pLand;

  struct STRUCTURE *pStructure;
  BOOLEAN fStructuresOnRoof;

  uint8_t ubDirLoop;

  /*
   */

  if (GridNoOnVisibleWorldTile(usGridNo)) {
    // check for land of a different height in adjacent locations
    for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
      if (gpWorldLevelData[usGridNo].sHeight !=
          gpWorldLevelData[usGridNo + DirectionInc(ubDirLoop)].sHeight) {
        SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_OBSTACLE);
      }
    }

    // check for exit grids
    if (ExitGridAtGridNo(usGridNo)) {
      for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
        SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_EXITGRID);
      }
      // leave the roof alone, and continue, so that we can get values for the roof if traversable
    }

  } else {
    for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
      SET_MOVEMENTCOST(usGridNo, ubDirLoop, 0, TRAVELCOST_OFF_MAP);
      SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_OFF_MAP);
    }
    if (gpWorldLevelData[usGridNo].pStructureHead == NULL) {
      return;
    }
  }

  if (gpWorldLevelData[usGridNo].pStructureHead != NULL) {  // structures in tile
    // consider the land
    pLand = gpWorldLevelData[usGridNo].pLandHead;
    if (pLand != NULL) {
      // Get terrain type
      ubTerrainID =
          gpWorldLevelData[usGridNo].ubTerrainID;  // = GetTerrainType( (int16_t)usGridNo );

      for (ubDirLoop = 0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++) {
        SET_CURRMOVEMENTCOST(ubDirLoop, gTileTypeMovementCost[ubTerrainID]);
      }
    }

    // now consider all structures
    pStructure = gpWorldLevelData[usGridNo].pStructureHead;
    fStructuresOnRoof = FALSE;
    do {
      if (pStructure->sCubeOffset == STRUCTURE_ON_GROUND) {
        if (pStructure->fFlags & STRUCTURE_PASSABLE) {
          if (pStructure->fFlags & STRUCTURE_WIREFENCE && pStructure->fFlags & STRUCTURE_OPEN) {
            // prevent movement along the fence but allow in all other directions
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_NOT_STANDING);
                break;

              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_NOT_STANDING);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_NOT_STANDING);
                break;
            }
          }
          // all other passable structures do not block movement in any way
        } else if (pStructure->fFlags & STRUCTURE_BLOCKSMOVES) {
          if ((pStructure->fFlags & STRUCTURE_FENCE) && !(pStructure->fFlags & STRUCTURE_SPECIAL)) {
            // jumpable!
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:
                // can be jumped north and south
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_FENCE);
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_FENCE);
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_OBSTACLE);
                // set values for the tiles EXITED from this location
                FORCE_SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_NONE);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                FORCE_SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_NONE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo - 1, WEST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE);
                break;

              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:
                // can be jumped east and west
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_FENCE);
                SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_FENCE);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_OBSTACLE);
                // set values for the tiles EXITED from this location
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);
                // make sure no obstacle costs exists before changing path cost to 0
                if (gubWorldMovementCosts[usGridNo + 1][EAST][0] < TRAVELCOST_BLOCKED) {
                  FORCE_SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_NONE);
                }
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);
                if (gubWorldMovementCosts[usGridNo - 1][WEST][0] < TRAVELCOST_BLOCKED) {
                  FORCE_SET_MOVEMENTCOST(usGridNo - 1, WEST, 0, TRAVELCOST_NONE);
                }
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE);
                break;

              default:
                // corners aren't jumpable
                for (ubDirLoop = 0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++) {
                  SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_OBSTACLE);
                }
                break;
            }
          } else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_SANDBAG &&
                     StructureHeight(pStructure) < 2) {
            for (ubDirLoop = 0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++) {
              SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_OBSTACLE);
            }

            if (FindStructure((uint16_t)(usGridNo - WORLD_COLS), STRUCTURE_OBSTACLE) == FALSE &&
                FindStructure((uint16_t)(usGridNo + WORLD_COLS), STRUCTURE_OBSTACLE) == FALSE) {
              FORCE_SET_MOVEMENTCOST(usGridNo, NORTH, 0, TRAVELCOST_FENCE);
              FORCE_SET_MOVEMENTCOST(usGridNo, SOUTH, 0, TRAVELCOST_FENCE);
            }

            if (FindStructure((uint16_t)(usGridNo - 1), STRUCTURE_OBSTACLE) == FALSE &&
                FindStructure((uint16_t)(usGridNo + 1), STRUCTURE_OBSTACLE) == FALSE) {
              FORCE_SET_MOVEMENTCOST(usGridNo, EAST, 0, TRAVELCOST_FENCE);
              FORCE_SET_MOVEMENTCOST(usGridNo, WEST, 0, TRAVELCOST_FENCE);
            }
          } else if ((pStructure->fFlags & STRUCTURE_CAVEWALL)) {
            for (ubDirLoop = 0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++) {
              SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_CAVEWALL);
            }
          } else {
            for (ubDirLoop = 0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++) {
              SET_CURRMOVEMENTCOST(ubDirLoop, TRAVELCOST_OBSTACLE);
            }
          }
        } else if (pStructure->fFlags &
                   STRUCTURE_ANYDOOR) /*&& (pStructure->fFlags & STRUCTURE_OPEN))*/
        {  // NB closed doors are treated just like walls, in the section after this

          if (pStructure->fFlags & STRUCTURE_DDOOR_LEFT &&
              (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
               pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)) {
            // double door, left side (as you look on the screen)
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_RIGHT:
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {  // doorpost
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                  SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL);
                  // corner
                  SET_MOVEMENTCOST(usGridNo + 1 + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL);
                } else {  // door
                  SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_OPEN_W);
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_DOOR_OPEN_W);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_OPEN_NW);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_NW);
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_W_W);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0,
                                   TRAVELCOST_DOOR_OPEN_NW_W);
                }
                break;

              case INSIDE_TOP_RIGHT:
                // doorpost
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
                SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                // door
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_OPEN_HERE);
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_DOOR_OPEN_HERE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_OPEN_N);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_N);
                SET_MOVEMENTCOST(usGridNo - 1, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_E);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_NE);
                break;

              default:
                // door with no orientation specified!?
                break;
            }
          } else if (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT &&
                     (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
                      pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT)) {
            // double door, right side (as you look on the screen)
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {  // doorpost
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N)
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL);
                  ;
                  // corner
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                } else {  // door
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_DOOR_OPEN_N);
                  SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_OPEN_N);
                  SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_OPEN_NW);
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_NW);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_N_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0,
                                   TRAVELCOST_DOOR_OPEN_NW_N);
                }
                break;

              case INSIDE_TOP_LEFT:
                // doorpost
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);
                // corner
                SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                // door
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_OPEN_HERE);
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_DOOR_OPEN_HERE);
                SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_OPEN_W);
                SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_W);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_S);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_SW);
                break;
              default:
                // door with no orientation specified!?
                break;
            }
          } else if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR &&
                     pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1) {
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:
                // doorframe post in one corner of each of the tiles
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_CLOSED_N);

                } else {
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_CLOSED_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL);
                }
                break;
              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:
                // doorframe post in one corner of each of the tiles
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_DOOR_CLOSED_HERE);

                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                  SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                } else {
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_WALL);

                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                  SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                  SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL);
                }
                break;
            }
          } else {
            // standard door
            switch (pStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {  // doorframe
                  SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);

                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);

                  // DO CORNERS
                  SET_MOVEMENTCOST(usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL);

                  // SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
                  // SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
                  // SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
                  // SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
                  // corner
                  // SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
                } else if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR)) {  // door
                  SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                  SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_DOOR_OPEN_N);
                  SET_MOVEMENTCOST(usGridNo - 1, WEST, 0, TRAVELCOST_DOOR_OPEN_NE);
                  SET_MOVEMENTCOST(usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_N_N);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0,
                                   TRAVELCOST_DOOR_OPEN_NE_N);
                }
                break;

              case INSIDE_TOP_LEFT:
                SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
                SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_DOOR_CLOSED_HERE);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);

                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);

                // DO CORNERS
                SET_MOVEMENTCOST(usGridNo - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);

                // doorframe
                // SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
                // SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
                // SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
                // SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
                // corner
                // SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
                // door
                if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR)) {
                  SET_CURRMOVEMENTCOST(EAST, TRAVELCOST_DOOR_OPEN_HERE);
                  SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_DOOR_OPEN_HERE);
                  SET_MOVEMENTCOST(usGridNo - 1, WEST, 0, TRAVELCOST_DOOR_OPEN_E);
                  SET_MOVEMENTCOST(usGridNo - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_E);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_S);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS - 1, NORTHWEST, 0,
                                   TRAVELCOST_DOOR_OPEN_SE);
                }
                break;

              case OUTSIDE_TOP_RIGHT:
                if (pStructure->fFlags & STRUCTURE_BASE_TILE) {  // doorframe
                  SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_OBSTACLE);
                  SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_CLOSED_HERE);
                  SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_OBSTACLE);

                  SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                  SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                  SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);

                  // DO CORNERS
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_OBSTACLE);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                  SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);

                  // SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
                  // SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
                  // SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
                  // SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
                  // corner
                  // SET_MOVEMENTCOST( usGridNo + 1 + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE
                  // );
                } else if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR)) {  // door
                  SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_DOOR_OPEN_W);
                  SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_DOOR_OPEN_W);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_DOOR_OPEN_SW);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_SW);
                  SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_W_W);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0,
                                   TRAVELCOST_DOOR_OPEN_SW_W);
                }
                break;

              case INSIDE_TOP_RIGHT:
                SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_OBSTACLE);
                SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_DOOR_CLOSED_HERE);
                SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_OBSTACLE);

                SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W);
                SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);

                // DO CORNERS
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE);
                SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE);

                // doorframe
                /*
                SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
                SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
                SET_MOVEMENTCOST( usGridNo + 1,SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
                SET_MOVEMENTCOST( usGridNo + 1,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
                // corner
                SET_MOVEMENTCOST( usGridNo - WORLD_COLS,  NORTHWEST, 0, TRAVELCOST_OBSTACLE );
                */
                if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR)) {
                  // door
                  SET_CURRMOVEMENTCOST(SOUTH, TRAVELCOST_DOOR_OPEN_HERE);
                  SET_CURRMOVEMENTCOST(SOUTHEAST, TRAVELCOST_DOOR_OPEN_HERE);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_DOOR_OPEN_S);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_S);
                  SET_MOVEMENTCOST(usGridNo - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_E);
                  SET_MOVEMENTCOST(usGridNo - WORLD_COLS - 1, NORTHWEST, 0,
                                   TRAVELCOST_DOOR_OPEN_SE);
                }
                break;

              default:
                // door with no orientation specified!?
                break;
            }
          }

          /*
          switch( pStructure->ubWallOrientation )
          {
                  case OUTSIDE_TOP_LEFT:
                  case INSIDE_TOP_LEFT:
                          SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
                          SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
                          SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

                          SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE
          ); SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N );
                          SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE
          );

                          // DO CORNERS
                          SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
                          SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
                          SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0,
          TRAVELCOST_OBSTACLE ); SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0,
          TRAVELCOST_OBSTACLE ); break;

                  case OUTSIDE_TOP_RIGHT:
                  case INSIDE_TOP_RIGHT:
                          SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
                          SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
                          SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

                          SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
                          SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
                          SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );

                          // DO CORNERS
                          SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0,
          TRAVELCOST_OBSTACLE ); SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0,
          TRAVELCOST_OBSTACLE ); SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0,
          TRAVELCOST_OBSTACLE ); SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0,
          TRAVELCOST_OBSTACLE ); break;

                  default:
                          // wall with no orientation specified!?
                          break;
          }
          */

        } else if (pStructure->fFlags & STRUCTURE_WALLSTUFF) {
          // ATE: IF a closed door, set to door value
          switch (pStructure->ubWallOrientation) {
            case OUTSIDE_TOP_LEFT:
            case INSIDE_TOP_LEFT:
              SET_CURRMOVEMENTCOST(NORTHEAST, TRAVELCOST_WALL);
              SET_CURRMOVEMENTCOST(NORTH, TRAVELCOST_WALL);
              SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);

              // DO CORNERS
              SET_MOVEMENTCOST(usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL);
              break;

            case OUTSIDE_TOP_RIGHT:
            case INSIDE_TOP_RIGHT:
              SET_CURRMOVEMENTCOST(SOUTHWEST, TRAVELCOST_WALL);
              SET_CURRMOVEMENTCOST(WEST, TRAVELCOST_WALL);
              SET_CURRMOVEMENTCOST(NORTHWEST, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + 1, EAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL);

              // DO CORNERS
              SET_MOVEMENTCOST(usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL);
              SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL);
              break;

            default:
              // wall with no orientation specified!?
              break;
          }
        }
      } else {
        if (!(pStructure->fFlags & STRUCTURE_PASSABLE ||
              pStructure->fFlags & STRUCTURE_NORMAL_ROOF)) {
          fStructuresOnRoof = TRUE;
        }
      }
      pStructure = pStructure->pNext;
    } while (pStructure != NULL);

    // HIGHEST LAYER
    if ((gpWorldLevelData[usGridNo].pRoofHead != NULL)) {
      if (!fStructuresOnRoof) {
        for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
          SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_FLAT);
        }
      } else {
        for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
          SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_OBSTACLE);
        }
      }
    } else {
      for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
        SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_OBSTACLE);
      }
    }
  } else {  // NO STRUCTURES IN TILE
    // consider just the land

    // Get terrain type
    ubTerrainID = gpWorldLevelData[usGridNo].ubTerrainID;  // = GetTerrainType( (int16_t)usGridNo );
    for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
      SET_MOVEMENTCOST(usGridNo, ubDirLoop, 0, gTileTypeMovementCost[ubTerrainID]);
    }

    /*
                    pLand = gpWorldLevelData[ usGridNo ].pLandHead;
                    if ( pLand != NULL )
                    {
                            // Set cost here
                            // Get from tile database
                            TileElem = gTileDatabase[ pLand->usIndex ];

                            // Get terrain type
                            ubTerrainID =	GetTerrainType( (int16_t)usGridNo );

                            for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
                            {
                                    SET_MOVEMENTCOST( usGridNo ,ubDirLoop, 0, gTileTypeMovementCost[
       ubTerrainID ] );
                            }
                    }
    */
    // HIGHEST LEVEL
    if (gpWorldLevelData[usGridNo].pRoofHead != NULL) {
      for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
        SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_FLAT);
      }
    } else {
      for (ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++) {
        SET_MOVEMENTCOST(usGridNo, ubDirLoop, 1, TRAVELCOST_OBSTACLE);
      }
    }
  }
}

#define LOCAL_RADIUS 4

void RecompileLocalMovementCosts(int16_t sCentreGridNo) {
  int16_t usGridNo;
  int16_t sGridX, sGridY;
  int16_t sCentreGridX, sCentreGridY;
  int8_t bDirLoop;

  ConvertGridNoToXY(sCentreGridNo, &sCentreGridX, &sCentreGridY);
  for (sGridY = sCentreGridY - LOCAL_RADIUS; sGridY < sCentreGridY + LOCAL_RADIUS; sGridY++) {
    for (sGridX = sCentreGridX - LOCAL_RADIUS; sGridX < sCentreGridX + LOCAL_RADIUS; sGridX++) {
      usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
      // times 2 for 2 levels, times 2 for UINT16s
      //			memset( &(gubWorldMovementCosts[usGridNo]), 0, MAXDIR * 2 * 2 );
      if (usGridNo < WORLD_MAX) {
        for (bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++) {
          gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
          gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
        }
      }
    }
  }

  // note the radius used in this loop is larger, to guarantee that the
  // edges of the recompiled areas are correct (i.e. there could be spillover)
  for (sGridY = sCentreGridY - LOCAL_RADIUS - 1; sGridY < sCentreGridY + LOCAL_RADIUS + 1;
       sGridY++) {
    for (sGridX = sCentreGridX - LOCAL_RADIUS - 1; sGridX < sCentreGridX + LOCAL_RADIUS + 1;
         sGridX++) {
      usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
      if (usGridNo < WORLD_MAX) {
        CompileTileMovementCosts(usGridNo);
      }
    }
  }
}

void RecompileLocalMovementCostsFromRadius(int16_t sCentreGridNo, int8_t bRadius) {
  int16_t usGridNo;
  int16_t sGridX, sGridY;
  int16_t sCentreGridX, sCentreGridY;
  int8_t bDirLoop;

  ConvertGridNoToXY(sCentreGridNo, &sCentreGridX, &sCentreGridY);
  if (bRadius == 0) {
    // one tile check only
    for (bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++) {
      gubWorldMovementCosts[sCentreGridNo][bDirLoop][0] = 0;
      gubWorldMovementCosts[sCentreGridNo][bDirLoop][1] = 0;
    }
    CompileTileMovementCosts(sCentreGridNo);
  } else {
    for (sGridY = sCentreGridY - bRadius; sGridY < sCentreGridY + bRadius; sGridY++) {
      for (sGridX = sCentreGridX - bRadius; sGridX < sCentreGridX + bRadius; sGridX++) {
        usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
        // times 2 for 2 levels, times 2 for UINT16s
        //			memset( &(gubWorldMovementCosts[usGridNo]), 0, MAXDIR * 2 * 2 );
        if (usGridNo < WORLD_MAX) {
          for (bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++) {
            gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
            gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
          }
        }
      }
    }

    // note the radius used in this loop is larger, to guarantee that the
    // edges of the recompiled areas are correct (i.e. there could be spillover)
    for (sGridY = sCentreGridY - bRadius - 1; sGridY < sCentreGridY + bRadius + 1; sGridY++) {
      for (sGridX = sCentreGridX - bRadius - 1; sGridX < sCentreGridX + bRadius + 1; sGridX++) {
        usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
        if (usGridNo < WORLD_MAX) {
          CompileTileMovementCosts(usGridNo);
        }
      }
    }
  }
}

void AddTileToRecompileArea(int16_t sGridNo) {
  int16_t sCheckGridNo;
  int16_t sCheckX;
  int16_t sCheckY;

  // Set flag to wipe and recompile MPs in this tile
  if (sGridNo < 0 || sGridNo >= WORLD_MAX) {
    return;
  }

  gpWorldLevelData[sGridNo].ubExtFlags[0] |= MAPELEMENT_EXT_RECALCULATE_MOVEMENT;

  // check Top/Left of recompile region
  sCheckGridNo = NewGridNo(sGridNo, DirectionInc(NORTHWEST));
  sCheckX = sCheckGridNo % WORLD_COLS;
  sCheckY = sCheckGridNo / WORLD_COLS;
  if (sCheckX < gsRecompileAreaLeft) {
    gsRecompileAreaLeft = sCheckX;
  }
  if (sCheckY < gsRecompileAreaTop) {
    gsRecompileAreaTop = sCheckY;
  }

  // check Bottom/Right
  sCheckGridNo = NewGridNo(sGridNo, DirectionInc(SOUTHEAST));
  sCheckX = sCheckGridNo % WORLD_COLS;
  sCheckY = sCheckGridNo / WORLD_COLS;
  if (sCheckX > gsRecompileAreaRight) {
    gsRecompileAreaRight = sCheckX;
  }
  if (sCheckY > gsRecompileAreaBottom) {
    gsRecompileAreaBottom = sCheckY;
  }
}

void RecompileLocalMovementCostsInAreaWithFlags(void) {
  int16_t usGridNo;
  int16_t sGridX, sGridY;
  int8_t bDirLoop;

  for (sGridY = gsRecompileAreaTop; sGridY <= gsRecompileAreaBottom; sGridY++) {
    for (sGridX = gsRecompileAreaLeft; sGridX < gsRecompileAreaRight; sGridX++) {
      usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
      if (usGridNo < WORLD_MAX &&
          gpWorldLevelData[usGridNo].ubExtFlags[0] & MAPELEMENT_EXT_RECALCULATE_MOVEMENT) {
        // wipe MPs in this tile!
        for (bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++) {
          gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
          gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
        }
        // reset flag
        gpWorldLevelData[usGridNo].ubExtFlags[0] &= (~MAPELEMENT_EXT_RECALCULATE_MOVEMENT);
      }
    }
  }

  for (sGridY = gsRecompileAreaTop; sGridY <= gsRecompileAreaBottom; sGridY++) {
    for (sGridX = gsRecompileAreaLeft; sGridX <= gsRecompileAreaRight; sGridX++) {
      usGridNo = MAPROWCOLTOPOS(sGridY, sGridX);
      if (usGridNo < WORLD_MAX) {
        CompileTileMovementCosts(usGridNo);
      }
    }
  }
}

void RecompileLocalMovementCostsForWall(int16_t sGridNo, uint8_t ubOrientation) {
  int8_t bDirLoop;
  int16_t sUp, sDown, sLeft, sRight;
  int16_t sX, sY, sTempGridNo;

  switch (ubOrientation) {
    case OUTSIDE_TOP_RIGHT:
    case INSIDE_TOP_RIGHT:
      sUp = -1;
      sDown = 1;
      sLeft = 0;
      sRight = 1;
      break;
    case OUTSIDE_TOP_LEFT:
    case INSIDE_TOP_LEFT:
      sUp = 0;
      sDown = 1;
      sLeft = -1;
      sRight = 1;
      break;
    default:
      return;
  }

  for (sY = sUp; sY <= sDown; sY++) {
    for (sX = sLeft; sX <= sRight; sX++) {
      sTempGridNo = sGridNo + sX + sY * WORLD_COLS;
      for (bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++) {
        gubWorldMovementCosts[sTempGridNo][bDirLoop][0] = 0;
        gubWorldMovementCosts[sTempGridNo][bDirLoop][1] = 0;
      }

      CompileTileMovementCosts(sTempGridNo);
    }
  }
}

// GLOBAL WORLD MANIPULATION FUNCTIONS
void CompileWorldMovementCosts() {
  uint16_t usGridNo;

  memset(gubWorldMovementCosts, 0, sizeof(gubWorldMovementCosts));

  CompileWorldTerrainIDs();
  for (usGridNo = 0; usGridNo < WORLD_MAX; usGridNo++) {
    CompileTileMovementCosts(usGridNo);
  }
}

// SAVING CODE
BOOLEAN SaveWorld(char *puiFilename) {
#ifdef JA2EDITOR
  int32_t cnt;
  uint32_t uiSoldierSize;
  uint32_t uiType;
  uint32_t uiFlags;
  uint32_t uiBytesWritten;
  uint32_t uiNumWarningsCaught = 0;
  FileID hfile = FILE_ID_ERR;
  struct LEVELNODE *pLand;
  struct LEVELNODE *pObject;
  struct LEVELNODE *pStruct;
  struct LEVELNODE *pShadow;
  struct LEVELNODE *pRoof;
  struct LEVELNODE *pOnRoof;
  struct LEVELNODE *pTailLand = NULL;
  uint16_t usNumExitGrids = 0;
  uint16_t usTypeSubIndex;
  uint8_t LayerCount;
  uint8_t ObjectCount;
  uint8_t StructCount;
  uint8_t ShadowCount;
  uint8_t RoofCount;
  uint8_t OnRoofCount;
  uint8_t ubType;
  uint8_t ubTypeSubIndex;
  uint8_t ubTest = 1;
  char aFilename[255];
  uint8_t ubCombine;
  uint8_t bCounts[WORLD_MAX][8];

  sprintf(aFilename, "MAPS\\%s", puiFilename);

  // Open file
  hfile = File_OpenForWriting(aFilename);

  if (!hfile) {
    return (FALSE);
  }

  // Write JA2 Version ID
  File_Write(hfile, &gdMajorMapVersion, sizeof(float), &uiBytesWritten);
  if (gdMajorMapVersion >= 4.00) {
    File_Write(hfile, &gubMinorMapVersion, sizeof(uint8_t), &uiBytesWritten);
  }

  // Write FLAGS FOR WORLD
  // uiFlags = MAP_WORLDONLY_SAVED;
  uiFlags = 0;
  uiFlags |= MAP_FULLSOLDIER_SAVED;
  uiFlags |= MAP_EXITGRIDS_SAVED;
  uiFlags |= MAP_WORLDLIGHTS_SAVED;
  uiFlags |= MAP_DOORTABLE_SAVED;
  uiFlags |= MAP_WORLDITEMS_SAVED;
  uiFlags |= MAP_EDGEPOINTS_SAVED;
  if (gfBasement || gfCaves) uiFlags |= MAP_AMBIENTLIGHTLEVEL_SAVED;
  uiFlags |= MAP_NPCSCHEDULES_SAVED;

  File_Write(hfile, &uiFlags, sizeof(int32_t), &uiBytesWritten);

  // Write tileset ID
  File_Write(hfile, &giCurrentTilesetID, sizeof(int32_t), &uiBytesWritten);

  // Write SOLDIER CONTROL SIZE
  uiSoldierSize = sizeof(struct SOLDIERTYPE);
  File_Write(hfile, &uiSoldierSize, sizeof(int32_t), &uiBytesWritten);

  // REMOVE WORLD VISIBILITY TILES
  RemoveWorldWireFrameTiles();

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write out height values
    File_Write(hfile, &gpWorldLevelData[cnt].sHeight, sizeof(int16_t), &uiBytesWritten);
  }

  // Write out # values - we'll have no more than 15 per level!
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Determine number of land
    pLand = gpWorldLevelData[cnt].pLandHead;
    LayerCount = 0;

    while (pLand != NULL) {
      LayerCount++;
      pLand = pLand->pNext;
    }
    if (LayerCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  Land count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               LayerCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (LayerCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  Land count warning of %d for gridno %d.",
               uiNumWarningsCaught, LayerCount, cnt);
    }
    bCounts[cnt][0] = LayerCount;

    // Combine # of land layers with worlddef flags ( first 4 bits )
    ubCombine = (uint8_t)((LayerCount & 0xf) | ((gpWorldLevelData[cnt].uiFlags & 0xf) << 4));
    // Write combination
    File_Write(hfile, &ubCombine, sizeof(ubCombine), &uiBytesWritten);

    // Determine # of objects
    pObject = gpWorldLevelData[cnt].pObjectHead;
    ObjectCount = 0;
    while (pObject != NULL) {
      // DON'T WRITE ANY ITEMS
      if (!(pObject->uiFlags & (LEVELNODE_ITEM))) {
        uint32_t uiTileType;
        // Make sure this isn't a UI Element
        GetTileType(pObject->usIndex, &uiTileType);
        if (uiTileType < FIRSTPOINTERS) ObjectCount++;
      }
      pObject = pObject->pNext;
    }
    if (ObjectCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  Object count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               ObjectCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (ObjectCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  Object count warning of %d for gridno %d.",
               uiNumWarningsCaught, ObjectCount, cnt);
    }
    bCounts[cnt][1] = ObjectCount;

    // Determine # of structs
    pStruct = gpWorldLevelData[cnt].pStructHead;
    StructCount = 0;
    while (pStruct != NULL) {
      // DON'T WRITE ANY ITEMS
      if (!(pStruct->uiFlags & (LEVELNODE_ITEM))) {
        StructCount++;
      }
      pStruct = pStruct->pNext;
    }
    if (StructCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  Struct count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               StructCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (StructCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  Struct count warning of %d for gridno %d.",
               uiNumWarningsCaught, StructCount, cnt);
    }
    bCounts[cnt][2] = StructCount;

    ubCombine = (uint8_t)((ObjectCount & 0xf) | ((StructCount & 0xf) << 4));
    // Write combination
    File_Write(hfile, &ubCombine, sizeof(ubCombine), &uiBytesWritten);

    // Determine # of shadows
    pShadow = gpWorldLevelData[cnt].pShadowHead;
    ShadowCount = 0;
    while (pShadow != NULL) {
      // Don't write any shadowbuddys or exit grids
      if (!(pShadow->uiFlags & (LEVELNODE_BUDDYSHADOW | LEVELNODE_EXITGRID))) {
        ShadowCount++;
      }
      pShadow = pShadow->pNext;
    }
    if (ShadowCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  Shadow count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               ShadowCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (ShadowCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  Shadow count warning of %d for gridno %d.",
               uiNumWarningsCaught, ShadowCount, cnt);
    }
    bCounts[cnt][3] = ShadowCount;

    // Determine # of Roofs
    pRoof = gpWorldLevelData[cnt].pRoofHead;
    RoofCount = 0;
    while (pRoof != NULL) {
      // ATE: Don't save revealed roof info...
      if (pRoof->usIndex != SLANTROOFCEILING1) {
        RoofCount++;
      }
      pRoof = pRoof->pNext;
    }
    if (RoofCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  Roof count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               RoofCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (RoofCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  Roof count warning of %d for gridno %d.",
               uiNumWarningsCaught, RoofCount, cnt);
    }
    bCounts[cnt][4] = RoofCount;

    ubCombine = (uint8_t)((ShadowCount & 0xf) | ((RoofCount & 0xf) << 4));
    // Write combination
    File_Write(hfile, &ubCombine, sizeof(ubCombine), &uiBytesWritten);

    // Write OnRoof layer
    // Determine # of OnRoofs
    pOnRoof = gpWorldLevelData[cnt].pOnRoofHead;
    OnRoofCount = 0;

    while (pOnRoof != NULL) {
      OnRoofCount++;
      pOnRoof = pOnRoof->pNext;
    }
    if (OnRoofCount > 15) {
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"SAVE ABORTED!  OnRoof count too high (%d) for gridno %d."
               L"  Need to fix before map can be saved!  There are %d additional warnings.",
               OnRoofCount, cnt, uiNumWarningsCaught);
      gfErrorCatch = TRUE;
      File_Close(hfile);
      return FALSE;
    }
    if (OnRoofCount > 10) {
      uiNumWarningsCaught++;
      gfErrorCatch = TRUE;
      swprintf(gzErrorCatchString, ARR_SIZE(gzErrorCatchString),
               L"Warnings %d -- Last warning:  OnRoof count warning of %d for gridno %d.",
               uiNumWarningsCaught, OnRoofCount, cnt);
    }
    bCounts[cnt][5] = RoofCount;

    // Write combination of onroof and nothing...
    ubCombine = (uint8_t)((OnRoofCount & 0xf));
    // Write combination
    File_Write(hfile, &ubCombine, sizeof(ubCombine), &uiBytesWritten);
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (bCounts[cnt][0] == 0) {
      File_Write(hfile, &ubTest, sizeof(uint8_t), &uiBytesWritten);
      File_Write(hfile, &ubTest, sizeof(uint8_t), &uiBytesWritten);
    } else {
      // Write land layers
      // Write out land peices backwards so that they are loaded properly
      pLand = gpWorldLevelData[cnt].pLandHead;
      // GET TAIL
      while (pLand != NULL) {
        pTailLand = pLand;
        pLand = pLand->pNext;
      }

      while (pTailLand != NULL) {
        // Write out object type and sub-index
        GetTileType(pTailLand->usIndex, &uiType);
        ubType = (uint8_t)uiType;
        GetTypeSubIndexFromTileIndexChar(uiType, pTailLand->usIndex, &ubTypeSubIndex);
        File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
        File_Write(hfile, &ubTypeSubIndex, sizeof(uint8_t), &uiBytesWritten);

        pTailLand = pTailLand->pPrevNode;
      }
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write object layer
    pObject = gpWorldLevelData[cnt].pObjectHead;
    while (pObject != NULL) {
      // DON'T WRITE ANY ITEMS
      if (!(pObject->uiFlags & (LEVELNODE_ITEM))) {
        // Write out object type and sub-index
        GetTileType(pObject->usIndex, &uiType);
        // Make sure this isn't a UI Element
        if (uiType < FIRSTPOINTERS) {
          // We are writing 2 bytes for the type subindex in the object layer because the
          // ROADPIECES slot contains more than 256 subindices.
          ubType = (uint8_t)uiType;
          GetTypeSubIndexFromTileIndex(uiType, pObject->usIndex, &usTypeSubIndex);
          File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
          File_Write(hfile, &usTypeSubIndex, sizeof(uint16_t), &uiBytesWritten);
        }
      }
      pObject = pObject->pNext;
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write struct layer
    pStruct = gpWorldLevelData[cnt].pStructHead;
    while (pStruct != NULL) {
      // DON'T WRITE ANY ITEMS
      if (!(pStruct->uiFlags & (LEVELNODE_ITEM))) {
        // Write out object type and sub-index
        GetTileType(pStruct->usIndex, &uiType);
        ubType = (uint8_t)uiType;
        GetTypeSubIndexFromTileIndexChar(uiType, pStruct->usIndex, &ubTypeSubIndex);
        File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
        File_Write(hfile, &ubTypeSubIndex, sizeof(uint8_t), &uiBytesWritten);
      }

      pStruct = pStruct->pNext;
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write shadows
    pShadow = gpWorldLevelData[cnt].pShadowHead;
    while (pShadow != NULL) {
      // Dont't write any buddys or exit grids
      if (!(pShadow->uiFlags & (LEVELNODE_BUDDYSHADOW | LEVELNODE_EXITGRID))) {
        // Write out object type and sub-index
        // Write out object type and sub-index
        GetTileType(pShadow->usIndex, &uiType);
        ubType = (uint8_t)uiType;
        GetTypeSubIndexFromTileIndexChar(uiType, pShadow->usIndex, &ubTypeSubIndex);
        File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
        File_Write(hfile, &ubTypeSubIndex, sizeof(uint8_t), &uiBytesWritten);

      } else if (pShadow->uiFlags & LEVELNODE_EXITGRID) {  // count the number of exitgrids
        usNumExitGrids++;
      }

      pShadow = pShadow->pNext;
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    pRoof = gpWorldLevelData[cnt].pRoofHead;
    while (pRoof != NULL) {
      // ATE: Don't save revealed roof info...
      if (pRoof->usIndex != SLANTROOFCEILING1) {
        // Write out object type and sub-index
        GetTileType(pRoof->usIndex, &uiType);
        ubType = (uint8_t)uiType;
        GetTypeSubIndexFromTileIndexChar(uiType, pRoof->usIndex, &ubTypeSubIndex);
        File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
        File_Write(hfile, &ubTypeSubIndex, sizeof(uint8_t), &uiBytesWritten);
      }

      pRoof = pRoof->pNext;
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write OnRoofs
    pOnRoof = gpWorldLevelData[cnt].pOnRoofHead;
    while (pOnRoof != NULL) {
      // Write out object type and sub-index
      GetTileType(pOnRoof->usIndex, &uiType);
      ubType = (uint8_t)uiType;
      GetTypeSubIndexFromTileIndexChar(uiType, pOnRoof->usIndex, &ubTypeSubIndex);
      File_Write(hfile, &ubType, sizeof(uint8_t), &uiBytesWritten);
      File_Write(hfile, &ubTypeSubIndex, sizeof(uint8_t), &uiBytesWritten);

      pOnRoof = pOnRoof->pNext;
    }
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Write out room information
    File_Write(hfile, &gubWorldRoomInfo[cnt], sizeof(int8_t), &uiBytesWritten);
  }

  if (uiFlags & MAP_WORLDITEMS_SAVED) {
    // Write out item information
    SaveWorldItemsToMap(hfile);
  }

  if (uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED) {
    File_Write(hfile, &gfBasement, 1, &uiBytesWritten);
    File_Write(hfile, &gfCaves, 1, &uiBytesWritten);
    File_Write(hfile, &ubAmbientLightLevel, 1, &uiBytesWritten);
  }

  if (uiFlags & MAP_WORLDLIGHTS_SAVED) {
    SaveMapLights(hfile);
  }

  SaveMapInformation(hfile);

  if (uiFlags & MAP_FULLSOLDIER_SAVED) {
    SaveSoldiersToMap(hfile);
  }
  if (uiFlags & MAP_EXITGRIDS_SAVED) {
    SaveExitGrids(hfile, usNumExitGrids);
  }
  if (uiFlags & MAP_DOORTABLE_SAVED) {
    SaveDoorTableToMap(hfile);
  }
  if (uiFlags & MAP_EDGEPOINTS_SAVED) {
    CompileWorldMovementCosts();
    GenerateMapEdgepoints();
    SaveMapEdgepoints(hfile);
  }
  if (uiFlags & MAP_NPCSCHEDULES_SAVED) {
    SaveSchedules(hfile);
  }

  File_Close(hfile);

  sprintf(gubFilename, puiFilename);
#endif  // JA2EDITOR

  return (TRUE);
}

#define NUM_DIR_SEARCHES 5
int8_t bDirectionsForShadowSearch[NUM_DIR_SEARCHES] = {WEST, SOUTHWEST, SOUTH, SOUTHEAST, EAST};

void OptimizeMapForShadows() {
  int32_t cnt, dir;
  int16_t sNewGridNo;
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // CHECK IF WE ARE A TREE HERE
    if (IsTreePresentAtGridno((int16_t)cnt)) {
      // CHECK FOR A struct STRUCTURE A FOOTPRINT AWAY
      for (dir = 0; dir < NUM_DIR_SEARCHES; dir++) {
        sNewGridNo =
            NewGridNo((int16_t)cnt, (uint16_t)DirectionInc(bDirectionsForShadowSearch[dir]));

        if (gpWorldLevelData[sNewGridNo].pStructureHead == NULL) {
          break;
        }
      }
      // If we made it here, remove shadow!
      // We're full of structures
      if (dir == NUM_DIR_SEARCHES) {
        RemoveAllShadows(cnt);
        // Display message to effect
      }
    }
  }
}

void SetBlueFlagFlags(void) {
  int32_t cnt;
  struct LEVELNODE *pNode;

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    pNode = gpWorldLevelData[cnt].pStructHead;
    while (pNode) {
      if (pNode->usIndex == BLUEFLAG_GRAPHIC) {
        gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;
        break;
      }
      pNode = pNode->pNext;
    }
  }
}

void InitLoadedWorld() {
  // if the current sector is not valid, dont init the world
  if (gWorldSectorX == 0 || gWorldSectorY == 0) {
    return;
  }

  // COMPILE MOVEMENT COSTS
  CompileWorldMovementCosts();

  // COMPILE INTERACTIVE TILES
  CompileInteractiveTiles();

  // COMPILE WORLD VISIBLIY TILES
  CalculateWorldWireFrameTiles(TRUE);

  LightSpriteRenderAll();

  OptimizeMapForShadows();

  SetInterfaceHeightLevel();

  // ATE: if we have a slide location, remove it!
  gTacticalStatus.sSlideTarget = NOWHERE;

  SetBlueFlagFlags();
}

// This is a specialty function that is very similar to LoadWorld, except that it
// doesn't actually load the world, it instead evaluates the map and generates summary
// information for use within the summary editor.  The header is defined in Summary Info.h,
// not worlddef.h -- though it's not likely this is going to be used anywhere where it would
// matter.
extern double MasterStart, MasterEnd;
extern BOOLEAN gfUpdatingNow;

BOOLEAN EvaluateWorld(char *pSector, uint8_t ubLevel) {
  float dMajorMapVersion;
  SUMMARYFILE *pSummary;
  FileID hfile = FILE_ID_ERR;
  MAPCREATE_STRUCT mapInfo;
  int8_t *pBuffer, *pBufferHead;
  uint32_t uiFileSize;
  uint32_t uiFlags;
  uint32_t uiBytesRead;
  int32_t cnt;
  int32_t i;
  int32_t iTilesetID;
  wchar_t str[40];
  uint8_t bCounts[WORLD_MAX][8];
  uint8_t ubCombine;
  char szDirFilename[50];
  char szFilename[40];
  uint8_t ubMinorMapVersion;

  // Make sure the file exists... if not, then return false
  strcpy(szFilename, pSector);
  if (ubLevel % 4) {
    char str[4];
    sprintf(str, "_b%d", ubLevel % 4);
    strcat(szFilename, str);
  }
  if (ubLevel >= 4) {
    strcat(szFilename, "_a");
  }
  strcat(szFilename, ".dat");
  sprintf(szDirFilename, "MAPS\\%s", szFilename);

  if (gfMajorUpdate) {
    if (!LoadWorld(szFilename))  // error
      return FALSE;
    Plat_RemoveReadOnlyAttribute(szDirFilename);
    SaveWorld(szFilename);
  }

  hfile = File_OpenForReading(szDirFilename);
  if (!hfile) return FALSE;

  uiFileSize = File_GetSize(hfile);
  pBuffer = (int8_t *)MemAlloc(uiFileSize);
  pBufferHead = pBuffer;
  File_Read(hfile, pBuffer, uiFileSize, &uiBytesRead);
  File_Close(hfile);

  swprintf(str, ARR_SIZE(str), L"Analyzing map %S", szFilename);
  if (!gfUpdatingNow)
    SetRelativeStartAndEndPercentage(0, 0, 100, str);
  else
    SetRelativeStartAndEndPercentage(0, (uint16_t)MasterStart, (uint16_t)MasterEnd, str);

  RenderProgressBar(0, 0);
  // RenderProgressBar( 1, 0 );

  // clear the summary file info
  pSummary = (SUMMARYFILE *)MemAlloc(sizeof(SUMMARYFILE));
  Assert(pSummary);
  memset(pSummary, 0, sizeof(SUMMARYFILE));
  pSummary->ubSummaryVersion = GLOBAL_SUMMARY_VERSION;
  pSummary->dMajorMapVersion = gdMajorMapVersion;

  // skip JA2 Version ID
  LOADDATA(&dMajorMapVersion, pBuffer, sizeof(float));
  if (dMajorMapVersion >= 4.00) {
    LOADDATA(&ubMinorMapVersion, pBuffer, sizeof(uint8_t));
  }

  // Read FLAGS FOR WORLD
  LOADDATA(&uiFlags, pBuffer, sizeof(int32_t));

  // Read tilesetID
  LOADDATA(&iTilesetID, pBuffer, sizeof(int32_t));
  pSummary->ubTilesetID = (uint8_t)iTilesetID;

  // skip soldier size
  pBuffer += sizeof(int32_t);

  // skip height values
  pBuffer += sizeof(int16_t) * WORLD_MAX;

  // read layer counts
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (!(cnt % 2560)) {
      RenderProgressBar(0, (cnt / 2560) + 1);  // 1 - 10
      // RenderProgressBar( 1, (cnt / 2560)+1 ); //1 - 10
    }
    // Read combination of land/world flags
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));
    // split
    bCounts[cnt][0] = (uint8_t)(ubCombine & 0xf);
    gpWorldLevelData[cnt].uiFlags |= (uint8_t)((ubCombine & 0xf0) >> 4);
    // Read #objects, structs
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));
    // split
    bCounts[cnt][1] = (uint8_t)(ubCombine & 0xf);
    bCounts[cnt][2] = (uint8_t)((ubCombine & 0xf0) >> 4);
    // Read shadows, roof
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));
    // split
    bCounts[cnt][3] = (uint8_t)(ubCombine & 0xf);
    bCounts[cnt][4] = (uint8_t)((ubCombine & 0xf0) >> 4);
    // Read OnRoof, nothing
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));
    // split
    bCounts[cnt][5] = (uint8_t)(ubCombine & 0xf);
    // bCounts[ cnt ][4] = (uint8_t)((ubCombine&0xf0)>>4);
    bCounts[cnt][6] = bCounts[cnt][0] + bCounts[cnt][1] + bCounts[cnt][2] + bCounts[cnt][3] +
                      bCounts[cnt][4] + bCounts[cnt][5];
  }
  // skip all layers
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (!(cnt % 320)) {
      RenderProgressBar(0, (cnt / 320) + 11);  // 11 - 90
      // RenderProgressBar( 1, (cnt / 320)+11 ); //11 - 90
    }
    pBuffer += sizeof(uint16_t) * bCounts[cnt][6];
    pBuffer += bCounts[cnt][1];
  }

  // extract highest room number
  {
    uint8_t ubRoomNum;
    for (cnt = 0; cnt < WORLD_MAX; cnt++) {
      LOADDATA(&ubRoomNum, pBuffer, 1);
      if (ubRoomNum > pSummary->ubNumRooms) {
        pSummary->ubNumRooms = ubRoomNum;
      }
    }
  }

  if (uiFlags & MAP_WORLDITEMS_SAVED) {
    uint32_t temp;
    RenderProgressBar(0, 91);
    // RenderProgressBar( 1, 91 );
    // get number of items (for now)
    LOADDATA(&temp, pBuffer, 4);
    pSummary->usNumItems = (uint16_t)temp;
    // Important:  Saves the file position (byte offset) of the position where the numitems
    //            resides.  Checking this value and comparing to usNumItems will ensure validity.
    if (pSummary->usNumItems) {
      pSummary->uiNumItemsPosition = pBuffer - pBufferHead - 4;
    }
    // Skip the contents of the world items.
    pBuffer += sizeof(WORLDITEM) * pSummary->usNumItems;
  }

  if (uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED) {
    pBuffer += 3;
  }

  if (uiFlags & MAP_WORLDLIGHTS_SAVED) {
    uint8_t ubTemp;
    RenderProgressBar(0, 92);
    // RenderProgressBar( 1, 92 );
    // skip number of light palette entries
    LOADDATA(&ubTemp, pBuffer, 1);
    pBuffer += sizeof(struct SGPPaletteEntry) * ubTemp;
    // get number of lights
    LOADDATA(&pSummary->usNumLights, pBuffer, 2);
    // skip the light loading
    for (cnt = 0; cnt < pSummary->usNumLights; cnt++) {
      uint8_t ubStrLen;
      pBuffer += sizeof(LIGHT_SPRITE);
      LOADDATA(&ubStrLen, pBuffer, 1);
      if (ubStrLen) {
        pBuffer += ubStrLen;
      }
    }
  }

  // read the mapinformation
  LOADDATA(&mapInfo, pBuffer, sizeof(MAPCREATE_STRUCT));

  memcpy(&pSummary->MapInfo, &mapInfo, sizeof(MAPCREATE_STRUCT));

  if (uiFlags & MAP_FULLSOLDIER_SAVED) {
    TEAMSUMMARY *pTeam = NULL;
    BASIC_SOLDIERCREATE_STRUCT basic;
    SOLDIERCREATE_STRUCT priority;
    RenderProgressBar(0, 94);
    // RenderProgressBar( 1, 94 );

    pSummary->uiEnemyPlacementPosition = pBuffer - pBufferHead;

    for (i = 0; i < pSummary->MapInfo.ubNumIndividuals; i++) {
      LOADDATA(&basic, pBuffer, sizeof(BASIC_SOLDIERCREATE_STRUCT));

      switch (basic.bTeam) {
        case ENEMY_TEAM:
          pTeam = &pSummary->EnemyTeam;
          break;
        case CREATURE_TEAM:
          pTeam = &pSummary->CreatureTeam;
          break;
        case MILITIA_TEAM:
          pTeam = &pSummary->RebelTeam;
          break;
        case CIV_TEAM:
          pTeam = &pSummary->CivTeam;
          break;
      }
      if (basic.bOrders == RNDPTPATROL ||
          basic.bOrders == POINTPATROL) {  // make sure the placement has at least one waypoint.
        if (!basic.bPatrolCnt) {
          pSummary->ubEnemiesReqWaypoints++;
        }
      } else if (basic.bPatrolCnt) {
        pSummary->ubEnemiesHaveWaypoints++;
      }
      if (basic.fPriorityExistance) pTeam->ubExistance++;
      switch (basic.bRelativeAttributeLevel) {
        case 0:
          pTeam->ubBadA++;
          break;
        case 1:
          pTeam->ubPoorA++;
          break;
        case 2:
          pTeam->ubAvgA++;
          break;
        case 3:
          pTeam->ubGoodA++;
          break;
        case 4:
          pTeam->ubGreatA++;
          break;
      }
      switch (basic.bRelativeEquipmentLevel) {
        case 0:
          pTeam->ubBadE++;
          break;
        case 1:
          pTeam->ubPoorE++;
          break;
        case 2:
          pTeam->ubAvgE++;
          break;
        case 3:
          pTeam->ubGoodE++;
          break;
        case 4:
          pTeam->ubGreatE++;
          break;
      }
      if (basic.fDetailedPlacement) {  // skip static priority placement
        LOADDATA(&priority, pBuffer, sizeof(SOLDIERCREATE_STRUCT));
        if (priority.ubProfile != NO_PROFILE)
          pTeam->ubProfile++;
        else
          pTeam->ubDetailed++;
        if (basic.bTeam == CIV_TEAM) {
          if (priority.ubScheduleID) pSummary->ubCivSchedules++;
          if (priority.bBodyType == COW)
            pSummary->ubCivCows++;
          else if (priority.bBodyType == BLOODCAT)
            pSummary->ubCivBloodcats++;
        }
      }
      if (basic.bTeam == ENEMY_TEAM) {
        switch (basic.ubSoldierClass) {
          case SOLDIER_CLASS_ADMINISTRATOR:
            pSummary->ubNumAdmins++;
            if (basic.fPriorityExistance) pSummary->ubAdminExistance++;
            if (basic.fDetailedPlacement) {
              if (priority.ubProfile != NO_PROFILE)
                pSummary->ubAdminProfile++;
              else
                pSummary->ubAdminDetailed++;
            }
            break;
          case SOLDIER_CLASS_ELITE:
            pSummary->ubNumElites++;
            if (basic.fPriorityExistance) pSummary->ubEliteExistance++;
            if (basic.fDetailedPlacement) {
              if (priority.ubProfile != NO_PROFILE)
                pSummary->ubEliteProfile++;
              else
                pSummary->ubEliteDetailed++;
            }
            break;
          case SOLDIER_CLASS_ARMY:
            pSummary->ubNumTroops++;
            if (basic.fPriorityExistance) pSummary->ubTroopExistance++;
            if (basic.fDetailedPlacement) {
              if (priority.ubProfile != NO_PROFILE)
                pSummary->ubTroopProfile++;
              else
                pSummary->ubTroopDetailed++;
            }
            break;
        }
      } else if (basic.bTeam == CREATURE_TEAM) {
        if (basic.bBodyType == BLOODCAT) pTeam->ubNumAnimals++;
      }
      pTeam->ubTotal++;
    }
    RenderProgressBar(0, 96);
    // RenderProgressBar( 1, 96 );
  }

  if (uiFlags & MAP_EXITGRIDS_SAVED) {
    EXITGRID exitGrid;
    int32_t loop;
    uint16_t usMapIndex;
    BOOLEAN fExitGridFound;
    RenderProgressBar(0, 98);
    // RenderProgressBar( 1, 98 );

    LOADDATA(&cnt, pBuffer, 2);

    for (i = 0; i < cnt; i++) {
      LOADDATA(&usMapIndex, pBuffer, 2);
      LOADDATA(&exitGrid, pBuffer, 5);
      fExitGridFound = FALSE;
      for (loop = 0; loop < pSummary->ubNumExitGridDests; loop++) {
        if (pSummary->ExitGrid[loop].usGridNo == exitGrid.usGridNo &&
            pSummary->ExitGrid[loop].ubGotoSectorX == exitGrid.ubGotoSectorX &&
            pSummary->ExitGrid[loop].ubGotoSectorY == exitGrid.ubGotoSectorY &&
            pSummary->ExitGrid[loop].ubGotoSectorZ ==
                exitGrid.ubGotoSectorZ) {  // same destination.
          pSummary->usExitGridSize[loop]++;
          fExitGridFound = TRUE;
          break;
        }
      }
      if (!fExitGridFound) {
        if (loop >= 4) {
          pSummary->fTooManyExitGridDests = TRUE;
        } else {
          pSummary->ubNumExitGridDests++;
          pSummary->usExitGridSize[loop]++;
          pSummary->ExitGrid[loop].usGridNo = exitGrid.usGridNo;
          pSummary->ExitGrid[loop].ubGotoSectorX = exitGrid.ubGotoSectorX;
          pSummary->ExitGrid[loop].ubGotoSectorY = exitGrid.ubGotoSectorY;
          pSummary->ExitGrid[loop].ubGotoSectorZ = exitGrid.ubGotoSectorZ;
          if (pSummary->ExitGrid[loop].ubGotoSectorX != exitGrid.ubGotoSectorX ||
              pSummary->ExitGrid[loop].ubGotoSectorY != exitGrid.ubGotoSectorY) {
            pSummary->fInvalidDest[loop] = TRUE;
          }
        }
      }
    }
  }

  if (uiFlags & MAP_DOORTABLE_SAVED) {
    DOOR Door;

    LOADDATA(&pSummary->ubNumDoors, pBuffer, 1);

    for (cnt = 0; cnt < pSummary->ubNumDoors; cnt++) {
      LOADDATA(&Door, pBuffer, sizeof(DOOR));

      if (Door.ubTrapID && Door.ubLockID)
        pSummary->ubNumDoorsLockedAndTrapped++;
      else if (Door.ubLockID)
        pSummary->ubNumDoorsLocked++;
      else if (Door.ubTrapID)
        pSummary->ubNumDoorsTrapped++;
    }
  }

  RenderProgressBar(0, 100);
  // RenderProgressBar( 1, 100 );

  MemFree(pBufferHead);

  WriteSectorSummaryUpdate(szFilename, ubLevel, pSummary);
  return TRUE;
}

extern uint8_t GetCurrentSummaryVersion();
extern void LoadShadeTablesFromTextFile();

BOOLEAN LoadWorld(char *puiFilename) {
  FileID hfile = FILE_ID_ERR;
  float dMajorMapVersion;
  uint32_t uiFlags;
  uint32_t uiBytesRead;
  uint32_t uiSoldierSize;
  uint32_t uiFileSize;
  uint32_t fp, offset;
#ifdef JA2TESTVERSION
  uint32_t uiStartTime;
  uint32_t uiLoadWorldStartTime;
#endif
  int32_t cnt, cnt2;
  int32_t iTilesetID;
  uint16_t usTileIndex;
  uint16_t usTypeSubIndex;
  uint8_t ubType;
  uint8_t ubSubIndex;
  char aFilename[256];
  uint8_t ubCombine;
  uint8_t bCounts[WORLD_MAX][8];
  int8_t *pBuffer;
  int8_t *pBufferHead;
  BOOLEAN fGenerateEdgePoints = FALSE;
  uint8_t ubMinorMapVersion;
#ifdef JA2TESTVERSION
  uiLoadWorldStartTime = GetJA2Clock();
#endif

  LoadShadeTablesFromTextFile();

  // Append exension to filename!
  if (gfForceLoad) {
    sprintf(aFilename, "MAPS\\%s", gzForceLoadFile);
  } else {
    sprintf(aFilename, "MAPS\\%s", puiFilename);
  }

  // RESET FLAGS FOR OUTDOORS/INDOORS
  gfBasement = FALSE;
  gfCaves = FALSE;

  // Open file
  hfile = File_OpenForReading(aFilename);

  if (!hfile) {
    SET_ERROR("Could not load map file %S", aFilename);
    return (FALSE);
  }

  SetRelativeStartAndEndPercentage(0, 0, 1, L"Trashing world...");
#ifdef JA2TESTVERSION
  uiStartTime = GetJA2Clock();
#endif

  TrashWorld();

#ifdef JA2TESTVERSION
  uiTrashWorldTime = GetJA2Clock() - uiStartTime;
#endif

  LightReset();

  // Get the file size and alloc one huge buffer for it.
  // We will use this buffer to transfer all of the data from.
  uiFileSize = File_GetSize(hfile);
  pBuffer = (int8_t *)MemAlloc(uiFileSize);
  pBufferHead = pBuffer;
  File_Read(hfile, pBuffer, uiFileSize, &uiBytesRead);
  File_Close(hfile);

  // Read JA2 Version ID
  LOADDATA(&dMajorMapVersion, pBuffer, sizeof(float));

#ifdef RUSSIAN
  if (dMajorMapVersion != 6.00) {
    return FALSE;
  }
#endif

  LOADDATA(&ubMinorMapVersion, pBuffer, sizeof(uint8_t));

  // CHECK FOR NON-COMPATIBLE VERSIONS!
  // CHECK FOR MAJOR MAP VERSION INCOMPATIBLITIES
  // if ( dMajorMapVersion < gdMajorMapVersion )
  //{
  // AssertMsg( 0, "Major version conflict.  Should have force updated this map already!!!" );
  // SET_ERROR(  "Incompatible JA2 map version: %f, map version is now at %f", gdLoadedMapVersion,
  // gdMapVersion ); return( FALSE );
  //}

  // Read FLAGS FOR WORLD
  LOADDATA(&uiFlags, pBuffer, sizeof(int32_t));

  LOADDATA(&iTilesetID, pBuffer, sizeof(int32_t));

#ifdef JA2TESTVERSION
  uiStartTime = GetJA2Clock();
#endif
  if (!(LoadMapTileset(iTilesetID) != FALSE)) {
    return FALSE;
  }
#ifdef JA2TESTVERSION
  uiLoadMapTilesetTime = GetJA2Clock() - uiStartTime;
#endif

  // Load soldier size
  LOADDATA(&uiSoldierSize, pBuffer, sizeof(int32_t));

  // FP 0x000010

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Read height values
    LOADDATA(&gpWorldLevelData[cnt].sHeight, pBuffer, sizeof(int16_t));
  }

  // FP 0x00c810

  SetRelativeStartAndEndPercentage(0, 35, 40, L"Counting layers...");
  RenderProgressBar(0, 100);

  // Read layer counts
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Read combination of land/world flags
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));

    // split
    bCounts[cnt][0] = (uint8_t)(ubCombine & 0xf);
    gpWorldLevelData[cnt].uiFlags |= (uint8_t)((ubCombine & 0xf0) >> 4);

    // Read #objects, structs
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));

    // split
    bCounts[cnt][1] = (uint8_t)(ubCombine & 0xf);
    bCounts[cnt][2] = (uint8_t)((ubCombine & 0xf0) >> 4);

    // Read shadows, roof
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));

    // split
    bCounts[cnt][3] = (uint8_t)(ubCombine & 0xf);
    bCounts[cnt][4] = (uint8_t)((ubCombine & 0xf0) >> 4);

    // Read OnRoof, nothing
    LOADDATA(&ubCombine, pBuffer, sizeof(uint8_t));

    // split
    bCounts[cnt][5] = (uint8_t)(ubCombine & 0xf);
  }

  // FP 0x025810
  fp = 0x025810;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 40, 43, L"Loading land layers...");
  RenderProgressBar(0, 100);

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Read new values
    if (bCounts[cnt][0] > 10) {
      cnt = cnt;
    }
    for (cnt2 = 0; cnt2 < bCounts[cnt][0]; cnt2++) {
      LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
      LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));

      // Get tile index
      GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);

      // Add layer
      AddLandToHead(cnt, usTileIndex);

      offset += 2;
    }
  }

  fp += offset;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 43, 46, L"Loading object layer...");
  RenderProgressBar(0, 100);

  if (0) {  // Old loads
    for (cnt = 0; cnt < WORLD_MAX; cnt++) {
      // Set objects
      for (cnt2 = 0; cnt2 < bCounts[cnt][1]; cnt2++) {
        LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
        LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));
        if (ubType >= FIRSTPOINTERS) {
          continue;
        }
        // Get tile index
        GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);
        // Add layer
        AddObjectToTail(cnt, usTileIndex);
      }
    }
  } else {  // New load require uint16_t for the type subindex due to the fact that ROADPIECES
    // contain over 300 type subindices.
    for (cnt = 0; cnt < WORLD_MAX; cnt++) {
      // Set objects
      if (bCounts[cnt][1] > 10) {
        cnt = cnt;
      }
      for (cnt2 = 0; cnt2 < bCounts[cnt][1]; cnt2++) {
        LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
        LOADDATA(&usTypeSubIndex, pBuffer, sizeof(uint16_t));
        if (ubType >= FIRSTPOINTERS) {
          continue;
        }
        // Get tile index
        GetTileIndexFromTypeSubIndex(ubType, usTypeSubIndex, &usTileIndex);
        // Add layer
        AddObjectToTail(cnt, usTileIndex);

        offset += 3;
      }
    }
  }

  fp += offset;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 46, 49, L"Loading struct layer...");
  RenderProgressBar(0, 100);

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Set structs
    if (bCounts[cnt][2] > 10) {
      cnt = cnt;
    }
    for (cnt2 = 0; cnt2 < bCounts[cnt][2]; cnt2++) {
      LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
      LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));

      // Get tile index
      GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);

      if (ubMinorMapVersion <= 25) {
        // Check patching for phantom menace struct data...
        if (gTileDatabase[usTileIndex].uiFlags & UNDERFLOW_FILLER) {
          GetTileIndexFromTypeSubIndex(ubType, 1, &usTileIndex);
        }
      }

      // Add layer
      AddStructToTail(cnt, usTileIndex);

      offset += 2;
    }
  }

  fp += offset;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 49, 52, L"Loading shadow layer...");
  RenderProgressBar(0, 100);

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (bCounts[cnt][3] > 10) {
      cnt = cnt;
    }
    for (cnt2 = 0; cnt2 < bCounts[cnt][3]; cnt2++) {
      LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
      LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));

      // Get tile index
      GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);

      // Add layer
      AddShadowToTail(cnt, usTileIndex);

      offset += 2;
    }
  }

  fp += offset;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 52, 55, L"Loading roof layer...");
  RenderProgressBar(0, 100);

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (bCounts[cnt][4] > 10) {
      cnt = cnt;
    }
    for (cnt2 = 0; cnt2 < bCounts[cnt][4]; cnt2++) {
      LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
      LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));

      // Get tile index
      GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);

      // Add layer
      AddRoofToTail(cnt, usTileIndex);

      offset += 2;
    }
  }

  fp += offset;
  offset = 0;

  SetRelativeStartAndEndPercentage(0, 55, 58, L"Loading on roof layer...");
  RenderProgressBar(0, 100);

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (bCounts[cnt][5] > 10) {
      cnt = cnt;
    }
    for (cnt2 = 0; cnt2 < bCounts[cnt][5]; cnt2++) {
      LOADDATA(&ubType, pBuffer, sizeof(uint8_t));
      LOADDATA(&ubSubIndex, pBuffer, sizeof(uint8_t));

      // Get tile index
      GetTileIndexFromTypeSubIndex(ubType, ubSubIndex, &usTileIndex);

      // Add layer
      AddOnRoofToTail(cnt, usTileIndex);

      offset += 2;
    }
  }

  fp += offset;
  offset = 0;

#ifdef RUSSIAN
  {
    uint32_t uiNums[37];
    LOADDATA(uiNums, pBuffer, 37 * sizeof(int32_t));
  }
#endif

  SetRelativeStartAndEndPercentage(0, 58, 59, L"Loading room information...");
  RenderProgressBar(0, 100);

#ifdef JA2EDITOR
  gubMaxRoomNumber = 0;
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Read room information
    LOADDATA(&gubWorldRoomInfo[cnt], pBuffer, sizeof(int8_t));
    // Got to set the max room number
    if (gubWorldRoomInfo[cnt] > gubMaxRoomNumber) gubMaxRoomNumber = gubWorldRoomInfo[cnt];
  }
  if (gubMaxRoomNumber < 255) gubMaxRoomNumber++;
#else
  LOADDATA(gubWorldRoomInfo, pBuffer, sizeof(int8_t) * WORLD_MAX);
#endif

  // ATE; Memset this array!
  if (0) {  // for debugging purposes
    memset(gubWorldRoomInfo, 0, sizeof(gubWorldRoomInfo));
  }

  memset(gubWorldRoomHidden, TRUE, sizeof(gubWorldRoomHidden));

  SetRelativeStartAndEndPercentage(0, 59, 61, L"Loading items...");
  RenderProgressBar(0, 100);

  if (uiFlags & MAP_WORLDITEMS_SAVED) {
    // Load out item information
    gfLoadPitsWithoutArming = TRUE;
    LoadWorldItemsFromMap(&pBuffer);
    gfLoadPitsWithoutArming = FALSE;
  }

  SetRelativeStartAndEndPercentage(0, 62, 85, L"Loading lights...");
  RenderProgressBar(0, 0);

  if (uiFlags &
      MAP_AMBIENTLIGHTLEVEL_SAVED) {  // Ambient light levels are only saved in underground levels
    LOADDATA(&gfBasement, pBuffer, 1);
    LOADDATA(&gfCaves, pBuffer, 1);
    LOADDATA(&ubAmbientLightLevel, pBuffer, 1);
  } else {  // We are above ground.
    gfBasement = FALSE;
    gfCaves = FALSE;
    if (!gfEditMode) {
      ubAmbientLightLevel = GetTimeOfDayAmbientLightLevel();
    } else {
      ubAmbientLightLevel = 4;
    }
  }
#ifdef JA2TESTVERSION
  uiStartTime = GetJA2Clock();
#endif
  if (uiFlags & MAP_WORLDLIGHTS_SAVED) {
    LoadMapLights(&pBuffer);
  } else {
    // Set some default value for lighting
    SetDefaultWorldLightingColors();
  }
  LightSetBaseLevel(ubAmbientLightLevel);
#ifdef JA2TESTVERSION
  uiLoadMapLightsTime = GetJA2Clock() - uiStartTime;
#endif

  SetRelativeStartAndEndPercentage(0, 85, 86, L"Loading map information...");
  RenderProgressBar(0, 0);

  LoadMapInformation(&pBuffer);

  if (uiFlags & MAP_FULLSOLDIER_SAVED) {
    SetRelativeStartAndEndPercentage(0, 86, 87, L"Loading placements...");
    RenderProgressBar(0, 0);
    LoadSoldiersFromMap(&pBuffer);
  }
  if (uiFlags & MAP_EXITGRIDS_SAVED) {
    SetRelativeStartAndEndPercentage(0, 87, 88, L"Loading exit grids...");
    RenderProgressBar(0, 0);
    LoadExitGrids(&pBuffer);
  }
  if (uiFlags & MAP_DOORTABLE_SAVED) {
    SetRelativeStartAndEndPercentage(0, 89, 90, L"Loading door tables...");
    RenderProgressBar(0, 0);
    LoadDoorTableFromMap(&pBuffer);
  }
  if (uiFlags & MAP_EDGEPOINTS_SAVED) {
    SetRelativeStartAndEndPercentage(0, 90, 91, L"Loading edgepoints...");
    RenderProgressBar(0, 0);
    if (!LoadMapEdgepoints(&pBuffer))
      fGenerateEdgePoints = TRUE;  // only if the map had the older edgepoint system
  } else {
    fGenerateEdgePoints = TRUE;
  }
  if (uiFlags & MAP_NPCSCHEDULES_SAVED) {
    SetRelativeStartAndEndPercentage(0, 91, 92, L"Loading NPC schedules...");
    RenderProgressBar(0, 0);
    LoadSchedules(&pBuffer);
  }

  ValidateAndUpdateMapVersionIfNecessary();

  // if we arent loading a saved game
  //	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
  {
    SetRelativeStartAndEndPercentage(0, 93, 94, L"Init Loaded World...");
    RenderProgressBar(0, 0);
    InitLoadedWorld();
  }

  if (fGenerateEdgePoints) {
    SetRelativeStartAndEndPercentage(0, 94, 95, L"Generating map edgepoints...");
    RenderProgressBar(0, 0);
    CompileWorldMovementCosts();
    GenerateMapEdgepoints();
  }

  RenderProgressBar(0, 20);

  SetRelativeStartAndEndPercentage(0, 95, 100, L"General initialization...");
  // RESET AI!
  InitOpponentKnowledgeSystem();

  RenderProgressBar(0, 30);

  // AllTeamsLookForAll(NO_INTERRUPTS);

  RenderProgressBar(0, 40);

  // Reset some override flags
  gfForceLoadPlayers = FALSE;
  gfForceLoad = FALSE;

  // CHECK IF OUR SELECTED GUY IS GONE!
  if (gusSelectedSoldier != NO_SOLDIER) {
    if (MercPtrs[gusSelectedSoldier]->bActive == FALSE) {
      gusSelectedSoldier = NO_SOLDIER;
    }
  }

  AdjustSoldierCreationStartValues();

  RenderProgressBar(0, 60);

  InvalidateWorldRedundency();

  // SAVE FILENAME
  strcpy(gzLastLoadedFile, puiFilename);
  LoadRadarScreenBitmap(puiFilename);

  RenderProgressBar(0, 80);

  gfWorldLoaded = TRUE;

  strcpy(gubFilename, puiFilename);

  // Remove this rather large chunk of memory from the system now!
  MemFree(pBufferHead);

#ifdef JA2TESTVERSION
  uiLoadWorldTime = GetJA2Clock() - uiLoadWorldStartTime;
#endif

  RenderProgressBar(0, 100);

  DequeueAllKeyBoardEvents();

  return (TRUE);
}

//****************************************************************************************
//
//	Deletes everything then re-creates the world with simple ground tiles
//
//****************************************************************************************
BOOLEAN NewWorld(void) {
  uint16_t NewIndex;
  int32_t cnt;

  gusSelectedSoldier = gusOldSelectedSoldier = NO_SOLDIER;

  AdjustSoldierCreationStartValues();

  TrashWorld();

  // Create world randomly from tiles
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Set land index
    NewIndex = (uint16_t)(rand() % 10);
    AddLandToHead(cnt, NewIndex);
  }

  InitRoomDatabase();

  gfWorldLoaded = TRUE;

  return (TRUE);
}

void TrashWorld(void) {
  MAP_ELEMENT *pMapTile;
  struct LEVELNODE *pLandNode;
  struct LEVELNODE *pObjectNode;
  struct LEVELNODE *pStructNode;
  struct LEVELNODE *pShadowNode;
  struct LEVELNODE *pMercNode;
  struct LEVELNODE *pRoofNode;
  struct LEVELNODE *pOnRoofNode;
  struct LEVELNODE *pTopmostNode;
  //	struct STRUCTURE			*pStructureNode;
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  if (!gfWorldLoaded) return;

  // REMOVE ALL ITEMS FROM WORLD
  TrashWorldItems();

  // Trash the overhead map
  TrashOverheadMap();

  // Reset the smoke effects.
  ResetSmokeEffects();

  // Reset the light effects
  ResetLightEffects();

  // Set soldiers to not active!
  // ATE: FOR NOW, ONLY TRASH FROM NPC UP!!!!
  // cnt = gTacticalStatus.Team[ gbPlayerNum ].bLastID + 1;
  cnt = 0;

  for (pSoldier = MercPtrs[cnt]; cnt < MAX_NUM_SOLDIERS; pSoldier++, cnt++) {
    if (IsSolActive(pSoldier)) {
      if (pSoldier->bTeam == gbPlayerNum) {
        // Just delete levelnode
        pSoldier->pLevelNode = NULL;
      } else {
        // Delete from world
        TacticalRemoveSoldier((uint16_t)cnt);
      }
    }
  }

  RemoveCorpses();

  // Remove all ani tiles...
  DeleteAniTiles();

  // Kill both soldier init lists.
  UseEditorAlternateList();
  KillSoldierInitList();
  UseEditorOriginalList();
  KillSoldierInitList();

  // Remove the schedules
  DestroyAllSchedules();

  // on trash world sheck if we have to set up the first meanwhile
  HandleFirstMeanWhileSetUpWithTrashWorld();

  // Create world randomly from tiles
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    pMapTile = &gpWorldLevelData[cnt];

    // Free the memory associated with the map tile link lists
    pLandNode = pMapTile->pLandHead;
    while (pLandNode != NULL) {
      pMapTile->pLandHead = pLandNode->pNext;
      MemFree(pLandNode);
      pLandNode = pMapTile->pLandHead;
    }

    pObjectNode = pMapTile->pObjectHead;
    while (pObjectNode != NULL) {
      pMapTile->pObjectHead = pObjectNode->pNext;
      MemFree(pObjectNode);
      pObjectNode = pMapTile->pObjectHead;
    }

    pStructNode = pMapTile->pStructHead;
    while (pStructNode != NULL) {
      pMapTile->pStructHead = pStructNode->pNext;
      MemFree(pStructNode);
      pStructNode = pMapTile->pStructHead;
    }

    pShadowNode = pMapTile->pShadowHead;
    while (pShadowNode != NULL) {
      pMapTile->pShadowHead = pShadowNode->pNext;
      MemFree(pShadowNode);
      pShadowNode = pMapTile->pShadowHead;
    }

    pMercNode = pMapTile->pMercHead;
    while (pMercNode != NULL) {
      pMapTile->pMercHead = pMercNode->pNext;
      MemFree(pMercNode);
      pMercNode = pMapTile->pMercHead;
    }

    pRoofNode = pMapTile->pRoofHead;
    while (pRoofNode != NULL) {
      pMapTile->pRoofHead = pRoofNode->pNext;
      MemFree(pRoofNode);
      pRoofNode = pMapTile->pRoofHead;
    }

    pOnRoofNode = pMapTile->pOnRoofHead;
    while (pOnRoofNode != NULL) {
      pMapTile->pOnRoofHead = pOnRoofNode->pNext;
      MemFree(pOnRoofNode);
      pOnRoofNode = pMapTile->pOnRoofHead;
    }

    pTopmostNode = pMapTile->pTopmostHead;
    while (pTopmostNode != NULL) {
      pMapTile->pTopmostHead = pTopmostNode->pNext;
      MemFree(pTopmostNode);
      pTopmostNode = pMapTile->pTopmostHead;
    }

    while (pMapTile->pStructureHead != NULL) {
      if (DeleteStructureFromWorld(pMapTile->pStructureHead) == FALSE) {
        // ERROR!!!!!!
        break;
      }
    }
  }

  // Zero world
  memset(gpWorldLevelData, 0, WORLD_MAX * sizeof(MAP_ELEMENT));

  // Set some default flags
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RECALCULATE_WIREFRAMES;
  }

  TrashDoorTable();
  TrashMapEdgepoints();
  TrashDoorStatusArray();

  // gfBlitBattleSectorLocator = FALSE;
  gfWorldLoaded = FALSE;
  sprintf(gubFilename, "none");
}

void TrashMapTile(int16_t MapTile) {
  MAP_ELEMENT *pMapTile;
  struct LEVELNODE *pLandNode;
  struct LEVELNODE *pObjectNode;
  struct LEVELNODE *pStructNode;
  struct LEVELNODE *pShadowNode;
  struct LEVELNODE *pMercNode;
  struct LEVELNODE *pRoofNode;
  struct LEVELNODE *pOnRoofNode;
  struct LEVELNODE *pTopmostNode;

  pMapTile = &gpWorldLevelData[MapTile];

  // Free the memory associated with the map tile link lists
  pLandNode = pMapTile->pLandHead;
  while (pLandNode != NULL) {
    pMapTile->pLandHead = pLandNode->pNext;
    MemFree(pLandNode);
    pLandNode = pMapTile->pLandHead;
  }
  pMapTile->pLandHead = pMapTile->pLandStart = NULL;

  pObjectNode = pMapTile->pObjectHead;
  while (pObjectNode != NULL) {
    pMapTile->pObjectHead = pObjectNode->pNext;
    MemFree(pObjectNode);
    pObjectNode = pMapTile->pObjectHead;
  }
  pMapTile->pObjectHead = NULL;

  pStructNode = pMapTile->pStructHead;
  while (pStructNode != NULL) {
    pMapTile->pStructHead = pStructNode->pNext;
    MemFree(pStructNode);
    pStructNode = pMapTile->pStructHead;
  }
  pMapTile->pStructHead = NULL;

  pShadowNode = pMapTile->pShadowHead;
  while (pShadowNode != NULL) {
    pMapTile->pShadowHead = pShadowNode->pNext;
    MemFree(pShadowNode);
    pShadowNode = pMapTile->pShadowHead;
  }
  pMapTile->pShadowHead = NULL;

  pMercNode = pMapTile->pMercHead;
  while (pMercNode != NULL) {
    pMapTile->pMercHead = pMercNode->pNext;
    MemFree(pMercNode);
    pMercNode = pMapTile->pMercHead;
  }
  pMapTile->pMercHead = NULL;

  pRoofNode = pMapTile->pRoofHead;
  while (pRoofNode != NULL) {
    pMapTile->pRoofHead = pRoofNode->pNext;
    MemFree(pRoofNode);
    pRoofNode = pMapTile->pRoofHead;
  }
  pMapTile->pRoofHead = NULL;

  pOnRoofNode = pMapTile->pOnRoofHead;
  while (pOnRoofNode != NULL) {
    pMapTile->pOnRoofHead = pOnRoofNode->pNext;
    MemFree(pOnRoofNode);
    pOnRoofNode = pMapTile->pOnRoofHead;
  }
  pMapTile->pOnRoofHead = NULL;

  pTopmostNode = pMapTile->pTopmostHead;
  while (pTopmostNode != NULL) {
    pMapTile->pTopmostHead = pTopmostNode->pNext;
    MemFree(pTopmostNode);
    pTopmostNode = pMapTile->pTopmostHead;
  }
  pMapTile->pTopmostHead = NULL;

  while (pMapTile->pStructureHead != NULL) {
    DeleteStructureFromWorld(pMapTile->pStructureHead);
  }
  pMapTile->pStructureHead = pMapTile->pStructureTail = NULL;
}

BOOLEAN LoadMapTileset(int32_t iTilesetID) {
  if (iTilesetID >= NUM_TILESETS) {
    return (FALSE);
  }

  // Init tile surface used values
  memset(gbNewTileSurfaceLoaded, 0, sizeof(gbNewTileSurfaceLoaded));

  if (iTilesetID == giCurrentTilesetID) {
    return (TRUE);
  }

  // LOAD SURFACES
  CHECKF(LoadTileSurfaces(&(gTilesets[iTilesetID].TileSurfaceFilenames[0]), (uint8_t)iTilesetID) !=
         FALSE);

  // SET TERRAIN COSTS
  if (gTilesets[iTilesetID].MovementCostFnc != NULL) {
    gTilesets[iTilesetID].MovementCostFnc();
  } else {
    DebugMsg(TOPIC_JA2, DBG_INFO,
             String("Tileset %d has no callback function for movement costs. Using default.",
                    iTilesetID));
    SetTilesetOneTerrainValues();
  }

  // RESET TILE DATABASE
  DeallocateTileDatabase();

  CreateTileDatabase();

  // SET GLOBAL ID FOR TILESET ( FOR SAVING! )
  giCurrentTilesetID = iTilesetID;

  return (TRUE);
}

BOOLEAN SaveMapTileset(int32_t iTilesetID) {
  //	FILE *hTSet;
  FileID hTSet = FILE_ID_ERR;
  char zTilesetName[65];
  int cnt;
  uint32_t uiBytesWritten;

  // Are we trying to save the default tileset?
  if (iTilesetID == 0) return (TRUE);

  sprintf(zTilesetName, "TSET%04d.SET", iTilesetID);

  // Open file
  hTSet = File_OpenForWriting(zTilesetName);

  if (!hTSet) {
    return (FALSE);
  }

  // Save current tile set in map file.
  for (cnt = 0; cnt < NUMBEROFTILETYPES; cnt++)
    File_Write(hTSet, TileSurfaceFilenames[cnt], 65, &uiBytesWritten);
  File_Close(hTSet);

  return (TRUE);
}

void SetLoadOverrideParams(BOOLEAN fForceLoad, BOOLEAN fForceFile, char *zLoadName) {
  gfForceLoadPlayers = fForceLoad;
  gfForceLoad = fForceFile;

  if (zLoadName != NULL) {
    strcpy(gzForceLoadFile, zLoadName);
  }
}

void AddWireFrame(int16_t sGridNo, uint16_t usIndex, BOOLEAN fForced) {
  struct LEVELNODE *pTopmost, *pTopmostTail;

  pTopmost = gpWorldLevelData[sGridNo].pTopmostHead;

  while (pTopmost != NULL) {
    // Check if one of the samer type exists!
    if (pTopmost->usIndex == usIndex) {
      return;
    }
    pTopmost = pTopmost->pNext;
  }

  pTopmostTail = AddTopmostToTail(sGridNo, usIndex);

  if (fForced) {
    pTopmostTail->uiFlags |= LEVELNODE_WIREFRAME;
  }
}

uint16_t GetWireframeGraphicNumToUseForWall(int16_t sGridNo, struct STRUCTURE *pStructure) {
  struct LEVELNODE *pNode = NULL;
  uint8_t ubWallOrientation;
  uint16_t usValue = 0;
  uint16_t usSubIndex;
  struct STRUCTURE *pBaseStructure;

  ubWallOrientation = pStructure->ubWallOrientation;

  pBaseStructure = FindBaseStructure(pStructure);

  if (pBaseStructure) {
    // Find levelnode...
    pNode = gpWorldLevelData[sGridNo].pStructHead;
    while (pNode != NULL) {
      if (pNode->pStructureData == pBaseStructure) {
        break;
      }
      pNode = pNode->pNext;
    }

    if (pNode != NULL) {
      // Get Subindex for this wall...
      GetSubIndexFromTileIndex(pNode->usIndex, &usSubIndex);

      // Check for broken peices...
      if (usSubIndex == 48 || usSubIndex == 52) {
        return (WIREFRAMES12);
      } else if (usSubIndex == 49 || usSubIndex == 53) {
        return (WIREFRAMES13);
      } else if (usSubIndex == 50 || usSubIndex == 54) {
        return (WIREFRAMES10);
      } else if (usSubIndex == 51 || usSubIndex == 55) {
        return (WIREFRAMES11);
      }
    }
  }

  switch (ubWallOrientation) {
    case OUTSIDE_TOP_LEFT:
    case INSIDE_TOP_LEFT:

      usValue = WIREFRAMES6;
      break;

    case OUTSIDE_TOP_RIGHT:
    case INSIDE_TOP_RIGHT:
      usValue = WIREFRAMES5;
      break;
  }

  return (usValue);
}

void CalculateWorldWireFrameTiles(BOOLEAN fForce) {
  int32_t cnt;
  struct STRUCTURE *pStructure;
  int16_t sGridNo;
  uint8_t ubWallOrientation;
  int8_t bHiddenVal;
  int8_t bNumWallsSameGridNo;
  uint16_t usWireFrameIndex;

  // Create world randomly from tiles
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RECALCULATE_WIREFRAMES || fForce) {
      // Turn off flag
      gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RECALCULATE_WIREFRAMES);

      // Remove old ones
      RemoveWireFrameTiles((int16_t)cnt);

      bNumWallsSameGridNo = 0;

      // Check our gridno, if we have a roof over us that has not beenr evealed, no need for a
      // wiereframe
      if (IsRoofVisibleForWireframe((uint16_t)cnt) &&
          !(gpWorldLevelData[cnt].uiFlags & MAPELEMENT_REVEALED)) {
        continue;
      }

      pStructure = gpWorldLevelData[cnt].pStructureHead;

      while (pStructure != NULL) {
        // Check for doors
        if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
          // ATE: need this additional check here for hidden doors!
          if (pStructure->fFlags & STRUCTURE_OPENABLE) {
            // Does the gridno we are over have a non-visible tile?
            // Based on orientation
            ubWallOrientation = pStructure->ubWallOrientation;

            switch (ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(SOUTH));

                if (IsRoofVisibleForWireframe(sGridNo) &&
                    !(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED)) {
                  AddWireFrame(
                      (int16_t)cnt, WIREFRAMES4,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                }
                break;

              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(EAST));

                if (IsRoofVisibleForWireframe(sGridNo) &&
                    !(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED)) {
                  AddWireFrame(
                      (int16_t)cnt, WIREFRAMES3,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                }
                break;
            }
          }
        }
        // Check for windows
        else {
          if (pStructure->fFlags & STRUCTURE_WALLNWINDOW) {
            // Does the gridno we are over have a non-visible tile?
            // Based on orientation
            ubWallOrientation = pStructure->ubWallOrientation;

            switch (ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(SOUTH));

                if (IsRoofVisibleForWireframe(sGridNo) &&
                    !(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED)) {
                  AddWireFrame(
                      (int16_t)cnt, WIREFRAMES2,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                }
                break;

              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(EAST));

                if (IsRoofVisibleForWireframe(sGridNo) &&
                    !(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED)) {
                  AddWireFrame(
                      (int16_t)cnt, WIREFRAMES1,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                }
                break;
            }
          }

          // Check for walls
          if (pStructure->fFlags & STRUCTURE_WALLSTUFF) {
            // Does the gridno we are over have a non-visible tile?
            // Based on orientation
            ubWallOrientation = pStructure->ubWallOrientation;

            usWireFrameIndex = GetWireframeGraphicNumToUseForWall((uint16_t)cnt, pStructure);

            switch (ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(SOUTH));

                if (IsRoofVisibleForWireframe(sGridNo)) {
                  bNumWallsSameGridNo++;

                  AddWireFrame(
                      (int16_t)cnt, usWireFrameIndex,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));

                  // Check along our direction to see if we are a corner
                  sGridNo = NewGridNo((int16_t)cnt, DirectionInc(WEST));
                  sGridNo = NewGridNo(sGridNo, DirectionInc(SOUTH));
                  bHiddenVal = IsHiddenTileMarkerThere(sGridNo);
                  // If we do not exist ( -1 ) or are revealed ( 1 )
                  if (bHiddenVal == -1 || bHiddenVal == 1) {
                    // Place corner!
                    AddWireFrame(
                        (int16_t)cnt, WIREFRAMES9,
                        (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                  }
                }
                break;

              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:

                // Get gridno
                sGridNo = NewGridNo((int16_t)cnt, DirectionInc(EAST));

                if (IsRoofVisibleForWireframe(sGridNo)) {
                  bNumWallsSameGridNo++;

                  AddWireFrame(
                      (int16_t)cnt, usWireFrameIndex,
                      (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));

                  // Check along our direction to see if we are a corner
                  sGridNo = NewGridNo((int16_t)cnt, DirectionInc(NORTH));
                  sGridNo = NewGridNo(sGridNo, DirectionInc(EAST));
                  bHiddenVal = IsHiddenTileMarkerThere(sGridNo);
                  // If we do not exist ( -1 ) or are revealed ( 1 )
                  if (bHiddenVal == -1 || bHiddenVal == 1) {
                    // Place corner!
                    AddWireFrame(
                        (int16_t)cnt, WIREFRAMES8,
                        (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
                  }
                }
                break;
            }

            // Check for both walls
            if (bNumWallsSameGridNo == 2) {
              sGridNo = NewGridNo((int16_t)cnt, DirectionInc(EAST));
              sGridNo = NewGridNo(sGridNo, DirectionInc(SOUTH));
              AddWireFrame(
                  (int16_t)cnt, WIREFRAMES7,
                  (BOOLEAN)((gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0));
            }
          }
        }

        pStructure = pStructure->pNext;
      }
    }
  }
}

void RemoveWorldWireFrameTiles() {
  int32_t cnt;

  // Create world randomly from tiles
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    RemoveWireFrameTiles((int16_t)cnt);
  }
}

void RemoveWireFrameTiles(int16_t sGridNo) {
  struct LEVELNODE *pTopmost, *pNewTopmost;
  TILE_ELEMENT *pTileElement;

  pTopmost = gpWorldLevelData[sGridNo].pTopmostHead;

  while (pTopmost != NULL) {
    pNewTopmost = pTopmost->pNext;

    if (pTopmost->usIndex < NUMBEROFTILES) {
      pTileElement = &(gTileDatabase[pTopmost->usIndex]);

      if (pTileElement->fType == WIREFRAMES) {
        RemoveTopmost(sGridNo, pTopmost->usIndex);
      }
    }

    pTopmost = pNewTopmost;
  }
}

int8_t IsHiddenTileMarkerThere(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  if (!gfBasement) {
    pStructure = FindStructure(sGridNo, STRUCTURE_ROOF);

    if (pStructure != NULL) {
      // if ( !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
      { return (2); }

      // if we are here, a roof exists but has been revealed
      return (1);
    }
  } else {
    // if ( InARoom( sGridNo, &ubRoom ) )
    {
      // if ( !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
      { return (2); }

      return (1);
    }
  }

  return (-1);
}

void ReloadTileset(uint8_t ubID) {
  char aFilename[255];
  int32_t iCurrTilesetID = giCurrentTilesetID;

  // Set gloabal
  giCurrentTilesetID = ubID;

  // Save Map
  SaveWorld(TEMP_FILE_FOR_TILESET_CHANGE);

  // IMPORTANT:  If this is not set, the LoadTileset() will assume that
  // it is loading the same tileset and ignore it...
  giCurrentTilesetID = iCurrTilesetID;

  // Load Map with new tileset
  LoadWorld(TEMP_FILE_FOR_TILESET_CHANGE);

  // Delete file
  sprintf(aFilename, "MAPS\\%s", TEMP_FILE_FOR_TILESET_CHANGE);

  Plat_DeleteFile(aFilename);
}

void SaveMapLights(FileID hfile) {
  struct SOLDIERTYPE *pSoldier;
  struct SGPPaletteEntry LColors[3];
  uint8_t ubNumColors;
  BOOLEAN fSoldierLight;
  uint16_t usNumLights = 0;
  uint16_t cnt, cnt2;
  uint8_t ubStrLen;
  uint32_t uiBytesWritten;

  ubNumColors = LightGetColors(LColors);

  // Save the current light colors!
  File_Write(hfile, &ubNumColors, 1, &uiBytesWritten);
  File_Write(hfile, LColors, sizeof(struct SGPPaletteEntry) * ubNumColors, &uiBytesWritten);

  // count number of non-merc lights.
  for (cnt = 0; cnt < MAX_LIGHT_SPRITES; cnt++) {
    if (LightSprites[cnt].uiFlags & LIGHT_SPR_ACTIVE) {  // found an active light.  Check to make
                                                         // sure it doesn't belong to a merc.
      fSoldierLight = FALSE;
      for (cnt2 = 0; cnt2 < MAX_NUM_SOLDIERS && !fSoldierLight; cnt2++) {
        if (GetSoldier(&pSoldier, cnt2)) {
          if (pSoldier->iLight == (int32_t)cnt) fSoldierLight = TRUE;
        }
      }
      if (!fSoldierLight) usNumLights++;
    }
  }

  // save the number of lights.
  File_Write(hfile, &usNumLights, 2, &uiBytesWritten);

  for (cnt = 0; cnt < MAX_LIGHT_SPRITES; cnt++) {
    if (LightSprites[cnt].uiFlags & LIGHT_SPR_ACTIVE) {  // found an active light.  Check to make
                                                         // sure it doesn't belong to a merc.
      fSoldierLight = FALSE;
      for (cnt2 = 0; cnt2 < MAX_NUM_SOLDIERS && !fSoldierLight; cnt2++) {
        if (GetSoldier(&pSoldier, cnt2)) {
          if (pSoldier->iLight == (int32_t)cnt) fSoldierLight = TRUE;
        }
      }
      if (!fSoldierLight) {  // save the light
        File_Write(hfile, &LightSprites[cnt], sizeof(LIGHT_SPRITE), &uiBytesWritten);

        ubStrLen = (uint8_t)(strlen(pLightNames[LightSprites[cnt].iTemplate]) + 1);
        File_Write(hfile, &ubStrLen, 1, &uiBytesWritten);
        File_Write(hfile, pLightNames[LightSprites[cnt].iTemplate], ubStrLen, &uiBytesWritten);
      }
    }
  }
}

void LoadMapLights(int8_t **hBuffer) {
  struct SGPPaletteEntry LColors[3];
  uint8_t ubNumColors;
  uint16_t usNumLights;
  int32_t cnt;
  char str[30];
  uint8_t ubStrLen;
  LIGHT_SPRITE TmpLight;
  int32_t iLSprite;
  uint32_t uiHour;
  BOOLEAN fPrimeTime = FALSE, fNightTime = FALSE;

  // reset the lighting system, so that any current lights are toasted.
  LightReset();

  // read in the light colors!
  LOADDATA(&ubNumColors, *hBuffer, 1);
  LOADDATA(LColors, *hBuffer, sizeof(struct SGPPaletteEntry) * ubNumColors);

  LOADDATA(&usNumLights, *hBuffer, 2);

  ubNumColors = 1;

  // ATE: OK, only regenrate if colors are different.....
  // if ( LColors[0].red != gpLightColors[0].red ||
  //		 LColors[0].green != gpLightColors[0].green ||
  //		 LColors[0].blue != gpLightColors[0].blue )
  { LightSetColors(LColors, ubNumColors); }

  // Determine which lights are valid for the current time.
  if (!gfEditMode) {
    uiHour = GetGameClockHour();
    if (uiHour >= NIGHT_TIME_LIGHT_START_HOUR || uiHour < NIGHT_TIME_LIGHT_END_HOUR) {
      fNightTime = TRUE;
    }
    if (uiHour >= PRIME_TIME_LIGHT_START_HOUR) {
      fPrimeTime = TRUE;
    }
  }

  for (cnt = 0; cnt < usNumLights; cnt++) {
    LOADDATA(&TmpLight, *hBuffer, sizeof(LIGHT_SPRITE));
    LOADDATA(&ubStrLen, *hBuffer, 1);

    if (ubStrLen) {
      LOADDATA(str, *hBuffer, ubStrLen);
    }

    str[ubStrLen] = 0;

    iLSprite = LightSpriteCreate(str, TmpLight.uiLightType);
    // if this fails, then we will ignore the light.
    // ATE: Don't add ANY lights of mapscreen util is on
    if (iLSprite != -1) {
      if (!gfCaves || gfEditMode) {
        if (gfEditMode || (TmpLight.uiFlags & LIGHT_PRIMETIME && fPrimeTime) ||
            (TmpLight.uiFlags & LIGHT_NIGHTTIME && fNightTime) ||
            !(TmpLight.uiFlags &
              (LIGHT_PRIMETIME | LIGHT_NIGHTTIME))) {  // power only valid lights.
          LightSpritePower(iLSprite, TRUE);
        }
      }
      LightSpritePosition(iLSprite, TmpLight.iX, TmpLight.iY);
      if (TmpLight.uiFlags & LIGHT_PRIMETIME)
        LightSprites[iLSprite].uiFlags |= LIGHT_PRIMETIME;
      else if (TmpLight.uiFlags & LIGHT_NIGHTTIME)
        LightSprites[iLSprite].uiFlags |= LIGHT_NIGHTTIME;
    }
  }
}

BOOLEAN IsRoofVisibleForWireframe(int16_t sMapPos) {
  struct STRUCTURE *pStructure;

  if (!gfBasement) {
    pStructure = FindStructure(sMapPos, STRUCTURE_ROOF);

    if (pStructure != NULL) {
      return (TRUE);
    }
  } else {
    // if ( InARoom( sMapPos, &ubRoom ) )
    {
      // if ( !( gpWorldLevelData[ sMapPos ].uiFlags & MAPELEMENT_REVEALED ) )
      { return (TRUE); }
    }
  }

  return (FALSE);
}
