#include "TileEngine/SaveLoadMap.h"

#include "GameSettings.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Types.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/FOV.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Overhead.h"
#include "Tactical/TacticalSave.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/Smell.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"

#define NUM_REVEALED_BYTES 3200

extern BOOLEAN gfLoadingExitGrids;

BOOLEAN gfApplyChangesToTempFile = FALSE;

//  There are 3200 bytes, and each bit represents the revelaed status.
//	3200 bytes * 8 bits = 25600 map elements
uint8_t *gpRevealedMap;

void RemoveSavedStructFromMap(uint32_t uiMapIndex, uint16_t usIndex);
void AddObjectFromMapTempFileToMap(uint32_t uiMapIndex, uint16_t usIndex);
void AddBloodOrSmellFromMapTempFileToMap(MODIFY_MAP *pMap);
void SetSectorsRevealedBit(uint16_t usMapIndex);
void SetMapRevealedStatus();
void DamageStructsFromMapTempFile(MODIFY_MAP *pMap);
BOOLEAN ModifyWindowStatus(uint32_t uiMapIndex);
// ppp

void ApplyMapChangesToMapTempFile(BOOLEAN fAddToMap) { gfApplyChangesToTempFile = fAddToMap; }

BOOLEAN SaveModifiedMapStructToMapTempFile(MODIFY_MAP *pMap, u8 sSectorX, u8 sSectorY,
                                           INT8 bSectorZ) {
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesWritten;

  // Convert the current sector location into a file name
  //	GetMapFileName( sSectorX, sSectorY, bSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\m_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ);

  // Open the file for writing, Create it if it doesnt exist
  hFile = FileMan_Open(zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Move to the end of the file
  FileMan_Seek(hFile, 0, FILE_SEEK_FROM_END);

  FileMan_Write(hFile, pMap, sizeof(MODIFY_MAP), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(MODIFY_MAP)) {
    // Error Writing size of array to disk
    FileMan_Close(hFile);
    return (FALSE);
  }

  FileMan_Close(hFile);

  SetSectorFlag(sSectorX, sSectorY, bSectorZ, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS);

  return (TRUE);
}

BOOLEAN LoadAllMapChangesFromMapTempFileAndApplyThem() {
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesRead;
  uint32_t uiFileSize;
  uint32_t uiNumberOfElements;
  uint32_t uiNumberOfElementsSavedBackToFile =
      0;  // added becuase if no files get saved back to disk, the flag needs to be erased
  uint32_t cnt;
  MODIFY_MAP *pMap;
  MODIFY_MAP *pTempArrayOfMaps = NULL;
  uint16_t usIndex;

  // Convert the current sector location into a file name
  //	GetMapFileName( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\m_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY,
                     gbWorldSectorZ);

  // Check to see if the file exists
  if (!FileMan_Exists(zMapName)) {
    // If the file doesnt exists, its no problem.
    return (TRUE);
  }

  // Open the file for reading
  hFile = FileMan_Open(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Get the size of the file
  uiFileSize = FileMan_GetSize(hFile);

  // Allocate memory for the buffer
  pTempArrayOfMaps = (MODIFY_MAP *)MemAlloc(uiFileSize);
  if (pTempArrayOfMaps == NULL) {
    Assert(0);
    return (TRUE);
  }

  // Read the map temp file into a buffer
  FileMan_Read(hFile, pTempArrayOfMaps, uiFileSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiFileSize) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Close the file
  FileMan_Close(hFile);

  // Delete the file
  FileMan_Delete(zMapName);

  uiNumberOfElements = uiFileSize / sizeof(MODIFY_MAP);

  for (cnt = 0; cnt < uiNumberOfElements; cnt++) {
    pMap = &pTempArrayOfMaps[cnt];

    // Switch on the type that should either be added or removed from the map
    switch (pMap->ubType) {
      // If we are adding to the map
      case SLM_LAND:
        break;
      case SLM_OBJECT:
        GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex, &usIndex);

        AddObjectFromMapTempFileToMap(pMap->usGridNo, usIndex);

        // Save this struct back to the temp file
        SaveModifiedMapStructToMapTempFile(pMap, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

        // Since the element is being saved back to the temp file, increment the #
        uiNumberOfElementsSavedBackToFile++;

        break;
      case SLM_STRUCT:
        GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex, &usIndex);

        AddStructFromMapTempFileToMap(pMap->usGridNo, usIndex);

        // Save this struct back to the temp file
        SaveModifiedMapStructToMapTempFile(pMap, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

        // Since the element is being saved back to the temp file, increment the #
        uiNumberOfElementsSavedBackToFile++;
        break;
      case SLM_SHADOW:
        break;
      case SLM_MERC:
        break;
      case SLM_ROOF:
        break;
      case SLM_ONROOF:
        break;
      case SLM_TOPMOST:
        break;

      // Remove objects out of the world
      case SLM_REMOVE_LAND:
        break;
      case SLM_REMOVE_OBJECT:
        break;
      case SLM_REMOVE_STRUCT:

        // ATE: OK, dor doors, the usIndex can be varied, opened, closed, etc
        // we MUSTR delete ANY door type on this gridno
        // Since we can only have one door per gridno, we're safe to do so.....
        if (pMap->usImageType >= FIRSTDOOR && pMap->usImageType <= FOURTHDOOR) {
          // Remove ANY door...
          RemoveAllStructsOfTypeRange(pMap->usGridNo, FIRSTDOOR, FOURTHDOOR);
        } else {
          GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex, &usIndex);
          RemoveSavedStructFromMap(pMap->usGridNo, usIndex);
        }

        // Save this struct back to the temp file
        SaveModifiedMapStructToMapTempFile(pMap, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

        // Since the element is being saved back to the temp file, increment the #
        uiNumberOfElementsSavedBackToFile++;
        break;
      case SLM_REMOVE_SHADOW:
        break;
      case SLM_REMOVE_MERC:
        break;
      case SLM_REMOVE_ROOF:
        break;
      case SLM_REMOVE_ONROOF:
        break;
      case SLM_REMOVE_TOPMOST:
        break;

      case SLM_BLOOD_SMELL:
        AddBloodOrSmellFromMapTempFileToMap(pMap);
        break;

      case SLM_DAMAGED_STRUCT:
        DamageStructsFromMapTempFile(pMap);
        break;

      case SLM_EXIT_GRIDS: {
        EXITGRID ExitGrid;
        gfLoadingExitGrids = TRUE;
        ExitGrid.usGridNo = pMap->usSubImageIndex;
        ExitGrid.ubGotoSectorX = (uint8_t)pMap->usImageType;
        ExitGrid.ubGotoSectorY = (uint8_t)(pMap->usImageType >> 8);
        ExitGrid.ubGotoSectorZ = pMap->ubExtra;

        AddExitGridToWorld(pMap->usGridNo, &ExitGrid);
        gfLoadingExitGrids = FALSE;

        // Save this struct back to the temp file
        SaveModifiedMapStructToMapTempFile(pMap, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

        // Since the element is being saved back to the temp file, increment the #
        uiNumberOfElementsSavedBackToFile++;
      } break;

      case SLM_OPENABLE_STRUCT:
        SetOpenableStructStatusFromMapTempFile(pMap->usGridNo, (BOOLEAN)pMap->usImageType);
        break;

      case SLM_WINDOW_HIT:
        if (ModifyWindowStatus(pMap->usGridNo)) {
          // Save this struct back to the temp file
          SaveModifiedMapStructToMapTempFile(pMap, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

          // Since the element is being saved back to the temp file, increment the #
          uiNumberOfElementsSavedBackToFile++;
        }
        break;

      default:
        AssertMsg(0, "ERROR!  Map Type not in switch when loading map changes from temp file");
        break;
    }
  }

  // if no elements are saved back to the file, remove the flag indicating that there is a temp file
  if (uiNumberOfElementsSavedBackToFile == 0) {
    ReSetSectorFlag(gWorldSectorX, gWorldSectorY, gbWorldSectorZ,
                    SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS);
  }

  FileMan_Close(hFile);

  // Free the memory used for the temp array
  MemFree(pTempArrayOfMaps);
  pTempArrayOfMaps = NULL;

  return (TRUE);
}

void AddStructToMapTempFile(uint32_t uiMapIndex, uint16_t usIndex) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (!gfApplyChangesToTempFile) return;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_STRUCT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

void AddStructFromMapTempFileToMap(uint32_t uiMapIndex, uint16_t usIndex) {
  AddStructToTailCommon(uiMapIndex, usIndex, TRUE);
}

void AddObjectToMapTempFile(uint32_t uiMapIndex, uint16_t usIndex) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (!gfApplyChangesToTempFile) return;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_OBJECT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

void AddObjectFromMapTempFileToMap(uint32_t uiMapIndex, uint16_t usIndex) {
  AddObjectToHead(uiMapIndex, usIndex);
}

void AddRemoveObjectToMapTempFile(uint32_t uiMapIndex, uint16_t usIndex) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (!gfApplyChangesToTempFile) return;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_REMOVE_OBJECT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

void RemoveStructFromMapTempFile(uint32_t uiMapIndex, uint16_t usIndex) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (!gfApplyChangesToTempFile) return;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex			= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_REMOVE_STRUCT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

void RemoveSavedStructFromMap(uint32_t uiMapIndex, uint16_t usIndex) {
  RemoveStruct(uiMapIndex, usIndex);
}

void SaveBloodSmellAndRevealedStatesFromMapToTempFile() {
  MODIFY_MAP Map;
  uint16_t cnt;
  struct STRUCTURE *pStructure;

  gpRevealedMap = (uint8_t *)MemAlloc(NUM_REVEALED_BYTES);
  if (gpRevealedMap == NULL) AssertMsg(0, "Failed allocating memory for the revealed map");
  memset(gpRevealedMap, 0, NUM_REVEALED_BYTES);

  // Loop though all the map elements
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // if there is either blood or a smell on the tile, save it
    if (gpWorldLevelData[cnt].ubBloodInfo || gpWorldLevelData[cnt].ubSmellInfo) {
      memset(&Map, 0, sizeof(MODIFY_MAP));

      // Save the BloodInfo in the bottom byte and the smell info in the upper byte
      Map.usGridNo = cnt;
      //			Map.usIndex			= gpWorldLevelData[cnt].ubBloodInfo
      //| ( gpWorldLevelData[cnt].ubSmellInfo << 8 );
      Map.usImageType = gpWorldLevelData[cnt].ubBloodInfo;
      Map.usSubImageIndex = gpWorldLevelData[cnt].ubSmellInfo;

      Map.ubType = SLM_BLOOD_SMELL;

      // Save the change to the map file
      SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
    }

    // if the element has been revealed
    if (gpWorldLevelData[cnt].uiFlags & MAPELEMENT_REVEALED) {
      SetSectorsRevealedBit(cnt);
    }

    // if there is a structure that is damaged
    if (gpWorldLevelData[cnt].uiFlags & MAPELEMENT_STRUCTURE_DAMAGED) {
      struct STRUCTURE *pCurrent;

      pCurrent = gpWorldLevelData[cnt].pStructureHead;

      pCurrent = FindStructure(cnt, STRUCTURE_BASE_TILE);

      // loop through all the structures and add all that are damaged
      while (pCurrent) {
        // if the structure has been damaged
        if (pCurrent->ubHitPoints < pCurrent->pDBStructureRef->pDBStructure->ubHitPoints) {
          uint8_t ubBitToSet = 0x80;
          uint8_t ubLevel = 0;

          if (pCurrent->sCubeOffset != 0) ubLevel |= ubBitToSet;

          memset(&Map, 0, sizeof(MODIFY_MAP));

          // Save the Damaged value
          Map.usGridNo = cnt;
          //					Map.usIndex			=
          // StructureFlagToType( pCurrent->fFlags ) | ( pCurrent->ubHitPoints << 8 );
          Map.usImageType = StructureFlagToType(pCurrent->fFlags);
          Map.usSubImageIndex = pCurrent->ubHitPoints;

          Map.ubType = SLM_DAMAGED_STRUCT;
          Map.ubExtra = pCurrent->ubWallOrientation | ubLevel;

          // Save the change to the map file
          SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
        }

        pCurrent = FindNextStructure(pCurrent, STRUCTURE_BASE_TILE);
      }
    }

    pStructure = FindStructure(cnt, STRUCTURE_OPENABLE);

    // if this structure
    if (pStructure) {
      // if the current structure has an openable structure in it, and it is NOT a door
      if (!(pStructure->fFlags & STRUCTURE_ANYDOOR)) {
        BOOLEAN fStatusOnTheMap;

        fStatusOnTheMap = ((pStructure->fFlags & STRUCTURE_OPEN) != 0);

        AddOpenableStructStatusToMapTempFile(cnt, fStatusOnTheMap);
      }
    }
  }
}

// The BloodInfo is saved in the bottom byte and the smell info in the upper byte
void AddBloodOrSmellFromMapTempFileToMap(MODIFY_MAP *pMap) {
  gpWorldLevelData[pMap->usGridNo].ubBloodInfo = (uint8_t)pMap->usImageType;

  // if the blood and gore option IS set, add blood
  if (gGameSettings.fOptions[TOPTION_BLOOD_N_GORE]) {
    // Update graphics for both levels...
    gpWorldLevelData[pMap->usGridNo].uiFlags |= MAPELEMENT_REEVALUATEBLOOD;
    UpdateBloodGraphics(pMap->usGridNo, 0);
    gpWorldLevelData[pMap->usGridNo].uiFlags |= MAPELEMENT_REEVALUATEBLOOD;
    UpdateBloodGraphics(pMap->usGridNo, 1);
  }

  gpWorldLevelData[pMap->usGridNo].ubSmellInfo = (uint8_t)pMap->usSubImageIndex;
}

BOOLEAN SaveRevealedStatusArrayToRevealedTempFile(u8 sSectorX, u8 sSectorY, INT8 bSectorZ) {
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesWritten;

  Assert(gpRevealedMap != NULL);

  // Convert the current sector location into a file name
  //	GetMapFileName( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTempName, FALSE );

  // add the 'v' for 'reVeiled Map' to the front of the map name
  //	sprintf( zMapName, "%s\\v_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_REVEALED_STATUS_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ);

  // Open the file for writing, Create it if it doesnt exist
  hFile = FileMan_Open(zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Write the revealed array to the Revealed temp file
  FileMan_Write(hFile, gpRevealedMap, NUM_REVEALED_BYTES, &uiNumBytesWritten);
  if (uiNumBytesWritten != NUM_REVEALED_BYTES) {
    // Error Writing size of array to disk
    FileMan_Close(hFile);
    return (FALSE);
  }

  FileMan_Close(hFile);

  SetSectorFlag(sSectorX, sSectorY, bSectorZ, SF_REVEALED_STATUS_TEMP_FILE_EXISTS);

  MemFree(gpRevealedMap);
  gpRevealedMap = NULL;

  return (TRUE);
}

BOOLEAN LoadRevealedStatusArrayFromRevealedTempFile() {
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesRead;

  // Convert the current sector location into a file name
  //	GetMapFileName( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, zTempName, FALSE );

  // add the 'v' for 'reVeiled Map' to the front of the map name
  //	sprintf( zMapName, "%s\\v_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_REVEALED_STATUS_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY,
                     gbWorldSectorZ);

  // Check to see if the file exists
  if (!FileMan_Exists(zMapName)) {
    // If the file doesnt exists, its no problem.
    return (TRUE);
  }

  // Open the file for reading
  hFile = FileMan_Open(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Allocate memory
  Assert(gpRevealedMap == NULL);
  gpRevealedMap = (uint8_t *)MemAlloc(NUM_REVEALED_BYTES);
  if (gpRevealedMap == NULL) AssertMsg(0, "Failed allocating memory for the revealed map");
  memset(gpRevealedMap, 0, NUM_REVEALED_BYTES);

  // Load the Reveal map array structure
  FileMan_Read(hFile, gpRevealedMap, NUM_REVEALED_BYTES, &uiNumBytesRead);
  if (uiNumBytesRead != NUM_REVEALED_BYTES) {
    return (FALSE);
  }

  FileMan_Close(hFile);

  // Loop through and set the bits in the map that are revealed
  SetMapRevealedStatus();

  MemFree(gpRevealedMap);
  gpRevealedMap = NULL;

  return (TRUE);
}

void SetSectorsRevealedBit(uint16_t usMapIndex) {
  uint16_t usByteNumber;
  uint8_t ubBitNumber;

  usByteNumber = usMapIndex / 8;
  ubBitNumber = usMapIndex % 8;

  gpRevealedMap[usByteNumber] |= 1 << ubBitNumber;
}

void SetMapRevealedStatus() {
  uint16_t usByteCnt;
  uint8_t ubBitCnt;
  uint16_t usMapIndex;

  if (gpRevealedMap == NULL) AssertMsg(0, "gpRevealedMap is NULL.  DF 1");

  ClearSlantRoofs();

  // Loop through all bytes in the array
  for (usByteCnt = 0; usByteCnt < 3200; usByteCnt++) {
    // loop through all the bits in the byte
    for (ubBitCnt = 0; ubBitCnt < 8; ubBitCnt++) {
      usMapIndex = (usByteCnt * 8) + ubBitCnt;

      if (gpRevealedMap[usByteCnt] & (1 << ubBitCnt)) {
        gpWorldLevelData[usMapIndex].uiFlags |= MAPELEMENT_REVEALED;
        SetGridNoRevealedFlag(usMapIndex);
      } else {
        gpWorldLevelData[usMapIndex].uiFlags &= (~MAPELEMENT_REVEALED);
      }
    }
  }

  ExamineSlantRoofFOVSlots();
}

void DamageStructsFromMapTempFile(MODIFY_MAP *pMap) {
  struct STRUCTURE *pCurrent = NULL;
  INT8 bLevel;
  uint8_t ubWallOrientation;
  uint8_t ubBitToSet = 0x80;
  uint8_t ubType = 0;

  // Find the base structure
  pCurrent = FindStructure((INT16)pMap->usGridNo, STRUCTURE_BASE_TILE);

  if (pCurrent == NULL) return;

  bLevel = pMap->ubExtra & ubBitToSet;
  ubWallOrientation = pMap->ubExtra & ~ubBitToSet;
  ubType = (uint8_t)pMap->usImageType;

  // Check to see if the desired strucure node is in this tile
  pCurrent = FindStructureBySavedInfo(pMap->usGridNo, ubType, ubWallOrientation, bLevel);

  if (pCurrent != NULL) {
    // Assign the hitpoints
    pCurrent->ubHitPoints = (uint8_t)(pMap->usSubImageIndex);

    gpWorldLevelData[pCurrent->sGridNo].uiFlags |= MAPELEMENT_STRUCTURE_DAMAGED;
  }
}

//////////////

void AddStructToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                    INT16 sSectorY, uint8_t ubSectorZ) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_STRUCT;

  SaveModifiedMapStructToMapTempFile(&Map, sSectorX, sSectorY, ubSectorZ);
}

void AddObjectToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                    INT16 sSectorY, uint8_t ubSectorZ) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_OBJECT;

  SaveModifiedMapStructToMapTempFile(&Map, sSectorX, sSectorY, ubSectorZ);
}

void RemoveStructFromUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                         INT16 sSectorY, uint8_t ubSectorZ) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex			= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_REMOVE_STRUCT;

  SaveModifiedMapStructToMapTempFile(&Map, sSectorX, sSectorY, ubSectorZ);
}

void AddRemoveObjectToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                          INT16 sSectorY, uint8_t ubSectorZ) {
  MODIFY_MAP Map;
  uint32_t uiType;
  uint16_t usSubIndex;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  //	Map.usIndex		= usIndex;
  Map.usImageType = (uint16_t)uiType;
  Map.usSubImageIndex = usSubIndex;

  Map.ubType = SLM_REMOVE_OBJECT;

  SaveModifiedMapStructToMapTempFile(&Map, sSectorX, sSectorY, ubSectorZ);
}

void AddExitGridToMapTempFile(uint16_t usGridNo, EXITGRID *pExitGrid, u8 sSectorX, u8 sSectorY,
                              uint8_t ubSectorZ) {
  MODIFY_MAP Map;

  if (!gfApplyChangesToTempFile) {
    ScreenMsg(FONT_MCOLOR_WHITE, MSG_BETAVERSION,
              L"Called AddExitGridToMapTempFile() without calling ApplyMapChangesToMapTempFile()");
    return;
  }

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = usGridNo;
  //	Map.usIndex		= pExitGrid->ubGotoSectorX;

  Map.usImageType = pExitGrid->ubGotoSectorX | (pExitGrid->ubGotoSectorY << 8);
  Map.usSubImageIndex = pExitGrid->usGridNo;

  Map.ubExtra = pExitGrid->ubGotoSectorZ;
  Map.ubType = SLM_EXIT_GRIDS;

  SaveModifiedMapStructToMapTempFile(&Map, sSectorX, sSectorY, ubSectorZ);
}

