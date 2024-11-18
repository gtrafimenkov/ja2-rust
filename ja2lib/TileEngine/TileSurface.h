#ifndef _TILE_SURFACE_H
#define _TILE_SURFACE_H

#include "SGP/Types.h"
#include "TileEngine/TileDat.h"

extern struct TILE_IMAGERY *gTileSurfaceArray[NUMBEROFTILETYPES];
extern uint8_t gbDefaultSurfaceUsed[NUMBEROFTILETYPES];
extern uint8_t gbSameAsDefaultSurfaceUsed[NUMBEROFTILETYPES];

struct TILE_IMAGERY *LoadTileSurface(char *cFilename);

void DeleteTileSurface(struct TILE_IMAGERY *pTileSurf);

void SetRaisedObjectFlag(char *cFilename, struct TILE_IMAGERY *pTileSurf);

#endif
