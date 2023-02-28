#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "SGP/Types.h"

#define MAX_CACHE_SIZE 20
#define MIN_CACHE_SIZE 2

struct AnimationSurfaceCache {
  UINT16 *usCachedSurfaces;
  INT16 *sCacheHits;
  UINT8 ubCacheSize;
};

extern UINT32 guiCacheSize;

BOOLEAN GetCachedAnimationSurface(UINT16 usSoldierID, struct AnimationSurfaceCache *pAnimCache,
                                  UINT16 usSurfaceIndex, UINT16 usCurrentAnimation);
BOOLEAN InitAnimationCache(UINT16 usSoldierID, struct AnimationSurfaceCache *pAnimCache);
void DeleteAnimationCache(UINT16 usSoldierID, struct AnimationSurfaceCache *pAnimCache);
void DetermineOptimumAnimationCacheSize();
void UnLoadCachedAnimationSurfaces(UINT16 usSoldierID, struct AnimationSurfaceCache *pAnimCache);

#endif