BOOLEAN RemoveGraphicFromTempFile(uint32_t uiMapIndex, uint16_t usIndex, u8 sSectorX, u8 sSectorY,
                                  uint8_t ubSectorZ) {
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesRead;
  MODIFY_MAP *pTempArrayOfMaps = NULL;
  MODIFY_MAP *pMap;
  uint32_t uiFileSize;
  uint32_t uiNumberOfElements;
  BOOLEAN fRetVal = FALSE;
  uint32_t uiType;
  uint16_t usSubIndex;
  uint32_t cnt;

  // Convert the current sector location into a file name
  //	GetMapFileName( sSectorX, sSectorY, ubSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\m_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY,
                     ubSectorZ);

  // Check to see if the file exists
  if (!FileMan_Exists(zMapName)) {
    // If the file doesnt exists,
    return (FALSE);
  }

  // Open the file for writing, Create it if it doesnt exist
  hFile = FileMan_Open(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
  if (hFile == 0) {
    // Error opening map modification file
    return (FALSE);
  }

  // Get the size of the temp file
  uiFileSize = FileMan_GetSize(hFile);

  // Allocate memory for the buffer
  pTempArrayOfMaps = (MODIFY_MAP *)MemAlloc(uiFileSize);
  if (pTempArrayOfMaps == NULL) {
    Assert(0);
    return (FALSE);
  }

  // Read the map temp file into a buffer
  FileMan_Read(hFile, pTempArrayOfMaps, uiFileSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiFileSize) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Close the file
  FileMan_Close(hFile);

  // Delete the file
  FileMan_Delete(zMapName);

  // Get the number of elements in the file
  uiNumberOfElements = uiFileSize / sizeof(MODIFY_MAP);

  // Get the image type and subindex
  GetTileType(usIndex, &uiType);
  GetSubIndexFromTileIndex(usIndex, &usSubIndex);

  for (cnt = 0; cnt < uiNumberOfElements; cnt++) {
    pMap = &pTempArrayOfMaps[cnt];

    // if this is the peice we are looking for
    if (pMap->usGridNo == uiMapIndex && pMap->usImageType == uiType &&
        pMap->usSubImageIndex == usSubIndex) {
      // Do nothin
      fRetVal = TRUE;
    } else {
      // save the struct back to the temp file
      SaveModifiedMapStructToMapTempFile(pMap, sSectorX, sSectorY, ubSectorZ);
    }
  }

  return (fRetVal);
}

