#ifndef __STRATEGIC_H
#define __STRATEGIC_H

#include "SGP/Types.h"
#include "Sector.h"
#include "Strategic/MapScreen.h"

struct SOLDIERTYPE;

struct strategicmapelement {
  UINT8 UNUSEDuiFootEta[4];     // eta/mvt costs for feet
  UINT8 UNUSEDuiVehicleEta[4];  // eta/mvt costs for vehicles
  UINT8 uiBadFootSector[4];     // blocking mvt for foot
  UINT8 uiBadVehicleSector[4];  // blocking mvt from vehicles
  INT8 townID;
  BOOLEAN fEnemyControlled;  // enemy controlled or not
  BOOLEAN fEnemyAirControlled;
  BOOLEAN UNUSEDfLostControlAtSomeTime;
  INT8 __only_storage_bSAMCondition;  // SAM Condition .. 0 - 100, just like an item's status
  INT8 bPadding[20];
};

enum {
  INSERTION_CODE_NORTH,
  INSERTION_CODE_SOUTH,
  INSERTION_CODE_EAST,
  INSERTION_CODE_WEST,
  INSERTION_CODE_GRIDNO,
  INSERTION_CODE_ARRIVING_GAME,
  INSERTION_CODE_CHOPPER,
  INSERTION_CODE_PRIMARY_EDGEINDEX,
  INSERTION_CODE_SECONDARY_EDGEINDEX,
  INSERTION_CODE_CENTER,
};

// PLEASE USE GetSectorID16() macro instead (they're identical).
// #define			GETWORLDMAPNO( x, y )		( x+(MAP_WORLD_X*y) )

typedef struct strategicmapelement StrategicMapElement;
extern StrategicMapElement StrategicMap[MAP_WORLD_X * MAP_WORLD_Y];

BOOLEAN InitStrategicEngine();

void HandleSoldierDeadComments(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandleStrategicDeath(struct SOLDIERTYPE *pSoldier);

#endif
