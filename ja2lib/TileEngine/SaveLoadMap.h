#ifndef _SAVELOADMAP__H_
#define _SAVELOADMAP__H_

#include "TileEngine/ExitGrids.h"

// Used for the ubType in the MODIFY_MAP struct
enum {
  SLM_NONE,

  // Adding a map graphic
  SLM_LAND,
  SLM_OBJECT,
  SLM_STRUCT,
  SLM_SHADOW,
  SLM_MERC,  // Should never be used
  SLM_ROOF,
  SLM_ONROOF,
  SLM_TOPMOST,  // Should never be used

  // For Removing
  SLM_REMOVE_LAND,
  SLM_REMOVE_OBJECT,
  SLM_REMOVE_STRUCT,
  SLM_REMOVE_SHADOW,
  SLM_REMOVE_MERC,  // Should never be used
  SLM_REMOVE_ROOF,
  SLM_REMOVE_ONROOF,
  SLM_REMOVE_TOPMOST,  // Should never be used

  // Smell, or Blood is used
  SLM_BLOOD_SMELL,

  // Damage a particular struct
  SLM_DAMAGED_STRUCT,

  // Exit Grids
  SLM_EXIT_GRIDS,

  // State of Openable structs
  SLM_OPENABLE_STRUCT,

  // Modify window graphic & structure
  SLM_WINDOW_HIT,
};

typedef struct {
  uint16_t usGridNo;         // The gridno the graphic will be applied to
  uint16_t usImageType;      // graphic index
  uint16_t usSubImageIndex;  //
                             //	uint16_t	usIndex;
  UINT8 ubType;              // the layer it will be applied to

  UINT8 ubExtra;  // Misc. variable used to strore arbritary values
} MODIFY_MAP;

// Call this function, to set whether the map changes will be added to the  map temp file
void ApplyMapChangesToMapTempFile(BOOLEAN fAddToMap);

BOOLEAN SaveModifiedMapStructToMapTempFile(MODIFY_MAP *pMap, u8 sSectorX, u8 sSectorY,
                                           INT8 bSectorZ);

// Applies a change TO THE MAP TEMP file
void AddStructToMapTempFile(uint32_t iMapIndex, uint16_t usIndex);

// Applies a change TO THE MAP from the temp file
void AddStructFromMapTempFileToMap(uint32_t iMapIndex, uint16_t usIndex);

void AddObjectToMapTempFile(uint32_t uiMapIndex, uint16_t usIndex);

BOOLEAN LoadAllMapChangesFromMapTempFileAndApplyThem();

void RemoveStructFromMapTempFile(uint32_t uiMapIndex, uint16_t usIndex);

void AddRemoveObjectToMapTempFile(uint32_t uiMapIndex, uint16_t usIndex);

void SaveBloodSmellAndRevealedStatesFromMapToTempFile();

BOOLEAN SaveRevealedStatusArrayToRevealedTempFile(u8 sSectorX, u8 sSectorY, INT8 bSectorZ);

BOOLEAN LoadRevealedStatusArrayFromRevealedTempFile();

void AddRemoveObjectToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                          INT16 sSectorY, UINT8 ubSectorZ);
void RemoveStructFromUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                         INT16 sSectorY, UINT8 ubSectorZ);
void AddObjectToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                    INT16 sSectorY, UINT8 ubSectorZ);
void AddStructToUnLoadedMapTempFile(uint32_t uiMapIndex, uint16_t usIndex, INT16 sSectorX,
                                    INT16 sSectorY, UINT8 ubSectorZ);

// Adds the exit grid to
void AddExitGridToMapTempFile(uint16_t usGridNo, EXITGRID *pExitGrid, u8 sSectorX, u8 sSectorY,
                              UINT8 ubSectorZ);

// This function removes a struct with the same MapIndex and graphic index from the given sectors
// temp file
BOOLEAN RemoveGraphicFromTempFile(uint32_t uiMapIndex, uint16_t usIndex, u8 sSectorX, u8 sSectorY,
                                  UINT8 ubSectorZ);

void SetOpenableStructStatusFromMapTempFile(uint32_t uiMapIndex, BOOLEAN fOpened);
void AddOpenableStructStatusToMapTempFile(uint32_t uiMapIndex, BOOLEAN fOpened);

void AddWindowHitToMapTempFile(uint32_t uiMapIndex);

BOOLEAN ChangeStatusOfOpenableStructInUnloadedSector(uint16_t usSectorX, uint16_t usSectorY,
                                                     INT8 bSectorZ, uint16_t usGridNo,
                                                     BOOLEAN fChangeToOpen);

#endif