void AddOpenableStructStatusToMapTempFile(uint32_t uiMapIndex, BOOLEAN fOpened) {
  MODIFY_MAP Map;

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  Map.usImageType = fOpened;

  Map.ubType = SLM_OPENABLE_STRUCT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

void AddWindowHitToMapTempFile(uint32_t uiMapIndex) {
  MODIFY_MAP Map;

  memset(&Map, 0, sizeof(MODIFY_MAP));

  Map.usGridNo = (uint16_t)uiMapIndex;
  Map.ubType = SLM_WINDOW_HIT;

  SaveModifiedMapStructToMapTempFile(&Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}

BOOLEAN ModifyWindowStatus(uint32_t uiMapIndex) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure((INT16)uiMapIndex, STRUCTURE_WALLNWINDOW);
  if (pStructure) {
    SwapStructureForPartner((INT16)uiMapIndex, pStructure);
    return (TRUE);
  }
  // else forget it, window could be destroyed
  return (FALSE);
}

void SetOpenableStructStatusFromMapTempFile(uint32_t uiMapIndex, BOOLEAN fOpened) {
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBase;
  BOOLEAN fStatusOnTheMap;
  struct ITEM_POOL *pItemPool;
  INT16 sBaseGridNo = (INT16)uiMapIndex;

  pStructure = FindStructure((uint16_t)uiMapIndex, STRUCTURE_OPENABLE);

  if (pStructure == NULL) {
    //		ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION,
    // L"SetOpenableStructStatusFromMapTempFile( %d, %d ) failed to find the openable struct.
    // DF 1.", uiMapIndex, fOpened );
    return;
  }

  fStatusOnTheMap = ((pStructure->fFlags & STRUCTURE_OPEN) != 0);

  if (fStatusOnTheMap != fOpened) {
    // Adjust the item's gridno to the base of struct.....
    pBase = FindBaseStructure(pStructure);

    // Get struct LEVELNODE for struct and remove!
    if (pBase) {
      sBaseGridNo = pBase->sGridNo;
    }

    if (SwapStructureForPartnerWithoutTriggeringSwitches((uint16_t)uiMapIndex, pStructure) ==
        NULL) {
      // an error occured
    }

    // Adjust visiblity of any item pools here....
    // ATE: Nasty bug here - use base gridno for structure for items!
    // since items always drop to base gridno in AddItemToPool
    if (GetItemPool(sBaseGridNo, &pItemPool, 0)) {
      if (fOpened) {
        // We are open, make un-hidden if so....
        SetItemPoolVisibilityOn(pItemPool, ANY_VISIBILITY_VALUE, FALSE);
      } else {
        // Make sure items are hidden...
        SetItemPoolVisibilityHidden(pItemPool);
      }
    }
  }
}

BOOLEAN ChangeStatusOfOpenableStructInUnloadedSector(uint16_t usSectorX, uint16_t usSectorY,
                                                     INT8 bSectorZ, uint16_t usGridNo,
                                                     BOOLEAN fChangeToOpen) {
  //	struct STRUCTURE * pStructure;
  //	MODIFY_MAP Map;
  CHAR8 zMapName[128];
  HWFILE hFile;
  uint32_t uiNumBytesRead;
  uint32_t uiNumBytesWritten;
  uint32_t uiFileSize;
  uint32_t uiNumberOfElements;
  uint32_t cnt;
  MODIFY_MAP *pMap;
  MODIFY_MAP *pTempArrayOfMaps = NULL;
  //	uint16_t	usIndex;

  // Convert the current sector location into a file name
  //	GetMapFileName( usSectorX, usSectorY, bSectorZ, zTempName, FALSE );

  // add the 'm' for 'Modifed Map' to the front of the map name
  //	sprintf( zMapName, "%s\\m_%s", MAPS_DIR, zTempName);

  GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, zMapName, usSectorX, usSectorY,
                     bSectorZ);

  // Check to see if the file exists
  if (!FileMan_Exists(zMapName)) {
    // If the file doesnt exists, its no problem.
    return (TRUE);
  }

  // Open the file for reading
  hFile = FileMan_Open(zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Get the size of the file
  uiFileSize = FileMan_GetSize(hFile);

  // Allocate memory for the buffer
  pTempArrayOfMaps = (MODIFY_MAP *)MemAlloc(uiFileSize);
  if (pTempArrayOfMaps == NULL) {
    Assert(0);
    return (TRUE);
  }

  // Read the map temp file into a buffer
  FileMan_Read(hFile, pTempArrayOfMaps, uiFileSize, &uiNumBytesRead);
  if (uiNumBytesRead != uiFileSize) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Close the file
  FileMan_Close(hFile);

  // Delete the file
  FileMan_Delete(zMapName);

  uiNumberOfElements = uiFileSize / sizeof(MODIFY_MAP);

  // loop through all the array elements to
  for (cnt = 0; cnt < uiNumberOfElements; cnt++) {
    pMap = &pTempArrayOfMaps[cnt];

    // if this element is of the same type
    if (pMap->ubType == SLM_OPENABLE_STRUCT) {
      // if its on the same gridno
      if (pMap->usGridNo == usGridNo) {
        // Change to the desired settings
        pMap->usImageType = fChangeToOpen;

        break;
      }
    }
  }

  // Open the file for writing
  hFile = FileMan_Open(zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);
  if (hFile == 0) {
    // Error opening map modification file,
    return (FALSE);
  }

  // Write the map temp file into a buffer
  FileMan_Write(hFile, pTempArrayOfMaps, uiFileSize, &uiNumBytesWritten);
  if (uiNumBytesWritten != uiFileSize) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  FileMan_Close(hFile);

  return (TRUE);
}
