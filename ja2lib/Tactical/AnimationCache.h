// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "SGP/Types.h"

#define MAX_CACHE_SIZE 20
#define MIN_CACHE_SIZE 2

struct AnimationSurfaceCache {
  uint16_t *usCachedSurfaces;
  int16_t *sCacheHits;
  uint8_t ubCacheSize;
};

extern uint32_t guiCacheSize;

BOOLEAN GetCachedAnimationSurface(uint16_t usSoldierID, struct AnimationSurfaceCache *pAnimCache,
                                  uint16_t usSurfaceIndex, uint16_t usCurrentAnimation);
BOOLEAN InitAnimationCache(uint16_t usSoldierID, struct AnimationSurfaceCache *pAnimCache);
void DeleteAnimationCache(uint16_t usSoldierID, struct AnimationSurfaceCache *pAnimCache);
void DetermineOptimumAnimationCacheSize();
void UnLoadCachedAnimationSurfaces(uint16_t usSoldierID, struct AnimationSurfaceCache *pAnimCache);

#endif
