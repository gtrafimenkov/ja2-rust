#include "TileEngine/Structure.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/MemMan.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "Strategic/StrategicMap.h"
#include "SysGlobals.h"
#include "Tactical/Interface.h"
#include "Tactical/LOS.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/Smell.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "rust_fileman.h"

// Kris:
#ifdef JA2EDITOR
#include "Editor/EditorUndo.h"  //for access to AddToUndoList( iMapIndex )
#endif

#include "TileEngine/Buildings.h"
#include "TileEngine/ExplosionControl.h"
#include "Utils/SoundControl.h"

#ifdef COUNT_PATHS
extern uint32_t guiSuccessfulPathChecks;
extern uint32_t guiTotalPathChecks;
extern uint32_t guiFailedPathChecks;
extern uint32_t guiUnsuccessfulPathChecks;
#endif

/*
 * NB:  STRUCTURE_SPECIAL
 *
 * Means different things depending on the context.
 *
 * WALLNWINDOW SPECIAL - opaque to sight
 * MULTI SPECIAL - second level (damaged) MULTI structure, should only be deleted if
 *    starting with the deletion of a MULTI SPECIAL structure
 */

uint8_t AtHeight[PROFILE_Z_SIZE] = {0x01, 0x02, 0x04, 0x08};

#define FIRST_AVAILABLE_STRUCTURE_ID (INVALID_STRUCTURE_ID + 2)

uint16_t gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;

struct STRUCTURE_FILE_REF *gpStructureFileRefs;

int32_t guiMaterialHitSound[NUM_MATERIAL_TYPES] = {
    -1,
    S_WOOD_IMPACT1,
    S_WOOD_IMPACT2,
    S_WOOD_IMPACT3,
    S_VEG_IMPACT1,
    -1,
    S_PORCELAIN_IMPACT1,
    -1,
    -1,
    -1,

    -1,
    S_STONE_IMPACT1,
    S_STONE_IMPACT1,
    S_STONE_IMPACT1,
    S_STONE_IMPACT1,
    S_RUBBER_IMPACT1,
    -1,
    -1,
    -1,
    -1,

    -1,
    S_METAL_IMPACT1,
    S_METAL_IMPACT2,
    S_METAL_IMPACT3,
    S_STONE_IMPACT1,
    S_METAL_IMPACT3,
};

uint8_t gubMaterialArmour[] =
    /*
    index  1-10, organics
    index 11-20, rocks and concretes
    index 21-30, metals

    index 1, dry timber
    index 2, furniture wood
    index 3, tree wood
    index 11, stone masonry
    index 12, non-reinforced concrete
    index 13, reinforced concrete
    index 14, rock
    index 21, light metal (furniture)
    index 22, heavy metal (doors etc)
    index 23, really heavy metal
    index 24, indestructable stone
    index 25, indestructable metal
    */

    {
        // note: must increase; r.c. should block *AP* 7.62mm rounds
        0,   // nothing
        25,  // dry timber; wood wall +1/2
        20,  // furniture wood (thin!) or plywood wall +1/2
        30,  // wood (live); 1.5x timber
        3,   // light vegetation
        10,  // upholstered furniture
        47,  // porcelain
        10,  // cactus, hay, bamboo
        0, 0, 0,
        55,  // stone masonry; 3x timber
        63,  // non-reinforced concrete; 4x timber???
        70,  // reinforced concrete; 6x timber
        85,  // rock? - number invented
        9,   // rubber - tires
        40,  // sand
        1,   // cloth
        40,  // sandbag
        0, 0,
        37,   // light metal (furniture; NB thin!)
        57,   // thicker metal (dumpster)
        85,   // heavy metal (vault doors) - block everything
              // note that vehicle armour will probably end up in here
        127,  // rock indestructable
        127,  // indestructable
        57,   // like 22 but with screen windows
};

void DeleteStructureFromTile(MAP_ELEMENT *pMapElement, struct STRUCTURE *pStructure);

// Function operating on a structure tile
uint8_t FilledTilePositions(DB_STRUCTURE_TILE *pTile) {
  uint8_t ubFilled = 0, ubShapeValue;
  int8_t bLoopX, bLoopY, bLoopZ;

  // Loop through all parts of a structure and add up the number of
  // filled spots
  for (bLoopX = 0; bLoopX < PROFILE_X_SIZE; bLoopX++) {
    for (bLoopY = 0; bLoopY < PROFILE_Y_SIZE; bLoopY++) {
      ubShapeValue = pTile->Shape[bLoopX][bLoopY];
      for (bLoopZ = 0; bLoopZ < PROFILE_Z_SIZE; bLoopZ++) {
        if (ubShapeValue & AtHeight[bLoopZ]) {
          ubFilled++;
        }
      }
    }
  }
  return (ubFilled);
}

//
// Structure database functions
//

void FreeStructureFileRef(
    struct STRUCTURE_FILE_REF
        *pFileRef) {  // Frees all of the memory associated with a file reference,
                      // including the file reference structure itself

  uint16_t usLoop;

  Assert(pFileRef != NULL);
  if (pFileRef->pDBStructureRef != NULL) {
    for (usLoop = 0; usLoop < pFileRef->usNumberOfStructures; usLoop++) {
      if (pFileRef->pDBStructureRef[usLoop].ppTile) {
        MemFree(pFileRef->pDBStructureRef[usLoop].ppTile);
      }
    }
    MemFree(pFileRef->pDBStructureRef);
  }
  if (pFileRef->pubStructureData != NULL) {
    MemFree(pFileRef->pubStructureData);
  }
  if (pFileRef->pAuxData != NULL) {
    MemFree(pFileRef->pAuxData);
    if (pFileRef->pTileLocData != NULL) {
      MemFree(pFileRef->pTileLocData);
    }
  }
  MemFree(pFileRef);
}

void FreeAllStructureFiles(void) {  // Frees all of the structure database!
  struct STRUCTURE_FILE_REF *pFileRef;
  struct STRUCTURE_FILE_REF *pNextRef;

  pFileRef = gpStructureFileRefs;
  while (pFileRef != NULL) {
    pNextRef = pFileRef->pNext;
    FreeStructureFileRef(pFileRef);
    pFileRef = pNextRef;
  }
}

BOOLEAN FreeStructureFile(struct STRUCTURE_FILE_REF *pStructureFile) {
  CHECKF(pStructureFile);

  // unlink the file ref
  if (pStructureFile->pPrev != NULL) {
    pStructureFile->pPrev->pNext = pStructureFile->pNext;
  } else {
    // freeing the head of the list!
    gpStructureFileRefs = pStructureFile->pNext;
  }
  if (pStructureFile->pNext != NULL) {
    pStructureFile->pNext->pPrev = pStructureFile->pPrev;
  }
  if (pStructureFile->pPrev == NULL && pStructureFile->pNext == NULL) {
    // toasting the list!
    gpStructureFileRefs = NULL;
  }
  // and free all the structures used!
  FreeStructureFileRef(pStructureFile);
  return (TRUE);
}

BOOLEAN LoadStructureData(char *szFileName, struct STRUCTURE_FILE_REF *pFileRef,
                          uint32_t *puiStructureDataSize)
// uint8_t **ppubStructureData, uint32_t * puiDataSize, STRUCTURE_FILE_HEADER * pHeader )
{  // Loads a structure file's data as a honking chunk o' memory
  FileID hInput = FILE_ID_ERR;
  STRUCTURE_FILE_HEADER Header;
  uint32_t uiBytesRead;
  uint32_t uiDataSize;
  BOOLEAN fOk;

  CHECKF(szFileName);
  CHECKF(pFileRef);
  hInput = File_OpenForReading(szFileName);
  if (hInput == 0) {
    return (FALSE);
  }
  fOk = File_Read(hInput, &Header, sizeof(STRUCTURE_FILE_HEADER), &uiBytesRead);
  if (!fOk || uiBytesRead != sizeof(STRUCTURE_FILE_HEADER) ||
      strncmp(Header.szId, STRUCTURE_FILE_ID, STRUCTURE_FILE_ID_LEN) != 0 ||
      Header.usNumberOfStructures == 0) {
    File_Close(hInput);
    return (FALSE);
  }
  pFileRef->usNumberOfStructures = Header.usNumberOfStructures;
  if (Header.fFlags & STRUCTURE_FILE_CONTAINS_AUXIMAGEDATA) {
    uiDataSize = sizeof(struct AuxObjectData) * Header.usNumberOfImages;
    pFileRef->pAuxData = (struct AuxObjectData *)MemAlloc(uiDataSize);
    if (pFileRef->pAuxData == NULL) {
      File_Close(hInput);
      return (FALSE);
    }
    fOk = File_Read(hInput, pFileRef->pAuxData, uiDataSize, &uiBytesRead);
    if (!fOk || uiBytesRead != uiDataSize) {
      MemFree(pFileRef->pAuxData);
      File_Close(hInput);
      return (FALSE);
    }
    if (Header.usNumberOfImageTileLocsStored > 0) {
      uiDataSize = sizeof(struct RelTileLoc) * Header.usNumberOfImageTileLocsStored;
      pFileRef->pTileLocData = (struct RelTileLoc *)MemAlloc(uiDataSize);
      if (pFileRef->pTileLocData == NULL) {
        MemFree(pFileRef->pAuxData);
        File_Close(hInput);
        return (FALSE);
      }
      fOk = File_Read(hInput, pFileRef->pTileLocData, uiDataSize, &uiBytesRead);
      if (!fOk || uiBytesRead != uiDataSize) {
        MemFree(pFileRef->pAuxData);
        File_Close(hInput);
        return (FALSE);
      }
    }
  }
  if (Header.fFlags & STRUCTURE_FILE_CONTAINS_STRUCTUREDATA) {
    pFileRef->usNumberOfStructuresStored = Header.usNumberOfStructuresStored;
    uiDataSize = Header.usStructureDataSize;
    // Determine the size of the data, from the header just read,
    // allocate enough memory and read it in
    pFileRef->pubStructureData = (uint8_t *)MemAlloc(uiDataSize);
    if (pFileRef->pubStructureData == NULL) {
      File_Close(hInput);
      if (pFileRef->pAuxData != NULL) {
        MemFree(pFileRef->pAuxData);
        if (pFileRef->pTileLocData != NULL) {
          MemFree(pFileRef->pTileLocData);
        }
      }
      return (FALSE);
    }
    fOk = File_Read(hInput, pFileRef->pubStructureData, uiDataSize, &uiBytesRead);
    if (!fOk || uiBytesRead != uiDataSize) {
      MemFree(pFileRef->pubStructureData);
      if (pFileRef->pAuxData != NULL) {
        MemFree(pFileRef->pAuxData);
        if (pFileRef->pTileLocData != NULL) {
          MemFree(pFileRef->pTileLocData);
        }
      }
      File_Close(hInput);
      return (FALSE);
    }
    *puiStructureDataSize = uiDataSize;
  }
  File_Close(hInput);
  return (TRUE);
}

