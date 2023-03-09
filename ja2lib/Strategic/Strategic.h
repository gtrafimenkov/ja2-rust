#ifndef __STRATEGIC_H
#define __STRATEGIC_H

#include "SGP/Types.h"
#include "Sector.h"
#include "Strategic/MapScreen.h"

struct SOLDIERTYPE;

struct strategicmapelement {
  uint8_t __unused_uiFootEta[4];     // eta/mvt costs for feet
  uint8_t __unused_uiVehicleEta[4];  // eta/mvt costs for vehicles
  uint8_t uiBadFootSector[4];        // blocking mvt for foot
  uint8_t uiBadVehicleSector[4];     // blocking mvt from vehicles
  int8_t townID;
  BOOLEAN __only_storage_fEnemyControlled;  // enemy controlled or not
  BOOLEAN __unused_fEnemyAirControlled;     // unused beause can be recalculated after load
  BOOLEAN __unused_fLostControlAtSomeTime;
  int8_t __only_storage_bSAMCondition;  // SAM Condition .. 0 - 100, just like an item's status
  int8_t bPadding[20];
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
