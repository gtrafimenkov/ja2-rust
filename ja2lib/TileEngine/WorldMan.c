// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/WorldMan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Editor/Smooth.h"
#include "Editor/SmoothingUtils.h"
#include "GameSettings.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "TacticalAI/AI.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/TileDef.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"

extern BOOLEAN gfBasement;

uint32_t guiLNCount[9];
static wchar_t gzLevelString[9][15] = {
    L"",           L"Land    %d", L"Object  %d", L"Struct  %d", L"Shadow  %d",
    L"Merc    %d", L"Roof    %d", L"Onroof  %d", L"Topmost %d",
};

// World management routines ( specific Double Linked list functions
void SetIndexLevelNodeFlags(struct LEVELNODE *pStartNode, uint32_t uiFlags, uint16_t usIndex);
void RemoveIndexLevelNodeFlags(struct LEVELNODE *pStartNode, uint32_t uiFlags, uint16_t usIndex);

void SetWorldFlagsFromNewNode(uint16_t sGridNo, uint16_t usIndex);
void RemoveWorldFlagsFromNewNode(uint16_t sGridNo, uint16_t usIndex);

BOOLEAN RemoveLandEx(uint32_t iMapIndex, uint16_t usIndex);

uint32_t guiLevelNodes = 0;

// LEVEL NODE MANIPLULATION FUNCTIONS
BOOLEAN CreateLevelNode(struct LEVELNODE **ppNode) {
  *ppNode = (struct LEVELNODE *)MemAlloc(sizeof(struct LEVELNODE));
  if (!(*ppNode != NULL)) {
    return FALSE;
  }

  // Clear all values
  memset(*ppNode, 0, sizeof(struct LEVELNODE));

  // Set default values
  (*ppNode)->ubShadeLevel = LightGetAmbient();
  (*ppNode)->ubNaturalShadeLevel = LightGetAmbient();
  (*ppNode)->pSoldier = NULL;
  (*ppNode)->pNext = NULL;
  (*ppNode)->sRelativeX = 0;
  (*ppNode)->sRelativeY = 0;

  guiLevelNodes++;

  return (TRUE);
}

void CountLevelNodes(void) {
  uint32_t uiLoop, uiLoop2;
  struct LEVELNODE *pLN;
  MAP_ELEMENT *pME;

  for (uiLoop2 = 0; uiLoop2 < 9; uiLoop2++) {
    guiLNCount[uiLoop2] = 0;
  }

  for (uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++) {
    pME = &(gpWorldLevelData[uiLoop]);
    // start at 1 to skip land head ptr; 0 stores total
    for (uiLoop2 = 1; uiLoop2 < 9; uiLoop2++) {
      pLN = pME->pLevelNodes[uiLoop2];
      while (pLN != NULL) {
        guiLNCount[uiLoop2]++;
        guiLNCount[0]++;
        pLN = pLN->pNext;
      }
    }
  }
}

#define LINE_HEIGHT 20
void DebugLevelNodePage(void) {
  uint32_t uiLoop;

  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG LEVELNODES PAGE 1 OF 1");

  for (uiLoop = 1; uiLoop < 9; uiLoop++) {
    gprintf(0, LINE_HEIGHT * (uiLoop + 1), gzLevelString[uiLoop], guiLNCount[uiLoop]);
  }
  gprintf(0, LINE_HEIGHT * 12, L"%d land nodes in excess of world max (25600)",
          guiLNCount[1] - WORLD_MAX);
  gprintf(0, LINE_HEIGHT * 13, L"Total # levelnodes %d, %d bytes each", guiLNCount[0],
          sizeof(struct LEVELNODE));
  gprintf(0, LINE_HEIGHT * 14, L"Total memory for levelnodes %d",
          guiLNCount[0] * sizeof(struct LEVELNODE));
}

BOOLEAN TypeExistsInLevel(struct LEVELNODE *pStartNode, uint32_t fType, uint16_t *pusIndex) {
  uint32_t fTileType;

  // Look through all objects and Search for type
  while (pStartNode != NULL) {
    if (pStartNode->usIndex != NO_TILE && pStartNode->usIndex < NUMBEROFTILES) {
      GetTileType(pStartNode->usIndex, &fTileType);

      if (fTileType == fType) {
        *pusIndex = pStartNode->usIndex;
        return (TRUE);
      }
    }

    pStartNode = pStartNode->pNext;
  }

  // Could not find it, return FALSE
  return (FALSE);
}

// SHADE LEVEL MANIPULATION FOR NODES
void SetLevelShadeLevel(struct LEVELNODE *pStartNode, uint8_t ubShadeLevel) {
  // Look through all objects and Search for type
  while (pStartNode != NULL) {
    pStartNode->ubShadeLevel = ubShadeLevel;

    // Advance to next
    pStartNode = pStartNode->pNext;
  }
}

void AdjustLevelShadeLevel(struct LEVELNODE *pStartNode, int8_t bShadeDiff) {
  // Look through all objects and Search for type
  while (pStartNode != NULL) {
    pStartNode->ubShadeLevel += bShadeDiff;

    if (pStartNode->ubShadeLevel > MAX_SHADE_LEVEL) {
      pStartNode->ubShadeLevel = MAX_SHADE_LEVEL;
    }

    if (pStartNode->ubShadeLevel < MIN_SHADE_LEVEL) {
      pStartNode->ubShadeLevel = MIN_SHADE_LEVEL;
    }

    // Advance to next
    pStartNode = pStartNode->pNext;
  }
}

void SetIndexLevelNodeFlags(struct LEVELNODE *pStartNode, uint32_t uiFlags, uint16_t usIndex) {
  // Look through all objects and Search for type
  while (pStartNode != NULL) {
    if (pStartNode->usIndex == usIndex) {
      pStartNode->uiFlags |= uiFlags;
      break;
    }

    // Advance to next
    pStartNode = pStartNode->pNext;
  }
}

void RemoveIndexLevelNodeFlags(struct LEVELNODE *pStartNode, uint32_t uiFlags, uint16_t usIndex) {
  // Look through all objects and Search for type
  while (pStartNode != NULL) {
    if (pStartNode->usIndex == usIndex) {
      pStartNode->uiFlags &= (~uiFlags);
      break;
    }

    // Advance to next
    pStartNode = pStartNode->pNext;
  }
}

// First for object layer
// #################################################################

struct LEVELNODE *AddObjectToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pObject = NULL;
  struct LEVELNODE *pNextObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  // If we're at the head, set here
  if (pObject == NULL) {
    if (!(CreateLevelNode(&pNextObject) != FALSE)) {
      return FALSE;
    }
    pNextObject->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pObjectHead = pNextObject;
  } else {
    while (pObject != NULL) {
      if (pObject->pNext == NULL) {
        if (!(CreateLevelNode(&pNextObject) != FALSE)) {
          return FALSE;
        }
        pObject->pNext = pNextObject;

        pNextObject->pNext = NULL;
        pNextObject->usIndex = usIndex;

        break;
      }

      pObject = pObject->pNext;
    }
  }

  // CheckForAndAddTileCacheStructInfo( pNextObject, (int16_t)iMapIndex, usIndex );

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_OBJECTS);
  return (pNextObject);
}

BOOLEAN AddObjectToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pObject = NULL;
  struct LEVELNODE *pNextObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  if (!(CreateLevelNode(&pNextObject) != FALSE)) {
    return FALSE;
  }

  pNextObject->pNext = pObject;
  pNextObject->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pObjectHead = pNextObject;

  // CheckForAndAddTileCacheStructInfo( pNextObject, (int16_t)iMapIndex, usIndex );

  // If it's NOT the first head
  ResetSpecificLayerOptimizing(TILES_DYNAMIC_OBJECTS);

  // Add the object to the map temp file, if we have to
  AddObjectToMapTempFile(iMapIndex, usIndex);

  return (TRUE);
}

BOOLEAN RemoveObject(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pObject = NULL;
  struct LEVELNODE *pOldObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  // Look through all objects and remove index if found

  while (pObject != NULL) {
    if (pObject->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldObject == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pObjectHead = pObject->pNext;
      } else {
        pOldObject->pNext = pObject->pNext;
      }

      CheckForAndDeleteTileCacheStructInfo(pObject, usIndex);

      // Delete memory assosiated with item
      MemFree(pObject);
      guiLevelNodes--;

      // Add the index to the maps temp file so we can remove it after reloading the map
      AddRemoveObjectToMapTempFile(iMapIndex, usIndex);

      return (TRUE);
    }

    pOldObject = pObject;
    pObject = pObject->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeRangeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                     uint16_t *pusObjectIndex) {
  struct LEVELNODE *pObject = NULL;
  struct LEVELNODE *pOldObject = NULL;
  uint32_t fTileType;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  // Look through all objects and Search for type

  while (pObject != NULL) {
    // Advance to next
    pOldObject = pObject;
    pObject = pObject->pNext;

    if (pOldObject->usIndex != NO_TILE && pOldObject->usIndex < NUMBEROFTILES) {
      GetTileType(pOldObject->usIndex, &fTileType);

      if (fTileType >= fStartType && fTileType <= fEndType) {
        *pusObjectIndex = pOldObject->usIndex;
        return (TRUE);
      }
    }
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusObjectIndex) {
  struct LEVELNODE *pObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  return (TypeExistsInLevel(pObject, fType, pusObjectIndex));
}

void SetAllObjectShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel) {
  struct LEVELNODE *pObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  SetLevelShadeLevel(pObject, ubShadeLevel);
}

void AdjustAllObjectShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff) {
  struct LEVELNODE *pObject = NULL;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  AdjustLevelShadeLevel(pObject, bShadeDiff);
}

BOOLEAN RemoveAllObjectsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pObject = NULL;
  struct LEVELNODE *pOldObject = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pObject = gpWorldLevelData[iMapIndex].pObjectHead;

  // Look through all objects and Search for type

  while (pObject != NULL) {
    // Advance to next
    pOldObject = pObject;
    pObject = pObject->pNext;

    if (pOldObject->usIndex != NO_TILE && pOldObject->usIndex < NUMBEROFTILES) {
      GetTileType(pOldObject->usIndex, &fTileType);

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveObject(iMapIndex, pOldObject->usIndex);
        fRetVal = TRUE;
      }
    }
  }
  return fRetVal;
}

// #######################################################
// Land Peice Layer
// #######################################################

struct LEVELNODE *AddLandToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pNextLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // If we're at the head, set here
  if (pLand == NULL) {
    if (!(CreateLevelNode(&pNextLand) != FALSE)) {
      return FALSE;
    }
    pNextLand->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pLandHead = pNextLand;

  } else {
    while (pLand != NULL) {
      if (pLand->pNext == NULL) {
        if (!(CreateLevelNode(&pNextLand) != FALSE)) {
          return FALSE;
        }
        pLand->pNext = pNextLand;

        pNextLand->pNext = NULL;
        pNextLand->pPrevNode = pLand;
        pNextLand->usIndex = usIndex;

        break;
      }

      pLand = pLand->pNext;
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
  return (pNextLand);
}

BOOLEAN AddLandToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pNextLand = NULL;
  TILE_ELEMENT TileElem;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Allocate head
  if (!(CreateLevelNode(&pNextLand) != FALSE)) {
    return FALSE;
  }

  pNextLand->pNext = pLand;
  pNextLand->pPrevNode = NULL;
  pNextLand->usIndex = usIndex;
  pNextLand->ubShadeLevel = LightGetAmbient();

  if (usIndex < NUMBEROFTILES) {
    // Get tile element
    TileElem = gTileDatabase[usIndex];

    // Check for full tile
    if (TileElem.ubFullTile) {
      gpWorldLevelData[iMapIndex].pLandStart = pNextLand;
    }
  }

  // Set head
  gpWorldLevelData[iMapIndex].pLandHead = pNextLand;

  // If it's NOT the first head
  if (pLand != NULL) {
    pLand->pPrevNode = pNextLand;
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
  return (TRUE);
}

BOOLEAN RemoveLand(uint32_t iMapIndex, uint16_t usIndex) {
  RemoveLandEx(iMapIndex, usIndex);

  AdjustForFullTile(iMapIndex);

  return (FALSE);
}

BOOLEAN RemoveLandEx(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all Lands and remove index if found

  while (pLand != NULL) {
    if (pLand->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pLand->pPrevNode == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pLandHead = pLand->pNext;
      } else {
        pLand->pPrevNode->pNext = pLand->pNext;
      }

      // Check for tail
      if (pLand->pNext == NULL) {
      } else {
        pLand->pNext->pPrevNode = pLand->pPrevNode;
      }

      // Delete memory assosiated with item
      MemFree(pLand);
      guiLevelNodes--;

      break;
    }
    pLand = pLand->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN AdjustForFullTile(uint32_t iMapIndex) {
  struct LEVELNODE *pLand = NULL;
  TILE_ELEMENT TileElem;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all Lands and remove index if found

  while (pLand != NULL) {
    if (pLand->usIndex < NUMBEROFTILES) {
      // If this is a full tile, set new full tile
      TileElem = gTileDatabase[pLand->usIndex];

      // Check for full tile
      if (TileElem.ubFullTile) {
        gpWorldLevelData[iMapIndex].pLandStart = pLand;
        return (TRUE);
      }
    }
    pLand = pLand->pNext;
  }

  // Could not find a full tile
  // Set to tail, and convert it to a full tile!
  // Add a land peice to tail from basic land
  {
    uint16_t NewIndex;
    struct LEVELNODE *pNewNode;

    NewIndex = (uint16_t)(Random(10));

    // Adjust for type
    NewIndex += gTileTypeStartIndex[gCurrentBackground];

    pNewNode = AddLandToTail(iMapIndex, NewIndex);

    gpWorldLevelData[iMapIndex].pLandStart = pNewNode;
  }

  return (FALSE);
}

BOOLEAN ReplaceLandIndex(uint32_t iMapIndex, uint16_t usOldIndex, uint16_t usNewIndex) {
  struct LEVELNODE *pLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all Lands and remove index if found

  while (pLand != NULL) {
    if (pLand->usIndex == usOldIndex) {
      // OK, set new index value
      pLand->usIndex = usNewIndex;

      AdjustForFullTile(iMapIndex);

      return (TRUE);
    }

    // Advance
    pLand = pLand->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeExistsInLandLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusLandIndex) {
  struct LEVELNODE *pLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  return (TypeExistsInLevel(pLand, fType, pusLandIndex));
}

BOOLEAN TypeRangeExistsInLandLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   uint16_t *pusLandIndex) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pOldLand = NULL;
  uint32_t fTileType;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all objects and Search for type

  while (pLand != NULL) {
    if (pLand->usIndex != NO_TILE) {
      GetTileType(pLand->usIndex, &fTileType);

      // Advance to next
      pOldLand = pLand;
      pLand = pLand->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        *pusLandIndex = pOldLand->usIndex;
        return (TRUE);
      }
    }
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeRangeExistsInLandHead(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                  uint16_t *pusLandIndex) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pOldLand = NULL;
  uint32_t fTileType;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all objects and Search for type

  if (pLand->usIndex != NO_TILE) {
    GetTileType(pLand->usIndex, &fTileType);

    // Advance to next
    pOldLand = pLand;
    pLand = pLand->pNext;

    if (fTileType >= fStartType && fTileType <= fEndType) {
      *pusLandIndex = pOldLand->usIndex;
      return (TRUE);
    }
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeRangeExistsInStructLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                     uint16_t *pusStructIndex) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t fTileType;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all objects and Search for type

  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &fTileType);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        *pusStructIndex = pOldStruct->usIndex;
        return (TRUE);
      }
    }
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveAllLandsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pOldLand = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all objects and Search for type

  while (pLand != NULL) {
    if (pLand->usIndex != NO_TILE) {
      GetTileType(pLand->usIndex, &fTileType);

      // Advance to next
      pOldLand = pLand;
      pLand = pLand->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveLand(iMapIndex, pOldLand->usIndex);
        fRetVal = TRUE;
      }
    }
  }
  return fRetVal;
}

void SetAllLandShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel) {
  struct LEVELNODE *pLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  SetLevelShadeLevel(pLand, ubShadeLevel);
}

void AdjustAllLandShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff) {
  struct LEVELNODE *pLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all objects and Search for type
  AdjustLevelShadeLevel(pLand, bShadeDiff);
}

BOOLEAN DeleteAllLandLayers(uint32_t iMapIndex) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pOldLand = NULL;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Look through all objects and Search for type

  while (pLand != NULL) {
    // Advance to next
    pOldLand = pLand;
    pLand = pLand->pNext;

    // Remove Item
    RemoveLandEx(iMapIndex, pOldLand->usIndex);
  }

  // Set world data values
  gpWorldLevelData[iMapIndex].pLandHead = NULL;
  gpWorldLevelData[iMapIndex].pLandStart = NULL;

  return (TRUE);
}

BOOLEAN InsertLandIndexAtLevel(uint32_t iMapIndex, uint16_t usIndex, uint8_t ubLevel) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pNextLand = NULL;
  uint8_t level = 0;
  BOOLEAN CanInsert = FALSE;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // If we want to insert at head;
  if (ubLevel == 0) {
    AddLandToHead(iMapIndex, usIndex);
    return (TRUE);
  }

  // Allocate memory for new item
  if (!(CreateLevelNode(&pNextLand) != FALSE)) {
    return FALSE;
  }
  pNextLand->usIndex = usIndex;

  // Move to index before insertion
  while (pLand != NULL) {
    if (level == (ubLevel - 1)) {
      CanInsert = TRUE;
      break;
    }

    pLand = pLand->pNext;
    level++;
  }

  // Check if level has been macthed
  if (!CanInsert) {
    return (FALSE);
  }

  // Set links, according to position!
  pNextLand->pPrevNode = pLand;
  pNextLand->pNext = pLand->pNext;
  pLand->pNext = pNextLand;

  // Check for tail
  if (pNextLand->pNext == NULL) {
  } else {
    pNextLand->pNext->pPrevNode = pNextLand;
  }

  AdjustForFullTile(iMapIndex);

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
  return (TRUE);
}

