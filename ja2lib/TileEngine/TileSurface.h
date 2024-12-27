// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

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
