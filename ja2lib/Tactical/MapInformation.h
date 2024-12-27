#ifndef __MAP_INFORMATION_H
#define __MAP_INFORMATION_H

#include "SGP/Types.h"

extern float gdMajorMapVersion;
extern uint8_t gubMinorMapVersion;

enum  // for use with MAPCREATE_STRUCT.ubEditorSmoothingType
{
  SMOOTHING_NORMAL,
  SMOOTHING_BASEMENT,
  SMOOTHING_CAVES
};

typedef struct {
  // These are the mandatory entry points for a map.  If any of the values are -1, then that means
  // that the point has been specifically not used and that the map is not traversable to or from an
  // adjacent sector in that direction.  The >0 value points must be validated before saving the
  // map. This is done by simply checking if those points are sittable by mercs, and that you can
  // plot a path from these points to each other.  These values can only be set by the editor :
  // mapinfo tab
  int16_t sNorthGridNo;
  int16_t sEastGridNo;
  int16_t sSouthGridNo;
  int16_t sWestGridNo;
  // This contains the number of individuals in the map.
  // Individuals include NPCs, enemy placements, creatures, civilians, rebels, and animals.
  uint8_t ubNumIndividuals;
  uint8_t ubMapVersion;
  uint8_t ubRestrictedScrollID;
  uint8_t ubEditorSmoothingType;  // normal, basement, or caves
  int16_t sCenterGridNo;
  int16_t sIsolatedGridNo;
  int8_t bPadding[83];  // I'm sure lots of map info will be added
} MAPCREATE_STRUCT;     // 99 bytes

extern MAPCREATE_STRUCT gMapInformation;

void SaveMapInformation(HWFILE fp);
void LoadMapInformation(int8_t **hBuffer);
void ValidateAndUpdateMapVersionIfNecessary();
BOOLEAN ValidateEntryPointGridNo(int16_t *sGridNo);

extern BOOLEAN gfWorldLoaded;

#endif