BOOLEAN RemoveHigherLandLevels(uint32_t iMapIndex, uint32_t fSrcType, uint32_t **puiHigherTypes,
                               uint8_t *pubNumHigherTypes) {
  struct LEVELNODE *pLand = NULL;
  struct LEVELNODE *pOldLand = NULL;
  uint32_t fTileType;
  uint8_t ubSrcLogHeight;

  *pubNumHigherTypes = 0;
  *puiHigherTypes = NULL;

  // Start at tail and up
  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // GEt tail
  while (pLand != NULL) {
    pOldLand = pLand;
    pLand = pLand->pNext;
  }

  pLand = pOldLand;

  // Get src height
  GetTileTypeLogicalHeight(fSrcType, &ubSrcLogHeight);

  // Look through all objects and Search for height
  while (pLand != NULL) {
    GetTileType(pLand->usIndex, &fTileType);

    // Advance to next
    pOldLand = pLand;
    pLand = pLand->pPrevNode;

    if (gTileTypeLogicalHeight[fTileType] > ubSrcLogHeight) {
      // Remove Item
      SetLandIndex(iMapIndex, pOldLand->usIndex, fTileType, TRUE);

      (*pubNumHigherTypes)++;

      *puiHigherTypes =
          (uint32_t *)MemRealloc(*puiHigherTypes, (*pubNumHigherTypes) * sizeof(uint32_t));

      (*puiHigherTypes)[(*pubNumHigherTypes) - 1] = fTileType;
    }
  }

  // Adjust full tile sets
  AdjustForFullTile(iMapIndex);

  return (TRUE);
}

BOOLEAN SetLowerLandLevels(uint32_t iMapIndex, uint32_t fSrcType, uint16_t usIndex) {
  struct LEVELNODE *pLand = NULL;
  uint32_t fTileType;
  uint8_t ubSrcLogHeight;
  uint16_t NewTile;

  pLand = gpWorldLevelData[iMapIndex].pLandHead;

  // Get src height
  GetTileTypeLogicalHeight(fSrcType, &ubSrcLogHeight);

  // Look through all objects and Search for height
  while (pLand != NULL) {
    GetTileType(pLand->usIndex, &fTileType);

    // Advance to next
    pLand = pLand->pNext;

    if (gTileTypeLogicalHeight[fTileType] < ubSrcLogHeight) {
      // Set item
      GetTileIndexFromTypeSubIndex(fTileType, usIndex, &NewTile);

      // Set as normal
      SetLandIndex(iMapIndex, NewTile, fTileType, FALSE);
    }
  }

  // Adjust full tile sets
  AdjustForFullTile(iMapIndex);

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
  return (TRUE);
}

// Struct layer
// #################################################################

struct LEVELNODE *AddStructToTail(uint32_t iMapIndex, uint16_t usIndex) {
  return (AddStructToTailCommon(iMapIndex, usIndex, TRUE));
}

struct LEVELNODE *ForceStructToTail(uint32_t iMapIndex, uint16_t usIndex) {
  return (AddStructToTailCommon(iMapIndex, usIndex, FALSE));
}

struct LEVELNODE *AddStructToTailCommon(uint32_t iMapIndex, uint16_t usIndex,
                                        BOOLEAN fAddStructDBInfo) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pTailStruct = NULL;
  struct LEVELNODE *pNextStruct = NULL;
  struct DB_STRUCTURE *pDBStructure;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Do we have an empty list?
  if (pStruct == NULL) {
    if (!(CreateLevelNode(&pNextStruct) != FALSE)) {
      return FALSE;
    }

    if (fAddStructDBInfo) {
      if (usIndex < NUMBEROFTILES) {
        if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
          if (AddStructureToWorld((int16_t)iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef,
                                  pNextStruct) == FALSE) {
            MemFree(pNextStruct);
            guiLevelNodes--;
            return (NULL);
          }
        } else {
          //				 pNextStruct->pStructureData = NULL;
        }
      }
    }

    pNextStruct->usIndex = usIndex;

    pNextStruct->pNext = NULL;

    gpWorldLevelData[iMapIndex].pStructHead = pNextStruct;
  } else {
    // MOVE TO TAIL
    while (pStruct != NULL) {
      pTailStruct = pStruct;
      pStruct = pStruct->pNext;
    }

    CHECKN(CreateLevelNode(&pNextStruct) != FALSE);

    if (fAddStructDBInfo) {
      if (usIndex < NUMBEROFTILES) {
        if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
          if (AddStructureToWorld((int16_t)iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef,
                                  pNextStruct) == FALSE) {
            MemFree(pNextStruct);
            guiLevelNodes--;
            return (NULL);
          } else {
            //					pNextStruct->pStructureData = NULL;
          }
        }
      }
    }
    pNextStruct->usIndex = usIndex;

    pNextStruct->pNext = NULL;
    pTailStruct->pNext = pNextStruct;
  }

  // Check flags for tiledat and set a shadow if we have a buddy
  if (usIndex < NUMBEROFTILES) {
    if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
        gTileDatabase[usIndex].sBuddyNum != -1) {
      AddShadowToHead(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
      gpWorldLevelData[iMapIndex].pShadowHead->uiFlags |= LEVELNODE_BUDDYSHADOW;
    }

    // Check for special flag to stop burn-through on same-tile structs...
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      pDBStructure = gTileDatabase[usIndex].pDBStructureRef->pDBStructure;

      // Default to off....
      gpWorldLevelData[iMapIndex].ubExtFlags[0] &= (~MAPELEMENT_EXT_NOBURN_STRUCT);

      // If we are NOT a wall and NOT multi-tiles, set mapelement flag...
      if (!FindStructure((int16_t)iMapIndex, STRUCTURE_WALLSTUFF) &&
          pDBStructure->ubNumberOfTiles == 1) {
        // Set flag...
        gpWorldLevelData[iMapIndex].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
      }
    }
  }

  // Add the structure the maps temp file
  AddStructToMapTempFile(iMapIndex, usIndex);

  // CheckForAndAddTileCacheStructInfo( pNextStruct, (int16_t)iMapIndex, usIndex );

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);

  return (pNextStruct);
}

BOOLEAN AddStructToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pNextStruct = NULL;
  struct DB_STRUCTURE *pDBStructure;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  if (!(CreateLevelNode(&pNextStruct) != FALSE)) {
    return FALSE;
  }

  if (usIndex < NUMBEROFTILES) {
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      if (AddStructureToWorld((int16_t)iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef,
                              pNextStruct) == FALSE) {
        MemFree(pNextStruct);
        guiLevelNodes--;
        return (FALSE);
      }
    }
  }

  pNextStruct->pNext = pStruct;
  pNextStruct->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pStructHead = pNextStruct;

  SetWorldFlagsFromNewNode((uint16_t)iMapIndex, pNextStruct->usIndex);

  if (usIndex < NUMBEROFTILES) {
    // Check flags for tiledat and set a shadow if we have a buddy
    if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
        gTileDatabase[usIndex].sBuddyNum != -1) {
      AddShadowToHead(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
      gpWorldLevelData[iMapIndex].pShadowHead->uiFlags |= LEVELNODE_BUDDYSHADOW;
    }

    // Check for special flag to stop burn-through on same-tile structs...
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      pDBStructure = gTileDatabase[usIndex].pDBStructureRef->pDBStructure;

      // Default to off....
      gpWorldLevelData[iMapIndex].ubExtFlags[0] &= (~MAPELEMENT_EXT_NOBURN_STRUCT);

      // If we are NOT a wall and NOT multi-tiles, set mapelement flag...
      if (!!FindStructure((int16_t)iMapIndex, STRUCTURE_WALLSTUFF) &&
          pDBStructure->ubNumberOfTiles == 1) {
        // Set flag...
        gpWorldLevelData[iMapIndex].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
      }
    }
  }

  // Add the structure the maps temp file
  AddStructToMapTempFile(iMapIndex, usIndex);

  // CheckForAndAddTileCacheStructInfo( pNextStruct, (int16_t)iMapIndex, usIndex );

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
  return (TRUE);
}

BOOLEAN InsertStructIndex(uint32_t iMapIndex, uint16_t usIndex, uint8_t ubLevel) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pNextStruct = NULL;
  uint8_t level = 0;
  BOOLEAN CanInsert = FALSE;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // If we want to insert at head;
  if (ubLevel == 0) {
    return (AddStructToHead(iMapIndex, usIndex));
  }

  // Allocate memory for new item
  if (!(CreateLevelNode(&pNextStruct) != FALSE)) {
    return FALSE;
  }

  pNextStruct->usIndex = usIndex;

  // Move to index before insertion
  while (pStruct != NULL) {
    if (level == (ubLevel - 1)) {
      CanInsert = TRUE;
      break;
    }

    pStruct = pStruct->pNext;
    level++;
  }

  // Check if level has been macthed
  if (!CanInsert) {
    MemFree(pNextStruct);
    guiLevelNodes--;
    return (FALSE);
  }

  if (usIndex < NUMBEROFTILES) {
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      if (AddStructureToWorld((int16_t)iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef,
                              pNextStruct) == FALSE) {
        MemFree(pNextStruct);
        guiLevelNodes--;
        return (FALSE);
      }
    }
  }

  // Set links, according to position!
  pNextStruct->pNext = pStruct->pNext;
  pStruct->pNext = pNextStruct;

  // CheckForAndAddTileCacheStructInfo( pNextStruct, (int16_t)iMapIndex, usIndex );

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
  return (TRUE);
}