BOOLEAN CreateFileStructureArrays(
    struct STRUCTURE_FILE_REF *pFileRef,
    uint32_t uiDataSize) {  // Based on a file chunk, creates all the dynamic arrays for the
                            // structure definitions contained within

  uint8_t *pCurrent;
  struct DB_STRUCTURE_REF *pDBStructureRef;
  DB_STRUCTURE_TILE **ppTileArray;
  uint16_t usLoop;
  uint16_t usIndex;
  uint16_t usTileLoop;
  uint32_t uiHitPoints;

  pCurrent = pFileRef->pubStructureData;
  pDBStructureRef = (struct DB_STRUCTURE_REF *)MemAlloc(pFileRef->usNumberOfStructures *
                                                        sizeof(struct DB_STRUCTURE_REF));
  if (pDBStructureRef == NULL) {
    return (FALSE);
  }
  memset(pDBStructureRef, 0, pFileRef->usNumberOfStructures * sizeof(struct DB_STRUCTURE_REF));
  pFileRef->pDBStructureRef = pDBStructureRef;
  for (usLoop = 0; usLoop < pFileRef->usNumberOfStructuresStored; usLoop++) {
    if (pCurrent + sizeof(struct DB_STRUCTURE) >
        pFileRef->pubStructureData + uiDataSize) {  // gone past end of file block?!
      // freeing of memory will occur outside of the function
      return (FALSE);
    }
    usIndex = ((struct DB_STRUCTURE *)pCurrent)->usStructureNumber;
    pDBStructureRef[usIndex].pDBStructure = (struct DB_STRUCTURE *)pCurrent;
    ppTileArray = (DB_STRUCTURE_TILE **)MemAlloc(
        pDBStructureRef[usIndex].pDBStructure->ubNumberOfTiles * sizeof(DB_STRUCTURE_TILE *));
    if (ppTileArray == NULL) {  // freeing of memory will occur outside of the function
      return (FALSE);
    }
    pDBStructureRef[usIndex].ppTile = ppTileArray;
    pCurrent += sizeof(struct DB_STRUCTURE);
    // Set things up to calculate hit points
    uiHitPoints = 0;
    for (usTileLoop = 0; usTileLoop < pDBStructureRef[usIndex].pDBStructure->ubNumberOfTiles;
         usTileLoop++) {
      if (pCurrent + sizeof(struct DB_STRUCTURE) >
          pFileRef->pubStructureData + uiDataSize) {  // gone past end of file block?!
        // freeing of memory will occur outside of the function
        return (FALSE);
      }
      ppTileArray[usTileLoop] = (DB_STRUCTURE_TILE *)pCurrent;
      // set the single-value relative position between this tile and the base tile
      ppTileArray[usTileLoop]->sPosRelToBase = ppTileArray[usTileLoop]->bXPosRelToBase +
                                               ppTileArray[usTileLoop]->bYPosRelToBase * WORLD_COLS;
      uiHitPoints += FilledTilePositions(ppTileArray[usTileLoop]);
      pCurrent += sizeof(DB_STRUCTURE_TILE);
    }
    // scale hit points down to something reasonable...
    uiHitPoints = uiHitPoints * 100 / 255;
    /*
    if (uiHitPoints > 255)
    {
            uiHitPoints = 255;
    }
    */
    pDBStructureRef[usIndex].pDBStructure->ubHitPoints = (uint8_t)uiHitPoints;
    /*
    if (pDBStructureRef[usIndex].pDBStructure->usStructureNumber + 1 ==
    pFileRef->usNumberOfStructures)
    {
            break;
    }
    */
  }
  return (TRUE);
}

struct STRUCTURE_FILE_REF *LoadStructureFile(
    char *szFileName) {  // NB should be passed in expected number
                         // of structures so we can check equality
  uint32_t uiDataSize = 0;
  BOOLEAN fOk;
  struct STRUCTURE_FILE_REF *pFileRef;

  pFileRef = (struct STRUCTURE_FILE_REF *)MemAlloc(sizeof(struct STRUCTURE_FILE_REF));
  if (pFileRef == NULL) {
    return (NULL);
  }
  memset(pFileRef, 0, sizeof(struct STRUCTURE_FILE_REF));
  fOk = LoadStructureData(szFileName, pFileRef, &uiDataSize);
  if (!fOk) {
    MemFree(pFileRef);
    return (NULL);
  }
  if (pFileRef->pubStructureData != NULL) {
    fOk = CreateFileStructureArrays(pFileRef, uiDataSize);
    if (fOk == FALSE) {
      FreeStructureFileRef(pFileRef);
      return (NULL);
    }
  }
  // Add the file reference to the master list, at the head for convenience
  if (gpStructureFileRefs != NULL) {
    gpStructureFileRefs->pPrev = pFileRef;
  }
  pFileRef->pNext = gpStructureFileRefs;
  gpStructureFileRefs = pFileRef;
  return (pFileRef);
}

//
// Structure creation functions
//

struct STRUCTURE *CreateStructureFromDB(
    struct DB_STRUCTURE_REF *pDBStructureRef,
    uint8_t ubTileNum) {  // Creates a struct STRUCTURE struct for one tile of a structure
  struct STRUCTURE *pStructure;
  struct DB_STRUCTURE *pDBStructure;
  DB_STRUCTURE_TILE *pTile;

  // set pointers to the DBStructure and Tile
  CHECKN(pDBStructureRef);
  CHECKN(pDBStructureRef->pDBStructure);
  pDBStructure = pDBStructureRef->pDBStructure;
  CHECKN(pDBStructureRef->ppTile);
  pTile = pDBStructureRef->ppTile[ubTileNum];
  CHECKN(pTile);

  // allocate memory...
  pStructure = (struct STRUCTURE *)MemAlloc(sizeof(struct STRUCTURE));
  CHECKN(pStructure);

  memset(pStructure, 0, sizeof(struct STRUCTURE));

  // setup
  pStructure->fFlags = pDBStructure->fFlags;
  pStructure->pShape = &(pTile->Shape);
  pStructure->pDBStructureRef = pDBStructureRef;
  if (pTile->sPosRelToBase == 0) {  // base tile
    pStructure->fFlags |= STRUCTURE_BASE_TILE;
    pStructure->ubHitPoints = pDBStructure->ubHitPoints;
  }
  if (pDBStructure->ubWallOrientation != NO_ORIENTATION) {
    if (pStructure->fFlags & STRUCTURE_WALL) {
      // for multi-tile walls, which are only the special corner pieces,
      // the non-base tile gets no orientation value because this copy
      // will be skipped
      if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
        pStructure->ubWallOrientation = pDBStructure->ubWallOrientation;
      }
    } else {
      pStructure->ubWallOrientation = pDBStructure->ubWallOrientation;
    }
  }
  pStructure->ubVehicleHitLocation = pTile->ubVehicleHitLocation;
  return (pStructure);
}

