#ifndef __MAP_EDGEPOINTS_H
#define __MAP_EDGEPOINTS_H

#include "SGP/Types.h"

typedef struct MAPEDGEPOINTINFO {
  uint8_t ubNumPoints;
  uint8_t ubStrategicInsertionCode;
  uint16_t sGridNo[32];
} MAPEDGEPOINTINFO;

uint16_t ChooseMapEdgepoint(uint8_t ubStrategicInsertionCode);
void ChooseMapEdgepoints(MAPEDGEPOINTINFO *pMapEdgepointInfo, uint8_t ubStrategicInsertionCode,
                         uint8_t ubNumDesiredPoints);
void GenerateMapEdgepoints();
void SaveMapEdgepoints(HWFILE fp);
BOOLEAN LoadMapEdgepoints(INT8 **hBuffer);
void TrashMapEdgepoints();

// dynamic arrays that contain the valid gridno's for each edge
extern INT16 *gps1stNorthEdgepointArray;
extern INT16 *gps1stEastEdgepointArray;
extern INT16 *gps1stSouthEdgepointArray;
extern INT16 *gps1stWestEdgepointArray;
// contains the size for each array
extern uint16_t gus1stNorthEdgepointArraySize;
extern uint16_t gus1stEastEdgepointArraySize;
extern uint16_t gus1stSouthEdgepointArraySize;
extern uint16_t gus1stWestEdgepointArraySize;
// contains the index value for the first array index of the second row of each edgepoint array.
// Because each edgepoint side has two rows, the outside most row is calculated first, then the
// inside row. For purposes of AI, it may become necessary to avoid this.
extern uint16_t gus1stNorthEdgepointMiddleIndex;
extern uint16_t gus1stEastEdgepointMiddleIndex;
extern uint16_t gus1stSouthEdgepointMiddleIndex;
extern uint16_t gus1stWestEdgepointMiddleIndex;

// dynamic arrays that contain the valid gridno's for each edge
extern INT16 *gps2ndNorthEdgepointArray;
extern INT16 *gps2ndEastEdgepointArray;
extern INT16 *gps2ndSouthEdgepointArray;
extern INT16 *gps2ndWestEdgepointArray;
// contains the size for each array
extern uint16_t gus2ndNorthEdgepointArraySize;
extern uint16_t gus2ndEastEdgepointArraySize;
extern uint16_t gus2ndSouthEdgepointArraySize;
extern uint16_t gus2ndWestEdgepointArraySize;
// contains the index value for the first array index of the second row of each edgepoint array.
// Because each edgepoint side has two rows, the outside most row is calculated first, then the
// inside row. For purposes of AI, it may become necessary to avoid this.
extern uint16_t gus2ndNorthEdgepointMiddleIndex;
extern uint16_t gus2ndEastEdgepointMiddleIndex;
extern uint16_t gus2ndSouthEdgepointMiddleIndex;
extern uint16_t gus2ndWestEdgepointMiddleIndex;

// This is the search code that will determine the closest map edgepoint to a given gridno and
// a strategic insertion code.  You must call the begin...() first, the call
// SearchForClosestMapEdgepoint() for each soldier that is looking.  After you are completely
// finished, call the end..().  The return value will return NOWHERE if the information is invalid
// (no edgepoint info for that particular side, or all spots are reserved.  There are only 20 spots
// that can be reserved (one for each player soldier).  This code shouldn't be used for enemies or
// anybody else.
void BeginMapEdgepointSearch();
void EndMapEdgepointSearch();
INT16 SearchForClosestPrimaryMapEdgepoint(INT16 sGridNo, uint8_t ubInsertionCode);
INT16 SearchForClosestSecondaryMapEdgepoint(INT16 sGridNo, uint8_t ubInsertionCode);

// There are two classes of edgepoints.
// PRIMARY		: The default list of edgepoints.  This list includes edgepoints that are
// easily accessible from the 						strategic level. SECONDARY
// : The isolated list of edgepoints.  Certain areas of the game are blocked off, but tactically
// traversing 						to these areas is possible.  Examples would
// be isolated sections of Grumm or Alma, which you can't immediately
//
uint8_t CalcMapEdgepointClassInsertionCode(INT16 sGridNo);

#ifdef JA2EDITOR
void ShowMapEdgepoints();
void HideMapEdgepoints();
#endif

#endif