BOOLEAN RemoveStructFromTail(uint32_t iMapIndex) {
  return (RemoveStructFromTailCommon(iMapIndex, TRUE));
}

BOOLEAN ForceRemoveStructFromTail(uint32_t iMapIndex) {
  return (RemoveStructFromTailCommon(iMapIndex, FALSE));
}

BOOLEAN RemoveStructFromTailCommon(uint32_t iMapIndex, BOOLEAN fRemoveStructDBInfo) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pPrevStruct = NULL;
  uint16_t usIndex;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // GOTO TAIL
  while (pStruct != NULL) {
    // AT THE TAIL
    if (pStruct->pNext == NULL) {
      if (pPrevStruct != NULL) {
        pPrevStruct->pNext = pStruct->pNext;
      } else
        gpWorldLevelData[iMapIndex].pStructHead = pPrevStruct;

      if (fRemoveStructDBInfo) {
        // Check for special flag to stop burn-through on same-tile structs...
        if (pStruct->pStructureData != NULL) {
          // If we are NOT a wall and NOT multi-tiles, set mapelement flag...
          // if ( !( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF ) &&
          // pStruct->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles == 1 )
          //{
          // UNSet flag...
          //	gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= ( ~MAPELEMENT_EXT_NOBURN_STRUCT );
          //}
        }

        DeleteStructureFromWorld(pStruct->pStructureData);
      }

      usIndex = pStruct->usIndex;

      // If we have to, make sure to remove this node when we reload the map from a saved game
      RemoveStructFromMapTempFile(iMapIndex, usIndex);

      MemFree(pStruct);
      guiLevelNodes--;

      if (usIndex < NUMBEROFTILES) {
        // Check flags for tiledat and set a shadow if we have a buddy
        if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
            gTileDatabase[usIndex].sBuddyNum != -1) {
          RemoveShadow(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
        }
      }
      return (TRUE);
    }

    pPrevStruct = pStruct;
    pStruct = pStruct->pNext;
  }

  return (TRUE);
}

BOOLEAN RemoveStruct(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all structs and remove index if found

  while (pStruct != NULL) {
    if (pStruct->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldStruct == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pStructHead = pStruct->pNext;
      } else {
        pOldStruct->pNext = pStruct->pNext;
      }

      // Check for special flag to stop burn-through on same-tile structs...
      if (pStruct->pStructureData != NULL) {
        // If we are NOT a wall and NOT multi-tiles, set mapelement flag...
        // if ( !( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF ) &&
        // pStruct->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles == 1 )
        //{
        // UNSet flag...
        //	gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= ( ~MAPELEMENT_EXT_NOBURN_STRUCT );
        //}
      }

      // Delete memory assosiated with item
      DeleteStructureFromWorld(pStruct->pStructureData);

      // If we have to, make sure to remove this node when we reload the map from a saved game
      RemoveStructFromMapTempFile(iMapIndex, usIndex);

      if (usIndex < NUMBEROFTILES) {
        // Check flags for tiledat and set a shadow if we have a buddy
        if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
            gTileDatabase[usIndex].sBuddyNum != -1) {
          RemoveShadow(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
        }
      }
      MemFree(pStruct);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldStruct = pStruct;
    pStruct = pStruct->pNext;
  }

  // Could not find it, return FALSE
  RemoveWorldFlagsFromNewNode((uint16_t)iMapIndex, usIndex);

  return (FALSE);
}

BOOLEAN RemoveStructFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint16_t usIndex;

  usIndex = pNode->usIndex;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all structs and remove index if found

  while (pStruct != NULL) {
    if (pStruct == pNode) {
      // OK, set links
      // Check for head or tail
      if (pOldStruct == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pStructHead = pStruct->pNext;
      } else {
        pOldStruct->pNext = pStruct->pNext;
      }

      // Delete memory assosiated with item
      DeleteStructureFromWorld(pStruct->pStructureData);

      // If we have to, make sure to remove this node when we reload the map from a saved game
      RemoveStructFromMapTempFile(iMapIndex, usIndex);

      if (pNode->usIndex < NUMBEROFTILES) {
        // Check flags for tiledat and set a shadow if we have a buddy
        if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
            gTileDatabase[usIndex].sBuddyNum != -1) {
          RemoveShadow(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
        }
      }
      MemFree(pStruct);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldStruct = pStruct;
    pStruct = pStruct->pNext;
  }

  // Could not find it, return FALSE
  RemoveWorldFlagsFromNewNode((uint16_t)iMapIndex, usIndex);

  return (FALSE);
}

BOOLEAN RemoveAllStructsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t fTileType;
  uint16_t usIndex;
  BOOLEAN fRetVal = FALSE;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all structs and Search for type

  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &fTileType);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        usIndex = pOldStruct->usIndex;

        // Remove Item
        if (usIndex < NUMBEROFTILES) {
          RemoveStruct(iMapIndex, pOldStruct->usIndex);
          fRetVal = TRUE;
          if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY &&
              gTileDatabase[usIndex].sBuddyNum != -1) {
            RemoveShadow(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
          }
        }
      }
    }
  }
  return fRetVal;
}

// Kris:  This was a serious problem.  When saving the map and then reloading it, the structure
//  information was invalid if you changed the types, etc.  This is the bulletproof way.
BOOLEAN ReplaceStructIndex(uint32_t iMapIndex, uint16_t usOldIndex, uint16_t usNewIndex) {
  RemoveStruct(iMapIndex, usOldIndex);
  AddWallToStructLayer(iMapIndex, usNewIndex, FALSE);
  return TRUE;
  //	struct LEVELNODE	*pStruct				= NULL;
  //	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;
  // Look through all Structs and remove index if found
  //	while( pStruct != NULL )
  //	{
  //		if ( pStruct->usIndex == usOldIndex )
  //		{
  //			// OK, set new index value
  //			pStruct->usIndex = usNewIndex;
  //			AdjustForFullTile( iMapIndex );
  //			return( TRUE );
  //		}
  //		// Advance
  //		pStruct = pStruct->pNext;
  //	}
  //	// Could not find it, return FALSE
  //	return( FALSE );
}

// When adding, put in order such that it's drawn before any walls of a
// lesser orientation value
BOOLEAN AddWallToStructLayer(int32_t iMapIndex, uint16_t usIndex, BOOLEAN fReplace) {
  struct LEVELNODE *pStruct = NULL;
  uint16_t usCheckWallOrient;
  uint16_t usWallOrientation;
  BOOLEAN fInsertFound = FALSE;
  BOOLEAN fRoofFound = FALSE;
  uint8_t ubRoofLevel = 0;
  uint32_t uiCheckType;
  uint8_t ubLevel = 0;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Get orientation of peice we want to add
  GetWallOrientation(usIndex, &usWallOrientation);

  // Look through all objects and Search for orientation
  while (pStruct != NULL) {
    GetWallOrientation(pStruct->usIndex, &usCheckWallOrient);
    // OLD CASE
    // if ( usCheckWallOrient > usWallOrientation )
    // Kris:
    // New case -- If placing a new wall which is at right angles to the current wall, then
    // we insert it.
    if (usCheckWallOrient > usWallOrientation) {
      if (((usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
           (usCheckWallOrient == INSIDE_TOP_LEFT || usCheckWallOrient == OUTSIDE_TOP_LEFT)) ||
          ((usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) &&
           (usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT))) {
        fInsertFound = TRUE;
      }
    }

    GetTileType(pStruct->usIndex, &uiCheckType);

    //		if ( uiCheckType >= FIRSTFLOOR && uiCheckType <= LASTFLOOR )
    if (uiCheckType >= FIRSTROOF && uiCheckType <= LASTROOF) {
      fRoofFound = TRUE;
      ubRoofLevel = ubLevel;
    }

    // OLD CHECK
    // Check if it's the same orientation
    // if ( usCheckWallOrient == usWallOrientation )
    // Kris:
    // New check -- we want to check for walls being parallel to each other.  If so, then
    // we we want to replace it.  This is because of an existing problem with say, INSIDE_TOP_LEFT
    // and OUTSIDE_TOP_LEFT walls coexisting.
    if (((usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
         (usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT)) ||
        ((usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) &&
         (usCheckWallOrient == INSIDE_TOP_LEFT || usCheckWallOrient == OUTSIDE_TOP_LEFT))) {
      // Same, if replace, replace here
      if (fReplace) {
        return (ReplaceStructIndex(iMapIndex, pStruct->usIndex, usIndex));
      } else {
        return (FALSE);
      }
    }

    // Advance to next
    pStruct = pStruct->pNext;

    ubLevel++;
  }

  // Check if we found an insert position, otherwise set to head
  if (fInsertFound) {
    // Insert struct at head
    AddStructToHead(iMapIndex, usIndex);
  } else {
    // Make sure it's ALWAYS after the roof ( if any )
    if (fRoofFound) {
      InsertStructIndex(iMapIndex, usIndex, ubRoofLevel);
    } else {
      AddStructToTail(iMapIndex, usIndex);
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
  // Could not find it, return FALSE
  return (TRUE);
}

BOOLEAN TypeExistsInStructLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusStructIndex) {
  struct LEVELNODE *pStruct = NULL;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  return (TypeExistsInLevel(pStruct, fType, pusStructIndex));
}

void SetAllStructShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel) {
  struct LEVELNODE *pStruct = NULL;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  SetLevelShadeLevel(pStruct, ubShadeLevel);
}

void AdjustAllStructShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff) {
  struct LEVELNODE *pStruct = NULL;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  AdjustLevelShadeLevel(pStruct, bShadeDiff);
}

void SetStructIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                      uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t fTileType;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all structs and Search for type

  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &fTileType);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        pOldStruct->uiFlags |= uiFlags;
      }
    }
  }
}