BOOLEAN OkayToAddStructureToTile(
    int16_t sBaseGridNo, int16_t sCubeOffset, struct DB_STRUCTURE_REF *pDBStructureRef,
    uint8_t ubTileIndex, int16_t sExclusionID,
    BOOLEAN fIgnorePeople) {  // Verifies whether a structure is blocked from
                              // being added to the map at a particular point
  struct DB_STRUCTURE *pDBStructure;
  DB_STRUCTURE_TILE **ppTile;
  struct STRUCTURE *pExistingStructure;
  struct STRUCTURE *pOtherExistingStructure;
  int8_t bLoop, bLoop2;
  int16_t sGridNo;
  int16_t sOtherGridNo;

  ppTile = pDBStructureRef->ppTile;
  sGridNo = sBaseGridNo + ppTile[ubTileIndex]->sPosRelToBase;
  if (sGridNo < 0 || sGridNo > WORLD_MAX) {
    return (FALSE);
  }

  if (gpWorldLevelData[sBaseGridNo].sHeight != gpWorldLevelData[sGridNo].sHeight) {
    // uneven terrain, one portion on top of cliff and another not! can't add!
    return (FALSE);
  }

  pDBStructure = pDBStructureRef->pDBStructure;
  pExistingStructure = gpWorldLevelData[sGridNo].pStructureHead;

  /*
          // If adding a mobile structure, always allow addition if the mobile structure tile is
     passable if ( (pDBStructure->fFlags & STRUCTURE_MOBILE) && (ppTile[ubTileIndex]->fFlags &
     TILE_PASSABLE) )
          {
                  return( TRUE );
          }
  */

  while (pExistingStructure != NULL) {
    if (sCubeOffset == pExistingStructure->sCubeOffset) {
      // CJC:
      // If adding a mobile structure, allow addition if existing structure is passable
      if ((pDBStructure->fFlags & STRUCTURE_MOBILE) &&
          (pExistingStructure->fFlags & STRUCTURE_PASSABLE)) {
        // Skip!
        pExistingStructure = pExistingStructure->pNext;
        continue;
      }

      if (pDBStructure->fFlags & STRUCTURE_OBSTACLE) {
        // CJC: NB these next two if states are probably COMPLETELY OBSOLETE but I'm leaving
        // them in there for now (no harm done)

        // ATE:
        // ignore this one if it has the same ID num as exclusion
        if (sExclusionID != INVALID_STRUCTURE_ID) {
          if (pExistingStructure->usStructureID == sExclusionID) {
            // Skip!
            pExistingStructure = pExistingStructure->pNext;
            continue;
          }
        }

        if (fIgnorePeople) {
          // If we are a person, skip!
          if (pExistingStructure->usStructureID < TOTAL_SOLDIERS) {
            // Skip!
            pExistingStructure = pExistingStructure->pNext;
            continue;
          }
        }

        // two obstacle structures aren't allowed in the same tile at the same height
        // ATE: There is more sophisticated logic for mobiles, so postpone this check if mobile....
        if ((pExistingStructure->fFlags & STRUCTURE_OBSTACLE) &&
            !(pDBStructure->fFlags & STRUCTURE_MOBILE)) {
          if (pExistingStructure->fFlags & STRUCTURE_PASSABLE &&
              !(pExistingStructure->fFlags & STRUCTURE_MOBILE)) {
            // no mobiles, existing structure is passable
          } else {
            return (FALSE);
          }
        } else if ((pDBStructure->ubNumberOfTiles > 1) &&
                   (pExistingStructure->fFlags & STRUCTURE_WALLSTUFF)) {
          // if not an open door...
          if (!((pExistingStructure->fFlags & STRUCTURE_ANYDOOR) &&
                (pExistingStructure->fFlags & STRUCTURE_OPEN))) {
            // we could be trying to place a multi-tile obstacle on top of a wall; we shouldn't
            // allow this if the structure is going to be on both sides of the wall
            for (bLoop = 1; bLoop < 4; bLoop++) {
              switch (pExistingStructure->ubWallOrientation) {
                case OUTSIDE_TOP_LEFT:
                case INSIDE_TOP_LEFT:
                  sOtherGridNo = NewGridNo(sGridNo, DirectionInc((int8_t)(bLoop + 2)));
                  break;
                case OUTSIDE_TOP_RIGHT:
                case INSIDE_TOP_RIGHT:
                  sOtherGridNo = NewGridNo(sGridNo, DirectionInc(bLoop));
                  break;
                default:
                  // @%?@#%?@%
                  sOtherGridNo = NewGridNo(sGridNo, DirectionInc(SOUTHEAST));
              }
              for (bLoop2 = 0; bLoop2 < pDBStructure->ubNumberOfTiles; bLoop2++) {
                if (sBaseGridNo + ppTile[bLoop2]->sPosRelToBase == sOtherGridNo) {
                  // obstacle will straddle wall!
                  return (FALSE);
                }
              }
            }
          }
        }
      } else if (pDBStructure->fFlags & STRUCTURE_WALLSTUFF) {
        // two walls with the same alignment aren't allowed in the same tile
        if ((pExistingStructure->fFlags & STRUCTURE_WALLSTUFF) &&
            (pDBStructure->ubWallOrientation == pExistingStructure->ubWallOrientation)) {
          return (FALSE);
        } else if (!(pExistingStructure->fFlags & (STRUCTURE_CORPSE | STRUCTURE_PERSON))) {
          // it's possible we're trying to insert this wall on top of a multitile obstacle
          for (bLoop = 1; bLoop < 4; bLoop++) {
            switch (pDBStructure->ubWallOrientation) {
              case OUTSIDE_TOP_LEFT:
              case INSIDE_TOP_LEFT:
                sOtherGridNo = NewGridNo(sGridNo, DirectionInc((int8_t)(bLoop + 2)));
                break;
              case OUTSIDE_TOP_RIGHT:
              case INSIDE_TOP_RIGHT:
                sOtherGridNo = NewGridNo(sGridNo, DirectionInc(bLoop));
                break;
              default:
                // @%?@#%?@%
                sOtherGridNo = NewGridNo(sGridNo, DirectionInc(SOUTHEAST));
                break;
            }
            for (ubTileIndex = 0; ubTileIndex < pDBStructure->ubNumberOfTiles; ubTileIndex++) {
              pOtherExistingStructure =
                  FindStructureByID(sOtherGridNo, pExistingStructure->usStructureID);
              if (pOtherExistingStructure) {
                return (FALSE);
              }
            }
          }
        }
      }

      if (pDBStructure->fFlags & STRUCTURE_MOBILE) {
        // ATE:
        // ignore this one if it has the same ID num as exclusion
        if (sExclusionID != INVALID_STRUCTURE_ID) {
          if (pExistingStructure->usStructureID == sExclusionID) {
            // Skip!
            pExistingStructure = pExistingStructure->pNext;
            continue;
          }
        }

        if (fIgnorePeople) {
          // If we are a person, skip!
          if (pExistingStructure->usStructureID < TOTAL_SOLDIERS) {
            // Skip!
            pExistingStructure = pExistingStructure->pNext;
            continue;
          }
        }

        // ATE: Added check here - UNLESS the part we are trying to add is PASSABLE!
        if (pExistingStructure->fFlags & STRUCTURE_MOBILE &&
            !(pExistingStructure->fFlags & STRUCTURE_PASSABLE) &&
            !(ppTile[ubTileIndex]->fFlags & TILE_PASSABLE)) {
          // don't allow 2 people in the same tile
          return (FALSE);
        }

        // ATE: Another rule: allow PASSABLE *IF* the PASSABLE is *NOT* MOBILE!
        if (!(pExistingStructure->fFlags & STRUCTURE_MOBILE) &&
            (pExistingStructure->fFlags & STRUCTURE_PASSABLE)) {
          // Skip!
          pExistingStructure = pExistingStructure->pNext;
          continue;
        }

        // ATE: Added here - UNLESS this part is PASSABLE....
        // two obstacle structures aren't allowed in the same tile at the same height
        if ((pExistingStructure->fFlags & STRUCTURE_OBSTACLE) &&
            !(ppTile[ubTileIndex]->fFlags & TILE_PASSABLE)) {
          return (FALSE);
        }
      }

      if ((pDBStructure->fFlags & STRUCTURE_OPENABLE)) {
        if (pExistingStructure->fFlags & STRUCTURE_OPENABLE) {
          // don't allow two openable structures in the same tile or things will screw
          // up on an interface level
          return (FALSE);
        }
      }
    }

    pExistingStructure = pExistingStructure->pNext;
  }

  return (TRUE);
}

BOOLEAN InternalOkayToAddStructureToWorld(int16_t sBaseGridNo, int8_t bLevel,
                                          struct DB_STRUCTURE_REF *pDBStructureRef,
                                          int16_t sExclusionID, BOOLEAN fIgnorePeople) {
  uint8_t ubLoop;
  int16_t sCubeOffset;

  CHECKF(pDBStructureRef);
  CHECKF(pDBStructureRef->pDBStructure);
  CHECKF(pDBStructureRef->pDBStructure->ubNumberOfTiles > 0);
  CHECKF(pDBStructureRef->ppTile);

  /*
          if (gpWorldLevelData[sGridNo].sHeight != sBaseTileHeight)
          {
                  // not level ground!
                  return( FALSE );
          }
  */

  for (ubLoop = 0; ubLoop < pDBStructureRef->pDBStructure->ubNumberOfTiles; ubLoop++) {
    if (pDBStructureRef->ppTile[ubLoop]->fFlags & TILE_ON_ROOF) {
      if (bLevel == 0) {
        sCubeOffset = PROFILE_Z_SIZE;
      } else {
        return (FALSE);
      }
    } else {
      sCubeOffset = bLevel * PROFILE_Z_SIZE;
    }
    if (!OkayToAddStructureToTile(sBaseGridNo, sCubeOffset, pDBStructureRef, ubLoop, sExclusionID,
                                  fIgnorePeople)) {
      return (FALSE);
    }
  }
  return (TRUE);
}

BOOLEAN OkayToAddStructureToWorld(int16_t sBaseGridNo, int8_t bLevel,
                                  struct DB_STRUCTURE_REF *pDBStructureRef, int16_t sExclusionID) {
  return (InternalOkayToAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef, sExclusionID,
                                            (BOOLEAN)(sExclusionID == IGNORE_PEOPLE_STRUCTURE_ID)));
}

BOOLEAN AddStructureToTile(
    MAP_ELEMENT *pMapElement, struct STRUCTURE *pStructure,
    uint16_t usStructureID) {  // adds a struct STRUCTURE to a MAP_ELEMENT (adds part
                               // of a structure to a location on the map)
  struct STRUCTURE *pStructureTail;

  CHECKF(pMapElement);
  CHECKF(pStructure);
  pStructureTail = pMapElement->pStructureTail;
  if (pStructureTail == NULL) {  // set the head and tail to the new structure
    pMapElement->pStructureHead = pStructure;
  } else {  // add to the end of the list
    pStructure->pPrev = pStructureTail;
    pStructureTail->pNext = pStructure;
  }
  pMapElement->pStructureTail = pStructure;
  pStructure->usStructureID = usStructureID;
  if (pStructure->fFlags & STRUCTURE_OPENABLE) {
    pMapElement->uiFlags |= MAPELEMENT_INTERACTIVETILE;
  }
  return (TRUE);
}

