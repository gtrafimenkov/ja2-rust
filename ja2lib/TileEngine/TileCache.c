#include "TileEngine/TileCache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "Tactical/AnimationCache.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/TileSurface.h"
#include "TileEngine/WorldDef.h"
#include "Utils/DebugControl.h"
#include "platform.h"
#include "platform_strings.h"

uint32_t guiNumTileCacheStructs = 0;
uint32_t guiMaxTileCacheSize = 50;
uint32_t guiCurTileCacheSize = 0;
int32_t giDefaultStructIndex = -1;

TILE_CACHE_ELEMENT *gpTileCache = NULL;
TILE_CACHE_STRUCT *gpTileCacheStructInfo = NULL;

BOOLEAN InitTileCache() {
  uint32_t cnt;
  struct GetFile FileInfo;
  int16_t sFiles = 0;

  gpTileCache = (TILE_CACHE_ELEMENT *)MemAlloc(sizeof(TILE_CACHE_ELEMENT) * guiMaxTileCacheSize);

  // Zero entries
  for (cnt = 0; cnt < guiMaxTileCacheSize; cnt++) {
    gpTileCache[cnt].pImagery = NULL;
    gpTileCache[cnt].sStructRefID = -1;
  }

  guiCurTileCacheSize = 0;

  // OK, look for JSD files in the tile cache directory and
  // load any we find....
  if (Plat_GetFileFirst("TILECACHE\\*.jsd", &FileInfo)) {
    while (Plat_GetFileNext(&FileInfo)) {
      sFiles++;
    }
    Plat_GetFileClose(&FileInfo);
  }

  // Allocate memory...
  if (sFiles > 0) {
    cnt = 0;

    guiNumTileCacheStructs = sFiles;

    gpTileCacheStructInfo = (TILE_CACHE_STRUCT *)MemAlloc(sizeof(TILE_CACHE_STRUCT) * sFiles);

    // Loop through and set filenames
    if (Plat_GetFileFirst("TILECACHE\\*.jsd", &FileInfo)) {
      while (Plat_GetFileNext(&FileInfo)) {
        sprintf(gpTileCacheStructInfo[cnt].Filename, "TILECACHE\\%s", FileInfo.zFileName);

        // Get root name
        GetRootName(gpTileCacheStructInfo[cnt].zRootName, gpTileCacheStructInfo[cnt].Filename);

        // Load struc data....
        gpTileCacheStructInfo[cnt].pStructureFileRef =
            LoadStructureFile(gpTileCacheStructInfo[cnt].Filename);

#ifdef JA2TESTVERSION
        if (gpTileCacheStructInfo[cnt].pStructureFileRef == NULL) {
          SET_ERROR("Cannot load tilecache JSD: %s", gpTileCacheStructInfo[cnt].Filename);
        }
#endif
        if (strcasecmp(gpTileCacheStructInfo[cnt].zRootName, "l_dead1") == 0) {
          giDefaultStructIndex = cnt;
        }

        cnt++;
      }
      Plat_GetFileClose(&FileInfo);
    }
  }

  return (TRUE);
}

void DeleteTileCache() {
  uint32_t cnt;

  // Allocate entries
  if (gpTileCache != NULL) {
    // Loop through and delete any entries
    for (cnt = 0; cnt < guiMaxTileCacheSize; cnt++) {
      if (gpTileCache[cnt].pImagery != NULL) {
        DeleteTileSurface(gpTileCache[cnt].pImagery);
      }
    }
    MemFree(gpTileCache);
  }

  if (gpTileCacheStructInfo != NULL) {
    MemFree(gpTileCacheStructInfo);
  }

  guiCurTileCacheSize = 0;
}

int16_t FindCacheStructDataIndex(STR8 cFilename) {
  uint32_t cnt;

  for (cnt = 0; cnt < guiNumTileCacheStructs; cnt++) {
    if (strcasecmp(gpTileCacheStructInfo[cnt].zRootName, cFilename) == 0) {
      return ((int16_t)cnt);
    }
  }

  return (-1);
}