BOOLEAN HideStructOfGivenType(uint32_t iMapIndex, uint32_t fType, BOOLEAN fHide) {
  if (fHide) {
    SetRoofIndexFlagsFromTypeRange(iMapIndex, fType, fType, LEVELNODE_HIDDEN);
  } else {
    // ONLY UNHIDE IF NOT REAVEALED ALREADY
    if (!(gpWorldLevelData[iMapIndex].uiFlags & MAPELEMENT_REVEALED)) {
      RemoveRoofIndexFlagsFromTypeRange(iMapIndex, fType, fType, LEVELNODE_HIDDEN);
    }
  }
  return (TRUE);
}

void RemoveStructIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                         uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t fTileType;

  pStruct = gpWorldLevelData[iMapIndex].pStructHead;

  // Look through all structs and Search for type

  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &fTileType);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        pOldStruct->uiFlags &= (~uiFlags);
      }
    }
  }
}

// Shadow layer
// #################################################################

BOOLEAN AddShadowToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pNextShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // If we're at the head, set here
  if (pShadow == NULL) {
    if (!(CreateLevelNode(&pShadow) != FALSE)) {
      return FALSE;
    }
    pShadow->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pShadowHead = pShadow;

  } else {
    while (pShadow != NULL) {
      if (pShadow->pNext == NULL) {
        if (!(CreateLevelNode(&pNextShadow) != FALSE)) {
          return FALSE;
        }
        pShadow->pNext = pNextShadow;
        pNextShadow->pNext = NULL;
        pNextShadow->usIndex = usIndex;
        break;
      }

      pShadow = pShadow->pNext;
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);
  return (TRUE);
}

// Kris:  identical shadows can exist in the same gridno, though it makes no sense
//		because it actually renders the shadows darker than the others.  This is an
//	  undesirable effect with walls and buildings so I added this function to make
//		sure there isn't already a shadow before placing it.
void AddExclusiveShadow(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pShadow;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;
  while (pShadow) {
    if (pShadow->usIndex == usIndex) return;
    pShadow = pShadow->pNext;
  }
  AddShadowToHead(iMapIndex, usIndex);
}

BOOLEAN AddShadowToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pShadow;
  struct LEVELNODE *pNextShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Allocate head
  if (!(CreateLevelNode(&pNextShadow) != FALSE)) {
    return FALSE;
  }
  pNextShadow->pNext = pShadow;
  pNextShadow->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pShadowHead = pNextShadow;

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);
  return (TRUE);
}

BOOLEAN RemoveShadow(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pOldShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Look through all shadows and remove index if found

  while (pShadow != NULL) {
    if (pShadow->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldShadow == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pShadowHead = pShadow->pNext;
      } else {
        pOldShadow->pNext = pShadow->pNext;
      }

      // Delete memory assosiated with item
      MemFree(pShadow);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldShadow = pShadow;
    pShadow = pShadow->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveShadowFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pOldShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Look through all shadows and remove index if found

  while (pShadow != NULL) {
    if (pShadow == pNode) {
      // OK, set links
      // Check for head or tail
      if (pOldShadow == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pShadowHead = pShadow->pNext;
      } else {
        pOldShadow->pNext = pShadow->pNext;
      }

      // Delete memory assosiated with item
      MemFree(pShadow);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldShadow = pShadow;
    pShadow = pShadow->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveStructShadowPartner(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pOldShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Look through all shadows and remove index if found

  while (pShadow != NULL) {
    if (pShadow->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldShadow == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pShadowHead = pShadow->pNext;
      } else {
        pOldShadow->pNext = pShadow->pNext;
      }

      // Delete memory assosiated with item
      MemFree(pShadow);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldShadow = pShadow;
    pShadow = pShadow->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveAllShadowsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pOldShadow = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Look through all shadows and Search for type

  while (pShadow != NULL) {
    if (pShadow->usIndex != NO_TILE) {
      GetTileType(pShadow->usIndex, &fTileType);

      // Advance to next
      pOldShadow = pShadow;
      pShadow = pShadow->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveShadow(iMapIndex, pOldShadow->usIndex);
        fRetVal = TRUE;
      }
    }
  }
  return fRetVal;
}

BOOLEAN RemoveAllShadows(uint32_t iMapIndex) {
  struct LEVELNODE *pShadow = NULL;
  struct LEVELNODE *pOldShadow = NULL;
  BOOLEAN fRetVal = FALSE;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Look through all shadows and Search for type

  while (pShadow != NULL) {
    if (pShadow->usIndex != NO_TILE) {
      // Advance to next
      pOldShadow = pShadow;
      pShadow = pShadow->pNext;

      // Remove Item
      RemoveShadow(iMapIndex, pOldShadow->usIndex);
      fRetVal = TRUE;
    }
  }
  return fRetVal;
}

BOOLEAN TypeExistsInShadowLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusShadowIndex) {
  struct LEVELNODE *pShadow = NULL;

  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  return (TypeExistsInLevel(pShadow, fType, pusShadowIndex));
}

// Merc layer
// #################################################################

BOOLEAN AddMercToHead(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fAddStructInfo) {
  struct LEVELNODE *pMerc = NULL;
  struct LEVELNODE *pNextMerc = NULL;

  pMerc = gpWorldLevelData[iMapIndex].pMercHead;

  // Allocate head
  if (!(CreateLevelNode(&pNextMerc) != FALSE)) {
    return FALSE;
  }
  pNextMerc->pNext = pMerc;
  pNextMerc->pSoldier = pSoldier;
  pNextMerc->uiFlags |= LEVELNODE_SOLDIER;

  // Add structure info if we want
  if (fAddStructInfo) {
    // Set soldier's levelnode
    pSoldier->pLevelNode = pNextMerc;

    AddMercStructureInfo((uint16_t)iMapIndex, pSoldier);
  }

  // Set head
  gpWorldLevelData[iMapIndex].pMercHead = pNextMerc;

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_MERCS | TILES_DYNAMIC_STRUCT_MERCS |
                               TILES_DYNAMIC_HIGHMERCS);
  return (TRUE);
}

BOOLEAN AddMercStructureInfo(int16_t sGridNo, struct SOLDIERTYPE *pSoldier) {
  uint16_t usAnimSurface;

  // Get surface data
  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  AddMercStructureInfoFromAnimSurface(sGridNo, pSoldier, usAnimSurface, pSoldier->usAnimState);

  return (TRUE);
}

BOOLEAN AddMercStructureInfoFromAnimSurface(int16_t sGridNo, struct SOLDIERTYPE *pSoldier,
                                            uint16_t usAnimSurface, uint16_t usAnimState) {
  struct STRUCTURE_FILE_REF *pStructureFileRef;
  BOOLEAN fReturn;

  // Turn off multi tile flag...
  pSoldier->uiStatusFlags &= (~SOLDIER_MULTITILE);

  if (pSoldier->pLevelNode == NULL) {
    return (FALSE);
  }

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    return (FALSE);
  }

  // Remove existing structs
  DeleteStructureFromWorld(pSoldier->pLevelNode->pStructureData);
  pSoldier->pLevelNode->pStructureData = NULL;

  pStructureFileRef = GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, usAnimState);

  // Now check if we are multi-tiled!
  if (pStructureFileRef != NULL) {
    if (pSoldier->ubBodyType == QUEENMONSTER) {
      // Queen uses onely one direction....
      fReturn = AddStructureToWorld(sGridNo, pSoldier->bLevel,
                                    &(pStructureFileRef->pDBStructureRef[0]), pSoldier->pLevelNode);
    } else {
      fReturn = AddStructureToWorld(
          sGridNo, pSoldier->bLevel,
          &(pStructureFileRef->pDBStructureRef[gOneCDirection[pSoldier->bDirection]]),
          pSoldier->pLevelNode);
    }
    /*
                    if ( fReturn == FALSE )
                    {
                            // try to add default
                            ScreenMsg( MSG_FONT_YELLOW, MSG_DEBUG, L"FAILED: add struct info for
       merc: %d, at %d direction %d, trying default instead", GetSolID(pSoldier), sGridNo,
       pSoldier->bDirection );

                            pStructureFileRef = GetDefaultStructureRef( GetSolID(pSoldier) );
                            if ( pStructureFileRef )
                            {
                                    fReturn = AddStructureToWorld( sGridNo, pSoldier->bLevel, &(
       pStructureFileRef->pDBStructureRef[ gOneCDirection[ pSoldier->bDirection ] ] ),
       pSoldier->pLevelNode );
                            }
                    }
                    */

    if (fReturn == FALSE) {
      // Debug msg
      ScreenMsg(MSG_FONT_RED, MSG_DEBUG,
                L"FAILED: add struct info for merc %d (%s), at %d direction %d", GetSolID(pSoldier),
                pSoldier->name, sGridNo, pSoldier->bDirection);

      if (pStructureFileRef->pDBStructureRef[gOneCDirection[pSoldier->bDirection]]
              .pDBStructure->ubNumberOfTiles > 1) {
        // If we have more than one tile
        pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_Z;
      }

      return (FALSE);
    } else {
      // Turn on if we are multi-tiled
      if (pSoldier->pLevelNode->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles >
          1) {
        // If we have more than one tile
        pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_Z;
      } else {
        // pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_NZ;
      }
    }
  }

  return (TRUE);
}