struct STRUCTURE *InternalAddStructureToWorld(
    int16_t sBaseGridNo, int8_t bLevel, struct DB_STRUCTURE_REF *pDBStructureRef,
    struct LEVELNODE *pLevelNode) {  // Adds a complete structure to the world at a location plus
                                     // all other locations covered by the structure
  int16_t sGridNo;
  struct STRUCTURE **ppStructure;
  struct STRUCTURE *pBaseStructure;
  struct DB_STRUCTURE *pDBStructure;
  DB_STRUCTURE_TILE **ppTile;
  uint8_t ubLoop;
  uint8_t ubLoop2;
  int16_t sBaseTileHeight = -1;
  uint16_t usStructureID;

  CHECKF(pDBStructureRef);
  CHECKF(pLevelNode);

  pDBStructure = pDBStructureRef->pDBStructure;
  CHECKF(pDBStructure);

  ppTile = pDBStructureRef->ppTile;
  CHECKF(ppTile);

  CHECKF(pDBStructure->ubNumberOfTiles > 0);

  // first check to see if the structure will be blocked
  if (!OkayToAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef, INVALID_STRUCTURE_ID)) {
    return (NULL);
  }

  // We go through a definition stage here and a later stage of
  // adding everything to the world so that we don't have to untangle
  // things if we run out of memory.  First we create an array of
  // pointers to point to all of the struct STRUCTURE elements created in
  // the first stage.  This array gets given to the base tile so
  // there is an easy way to remove an entire object from the world quickly

  // NB we add 1 because the 0th element is in fact the reference count!
  ppStructure =
      (struct STRUCTURE **)MemAlloc(pDBStructure->ubNumberOfTiles * sizeof(struct STRUCTURE *));
  CHECKF(ppStructure);
  memset(ppStructure, 0, pDBStructure->ubNumberOfTiles * sizeof(struct STRUCTURE *));

  for (ubLoop = BASE_TILE; ubLoop < pDBStructure->ubNumberOfTiles;
       ubLoop++) {  // for each tile, create the appropriate struct STRUCTURE struct
    ppStructure[ubLoop] = CreateStructureFromDB(pDBStructureRef, ubLoop);
    if (ppStructure[ubLoop] == NULL) {
      // Free allocated memory and abort!
      for (ubLoop2 = 0; ubLoop2 < ubLoop; ubLoop2++) {
        MemFree(ppStructure[ubLoop2]);
      }
      MemFree(ppStructure);
      return (NULL);
    }
    ppStructure[ubLoop]->sGridNo = sBaseGridNo + ppTile[ubLoop]->sPosRelToBase;
    if (ubLoop != BASE_TILE) {
#ifdef JA2EDITOR
      // Kris:
      // Added this undo code if in the editor.
      // It is important to save tiles effected by multitiles.  If the structure placement
      // fails below, it doesn't matter, because it won't hurt the undo code.
      if (gfEditMode) AddToUndoList(ppStructure[ubLoop]->sGridNo);
#endif

      ppStructure[ubLoop]->sBaseGridNo = sBaseGridNo;
    }
    if (ppTile[ubLoop]->fFlags & TILE_ON_ROOF) {
      ppStructure[ubLoop]->sCubeOffset = (bLevel + 1) * PROFILE_Z_SIZE;
    } else {
      ppStructure[ubLoop]->sCubeOffset = bLevel * PROFILE_Z_SIZE;
    }
    if (ppTile[ubLoop]->fFlags & TILE_PASSABLE) {
      ppStructure[ubLoop]->fFlags |= STRUCTURE_PASSABLE;
    }
    if (pLevelNode->uiFlags & LEVELNODE_SOLDIER) {
      // should now be unncessary
      ppStructure[ubLoop]->fFlags |= STRUCTURE_PERSON;
      ppStructure[ubLoop]->fFlags &= ~(STRUCTURE_BLOCKSMOVES);
    } else if (pLevelNode->uiFlags & LEVELNODE_ROTTINGCORPSE ||
               pDBStructure->fFlags & STRUCTURE_CORPSE) {
      ppStructure[ubLoop]->fFlags |= STRUCTURE_CORPSE;
      // attempted check to screen this out for queen creature or vehicle
      if (pDBStructure->ubNumberOfTiles < 10) {
        ppStructure[ubLoop]->fFlags |= STRUCTURE_PASSABLE;
        ppStructure[ubLoop]->fFlags &= ~(STRUCTURE_BLOCKSMOVES);
      } else {
        // make sure not transparent
        ppStructure[ubLoop]->fFlags &= ~(STRUCTURE_TRANSPARENT);
      }
    }
  }

  if (pLevelNode->uiFlags & LEVELNODE_SOLDIER) {
    // use the merc's ID as the structure ID for his/her structure
    usStructureID = pLevelNode->pSoldier->ubID;
  } else if (pLevelNode->uiFlags & LEVELNODE_ROTTINGCORPSE) {
    // ATE: Offset IDs so they don't collide with soldiers
    usStructureID = (uint16_t)(TOTAL_SOLDIERS + pLevelNode->pAniTile->uiUserData);
  } else {
    gusNextAvailableStructureID++;
    if (gusNextAvailableStructureID == 0) {
      // skip past the #s for soldiers' structures and the invalid structure #
      gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;
    }
    usStructureID = gusNextAvailableStructureID;
  }
  // now add all these to the world!
  for (ubLoop = BASE_TILE; ubLoop < pDBStructure->ubNumberOfTiles; ubLoop++) {
    sGridNo = ppStructure[ubLoop]->sGridNo;
    if (ubLoop == BASE_TILE) {
      sBaseTileHeight = gpWorldLevelData[sGridNo].sHeight;
    } else {
      if (gpWorldLevelData[sGridNo].sHeight != sBaseTileHeight) {
        // not level ground! abort!
        for (ubLoop2 = BASE_TILE; ubLoop2 < ubLoop; ubLoop2++) {
          DeleteStructureFromTile(&(gpWorldLevelData[ppStructure[ubLoop2]->sGridNo]),
                                  ppStructure[ubLoop2]);
        }
        MemFree(ppStructure);
        return (NULL);
      }
    }
    if (AddStructureToTile(&(gpWorldLevelData[sGridNo]), ppStructure[ubLoop], usStructureID) ==
        FALSE) {
      // error! abort!
      for (ubLoop2 = BASE_TILE; ubLoop2 < ubLoop; ubLoop2++) {
        DeleteStructureFromTile(&(gpWorldLevelData[ppStructure[ubLoop2]->sGridNo]),
                                ppStructure[ubLoop2]);
      }
      MemFree(ppStructure);
      return (NULL);
    }
  }

  pBaseStructure = ppStructure[BASE_TILE];
  pLevelNode->pStructureData = pBaseStructure;

  MemFree(ppStructure);
  // And we're done! return a pointer to the base structure!

  return (pBaseStructure);
}

BOOLEAN AddStructureToWorld(int16_t sBaseGridNo, int8_t bLevel,
                            struct DB_STRUCTURE_REF *pDBStructureRef, void *pLevelN) {
  struct STRUCTURE *pStructure;

  pStructure = InternalAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef,
                                           (struct LEVELNODE *)pLevelN);
  if (pStructure == NULL) {
    return (FALSE);
  }
  return (TRUE);
}

//
// Structure deletion functions
//

void DeleteStructureFromTile(
    MAP_ELEMENT *pMapElement,
    struct STRUCTURE *
        pStructure) {  // removes a struct STRUCTURE element at a particular location from the world
  // put location pointer in tile
  if (pMapElement->pStructureHead == pStructure) {
    if (pMapElement->pStructureTail == pStructure) {
      // only element in the list!
      pMapElement->pStructureHead = NULL;
      pMapElement->pStructureTail = NULL;
    } else {
      // first element in the list of 2+ members
      pMapElement->pStructureHead = pStructure->pNext;
    }
  } else if (pMapElement->pStructureTail == pStructure) {
    // last element in the list
    pStructure->pPrev->pNext = NULL;
    pMapElement->pStructureTail = pStructure->pPrev;
  } else {
    // second or later element in the list; it's guaranteed that there is a
    // previous element but not necessary a next
    pStructure->pPrev->pNext = pStructure->pNext;
    if (pStructure->pNext != NULL) {
      pStructure->pNext->pPrev = pStructure->pPrev;
    }
  }
  if (pStructure->fFlags &
      STRUCTURE_OPENABLE) {  // only one allowed in a tile, so we are safe to do this...
    pMapElement->uiFlags &= (~MAPELEMENT_INTERACTIVETILE);
  }
  MemFree(pStructure);
}

BOOLEAN DeleteStructureFromWorld(
    struct STRUCTURE *pStructure) {  // removes all of the struct STRUCTURE elements
                                     // for a structure from the world
  struct STRUCTURE *pBaseStructure;
  DB_STRUCTURE_TILE **ppTile;
  struct STRUCTURE *pCurrent;
  uint8_t ubLoop, ubLoop2;
  uint8_t ubNumberOfTiles;
  int16_t sBaseGridNo, sGridNo;
  uint16_t usStructureID;
  BOOLEAN fRecompileMPs;
  BOOLEAN fRecompileExtraRadius;  // for doors... yuck
  int16_t sCheckGridNo;

  CHECKF(pStructure);

  pBaseStructure = FindBaseStructure(pStructure);
  CHECKF(pBaseStructure);

  usStructureID = pBaseStructure->usStructureID;
  fRecompileMPs =
      ((gsRecompileAreaLeft != 0) && !((pBaseStructure->fFlags & STRUCTURE_MOBILE) != 0));
  if (fRecompileMPs) {
    fRecompileExtraRadius = ((pBaseStructure->fFlags & STRUCTURE_WALLSTUFF) != 0);
  } else {
    fRecompileExtraRadius = FALSE;
  }

  ppTile = pBaseStructure->pDBStructureRef->ppTile;
  sBaseGridNo = pBaseStructure->sGridNo;
  ubNumberOfTiles = pBaseStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles;
  // Free all the tiles
  for (ubLoop = BASE_TILE; ubLoop < ubNumberOfTiles; ubLoop++) {
    sGridNo = sBaseGridNo + ppTile[ubLoop]->sPosRelToBase;
    // there might be two structures in this tile, one on each level, but we just want to
    // delete one on each pass
    pCurrent = FindStructureByID(sGridNo, usStructureID);
    if (pCurrent) {
      DeleteStructureFromTile(&gpWorldLevelData[sGridNo], pCurrent);
    }

    if (!gfEditMode && (fRecompileMPs)) {
      if (fRecompileMPs) {
        AddTileToRecompileArea(sGridNo);
        if (fRecompileExtraRadius) {
          // add adjacent tiles too
          for (ubLoop2 = 0; ubLoop2 < NUM_WORLD_DIRECTIONS; ubLoop2++) {
            sCheckGridNo = NewGridNo(sGridNo, DirectionInc(ubLoop2));
            if (sCheckGridNo != sGridNo) {
              AddTileToRecompileArea(sCheckGridNo);
            }
          }
        }
      }
    }
  }
  return (TRUE);
}

