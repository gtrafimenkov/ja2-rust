#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Editor/Smooth.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldDef.h"
#include "TileEngine/WorldMan.h"
#include "platform_strings.h"
#include "rust_fileman.h"

struct TILE_IMAGERY *gTileSurfaceArray[NUMBEROFTILETYPES];
UINT8 gbDefaultSurfaceUsed[NUMBEROFTILETYPES];
UINT8 gbSameAsDefaultSurfaceUsed[NUMBEROFTILETYPES];

struct TILE_IMAGERY *LoadTileSurface(char *cFilename) {
  // Add tile surface
  struct TILE_IMAGERY *pTileSurf = NULL;
  struct VObject *hVObject;
  struct Image *hImage;
  SGPFILENAME cStructureFilename;
  STR cEndOfName;
  struct STRUCTURE_FILE_REF *pStructureFileRef;
  BOOLEAN fOk;

  hImage = CreateImage(cFilename, IMAGE_ALLDATA);
  if (hImage == NULL) {
    // Report error
    SET_ERROR("Could not load tile file: %s", cFilename);
    return (NULL);
  }

  hVObject = CreateVObjectFromHImage(hImage);

  if (hVObject == NULL) {
    // Report error
    SET_ERROR("Could not load tile file: %s", cFilename);
    // Video Object will set error conition.]
    DestroyImage(hImage);
    return (NULL);
  }

  // Load structure data, if any.
  // Start by hacking the image filename into that for the structure data
  strcpy(cStructureFilename, cFilename);
  cEndOfName = strchr(cStructureFilename, '.');
  if (cEndOfName != NULL) {
    cEndOfName++;
    *cEndOfName = '\0';
  } else {
    strcat(cStructureFilename, ".");
  }
  strcat(cStructureFilename, STRUCTURE_FILE_EXTENSION);
  if (File_Exists(cStructureFilename)) {
    pStructureFileRef = LoadStructureFile(cStructureFilename);
    if (pStructureFileRef == NULL ||
        hVObject->usNumberOfObjects != pStructureFileRef->usNumberOfStructures) {
      DestroyImage(hImage);
      DeleteVideoObject(hVObject);
      SET_ERROR("Structure file error: %s", cStructureFilename);
      return (NULL);
    }

    DebugMsg(TOPIC_JA2, DBG_INFO, cStructureFilename);

    fOk = AddZStripInfoToVObject(hVObject, pStructureFileRef, FALSE, 0);
    if (fOk == FALSE) {
      DestroyImage(hImage);
      DeleteVideoObject(hVObject);
      SET_ERROR("ZStrip creation error: %s", cStructureFilename);
      return (NULL);
    }

  } else {
    pStructureFileRef = NULL;
  }

  pTileSurf = (struct TILE_IMAGERY *)MemAlloc(sizeof(struct TILE_IMAGERY));

  // Set all values to zero
  memset(pTileSurf, 0, sizeof(struct TILE_IMAGERY));

  pTileSurf->vo = hVObject;
  pTileSurf->pStructureFileRef = pStructureFileRef;

  if (pStructureFileRef && pStructureFileRef->pAuxData != NULL) {
    pTileSurf->pAuxData = pStructureFileRef->pAuxData;
    pTileSurf->pTileLocData = pStructureFileRef->pTileLocData;
  } else if (hImage->uiAppDataSize == hVObject->usNumberOfObjects * sizeof(struct AuxObjectData)) {
    // Valid auxiliary data, so make a copy of it for TileSurf
    pTileSurf->pAuxData = (struct AuxObjectData *)MemAlloc(hImage->uiAppDataSize);
    if (pTileSurf->pAuxData == NULL) {
      DestroyImage(hImage);
      DeleteVideoObject(hVObject);
      return (NULL);
    }
    memcpy(pTileSurf->pAuxData, hImage->pAppData, hImage->uiAppDataSize);
  } else {
    pTileSurf->pAuxData = NULL;
  }
  // the hImage is no longer needed
  DestroyImage(hImage);

  return (pTileSurf);
}

void DeleteTileSurface(struct TILE_IMAGERY *pTileSurf) {
  if (pTileSurf->pStructureFileRef != NULL) {
    FreeStructureFile(pTileSurf->pStructureFileRef);
  } else {
    // If a structure file exists, it will free the auxdata.
    // Since there is no structure file in this instance, we
    // free it ourselves.
    if (pTileSurf->pAuxData != NULL) {
      MemFree(pTileSurf->pAuxData);
    }
  }

  DeleteVideoObject(pTileSurf->vo);
  MemFree(pTileSurf);
}

void SetRaisedObjectFlag(char *cFilename, struct TILE_IMAGERY *pTileSurf) {
  INT32 cnt = 0;
  CHAR8 cRootFile[128];
  CHAR8 ubRaisedObjectFiles[][80] = {"bones",    "bones2", "grass2", "grass3", "l_weed3", "litter",
                                     "miniweed", "sblast", "sweeds", "twigs",  "wing",    "1"};

  // Loop through array of RAISED objecttype imagery and
  // set global value...
  if ((pTileSurf->fType >= DEBRISWOOD && pTileSurf->fType <= DEBRISWEEDS) ||
      pTileSurf->fType == DEBRIS2MISC || pTileSurf->fType == ANOTHERDEBRIS) {
    GetRootName(cRootFile, cFilename);
    while (ubRaisedObjectFiles[cnt][0] != '1') {
      if (strcasecmp(ubRaisedObjectFiles[cnt], cRootFile) == 0) {
        pTileSurf->bRaisedObjectType = TRUE;
      }

      cnt++;
    }
  }
}