int32_t GetCachedTile(STR8 cFilename) {
  uint32_t cnt;
  uint32_t ubLowestIndex = 0;
  int16_t sMostHits = (int16_t)15000;

  // Check to see if surface exists already
  for (cnt = 0; cnt < guiCurTileCacheSize; cnt++) {
    if (gpTileCache[cnt].pImagery != NULL) {
      if (strcasecmp(gpTileCache[cnt].zName, cFilename) == 0) {
        // Found surface, return
        gpTileCache[cnt].sHits++;
        return ((int32_t)cnt);
      }
    }
  }

  // Check if max size has been reached
  if (guiCurTileCacheSize == guiMaxTileCacheSize) {
    // cache out least used file
    for (cnt = 0; cnt < guiCurTileCacheSize; cnt++) {
      if (gpTileCache[cnt].sHits < sMostHits) {
        sMostHits = gpTileCache[cnt].sHits;
        ubLowestIndex = cnt;
      }
    }

    // Bump off lowest index
    DeleteTileSurface(gpTileCache[ubLowestIndex].pImagery);

    // Decrement
    gpTileCache[ubLowestIndex].sHits = 0;
    gpTileCache[ubLowestIndex].pImagery = NULL;
    gpTileCache[ubLowestIndex].sStructRefID = -1;
  }

  // If here, Insert at an empty slot
  // Find an empty slot
  for (cnt = 0; cnt < guiMaxTileCacheSize; cnt++) {
    if (gpTileCache[cnt].pImagery == NULL) {
      // Insert here
      gpTileCache[cnt].pImagery = LoadTileSurface(cFilename);

      if (gpTileCache[cnt].pImagery == NULL) {
        return (-1);
      }

      strcpy(gpTileCache[cnt].zName, cFilename);
      gpTileCache[cnt].sHits = 1;

      // Get root name
      GetRootName(gpTileCache[cnt].zRootName, cFilename);

      gpTileCache[cnt].sStructRefID = FindCacheStructDataIndex(gpTileCache[cnt].zRootName);

      // ATE: Add z-strip info
      if (gpTileCache[cnt].sStructRefID != -1) {
        AddZStripInfoToVObject(
            gpTileCache[cnt].pImagery->vo,
            gpTileCacheStructInfo[gpTileCache[cnt].sStructRefID].pStructureFileRef, TRUE, 0);
      }

      if (gpTileCache[cnt].pImagery->pAuxData != NULL) {
        gpTileCache[cnt].ubNumFrames = gpTileCache[cnt].pImagery->pAuxData->ubNumberOfFrames;
      } else {
        gpTileCache[cnt].ubNumFrames = 1;
      }

      // Has our cache size increased?
      if (cnt >= guiCurTileCacheSize) {
        guiCurTileCacheSize = cnt + 1;
        ;
      }

      return (cnt);
    }
  }

  // Can't find one!
  return (-1);
}

BOOLEAN RemoveCachedTile(int32_t iCachedTile) {
  uint32_t cnt;

  // Find tile
  for (cnt = 0; cnt < guiCurTileCacheSize; cnt++) {
    if (gpTileCache[cnt].pImagery != NULL) {
      if (cnt == (uint32_t)iCachedTile) {
        // Found surface, decrement hits
        gpTileCache[cnt].sHits--;

        // Are we at zero?
        if (gpTileCache[cnt].sHits == 0) {
          DeleteTileSurface(gpTileCache[cnt].pImagery);
          gpTileCache[cnt].pImagery = NULL;
          gpTileCache[cnt].sStructRefID = -1;
          return (TRUE);
          ;
        }
      }
    }
  }

  return (FALSE);
}

struct VObject *GetCachedTileVideoObject(int32_t iIndex) {
  if (iIndex == -1) {
    return (NULL);
  }

  if (gpTileCache[iIndex].pImagery == NULL) {
    return (NULL);
  }

  return (gpTileCache[iIndex].pImagery->vo);
}

struct STRUCTURE_FILE_REF *GetCachedTileStructureRef(int32_t iIndex) {
  if (iIndex == -1) {
    return (NULL);
  }

  if (gpTileCache[iIndex].sStructRefID == -1) {
    return (NULL);
  }

  return (gpTileCacheStructInfo[gpTileCache[iIndex].sStructRefID].pStructureFileRef);
}

struct STRUCTURE_FILE_REF *GetCachedTileStructureRefFromFilename(STR8 cFilename) {
  int16_t sStructDataIndex;

  // Given filename, look for index
  sStructDataIndex = FindCacheStructDataIndex(cFilename);

  if (sStructDataIndex == -1) {
    return (NULL);
  }

  return (gpTileCacheStructInfo[sStructDataIndex].pStructureFileRef);
}

void CheckForAndAddTileCacheStructInfo(struct LEVELNODE *pNode, int16_t sGridNo, uint16_t usIndex,
                                       uint16_t usSubIndex) {
  struct STRUCTURE_FILE_REF *pStructureFileRef;

  pStructureFileRef = GetCachedTileStructureRef(usIndex);

  if (pStructureFileRef != NULL) {
    if (!AddStructureToWorld(sGridNo, 0, &(pStructureFileRef->pDBStructureRef[usSubIndex]),
                             pNode)) {
      if (giDefaultStructIndex != -1) {
        pStructureFileRef = gpTileCacheStructInfo[giDefaultStructIndex].pStructureFileRef;

        if (pStructureFileRef != NULL) {
          AddStructureToWorld(sGridNo, 0, &(pStructureFileRef->pDBStructureRef[usSubIndex]), pNode);
        }
      }
    }
  }
}

void CheckForAndDeleteTileCacheStructInfo(struct LEVELNODE *pNode, uint16_t usIndex) {
  struct STRUCTURE_FILE_REF *pStructureFileRef;

  if (usIndex >= TILE_CACHE_START_INDEX) {
    pStructureFileRef = GetCachedTileStructureRef((usIndex - TILE_CACHE_START_INDEX));

    if (pStructureFileRef != NULL) {
      DeleteStructureFromWorld(pNode->pStructureData);
    }
  }
}

void GetRootName(STR8 pDestStr, STR8 pSrcStr) {
  // Remove path and extension
  CHAR8 cTempFilename[120];
  STR cEndOfName;

  // Remove path
  strcpy(cTempFilename, pSrcStr);
  cEndOfName = strrchr(cTempFilename, '\\');
  if (cEndOfName != NULL) {
    cEndOfName++;
    strcpy(pDestStr, cEndOfName);
  } else {
    strcpy(pDestStr, cTempFilename);
  }

  // Now remove extension...
  cEndOfName = strchr(pDestStr, '.');
  if (cEndOfName != NULL) {
    *cEndOfName = '\0';
  }
}