struct STRUCTURE *InternalSwapStructureForPartner(int16_t sGridNo, struct STRUCTURE *pStructure,
                                                  BOOLEAN fFlipSwitches,
                                                  BOOLEAN fStoreInMap) {  // switch structure
  struct LEVELNODE *pLevelNode;
  struct LEVELNODE *pShadowNode;
  struct STRUCTURE *pBaseStructure;
  struct STRUCTURE *pNewBaseStructure;
  struct DB_STRUCTURE_REF *pPartnerDBStructure;
  BOOLEAN fDoor;

  int8_t bDelta;
  uint8_t ubHitPoints;
  int16_t sCubeOffset;

  if (pStructure == NULL) {
    return (NULL);
  }
  pBaseStructure = FindBaseStructure(pStructure);
  CHECKF(pBaseStructure);
  if ((pBaseStructure->pDBStructureRef->pDBStructure)->bPartnerDelta == NO_PARTNER_STRUCTURE) {
    return (NULL);
  }
  fDoor = ((pBaseStructure->fFlags & STRUCTURE_ANYDOOR) > 0);
  pLevelNode = FindLevelNodeBasedOnStructure(pBaseStructure->sGridNo, pBaseStructure);
  if (pLevelNode == NULL) {
    return (NULL);
  }
  pShadowNode = FindShadow(pBaseStructure->sGridNo, pLevelNode->usIndex);

  // record values
  bDelta = pBaseStructure->pDBStructureRef->pDBStructure->bPartnerDelta;
  pPartnerDBStructure = pBaseStructure->pDBStructureRef + bDelta;
  sGridNo = pBaseStructure->sGridNo;
  ubHitPoints = pBaseStructure->ubHitPoints;
  sCubeOffset = pBaseStructure->sCubeOffset;
  // delete the old structure and add the new one
  if (DeleteStructureFromWorld(pBaseStructure) == FALSE) {
    return (NULL);
  }
  pNewBaseStructure = InternalAddStructureToWorld(sGridNo, (int8_t)(sCubeOffset / PROFILE_Z_SIZE),
                                                  pPartnerDBStructure, pLevelNode);
  if (pNewBaseStructure == NULL) {
    return (NULL);
  }
  // set values in the new structure
  pNewBaseStructure->ubHitPoints = ubHitPoints;
  if (!fDoor) {  // swap the graphics

    // store removal of previous if necessary
    if (fStoreInMap) {
      ApplyMapChangesToMapTempFile(TRUE);
      RemoveStructFromMapTempFile(sGridNo, pLevelNode->usIndex);
    }

    pLevelNode->usIndex += bDelta;

    // store removal of new one if necessary
    if (fStoreInMap) {
      AddStructToMapTempFile(sGridNo, pLevelNode->usIndex);
      ApplyMapChangesToMapTempFile(FALSE);
    }

    if (pShadowNode != NULL) {
      pShadowNode->usIndex += bDelta;
    }
  }

  // if ( (pNewBaseStructure->fFlags & STRUCTURE_SWITCH) && (pNewBaseStructure->fFlags &
  // STRUCTURE_OPEN) )
  if (0 /*fFlipSwitches*/) {
    if (pNewBaseStructure->fFlags & STRUCTURE_SWITCH) {
      // just turned a switch on!
      ActivateSwitchInGridNo(NOBODY, sGridNo);
    }
  }
  return (pNewBaseStructure);
}

struct STRUCTURE *SwapStructureForPartner(int16_t sGridNo, struct STRUCTURE *pStructure) {
  return (InternalSwapStructureForPartner(sGridNo, pStructure, TRUE, FALSE));
}

struct STRUCTURE *SwapStructureForPartnerWithoutTriggeringSwitches(int16_t sGridNo,
                                                                   struct STRUCTURE *pStructure) {
  return (InternalSwapStructureForPartner(sGridNo, pStructure, FALSE, FALSE));
}

struct STRUCTURE *SwapStructureForPartnerAndStoreChangeInMap(int16_t sGridNo,
                                                             struct STRUCTURE *pStructure) {
  return (InternalSwapStructureForPartner(sGridNo, pStructure, TRUE, TRUE));
}

struct STRUCTURE *FindStructure(
    int16_t sGridNo,
    uint32_t fFlags) {  // finds a structure that matches any of the given flags
  struct STRUCTURE *pCurrent;

  pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
  while (pCurrent != NULL) {
    if ((pCurrent->fFlags & fFlags) != 0) {
      return (pCurrent);
    }
    pCurrent = pCurrent->pNext;
  }
  return (NULL);
}

struct STRUCTURE *FindNextStructure(struct STRUCTURE *pStructure, uint32_t fFlags) {
  struct STRUCTURE *pCurrent;

  CHECKF(pStructure);
  pCurrent = pStructure->pNext;
  while (pCurrent != NULL) {
    if ((pCurrent->fFlags & fFlags) != 0) {
      return (pCurrent);
    }
    pCurrent = pCurrent->pNext;
  }
  return (NULL);
}

struct STRUCTURE *FindStructureByID(
    int16_t sGridNo,
    uint16_t usStructureID) {  // finds a structure that matches any of the given flags
  struct STRUCTURE *pCurrent;

  pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
  while (pCurrent != NULL) {
    if (pCurrent->usStructureID == usStructureID) {
      return (pCurrent);
    }
    pCurrent = pCurrent->pNext;
  }
  return (NULL);
}

struct STRUCTURE *FindBaseStructure(
    struct STRUCTURE *pStructure) {  // finds the base structure for any structure
  CHECKF(pStructure);
  if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
    return (pStructure);
  }
  return (FindStructureByID(pStructure->sBaseGridNo, pStructure->usStructureID));
}

struct STRUCTURE *FindNonBaseStructure(
    int16_t sGridNo, struct STRUCTURE *pStructure) {  // finds a non-base structure in a location
  CHECKF(pStructure);
  if (!(pStructure->fFlags & STRUCTURE_BASE_TILE)) {  // error!
    return (NULL);
  }

  return (FindStructureByID(sGridNo, pStructure->usStructureID));
}

int16_t GetBaseTile(struct STRUCTURE *pStructure) {
  if (pStructure == NULL) {
    return (-1);
  }
  if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
    return (pStructure->sGridNo);
  } else {
    return (pStructure->sBaseGridNo);
  }
}

int8_t StructureHeight(struct STRUCTURE *pStructure) {  // return the height of an object from 1-4
  uint8_t ubLoopX, ubLoopY;
  PROFILE *pShape;
  uint8_t ubShapeValue;
  int8_t bLoopZ;
  int8_t bGreatestHeight = -1;

  if (pStructure == NULL || pStructure->pShape == NULL) {
    return (0);
  }

  if (pStructure->ubStructureHeight != 0) {
    return (pStructure->ubStructureHeight);
  }

  pShape = pStructure->pShape;

  // loop horizontally on the X and Y planes
  for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++) {
    for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++) {
      ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
      // loop DOWN vertically so that we find the tallest point first
      // and don't need to check any below it
      for (bLoopZ = PROFILE_Z_SIZE - 1; bLoopZ > bGreatestHeight; bLoopZ--) {
        if (ubShapeValue & AtHeight[bLoopZ]) {
          bGreatestHeight = bLoopZ;
          if (bGreatestHeight == PROFILE_Z_SIZE - 1) {
            // store height
            pStructure->ubStructureHeight = bGreatestHeight + 1;
            return (bGreatestHeight + 1);
          }
          break;
        }
      }
    }
  }
  // store height
  pStructure->ubStructureHeight = bGreatestHeight + 1;
  return (bGreatestHeight + 1);
}

int8_t GetTallestStructureHeight(int16_t sGridNo, BOOLEAN fOnRoof) {
  struct STRUCTURE *pCurrent;
  int8_t iHeight;
  int8_t iTallest = 0;
  int16_t sDesiredHeight;

  if (fOnRoof) {
    sDesiredHeight = STRUCTURE_ON_ROOF;
  } else {
    sDesiredHeight = STRUCTURE_ON_GROUND;
  }
  pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
  while (pCurrent != NULL) {
    if (pCurrent->sCubeOffset == sDesiredHeight) {
      iHeight = StructureHeight(pCurrent);
      if (iHeight > iTallest) {
        iTallest = iHeight;
      }
    }
    pCurrent = pCurrent->pNext;
  }
  return (iTallest);
}

int8_t GetStructureTargetHeight(int16_t sGridNo, BOOLEAN fOnRoof) {
  struct STRUCTURE *pCurrent;
  int8_t iHeight;
  int8_t iTallest = 0;
  int16_t sDesiredHeight;

  if (fOnRoof) {
    sDesiredHeight = STRUCTURE_ON_ROOF;
  } else {
    sDesiredHeight = STRUCTURE_ON_GROUND;
  }

  // prioritize openable structures and doors
  pCurrent = FindStructure(sGridNo, (STRUCTURE_DOOR | STRUCTURE_OPENABLE));
  if (pCurrent) {
    // use this structure
    if (pCurrent->fFlags & STRUCTURE_DOOR) {
      iTallest = 3;  // don't aim at the very top of the door
    } else {
      iTallest = StructureHeight(pCurrent);
    }
  } else {
    pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
    while (pCurrent != NULL) {
      if (pCurrent->sCubeOffset == sDesiredHeight) {
        iHeight = StructureHeight(pCurrent);

        if (iHeight > iTallest) {
          iTallest = iHeight;
        }
      }
      pCurrent = pCurrent->pNext;
    }
  }
  return (iTallest);
}

int8_t StructureBottomLevel(
    struct STRUCTURE *pStructure) {  // return the bottom level of an object, from 1-4
  uint8_t ubLoopX, ubLoopY;
  PROFILE *pShape;
  uint8_t ubShapeValue;
  int8_t bLoopZ;
  int8_t bLowestHeight = PROFILE_Z_SIZE;

  if (pStructure == NULL || pStructure->pShape == NULL) {
    return (0);
  }
  pShape = pStructure->pShape;

  // loop horizontally on the X and Y planes
  for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++) {
    for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++) {
      ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
      // loop DOWN vertically so that we find the tallest point first
      // and don't need to check any below it
      for (bLoopZ = 0; bLoopZ < bLowestHeight; bLoopZ++) {
        if (ubShapeValue & AtHeight[bLoopZ]) {
          bLowestHeight = bLoopZ;
          if (bLowestHeight == 0) {
            return (1);
          }
          break;
        }
      }
    }
  }
  return (bLowestHeight + 1);
}

BOOLEAN StructureDensity(struct STRUCTURE *pStructure, uint8_t *pubLevel0, uint8_t *pubLevel1,
                         uint8_t *pubLevel2, uint8_t *pubLevel3) {
  uint8_t ubLoopX, ubLoopY;
  uint8_t ubShapeValue;
  PROFILE *pShape;

  CHECKF(pStructure);
  CHECKF(pubLevel0);
  CHECKF(pubLevel1);
  CHECKF(pubLevel2);
  CHECKF(pubLevel3);
  *pubLevel0 = 0;
  *pubLevel1 = 0;
  *pubLevel2 = 0;
  *pubLevel3 = 0;

  pShape = pStructure->pShape;

  for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++) {
    for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++) {
      ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
      if (ubShapeValue & AtHeight[0]) {
        (*pubLevel0)++;
      }
      if (ubShapeValue & AtHeight[1]) {
        (*pubLevel1)++;
      }
      if (ubShapeValue & AtHeight[2]) {
        (*pubLevel2)++;
      }
      if (ubShapeValue & AtHeight[3]) {
        (*pubLevel3)++;
      }
    }
  }
  // convert values to percentages!
  *pubLevel0 *= 4;
  *pubLevel1 *= 4;
  *pubLevel2 *= 4;
  *pubLevel3 *= 4;
  return (TRUE);
}