BOOLEAN OKToAddMercToWorld(struct SOLDIERTYPE *pSoldier, int8_t bDirection) {
  uint16_t usAnimSurface;
  struct STRUCTURE_FILE_REF *pStructFileRef;
  uint16_t usOKToAddStructID;

  // if ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE )
  {
    // Get surface data
    usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);
    if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
      return (FALSE);
    }

    pStructFileRef = GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, pSoldier->usAnimState);

    // Now check if we have multi-tile info!
    if (pStructFileRef != NULL) {
      // Try adding struct to this location, if we can it's good!
      if (pSoldier->pLevelNode && pSoldier->pLevelNode->pStructureData != NULL) {
        usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
      } else {
        usOKToAddStructID = INVALID_STRUCTURE_ID;
      }

      if (!OkayToAddStructureToWorld(pSoldier->sGridNo, pSoldier->bLevel,
                                     &(pStructFileRef->pDBStructureRef[gOneCDirection[bDirection]]),
                                     usOKToAddStructID)) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN UpdateMercStructureInfo(struct SOLDIERTYPE *pSoldier) {
  // Remove strucute info!
  if (pSoldier->pLevelNode == NULL) {
    return (FALSE);
  }

  // DeleteStructureFromWorld( pSoldier->pLevelNode->pStructureData );

  // Add new one!
  return (AddMercStructureInfo(pSoldier->sGridNo, pSoldier));
}

BOOLEAN RemoveMerc(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fPlaceHolder) {
  struct LEVELNODE *pMerc = NULL;
  struct LEVELNODE *pOldMerc = NULL;
  BOOLEAN fMercFound;

  if (iMapIndex == NOWHERE) {
    return (FALSE);
  }

  pMerc = gpWorldLevelData[iMapIndex].pMercHead;

  // Look through all mercs and remove index if found

  while (pMerc != NULL) {
    fMercFound = FALSE;

    if (pMerc->pSoldier == pSoldier) {
      // If it's a placeholder, check!
      if (fPlaceHolder) {
        if ((pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER)) {
          fMercFound = TRUE;
        }
      } else {
        if (!(pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER)) {
          fMercFound = TRUE;
        }
      }

      if (fMercFound) {
        // OK, set links
        // Check for head or tail
        if (pOldMerc == NULL) {
          // It's the head
          gpWorldLevelData[iMapIndex].pMercHead = pMerc->pNext;
        } else {
          pOldMerc->pNext = pMerc->pNext;
        }

        if (!fPlaceHolder) {
          // Set level node to NULL
          pSoldier->pLevelNode = NULL;

          // Remove strucute info!
          DeleteStructureFromWorld(pMerc->pStructureData);
        }

        // Delete memory assosiated with item
        MemFree(pMerc);
        guiLevelNodes--;

        return (TRUE);
      }
    }

    pOldMerc = pMerc;
    pMerc = pMerc->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

// Roof layer
// #################################################################

struct LEVELNODE *AddRoofToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pNextRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // If we're at the head, set here
  if (pRoof == NULL) {
    if (!(CreateLevelNode(&pRoof) != FALSE)) {
      return FALSE;
    }

    if (usIndex < NUMBEROFTILES) {
      if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
        if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                                pRoof) == FALSE) {
          MemFree(pRoof);
          guiLevelNodes--;
          return (FALSE);
        }
      }
    }
    pRoof->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pRoofHead = pRoof;

    pNextRoof = pRoof;
  } else {
    while (pRoof != NULL) {
      if (pRoof->pNext == NULL) {
        if (!(CreateLevelNode(&pNextRoof) != FALSE)) {
          return FALSE;
        }

        if (usIndex < NUMBEROFTILES) {
          if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
            if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                                    pNextRoof) == FALSE) {
              MemFree(pNextRoof);
              guiLevelNodes--;
              return (FALSE);
            }
          }
        }
        pRoof->pNext = pNextRoof;

        pNextRoof->pNext = NULL;
        pNextRoof->usIndex = usIndex;

        break;
      }

      pRoof = pRoof->pNext;
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_ROOF);

  return (pNextRoof);
}

BOOLEAN AddRoofToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pNextRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  if (!(CreateLevelNode(&pNextRoof) != FALSE)) {
    return FALSE;
  }

  if (usIndex < NUMBEROFTILES) {
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                              pNextRoof) == FALSE) {
        MemFree(pNextRoof);
        guiLevelNodes--;
        return (FALSE);
      }
    }
  }

  pNextRoof->pNext = pRoof;
  pNextRoof->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pRoofHead = pNextRoof;

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_ROOF);
  return (TRUE);
}

BOOLEAN RemoveRoof(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pOldRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and remove index if found

  while (pRoof != NULL) {
    if (pRoof->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldRoof == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pRoofHead = pRoof->pNext;
      } else {
        pOldRoof->pNext = pRoof->pNext;
      }
      // Delete memory assosiated with item
      DeleteStructureFromWorld(pRoof->pStructureData);
      MemFree(pRoof);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldRoof = pRoof;
    pRoof = pRoof->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusRoofIndex) {
  struct LEVELNODE *pRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  return (TypeExistsInLevel(pRoof, fType, pusRoofIndex));
}

BOOLEAN TypeRangeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   uint16_t *pusRoofIndex) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pOldRoof = NULL;
  uint32_t fTileType;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all objects and Search for type

  while (pRoof != NULL) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &fTileType);

      // Advance to next
      pOldRoof = pRoof;
      pRoof = pRoof->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        *pusRoofIndex = pOldRoof->usIndex;
        return (TRUE);
      }
    }
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN IndexExistsInRoofLayer(int16_t sGridNo, uint16_t usIndex) {
  struct LEVELNODE *pRoof = NULL;

  pRoof = gpWorldLevelData[sGridNo].pRoofHead;

  // Look through all objects and Search for type

  while (pRoof != NULL) {
    if (pRoof->usIndex == usIndex) {
      return (TRUE);
    }

    pRoof = pRoof->pNext;
  }

  // Could not find it, return FALSE
  return (FALSE);
}

void SetAllRoofShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel) {
  struct LEVELNODE *pRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  SetLevelShadeLevel(pRoof, ubShadeLevel);
}

void AdjustAllRoofShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff) {
  struct LEVELNODE *pRoof = NULL;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  AdjustLevelShadeLevel(pRoof, bShadeDiff);
}

BOOLEAN RemoveAllRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pOldRoof = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and Search for type

  while (pRoof != NULL) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &fTileType);

      // Advance to next
      pOldRoof = pRoof;
      pRoof = pRoof->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveRoof(iMapIndex, pOldRoof->usIndex);
        fRetVal = TRUE;
      }
    }
  }

  // Could not find it, return FALSE

  return fRetVal;
}

void RemoveRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                       uint32_t uiFlags) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pOldRoof = NULL;
  uint32_t fTileType;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and Search for type

  while (pRoof != NULL) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &fTileType);

      // Advance to next
      pOldRoof = pRoof;
      pRoof = pRoof->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        pOldRoof->uiFlags &= (~uiFlags);
      }
    }
  }
}

void SetRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                    uint32_t uiFlags) {
  struct LEVELNODE *pRoof = NULL;
  struct LEVELNODE *pOldRoof = NULL;
  uint32_t fTileType;

  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and Search for type

  while (pRoof != NULL) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &fTileType);

      // Advance to next
      pOldRoof = pRoof;
      pRoof = pRoof->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        pOldRoof->uiFlags |= uiFlags;
      }
    }
  }
}

// OnRoof layer
// #################################################################

struct LEVELNODE *AddOnRoofToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pOnRoof = NULL;
  struct LEVELNODE *pNextOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  // If we're at the head, set here
  if (pOnRoof == NULL) {
    if (!(CreateLevelNode(&pOnRoof) != FALSE)) {
      return FALSE;
    }

    if (usIndex < NUMBEROFTILES) {
      if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
        if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                                pOnRoof) == FALSE) {
          MemFree(pOnRoof);
          guiLevelNodes--;
          return (FALSE);
        }
      }
    }
    pOnRoof->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pOnRoofHead = pOnRoof;

    ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);
    return (pOnRoof);

  } else {
    while (pOnRoof != NULL) {
      if (pOnRoof->pNext == NULL) {
        if (!(CreateLevelNode(&pNextOnRoof) != FALSE)) {
          return FALSE;
        }

        if (usIndex < NUMBEROFTILES) {
          if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
            if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                                    pNextOnRoof) == FALSE) {
              MemFree(pNextOnRoof);
              guiLevelNodes--;
              return (NULL);
            }
          }
        }

        pOnRoof->pNext = pNextOnRoof;

        pNextOnRoof->pNext = NULL;
        pNextOnRoof->usIndex = usIndex;
        break;
      }

      pOnRoof = pOnRoof->pNext;
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);
  return (pNextOnRoof);
}

