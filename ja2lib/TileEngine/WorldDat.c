#include "TileEngine/WorldDat.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "SysGlobals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/TileSurface.h"
#include "rust_fileman.h"

// THIS FILE CONTAINS DEFINITIONS FOR TILESET FILES

void SetTilesetThreeTerrainValues();
void SetTilesetFourTerrainValues();

TILESET gTilesets[NUM_TILESETS];

void InitEngineTilesets() {
  uint8_t ubNumSets;
  uint32_t cnt, cnt2, uiNumFiles;
  //	FILE					*hfile;
  FileID hfile = FILE_ID_ERR;
  char zName[32];
  uint32_t uiNumBytesRead;

  // OPEN FILE
  //	hfile = fopen( "BINARYDATA\\JA2SET.DAT", "rb" );
  hfile = File_OpenForReading("BINARYDATA\\JA2SET.DAT");
  if (!hfile) {
    SET_ERROR("Cannot open tileset data file");
    return;
  }

  // READ # TILESETS and compare
  //	fread( &ubNumSets, sizeof( ubNumSets ), 1, hfile );
  File_Read(hfile, &ubNumSets, sizeof(ubNumSets), &uiNumBytesRead);
  // CHECK
  if (ubNumSets != NUM_TILESETS) {
    // Report error
    SET_ERROR("Number of tilesets in code does not match data file");
    return;
  }

  // READ #files
  //	fread( &uiNumFiles, sizeof( uiNumFiles ), 1, hfile );
  File_Read(hfile, &uiNumFiles, sizeof(uiNumFiles), &uiNumBytesRead);

  // COMPARE
  if (uiNumFiles != NUMBEROFTILETYPES) {
    // Report error
    SET_ERROR("Number of tilesets slots in code does not match data file");
    return;
  }

  // Loop through each tileset, load name then files
  for (cnt = 0; cnt < NUM_TILESETS; cnt++) {
    // Read name
    //		fread( &zName, sizeof( zName ), 1, hfile );
    File_Read(hfile, &zName, sizeof(zName), &uiNumBytesRead);

    // Read ambience value
    //		fread( &(gTilesets[ cnt ].ubAmbientID), sizeof( uint8_t), 1, hfile );
    File_Read(hfile, &(gTilesets[cnt].ubAmbientID), sizeof(uint8_t), &uiNumBytesRead);

    // Set into tileset
    swprintf(gTilesets[cnt].zName, ARR_SIZE(gTilesets[cnt].zName), L"%S", zName);

    // Loop for files
    for (cnt2 = 0; cnt2 < uiNumFiles; cnt2++) {
      // Read file name
      //			fread( &zName, sizeof( zName ), 1, hfile );
      File_Read(hfile, &zName, sizeof(zName), &uiNumBytesRead);

      // Set into database
      strcpy(gTilesets[cnt].TileSurfaceFilenames[cnt2], zName);
    }
  }

  //	fclose( hfile );
  File_Close(hfile);

  // SET CALLBACK FUNTIONS!!!!!!!!!!!!!
  gTilesets[CAVES_1].MovementCostFnc = (TILESET_CALLBACK)SetTilesetTwoTerrainValues;
  gTilesets[AIRSTRIP].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[DEAD_AIRSTRIP].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_14].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_18].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_19].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_26].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_27].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_28].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;
  gTilesets[TEMP_29].MovementCostFnc = (TILESET_CALLBACK)SetTilesetThreeTerrainValues;

  gTilesets[TROPICAL_1].MovementCostFnc = (TILESET_CALLBACK)SetTilesetFourTerrainValues;
  gTilesets[TEMP_20].MovementCostFnc = (TILESET_CALLBACK)SetTilesetFourTerrainValues;
}

void SetTilesetOneTerrainValues() {
  // FIRST TEXUTRES
  gTileSurfaceArray[FIRSTTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[SECONDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FOURTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FIFTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SIXTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SEVENTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[REGWATERTEXTURE]->ubTerrainID = LOW_WATER;
  gTileSurfaceArray[DEEPWATERTEXTURE]->ubTerrainID = DEEP_WATER;

  // NOW ROADS
  gTileSurfaceArray[FIRSTROAD]->ubTerrainID = DIRT_ROAD;
  gTileSurfaceArray[ROADPIECES]->ubTerrainID = DIRT_ROAD;

  // NOW FLOORS
  gTileSurfaceArray[FIRSTFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[SECONDFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[THIRDFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[FOURTHFLOOR]->ubTerrainID = FLAT_FLOOR;

  // NOW ANY TERRAIN MODIFYING DEBRIS
}

void SetTilesetTwoTerrainValues() {
  // FIRST TEXUTRES
  gTileSurfaceArray[FIRSTTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[SECONDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FOURTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FIFTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SIXTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SEVENTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[REGWATERTEXTURE]->ubTerrainID = LOW_WATER;
  gTileSurfaceArray[DEEPWATERTEXTURE]->ubTerrainID = DEEP_WATER;

  // NOW ROADS
  gTileSurfaceArray[FIRSTROAD]->ubTerrainID = DIRT_ROAD;
  gTileSurfaceArray[ROADPIECES]->ubTerrainID = DIRT_ROAD;

  // NOW FLOORS
  gTileSurfaceArray[FIRSTFLOOR]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[SECONDFLOOR]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDFLOOR]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FOURTHFLOOR]->ubTerrainID = FLAT_GROUND;
}

void SetTilesetThreeTerrainValues() {
  // DIFFERENCE FROM #1 IS THAT ROADS ARE PAVED

  // FIRST TEXUTRES
  gTileSurfaceArray[FIRSTTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[SECONDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FOURTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FIFTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SIXTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SEVENTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[REGWATERTEXTURE]->ubTerrainID = LOW_WATER;
  gTileSurfaceArray[DEEPWATERTEXTURE]->ubTerrainID = DEEP_WATER;

  // NOW ROADS
  gTileSurfaceArray[FIRSTROAD]->ubTerrainID = PAVED_ROAD;
  gTileSurfaceArray[ROADPIECES]->ubTerrainID = PAVED_ROAD;

  // NOW FLOORS
  gTileSurfaceArray[FIRSTFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[SECONDFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[THIRDFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[FOURTHFLOOR]->ubTerrainID = FLAT_FLOOR;

  // NOW ANY TERRAIN MODIFYING DEBRIS
}

void SetTilesetFourTerrainValues() {
  // DIFFERENCE FROM #1 IS THAT FLOOR2 IS NOT FLAT_FLOOR BUT FLAT_GROUND

  // FIRST TEXUTRES
  gTileSurfaceArray[FIRSTTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[SECONDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FOURTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[FIFTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SIXTHTEXTURE]->ubTerrainID = LOW_GRASS;
  gTileSurfaceArray[SEVENTHTEXTURE]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[REGWATERTEXTURE]->ubTerrainID = LOW_WATER;
  gTileSurfaceArray[DEEPWATERTEXTURE]->ubTerrainID = DEEP_WATER;

  // NOW ROADS
  gTileSurfaceArray[FIRSTROAD]->ubTerrainID = DIRT_ROAD;
  gTileSurfaceArray[ROADPIECES]->ubTerrainID = DIRT_ROAD;

  // NOW FLOORS
  gTileSurfaceArray[FIRSTFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[SECONDFLOOR]->ubTerrainID = FLAT_GROUND;
  gTileSurfaceArray[THIRDFLOOR]->ubTerrainID = FLAT_FLOOR;
  gTileSurfaceArray[FOURTHFLOOR]->ubTerrainID = FLAT_FLOOR;

  // NOW ANY TERRAIN MODIFYING DEBRIS
}