BOOLEAN DamageStructure(
    struct STRUCTURE *pStructure, uint8_t ubDamage, uint8_t ubReason, int16_t sGridNo, int16_t sX,
    int16_t sY,
    uint8_t ubOwner) {  // do damage to a structure; returns TRUE if the structure should be removed

  struct STRUCTURE *pBase;
  uint8_t ubArmour;
  // struct LEVELNODE			*pNode;

  CHECKF(pStructure);
  if (pStructure->fFlags & STRUCTURE_PERSON || pStructure->fFlags & STRUCTURE_CORPSE) {
    // don't hurt this structure, it's used for hit detection only!
    return (FALSE);
  }

  if ((pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_INDESTRUCTABLE_METAL) ||
      (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_INDESTRUCTABLE_STONE)) {
    return (FALSE);
  }

  // Account for armour!
  if (ubReason == STRUCTURE_DAMAGE_EXPLOSION) {
    if (pStructure->fFlags & STRUCTURE_EXPLOSIVE) {
      ubArmour = gubMaterialArmour[pStructure->pDBStructureRef->pDBStructure->ubArmour] / 3;
    } else {
      ubArmour = gubMaterialArmour[pStructure->pDBStructureRef->pDBStructure->ubArmour] / 2;
    }

    if (ubArmour > ubDamage) {
      // didn't even scratch the paint
      return (FALSE);
    } else {
      // did some damage to the structure
      ubDamage -= ubArmour;
    }
  } else {
    ubDamage = 0;
  }

  // OK, Let's check our reason
  if (ubReason == STRUCTURE_DAMAGE_GUNFIRE) {
    // If here, we have penetrated, check flags
    // Are we an explodable structure?
    if ((pStructure->fFlags & STRUCTURE_EXPLOSIVE) && Random(2)) {
      // Remove struct!
      pBase = FindBaseStructure(pStructure);

      // ATE: Set hit points to zero....
      pBase->ubHitPoints = 0;

      // Get struct LEVELNODE for struct and remove!
      // pNode = FindLevelNodeBasedOnStructure( pBase->sGridNo, pBase );

      // Set a flag indicating that the following changes are to go the the maps temp file
      // ApplyMapChangesToMapTempFile( TRUE );
      // Remove!
      // RemoveStructFromLevelNode( pBase->sGridNo, pNode );
      // ApplyMapChangesToMapTempFile( FALSE );

      // Generate an explosion here!
      IgniteExplosion(ubOwner, sX, sY, 0, sGridNo, STRUCTURE_IGNITE, 0);

      // ATE: Return false here, as we are dealing with deleting the graphic here...
      return (FALSE);
    }

    // Make hit sound....
    if (pStructure->fFlags & STRUCTURE_CAVEWALL) {
      PlayJA2Sample(S_VEG_IMPACT1, RATE_11025, SoundVolume(HIGHVOLUME, sGridNo), 1,
                    SoundDir(sGridNo));
    } else {
      if (guiMaterialHitSound[pStructure->pDBStructureRef->pDBStructure->ubArmour] != -1) {
        PlayJA2Sample(guiMaterialHitSound[pStructure->pDBStructureRef->pDBStructure->ubArmour],
                      RATE_11025, SoundVolume(HIGHVOLUME, sGridNo), 1, SoundDir(sGridNo));
      }
    }
    // Don't update damage HPs....
    return (TRUE);
  }

  // OK, LOOK FOR A SAM SITE, UPDATE....
  UpdateAndDamageSAMIfFound((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ, sGridNo,
                            ubDamage);

  // find the base so we can reduce the hit points!
  pBase = FindBaseStructure(pStructure);
  CHECKF(pBase);
  if (pBase->ubHitPoints <= ubDamage) {
    // boom! structure destroyed!
    return (TRUE);
  } else {
    pBase->ubHitPoints -= ubDamage;

    // Since the structure is being damaged, set the map element that a structure is damaged
    gpWorldLevelData[sGridNo].uiFlags |= MAPELEMENT_STRUCTURE_DAMAGED;

    // We are a little damaged....
    return (2);
  }
}

#define LINE_HEIGHT 20
void DebugStructurePage1(void) {
  struct STRUCTURE *pStructure;
  struct STRUCTURE *pBase;
  // struct LEVELNODE *		pLand;
  int16_t sGridNo, sDesiredLevel;
  uint8_t bHeight, bDens0, bDens1, bDens2, bDens3;
  int8_t bStructures;

  static wchar_t WallOrientationString[5][15] = {L"None", L"Inside left", L"Inside right",
                                                 L"Outside left", L"Outside right"};

  SetFont(LARGEFONT1);
  gprintf(0, 0, L"DEBUG STRUCTURES PAGE 1 OF 1");
  if (GetMouseMapPos(&sGridNo) == FALSE) {
    return;
    // gprintf( 0, LINE_HEIGHT * 1, L"No structure selected" );
  }

  if (gsInterfaceLevel == I_GROUND_LEVEL) {
    sDesiredLevel = STRUCTURE_ON_GROUND;
  } else {
    sDesiredLevel = STRUCTURE_ON_ROOF;
  }

  gprintf(320, 0, L"Building %d", gubBuildingInfo[sGridNo]);
  /*
  pLand = gpWorldLevelData[sGridNo].pLandHead;
  gprintf( 320, 0, L"Fake light %d", pLand->ubFakeShadeLevel );
  gprintf( 320, LINE_HEIGHT, L"Real light: ground %d roof %d", LightTrueLevel( sGridNo, 0 ),
  LightTrueLevel( sGridNo, 1 ) );
  */

  pStructure = gpWorldLevelData[sGridNo].pStructureHead;
  while (pStructure != NULL) {
    if (pStructure->sCubeOffset == sDesiredLevel) {
      break;
    }
    pStructure = pStructure->pNext;
  }

  if (pStructure != NULL) {
    if (pStructure->fFlags & STRUCTURE_GENERIC) {
      gprintf(0, LINE_HEIGHT * 1, L"Generic structure %x #%d", pStructure->fFlags,
              pStructure->pDBStructureRef->pDBStructure->usStructureNumber);
    } else if (pStructure->fFlags & STRUCTURE_TREE) {
      gprintf(0, LINE_HEIGHT * 1, L"Tree");
    } else if (pStructure->fFlags & STRUCTURE_WALL) {
      gprintf(0, LINE_HEIGHT * 1, L"Wall with orientation %s",
              WallOrientationString[pStructure->ubWallOrientation]);
    } else if (pStructure->fFlags & STRUCTURE_WALLNWINDOW) {
      gprintf(0, LINE_HEIGHT * 1, L"Wall with window");
    } else if (pStructure->fFlags & STRUCTURE_VEHICLE) {
      gprintf(0, LINE_HEIGHT * 1, L"Vehicle %d",
              pStructure->pDBStructureRef->pDBStructure->usStructureNumber);
    } else if (pStructure->fFlags & STRUCTURE_NORMAL_ROOF) {
      gprintf(0, LINE_HEIGHT * 1, L"Roof");
    } else if (pStructure->fFlags & STRUCTURE_SLANTED_ROOF) {
      gprintf(0, LINE_HEIGHT * 1, L"Slanted roof");
    } else if (pStructure->fFlags & STRUCTURE_DOOR) {
      gprintf(0, LINE_HEIGHT * 1, L"Door with orientation %s",
              WallOrientationString[pStructure->ubWallOrientation]);
    } else if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR) {
      gprintf(0, LINE_HEIGHT * 1, L"%s sliding door with orientation %s",
              (pStructure->fFlags & STRUCTURE_OPEN) ? L"Open" : L"Closed",
              WallOrientationString[pStructure->ubWallOrientation]);
    } else if (pStructure->fFlags & STRUCTURE_DDOOR_LEFT) {
      gprintf(0, LINE_HEIGHT * 1, L"DDoorLft with orientation %s",
              WallOrientationString[pStructure->ubWallOrientation]);
    } else if (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT) {
      gprintf(0, LINE_HEIGHT * 1, L"DDoorRt with orientation %s",
              WallOrientationString[pStructure->ubWallOrientation]);
    } else {
      gprintf(0, LINE_HEIGHT * 1, L"UNKNOWN struct STRUCTURE! (%x)", pStructure->fFlags);
    }
    bHeight = StructureHeight(pStructure);
    pBase = FindBaseStructure(pStructure);
    gprintf(0, LINE_HEIGHT * 2, L"Structure height %d, cube offset %d, armour %d, HP %d", bHeight,
            pStructure->sCubeOffset,
            gubMaterialArmour[pStructure->pDBStructureRef->pDBStructure->ubArmour],
            pBase->ubHitPoints);
    if (StructureDensity(pStructure, &bDens0, &bDens1, &bDens2, &bDens3) == TRUE) {
      gprintf(0, LINE_HEIGHT * 3, L"Structure fill %d%%/%d%%/%d%%/%d%% density %d", bDens0, bDens1,
              bDens2, bDens3, pStructure->pDBStructureRef->pDBStructure->ubDensity);
    }

#ifndef LOS_DEBUG
    gprintf(0, LINE_HEIGHT * 4, L"Structure ID %d", pStructure->usStructureID);
#endif

    pStructure = gpWorldLevelData[sGridNo].pStructureHead;
    for (bStructures = 0; pStructure != NULL; pStructure = pStructure->pNext) {
      bStructures++;
    }
    gprintf(0, LINE_HEIGHT * 12, L"Number of structures = %d", bStructures);
  }
#ifdef LOS_DEBUG
  if (gLOSTestResults.fLOSTestPerformed) {
    gprintf(0, LINE_HEIGHT * 4, L"LOS from (%7d,%7d,%7d)", gLOSTestResults.iStartX,
            gLOSTestResults.iStartY, gLOSTestResults.iStartZ);
    gprintf(0, LINE_HEIGHT * 5, L"to (%7d,%7d,%7d)", gLOSTestResults.iEndX, gLOSTestResults.iEndY,
            gLOSTestResults.iEndZ);
    if (gLOSTestResults.fOutOfRange) {
      gprintf(0, LINE_HEIGHT * 6, L"is out of range");
    } else if (gLOSTestResults.fLOSClear) {
      gprintf(0, LINE_HEIGHT * 6, L"is clear!");
    } else {
      gprintf(0, LINE_HEIGHT * 6, L"is blocked at (%7d,%7d,%7d)!", gLOSTestResults.iStoppedX,
              gLOSTestResults.iStoppedY, gLOSTestResults.iStoppedZ);
      gprintf(0, LINE_HEIGHT * 10, L"Blocked at cube level %d", gLOSTestResults.iCurrCubesZ);
    }
    gprintf(0, LINE_HEIGHT * 7, L"Passed through %d tree bits!", gLOSTestResults.ubTreeSpotsHit);
    gprintf(0, LINE_HEIGHT * 8, L"Maximum range was %7d", gLOSTestResults.iMaxDistance);
    gprintf(0, LINE_HEIGHT * 9, L"actual range was %7d", gLOSTestResults.iDistance);
    if (gLOSTestResults.ubChanceToGetThrough <= 100) {
      gprintf(0, LINE_HEIGHT * 11, L"Chance to get through was %d",
              gLOSTestResults.ubChanceToGetThrough);
    }
  }
#endif
  gprintf(0, LINE_HEIGHT * 13, L"N %d NE %d E %d SE %d",
          gubWorldMovementCosts[sGridNo][NORTH][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][NORTHEAST][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][EAST][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][SOUTHEAST][gsInterfaceLevel]);
  gprintf(0, LINE_HEIGHT * 14, L"S %d SW %d W %d NW %d",
          gubWorldMovementCosts[sGridNo][SOUTH][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][SOUTHWEST][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][WEST][gsInterfaceLevel],
          gubWorldMovementCosts[sGridNo][NORTHWEST][gsInterfaceLevel]);
  gprintf(0, LINE_HEIGHT * 15, L"Ground smell %d strength %d",
          SMELL_TYPE(gpWorldLevelData[sGridNo].ubSmellInfo),
          SMELL_STRENGTH(gpWorldLevelData[sGridNo].ubSmellInfo));

#ifdef COUNT_PATHS
  if (guiTotalPathChecks > 0) {
    gprintf(0, LINE_HEIGHT * 16, L"Total %ld, %%succ %3ld | %%failed %3ld | %%unsucc %3ld",
            guiTotalPathChecks, 100 * guiSuccessfulPathChecks / guiTotalPathChecks,
            100 * guiFailedPathChecks / guiTotalPathChecks,
            100 * guiUnsuccessfulPathChecks / guiTotalPathChecks);
  }
#else
  gprintf(0, LINE_HEIGHT * 16, L"Adj soldiers %d", gpWorldLevelData[sGridNo].ubAdjacentSoldierCnt);
#endif
}

