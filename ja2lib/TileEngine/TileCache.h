#ifndef __TILE_CACHE_H
#define __TILE_CACHE_H

#include "SGP/Types.h"

struct LEVELNODE;
struct VObject;

#define TILE_CACHE_START_INDEX 36000

typedef struct {
  char zName[128];               // Name of tile ( filename and directory here )
  char zRootName[30];            // Root name
  struct TILE_IMAGERY *pImagery;  // Tile imagery
  int16_t sHits;
  uint8_t ubNumFrames;
  int16_t sStructRefID;

} TILE_CACHE_ELEMENT;

typedef struct {
  char Filename[150];
  char zRootName[30];  // Root name
  struct STRUCTURE_FILE_REF *pStructureFileRef;

} TILE_CACHE_STRUCT;

extern TILE_CACHE_ELEMENT *gpTileCache;

BOOLEAN InitTileCache();
void DeleteTileCache();

int32_t GetCachedTile(char* cFilename);
BOOLEAN RemoveCachedTile(int32_t iCachedTile);

struct STRUCTURE_FILE_REF *GetCachedTileStructureRefFromFilename(char* cFilename);

struct VObject *GetCachedTileVideoObject(int32_t iIndex);
struct STRUCTURE_FILE_REF *GetCachedTileStructureRef(int32_t iIndex);
void CheckForAndAddTileCacheStructInfo(struct LEVELNODE *pNode, int16_t sGridNo, uint16_t usIndex,
                                       uint16_t usSubIndex);
void CheckForAndDeleteTileCacheStructInfo(struct LEVELNODE *pNode, uint16_t usIndex);
void GetRootName(char* pDestStr, char* pSrcStr);

// OF COURSE, FOR SPEED, WE EXPORT OUR ARRAY
// ACCESS FUNCTIONS IN RENDERER IS NOT TOO NICE
// ATE

#endif