BOOLEAN AddOnRoofToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pOnRoof = NULL;
  struct LEVELNODE *pNextOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  if (!(CreateLevelNode(&pNextOnRoof) != FALSE)) {
    return FALSE;
  }
  if (usIndex < NUMBEROFTILES) {
    if (gTileDatabase[usIndex].pDBStructureRef != NULL) {
      if (AddStructureToWorld((int16_t)iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef,
                              pNextOnRoof) == FALSE) {
        MemFree(pNextOnRoof);
        guiLevelNodes--;
        return (FALSE);
      }
    }
  }

  pNextOnRoof->pNext = pOnRoof;
  pNextOnRoof->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pOnRoofHead = pNextOnRoof;

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);
  return (TRUE);
}

BOOLEAN RemoveOnRoof(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pOnRoof = NULL;
  struct LEVELNODE *pOldOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  // Look through all OnRoofs and remove index if found

  while (pOnRoof != NULL) {
    if (pOnRoof->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldOnRoof == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pOnRoofHead = pOnRoof->pNext;
      } else {
        pOldOnRoof->pNext = pOnRoof->pNext;
      }

      // REMOVE ONROOF!
      MemFree(pOnRoof);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldOnRoof = pOnRoof;
    pOnRoof = pOnRoof->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveOnRoofFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode) {
  struct LEVELNODE *pOnRoof = NULL;
  struct LEVELNODE *pOldOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  // Look through all OnRoofs and remove index if found

  while (pOnRoof != NULL) {
    if (pOnRoof == pNode) {
      // OK, set links
      // Check for head or tail
      if (pOldOnRoof == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pOnRoofHead = pOnRoof->pNext;
      } else {
        pOldOnRoof->pNext = pOnRoof->pNext;
      }

      // REMOVE ONROOF!
      MemFree(pOnRoof);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldOnRoof = pOnRoof;
    pOnRoof = pOnRoof->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN TypeExistsInOnRoofLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusOnRoofIndex) {
  struct LEVELNODE *pOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  return (TypeExistsInLevel(pOnRoof, fType, pusOnRoofIndex));
}

void SetAllOnRoofShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel) {
  struct LEVELNODE *pOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  SetLevelShadeLevel(pOnRoof, ubShadeLevel);
}

void AdjustAllOnRoofShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff) {
  struct LEVELNODE *pOnRoof = NULL;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  AdjustLevelShadeLevel(pOnRoof, bShadeDiff);
}

BOOLEAN RemoveAllOnRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pOnRoof = NULL;
  struct LEVELNODE *pOldOnRoof = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead;

  // Look through all OnRoofs and Search for type

  while (pOnRoof != NULL) {
    if (pOnRoof->usIndex != NO_TILE) {
      GetTileType(pOnRoof->usIndex, &fTileType);

      // Advance to next
      pOldOnRoof = pOnRoof;
      pOnRoof = pOnRoof->pNext;

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveOnRoof(iMapIndex, pOldOnRoof->usIndex);
        fRetVal = TRUE;
      }
    }
  }
  return fRetVal;
}

// Topmost layer
// #################################################################

struct LEVELNODE *AddTopmostToTail(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pTopmost = NULL;
  struct LEVELNODE *pNextTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  // If we're at the head, set here
  if (pTopmost == NULL) {
    CHECKN(CreateLevelNode(&pNextTopmost) != FALSE);
    pNextTopmost->usIndex = usIndex;

    gpWorldLevelData[iMapIndex].pTopmostHead = pNextTopmost;

  } else {
    while (pTopmost != NULL) {
      if (pTopmost->pNext == NULL) {
        CHECKN(CreateLevelNode(&pNextTopmost) != FALSE);
        pTopmost->pNext = pNextTopmost;
        pNextTopmost->pNext = NULL;
        pNextTopmost->usIndex = usIndex;

        break;
      }

      pTopmost = pTopmost->pNext;
    }
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
  return (pNextTopmost);
}

BOOLEAN AddUIElem(uint32_t iMapIndex, uint16_t usIndex, int8_t sRelativeX, int8_t sRelativeY,
                  struct LEVELNODE **ppNewNode) {
  struct LEVELNODE *pTopmost = NULL;

  pTopmost = AddTopmostToTail(iMapIndex, usIndex);

  if (!(pTopmost != NULL)) {
    return FALSE;
  }

  // Set flags
  pTopmost->uiFlags |= LEVELNODE_USERELPOS;
  pTopmost->sRelativeX = sRelativeX;
  pTopmost->sRelativeY = sRelativeY;

  if (ppNewNode != NULL) {
    *ppNewNode = pTopmost;
  }

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
  return (TRUE);
}

void RemoveUIElem(uint32_t iMapIndex, uint16_t usIndex) { RemoveTopmost(iMapIndex, usIndex); }

BOOLEAN AddTopmostToHead(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pTopmost = NULL;
  struct LEVELNODE *pNextTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  // Allocate head
  if (!(CreateLevelNode(&pNextTopmost) != FALSE)) {
    return FALSE;
  }
  pNextTopmost->pNext = pTopmost;
  pNextTopmost->usIndex = usIndex;

  // Set head
  gpWorldLevelData[iMapIndex].pTopmostHead = pNextTopmost;

  ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
  return (TRUE);
}

BOOLEAN RemoveTopmost(uint32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pTopmost = NULL;
  struct LEVELNODE *pOldTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  // Look through all topmosts and remove index if found

  while (pTopmost != NULL) {
    if (pTopmost->usIndex == usIndex) {
      // OK, set links
      // Check for head or tail
      if (pOldTopmost == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pTopmostHead = pTopmost->pNext;
      } else {
        pOldTopmost->pNext = pTopmost->pNext;
      }

      // Delete memory assosiated with item
      MemFree(pTopmost);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldTopmost = pTopmost;
    pTopmost = pTopmost->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveTopmostFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode) {
  struct LEVELNODE *pTopmost = NULL;
  struct LEVELNODE *pOldTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  // Look through all topmosts and remove index if found

  while (pTopmost != NULL) {
    if (pTopmost == pNode) {
      // OK, set links
      // Check for head or tail
      if (pOldTopmost == NULL) {
        // It's the head
        gpWorldLevelData[iMapIndex].pTopmostHead = pTopmost->pNext;
      } else {
        pOldTopmost->pNext = pTopmost->pNext;
      }

      // Delete memory assosiated with item
      MemFree(pTopmost);
      guiLevelNodes--;

      return (TRUE);
    }

    pOldTopmost = pTopmost;
    pTopmost = pTopmost->pNext;
  }

  // Could not find it, return FALSE

  return (FALSE);
}

BOOLEAN RemoveAllTopmostsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType) {
  struct LEVELNODE *pTopmost = NULL;
  struct LEVELNODE *pOldTopmost = NULL;
  uint32_t fTileType;
  BOOLEAN fRetVal = FALSE;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  // Look through all topmosts and Search for type

  while (pTopmost != NULL) {
    // Advance to next
    pOldTopmost = pTopmost;
    pTopmost = pTopmost->pNext;

    if (pOldTopmost->usIndex != NO_TILE && pOldTopmost->usIndex < NUMBEROFTILES) {
      GetTileType(pOldTopmost->usIndex, &fTileType);

      if (fTileType >= fStartType && fTileType <= fEndType) {
        // Remove Item
        RemoveTopmost(iMapIndex, pOldTopmost->usIndex);
        fRetVal = TRUE;
      }
    }
  }
  return fRetVal;
}

BOOLEAN TypeExistsInTopmostLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusTopmostIndex) {
  struct LEVELNODE *pTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  return (TypeExistsInLevel(pTopmost, fType, pusTopmostIndex));
}

void SetTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, uint16_t usIndex) {
  struct LEVELNODE *pTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  SetIndexLevelNodeFlags(pTopmost, uiFlags, usIndex);
}

void RemoveTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, uint16_t usIndex) {
  struct LEVELNODE *pTopmost = NULL;

  pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;

  RemoveIndexLevelNodeFlags(pTopmost, uiFlags, usIndex);
}

BOOLEAN SetMapElementShadeLevel(uint32_t uiMapIndex, uint8_t ubShadeLevel) {
  SetAllLandShadeLevels(uiMapIndex, ubShadeLevel);
  SetAllObjectShadeLevels(uiMapIndex, ubShadeLevel);
  SetAllStructShadeLevels(uiMapIndex, ubShadeLevel);

  return (TRUE);
}