BOOLEAN AddZStripInfoToVObject(struct VObject *hVObject,
                               struct STRUCTURE_FILE_REF *pStructureFileRef, BOOLEAN fFromAnimation,
                               int16_t sSTIStartIndex) {
  uint32_t uiLoop;
  uint8_t ubLoop2;
  uint8_t ubNumIncreasing = 0;
  uint8_t ubNumStable = 0;
  uint8_t ubNumDecreasing = 0;
  BOOLEAN fFound = FALSE;
  ZStripInfo *pCurr;
  int16_t sLeftHalfWidth;
  int16_t sRightHalfWidth;
  int16_t sOffsetX;
  int16_t sOffsetY;
  uint16_t usWidth;
  struct DB_STRUCTURE_REF *pDBStructureRef;
  struct DB_STRUCTURE *pDBStructure = NULL;
  int16_t sSTIStep = 0;
  int16_t sStructIndex = 0;
  int16_t sNext;
  uint32_t uiDestVoIndex;
  BOOLEAN fCopyIntoVo;
  BOOLEAN fFirstTime;

  if (pStructureFileRef->usNumberOfStructuresStored == 0) {
    return (TRUE);
  }
  for (uiLoop = 0; uiLoop < pStructureFileRef->usNumberOfStructures; uiLoop++) {
    pDBStructureRef = &(pStructureFileRef->pDBStructureRef[uiLoop]);
    pDBStructure = pDBStructureRef->pDBStructure;
    // if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1 && !(pDBStructure->fFlags &
    // STRUCTURE_WALLSTUFF) )
    if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1) {
      for (ubLoop2 = 1; ubLoop2 < pDBStructure->ubNumberOfTiles; ubLoop2++) {
        if (pDBStructureRef->ppTile[ubLoop2]->sPosRelToBase != 0) {
          // spans multiple tiles! (could be two levels high in one tile)
          fFound = TRUE;
          break;
        }
      }
    }
  }

  // ATE: Make all corpses use z-strip info..
  if (pDBStructure != NULL && pDBStructure->fFlags & STRUCTURE_CORPSE) {
    fFound = TRUE;
  }

  if (!fFound) {
    // no multi-tile images in this vobject; that's okay... return!
    return (TRUE);
  }
  hVObject->ppZStripInfo =
      (ZStripInfo **)MemAlloc(sizeof(ZStripInfo *) * hVObject->usNumberOfObjects);
  if (hVObject->ppZStripInfo == NULL) {
    return (FALSE);
  }
  memset(hVObject->ppZStripInfo, 0, sizeof(ZStripInfo *) * hVObject->usNumberOfObjects);

  if (fFromAnimation) {
    // Determine step index for STI
    if (sSTIStartIndex == -1) {
      // one-direction only for this anim structure
      sSTIStep = hVObject->usNumberOfObjects;
      sSTIStartIndex = 0;
    } else {
      sSTIStep = (hVObject->usNumberOfObjects / pStructureFileRef->usNumberOfStructures);
    }
  } else {
    sSTIStep = 1;
  }

  sStructIndex = 0;
  sNext = sSTIStartIndex + sSTIStep;
  fFirstTime = TRUE;

  for (uiLoop = (uint8_t)sSTIStartIndex; uiLoop < hVObject->usNumberOfObjects; uiLoop++) {
    // Defualt to true
    fCopyIntoVo = TRUE;

    // Increment struct index....
    if (uiLoop == (uint32_t)sNext) {
      sNext = (uint16_t)(uiLoop + sSTIStep);
      sStructIndex++;
    } else {
      if (fFirstTime) {
        fFirstTime = FALSE;
      } else {
        fCopyIntoVo = FALSE;
      }
    }

    if (fFromAnimation) {
      uiDestVoIndex = sStructIndex;
    } else {
      uiDestVoIndex = uiLoop;
    }

    if (fCopyIntoVo && sStructIndex < pStructureFileRef->usNumberOfStructures) {
      pDBStructure = pStructureFileRef->pDBStructureRef[sStructIndex].pDBStructure;
      if (pDBStructure != NULL &&
          (pDBStructure->ubNumberOfTiles > 1 || (pDBStructure->fFlags & STRUCTURE_CORPSE)))
      // if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1 )
      {
        // ATE: We allow SLIDING DOORS of 2 tile sizes...
        if (!(pDBStructure->fFlags & STRUCTURE_ANYDOOR) ||
            ((pDBStructure->fFlags & (STRUCTURE_ANYDOOR)) &&
             (pDBStructure->fFlags & STRUCTURE_SLIDINGDOOR))) {
          hVObject->ppZStripInfo[uiDestVoIndex] = (ZStripInfo *)MemAlloc(sizeof(ZStripInfo));
          if (hVObject->ppZStripInfo[uiDestVoIndex] == NULL) {
            // augh!! out of memory!  free everything allocated and abort
            for (ubLoop2 = 0; ubLoop2 < uiLoop; ubLoop2++) {
              if (hVObject->ppZStripInfo[ubLoop2] != NULL) {
                MemFree(hVObject->ppZStripInfo[uiLoop]);
              }
            }
            MemFree(hVObject->ppZStripInfo);
            hVObject->ppZStripInfo = NULL;
            return (FALSE);
          } else {
            pCurr = hVObject->ppZStripInfo[uiDestVoIndex];

            ubNumIncreasing = 0;
            ubNumStable = 0;
            ubNumDecreasing = 0;

            // time to do our calculations!
            sOffsetX = hVObject->pETRLEObject[uiLoop].sOffsetX;
            sOffsetY = hVObject->pETRLEObject[uiLoop].sOffsetY;
            usWidth = hVObject->pETRLEObject[uiLoop].usWidth;
            if (pDBStructure->fFlags & (STRUCTURE_MOBILE | STRUCTURE_CORPSE)) {
              // adjust for the difference between the animation and structure base tile

              // if (pDBStructure->fFlags & (STRUCTURE_MOBILE ) )
              {
                sOffsetX = sOffsetX + (WORLD_TILE_X / 2);
                sOffsetY = sOffsetY + (WORLD_TILE_Y / 2);
              }
              // adjust for the tile offset
              sOffsetX = sOffsetX - pDBStructure->bZTileOffsetX * (WORLD_TILE_X / 2) +
                         pDBStructure->bZTileOffsetY * (WORLD_TILE_X / 2);
              sOffsetY = sOffsetY - pDBStructure->bZTileOffsetY * (WORLD_TILE_Y / 2);
            }

            // figure out how much of the image is on each side of
            // the bottom corner of the base tile
            if (sOffsetX <= 0) {
              // note that the adjustments here by (WORLD_TILE_X / 2) are to account for the X
              // difference between the blit position and the bottom corner of the base tile
              sRightHalfWidth = usWidth + sOffsetX - (WORLD_TILE_X / 2);

              if (sRightHalfWidth >= 0) {
                // Case 1: negative image offset, image straddles bottom corner

                // negative of a negative is positive
                sLeftHalfWidth = -sOffsetX + (WORLD_TILE_X / 2);
              } else {
                // Case 2: negative image offset, image all on left side

                // bump up the LeftHalfWidth to the right edge of the last tile-half,
                // so we can calculate the size of the leftmost portion accurately
                // NB subtracting a negative to add the absolute value
                sLeftHalfWidth = usWidth - (sRightHalfWidth % (WORLD_TILE_X / 2));
                sRightHalfWidth = 0;
              }
            } else if (sOffsetX < (WORLD_TILE_X / 2)) {
              sLeftHalfWidth = (WORLD_TILE_X / 2) - sOffsetX;
              sRightHalfWidth = usWidth - sLeftHalfWidth;
              if (sRightHalfWidth <= 0) {
                // Case 3: positive offset < 20, image all on left side
                // should never happen because these images are multi-tile!
                sRightHalfWidth = 0;
                // fake the left width to one half-tile
                sLeftHalfWidth = (WORLD_TILE_X / 2);
              } else {
                // Case 4: positive offset < 20, image straddles bottom corner

                // all okay?
              }
            } else {
              // Case 5: positive offset, image all on right side
              // should never happen either
              sLeftHalfWidth = 0;
              sRightHalfWidth = usWidth;
            }

            if (sLeftHalfWidth > 0) {
              ubNumIncreasing = sLeftHalfWidth / (WORLD_TILE_X / 2);
            }
            if (sRightHalfWidth > 0) {
              ubNumStable = 1;
              if (sRightHalfWidth > (WORLD_TILE_X / 2)) {
                ubNumDecreasing = sRightHalfWidth / (WORLD_TILE_X / 2);
              }
            }
            if (sLeftHalfWidth > 0) {
              pCurr->ubFirstZStripWidth = sLeftHalfWidth % (WORLD_TILE_X / 2);
              if (pCurr->ubFirstZStripWidth == 0) {
                ubNumIncreasing--;
                pCurr->ubFirstZStripWidth = (WORLD_TILE_X / 2);
              }
            } else  // right side only; offset is at least 20 (= WORLD_TILE_X / 2)
            {
              if (sOffsetX > WORLD_TILE_X) {
                pCurr->ubFirstZStripWidth =
                    (WORLD_TILE_X / 2) - (sOffsetX - WORLD_TILE_X) % (WORLD_TILE_X / 2);
              } else {
                pCurr->ubFirstZStripWidth = WORLD_TILE_X - sOffsetX;
              }
              if (pCurr->ubFirstZStripWidth == 0) {
                ubNumDecreasing--;
                pCurr->ubFirstZStripWidth = (WORLD_TILE_X / 2);
              }
            }

            // now create the array!
            pCurr->ubNumberOfZChanges = ubNumIncreasing + ubNumStable + ubNumDecreasing;
            pCurr->pbZChange = (int8_t *)MemAlloc(pCurr->ubNumberOfZChanges);
            if (pCurr->pbZChange == NULL) {
              // augh!
              for (ubLoop2 = 0; ubLoop2 < uiLoop; ubLoop2++) {
                if (hVObject->ppZStripInfo[ubLoop2] != NULL) {
                  MemFree(hVObject->ppZStripInfo[uiLoop]);
                }
              }
              MemFree(hVObject->ppZStripInfo);
              hVObject->ppZStripInfo = NULL;
              return (FALSE);
            }
            for (ubLoop2 = 0; ubLoop2 < ubNumIncreasing; ubLoop2++) {
              pCurr->pbZChange[ubLoop2] = 1;
            }
            for (; ubLoop2 < ubNumIncreasing + ubNumStable; ubLoop2++) {
              pCurr->pbZChange[ubLoop2] = 0;
            }
            for (; ubLoop2 < pCurr->ubNumberOfZChanges; ubLoop2++) {
              pCurr->pbZChange[ubLoop2] = -1;
            }
            if (ubNumIncreasing > 0) {
              pCurr->bInitialZChange = -(ubNumIncreasing);
            } else if (ubNumStable > 0) {
              pCurr->bInitialZChange = 0;
            } else {
              pCurr->bInitialZChange = -(ubNumDecreasing);
            }
          }
        }
      }
    }
  }
  return (TRUE);
}

BOOLEAN InitStructureDB(void) {
  gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;
  return (TRUE);
}

BOOLEAN FiniStructureDB(void) {
  gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;
  return (TRUE);
}

int8_t GetBlockingStructureInfo(int16_t sGridNo, int8_t bDir, int8_t bNextDir, int8_t bLevel,
                                int8_t *pStructHeight, struct STRUCTURE **ppTallestStructure,
                                BOOLEAN fWallsBlock) {
  struct STRUCTURE *pCurrent, *pStructure;
  int16_t sDesiredLevel;
  BOOLEAN fOKStructOnLevel = FALSE;
  BOOLEAN fMinimumBlockingFound = FALSE;

  if (bLevel == 0) {
    sDesiredLevel = STRUCTURE_ON_GROUND;
  } else {
    sDesiredLevel = STRUCTURE_ON_ROOF;
  }

  pCurrent = gpWorldLevelData[sGridNo].pStructureHead;

  // If no struct, return
  if (pCurrent == NULL) {
    (*pStructHeight) = StructureHeight(pCurrent);
    (*ppTallestStructure) = NULL;
    return (NOTHING_BLOCKING);
  }

  while (pCurrent != NULL) {
    // Check level!
    if (pCurrent->sCubeOffset == sDesiredLevel) {
      fOKStructOnLevel = TRUE;
      pStructure = pCurrent;

      // Turn off if we are on upper level!
      if (pCurrent->fFlags & STRUCTURE_ROOF && bLevel == 1) {
        fOKStructOnLevel = FALSE;
      }

      // Don't stop FOV for people
      if (pCurrent->fFlags & (STRUCTURE_CORPSE | STRUCTURE_PERSON)) {
        fOKStructOnLevel = FALSE;
      }

      if (pCurrent->fFlags & (STRUCTURE_TREE | STRUCTURE_ANYFENCE)) {
        fMinimumBlockingFound = TRUE;
      }

      // Default, if we are a wall, set full blocking
      if ((pCurrent->fFlags & STRUCTURE_WALL) && !fWallsBlock) {
        // Return full blocking!
        // OK! This will be handled by movement costs......!
        fOKStructOnLevel = FALSE;
      }

      // CHECK FOR WINDOW
      if (pCurrent->fFlags & STRUCTURE_WALLNWINDOW) {
        switch (pCurrent->ubWallOrientation) {
          case OUTSIDE_TOP_LEFT:
          case INSIDE_TOP_LEFT:

            (*pStructHeight) = StructureHeight(pCurrent);
            (*ppTallestStructure) = pCurrent;

            if (pCurrent->fFlags & STRUCTURE_OPEN) {
              return (BLOCKING_TOPLEFT_OPEN_WINDOW);
            } else {
              return (BLOCKING_TOPLEFT_WINDOW);
            }
            break;

          case OUTSIDE_TOP_RIGHT:
          case INSIDE_TOP_RIGHT:

            (*pStructHeight) = StructureHeight(pCurrent);
            (*ppTallestStructure) = pCurrent;

            if (pCurrent->fFlags & STRUCTURE_OPEN) {
              return (BLOCKING_TOPRIGHT_OPEN_WINDOW);
            } else {
              return (BLOCKING_TOPRIGHT_WINDOW);
            }
            break;
        }
      }

      // Check for door
      if (pCurrent->fFlags & STRUCTURE_ANYDOOR) {
        // If we are not opem, we are full blocking!
        if (!(pCurrent->fFlags & STRUCTURE_OPEN)) {
          (*pStructHeight) = StructureHeight(pCurrent);
          (*ppTallestStructure) = pCurrent;
          return (FULL_BLOCKING);
          break;
        } else {
          switch (pCurrent->ubWallOrientation) {
            case OUTSIDE_TOP_LEFT:
            case INSIDE_TOP_LEFT:

              (*pStructHeight) = StructureHeight(pCurrent);
              (*ppTallestStructure) = pCurrent;
              return (BLOCKING_TOPLEFT_DOOR);
              break;

            case OUTSIDE_TOP_RIGHT:
            case INSIDE_TOP_RIGHT:

              (*pStructHeight) = StructureHeight(pCurrent);
              (*ppTallestStructure) = pCurrent;
              return (BLOCKING_TOPRIGHT_DOOR);
              break;
          }
        }
      }
    }
    pCurrent = pCurrent->pNext;
  }

  // OK, here, we default to we've seen a struct, reveal just this one
  if (fOKStructOnLevel) {
    if (fMinimumBlockingFound) {
      (*pStructHeight) = StructureHeight(pStructure);
      (*ppTallestStructure) = pStructure;
      return (BLOCKING_REDUCE_RANGE);
    } else {
      (*pStructHeight) = StructureHeight(pStructure);
      (*ppTallestStructure) = pStructure;
      return (BLOCKING_NEXT_TILE);
    }
  } else {
    (*pStructHeight) = 0;
    (*ppTallestStructure) = NULL;
    return (NOTHING_BLOCKING);
  }
}

uint8_t StructureFlagToType(uint32_t uiFlag) {
  uint8_t ubLoop;
  uint32_t uiBit = STRUCTURE_GENERIC;

  for (ubLoop = 8; ubLoop < 32; ubLoop++) {
    if ((uiFlag & uiBit) != 0) {
      return (ubLoop);
    }
    uiBit = uiBit << 1;
  }
  return (0);
}

uint32_t StructureTypeToFlag(uint8_t ubType) {
  uint32_t uiFlag = 0x1;

  uiFlag = uiFlag << ubType;
  return (uiFlag);
}

struct STRUCTURE *FindStructureBySavedInfo(int16_t sGridNo, uint8_t ubType,
                                           uint8_t ubWallOrientation, int8_t bLevel) {
  struct STRUCTURE *pCurrent;
  uint32_t uiTypeFlag;

  uiTypeFlag = StructureTypeToFlag(ubType);

  pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
  while (pCurrent != NULL) {
    if (pCurrent->fFlags & uiTypeFlag && pCurrent->ubWallOrientation == ubWallOrientation &&
        ((bLevel == 0 && pCurrent->sCubeOffset == 0) ||
         (bLevel > 0 && pCurrent->sCubeOffset > 0))) {
      return (pCurrent);
    }
    pCurrent = pCurrent->pNext;
  }
  return (NULL);
}

uint32_t GetStructureOpenSound(struct STRUCTURE *pStructure, BOOLEAN fClose) {
  uint32_t uiSoundID;

  switch (pStructure->pDBStructureRef->pDBStructure->ubArmour) {
    case MATERIAL_LIGHT_METAL:
    case MATERIAL_THICKER_METAL:

      uiSoundID = OPEN_LOCKER;
      break;

    case MATERIAL_WOOD_WALL:
    case MATERIAL_PLYWOOD_WALL:
    case MATERIAL_FURNITURE:

      uiSoundID = OPEN_WOODEN_BOX;
      break;

    default:
      uiSoundID = OPEN_DEFAULT_OPENABLE;
  }

  if (fClose) {
    uiSoundID++;
  }

  return (uiSoundID);
}