BOOLEAN IsHeigherLevel(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_NORMAL_ROOF);

  if (pStructure != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsLowerLevel(int16_t sGridNo) {
  struct STRUCTURE *pStructure;

  pStructure = FindStructure(sGridNo, STRUCTURE_NORMAL_ROOF);

  if (pStructure == NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsRoofVisible(int16_t sMapPos) {
  struct STRUCTURE *pStructure;

  if (!gfBasement) {
    pStructure = FindStructure(sMapPos, STRUCTURE_ROOF);

    if (pStructure != NULL) {
      if (!(gpWorldLevelData[sMapPos].uiFlags & MAPELEMENT_REVEALED)) {
        return (TRUE);
      }
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

BOOLEAN IsRoofVisible2(int16_t sMapPos) {
  struct STRUCTURE *pStructure;

  if (!gfBasement) {
    pStructure = FindStructure(sMapPos, STRUCTURE_ROOF);

    if (pStructure != NULL) {
      if (!(gpWorldLevelData[sMapPos].uiFlags & MAPELEMENT_REVEALED)) {
        return (TRUE);
      }
    }
  } else {
    // if ( InARoom( sMapPos, &ubRoom ) )
    {
      if (!(gpWorldLevelData[sMapPos].uiFlags & MAPELEMENT_REVEALED)) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

uint8_t WhoIsThere2(int16_t sGridNo, int8_t bLevel) {
  struct STRUCTURE *pStructure;

  if (!GridNoOnVisibleWorldTile(sGridNo)) {
    return (NOBODY);
  }

  if (gpWorldLevelData[sGridNo].pStructureHead != NULL) {
    pStructure = gpWorldLevelData[sGridNo].pStructureHead;

    while (pStructure) {
      // person must either have their pSoldier->sGridNo here or be non-passable
      if ((pStructure->fFlags & STRUCTURE_PERSON) &&
          (!(pStructure->fFlags & STRUCTURE_PASSABLE) ||
           MercPtrs[pStructure->usStructureID]->sGridNo == sGridNo)) {
        if ((bLevel == 0 && pStructure->sCubeOffset == 0) ||
            (bLevel > 0 && pStructure->sCubeOffset > 0)) {
          // found a person, on the right level!
          // structure ID and merc ID are identical for merc structures
          return ((uint8_t)pStructure->usStructureID);
        }
      }
      pStructure = pStructure->pNext;
    }
  }

  return ((uint8_t)NOBODY);
}

uint8_t GetTerrainType(int16_t sGridNo) {
  return (gpWorldLevelData[sGridNo].ubTerrainID);
  /*
          struct LEVELNODE	*pNode;


          // Check if we have anything in object layer which has a terrain modifier
          pNode = gpWorldLevelData[ sGridNo ].pObjectHead;

          if ( pNode != NULL )
          {
                  if ( gTileDatabase[ pNode->usIndex ].ubTerrainID != NO_TERRAIN )
                  {
                          return( gTileDatabase[ pNode->usIndex ].ubTerrainID );
                  }
          }

          // Now try terrain!
          pNode = gpWorldLevelData[ sGridNo ].pLandHead;

          return( gTileDatabase[ pNode->usIndex ].ubTerrainID );
  */
}

BOOLEAN Water(int16_t sGridNo) {
  MAP_ELEMENT *pMapElement;

  if (sGridNo == NOWHERE) {
    return (FALSE);
  }

  pMapElement = &(gpWorldLevelData[sGridNo]);
  if (pMapElement->ubTerrainID == LOW_WATER || pMapElement->ubTerrainID == MED_WATER ||
      pMapElement->ubTerrainID == DEEP_WATER) {
    // check for a bridge!  otherwise...
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN DeepWater(int16_t sGridNo) {
  MAP_ELEMENT *pMapElement;

  pMapElement = &(gpWorldLevelData[sGridNo]);
  if (pMapElement->ubTerrainID == DEEP_WATER) {
    // check for a bridge!  otherwise...
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN WaterTooDeepForAttacks(int16_t sGridNo) { return (DeepWater(sGridNo)); }

void SetStructAframeFlags(uint32_t iMapIndex, uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t uiTileFlags;

  pStruct = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and Search for type
  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileFlags(pStruct->usIndex, &uiTileFlags);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (uiTileFlags & AFRAME_TILE) {
        pOldStruct->uiFlags |= uiFlags;
      }
    }
  }
}

void RemoveStructAframeFlags(uint32_t iMapIndex, uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t uiTileFlags;

  pStruct = gpWorldLevelData[iMapIndex].pRoofHead;

  // Look through all Roofs and Search for type
  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileFlags(pStruct->usIndex, &uiTileFlags);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      if (uiTileFlags & AFRAME_TILE) {
        pOldStruct->uiFlags &= (~uiFlags);
      }
    }
  }
}

struct LEVELNODE *FindLevelNodeBasedOnStructure(int16_t sGridNo, struct STRUCTURE *pStructure) {
  struct LEVELNODE *pLevelNode;

  // ATE: First look on the struct layer.....
  pLevelNode = gpWorldLevelData[sGridNo].pStructHead;
  while (pLevelNode != NULL) {
    if (pLevelNode->pStructureData == pStructure) {
      return (pLevelNode);
    }
    pLevelNode = pLevelNode->pNext;
  }

  // Next the roof layer....
  pLevelNode = gpWorldLevelData[sGridNo].pRoofHead;
  while (pLevelNode != NULL) {
    if (pLevelNode->pStructureData == pStructure) {
      return (pLevelNode);
    }
    pLevelNode = pLevelNode->pNext;
  }

  // Then the object layer....
  pLevelNode = gpWorldLevelData[sGridNo].pObjectHead;
  while (pLevelNode != NULL) {
    if (pLevelNode->pStructureData == pStructure) {
      return (pLevelNode);
    }
    pLevelNode = pLevelNode->pNext;
  }

  // Finally the onroof layer....
  pLevelNode = gpWorldLevelData[sGridNo].pOnRoofHead;
  while (pLevelNode != NULL) {
    if (pLevelNode->pStructureData == pStructure) {
      return (pLevelNode);
    }
    pLevelNode = pLevelNode->pNext;
  }

  // Assert here if it cannot be found....
  AssertMsg(0, "FindLevelNodeBasedOnStruct failed.");

  return (NULL);
}

struct LEVELNODE *FindShadow(int16_t sGridNo, uint16_t usStructIndex) {
  struct LEVELNODE *pLevelNode;
  uint16_t usShadowIndex;

  if (usStructIndex < FIRSTOSTRUCT1 || usStructIndex >= FIRSTSHADOW1) {
    return (NULL);
  }

  usShadowIndex = usStructIndex - FIRSTOSTRUCT1 + FIRSTSHADOW1;

  pLevelNode = gpWorldLevelData[sGridNo].pShadowHead;
  while (pLevelNode != NULL) {
    if (pLevelNode->usIndex == usShadowIndex) {
      break;
    }
    pLevelNode = pLevelNode->pNext;
  }
  return (pLevelNode);
}

void WorldHideTrees() {
  struct LEVELNODE *pNode;
  uint32_t fTileFlags;
  uint32_t cnt;

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    pNode = gpWorldLevelData[cnt].pStructHead;
    while (pNode != NULL) {
      GetTileFlags(pNode->usIndex, &fTileFlags);

      if (fTileFlags & FULL3D_TILE) {
        if (!(pNode->uiFlags & LEVELNODE_REVEALTREES)) {
          pNode->uiFlags |= (LEVELNODE_REVEALTREES);
        }
      }
      pNode = pNode->pNext;
    }
  }

  SetRenderFlags(RENDER_FLAG_FULL);
}

void WorldShowTrees() {
  struct LEVELNODE *pNode;
  uint32_t fTileFlags;
  uint32_t cnt;

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    pNode = gpWorldLevelData[cnt].pStructHead;
    while (pNode != NULL) {
      GetTileFlags(pNode->usIndex, &fTileFlags);

      if (fTileFlags & FULL3D_TILE) {
        if ((pNode->uiFlags & LEVELNODE_REVEALTREES)) {
          pNode->uiFlags &= (~(LEVELNODE_REVEALTREES));
        }
      }
      pNode = pNode->pNext;
    }
  }

  SetRenderFlags(RENDER_FLAG_FULL);
}

void SetWorldFlagsFromNewNode(uint16_t sGridNo, uint16_t usIndex) {}

void RemoveWorldFlagsFromNewNode(uint16_t sGridNo, uint16_t usIndex) {}

void SetWallLevelnodeFlags(uint16_t sGridNo, uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;

  pStruct = gpWorldLevelData[sGridNo].pStructHead;

  // Look through all objects and Search for type

  while (pStruct != NULL) {
    if (pStruct->pStructureData != NULL) {
      // See if we are a wall!
      if (pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF) {
        pStruct->uiFlags |= uiFlags;
      }
    }
    // Advance to next
    pStruct = pStruct->pNext;
  }
}

void RemoveWallLevelnodeFlags(uint16_t sGridNo, uint32_t uiFlags) {
  struct LEVELNODE *pStruct = NULL;

  pStruct = gpWorldLevelData[sGridNo].pStructHead;

  // Look through all objects and Search for type

  while (pStruct != NULL) {
    if (pStruct->pStructureData != NULL) {
      // See if we are a wall!
      if (pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF) {
        pStruct->uiFlags &= (~uiFlags);
      }
    }
    // Advance to next
    pStruct = pStruct->pNext;
  }
}

void SetTreeTopStateForMap() {
  if (!gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS]) {
    WorldHideTrees();
    gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
  } else {
    WorldShowTrees();
    gTacticalStatus.uiFlags &= (~NOHIDE_REDUNDENCY);
  }

  // FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
  InvalidateWorldRedundency();
}
