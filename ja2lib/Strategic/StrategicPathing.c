#include "Strategic/StrategicPathing.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SGP/Debug.h"

// #include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Overhead.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/WorldDef.h"
#include "TileEngine/WorldMan.h"

void AddSectorToFrontOfMercPath(struct path** ppMercPath, uint8_t ubSectorX, uint8_t ubSectorY);

// mvt modifier
// #define FOOT_MVT_MODIFIER 2

uint16_t gusPlottedPath[256];
uint16_t gusMapPathingData[256];
uint16_t gusPathDataSize;
BOOLEAN gfPlotToAvoidPlayerInfuencedSectors = FALSE;

// uint16_t gusEndPlotGridNo;

uint8_t ubFromMapDirToInsertionCode[] = {
    INSERTION_CODE_SOUTH,  // NORTH_STRATEGIC_MOVE
    INSERTION_CODE_WEST,   // EAST_STRATEGIC_MOVE
    INSERTION_CODE_NORTH,  // SOUTH_STRATEGIC_MOVE
    INSERTION_CODE_EAST    // WEST_STRATEGIC_MOVE
};

// Globals
struct path_s {
  int16_t nextLink;   // 2
  int16_t prevLink;   // 2
  int16_t location;   // 2
  int32_t costSoFar;  // 4
  int32_t costToGo;   // 4
  int16_t pathNdx;    // 2
};

typedef struct path_s path_t;

struct trail_s {
  short nextLink;
  short diStratDelta;
};
typedef struct trail_s trail_t;

#define MAXTRAILTREE (4096)
#define MAXpathQ (512)
#define MAP_WIDTH 18
#define MAP_LENGTH MAP_WIDTH* MAP_WIDTH

// #define EASYWATERCOST	TRAVELCOST_FLAT / 2
// #define ISWATER(t)	(((t)==TRAVELCOST_KNEEDEEP) || ((t)==TRAVELCOST_DEEPWATER))
// #define NOPASS (TRAVELCOST_OBSTACLE)
// #define VEINCOST TRAVELCOST_FLAT     //actual cost for bridges and doors and such
// #define ISVEIN(v) ((v==TRAVELCOST_VEINMID) || (v==TRAVELCOST_VEINEND))
#define TRAILCELLTYPE uint32_t

static path_t pathQB[MAXpathQ];
static TRAILCELLTYPE trailCostB[MAP_LENGTH];
static trail_t trailStratTreeB[MAXTRAILTREE];
short trailStratTreedxB = 0;

#define QHEADNDX (0)
#define QPOOLNDX (MAXpathQ - 1)

#define pathQNotEmpty (pathQB[QHEADNDX].nextLink != QHEADNDX)
#define pathFound (pathQB[pathQB[QHEADNDX].nextLink].location == sDestination)
#define pathNotYetFound (!pathFound)

#define REMQUENODE(ndx)                                           \
  {                                                               \
    pathQB[pathQB[ndx].prevLink].nextLink = pathQB[ndx].nextLink; \
    pathQB[pathQB[ndx].nextLink].prevLink = pathQB[ndx].prevLink; \
  }

#define INSQUENODEPREV(newNode, curNode)                 \
  {                                                      \
    pathQB[newNode].nextLink = curNode;                  \
    pathQB[newNode].prevLink = pathQB[curNode].prevLink; \
    pathQB[pathQB[curNode].prevLink].nextLink = newNode; \
    pathQB[curNode].prevLink = newNode;                  \
  }

#define INSQUENODE(newNode, curNode)                     \
  {                                                      \
    pathQB[newNode].prevLink = curNode;                  \
    pathQB[newNode].NextLink = pathQB[curNode].nextLink; \
    pathQB[pathQB[curNode].nextLink].prevLink = newNode; \
    pathQB[curNode].nextLink = newNode;                  \
  }

#define DELQUENODE(ndx)            \
  {                                \
    REMQUENODE(ndx);               \
    INSQUENODEPREV(ndx, QPOOLNDX); \
    pathQB[ndx].location = -1;     \
  }

#define NEWQUENODE                       \
  if (queRequests < QPOOLNDX)            \
    qNewNdx = queRequests++;             \
  else {                                 \
    qNewNdx = pathQB[QPOOLNDX].nextLink; \
    REMQUENODE(qNewNdx);                 \
  }

#define ESTIMATE0 ((dx > dy) ? (dx) : (dy))
#define ESTIMATE1 ((dx < dy) ? ((dx * 14) / 10 + dy) : ((dy * 14) / 10 + dx))
#define ESTIMATE2 FLATCOST*((dx < dy) ? ((dx * 14) / 10 + dy) : ((dy * 14) / 10 + dx))
#define ESTIMATEn ((int)(FLATCOST * sqrt(dx * dx + dy * dy)))
#define ESTIMATE ESTIMATE1

#define REMAININGCOST(ndx)                                                               \
  ((locY = pathQB[ndx].location / MAP_WIDTH), (locX = pathQB[ndx].location % MAP_WIDTH), \
   (dy = abs(iDestY - locY)), (dx = abs(iDestX - locX)), ESTIMATE)

#define MAXCOST (99900)
#define TOTALCOST(ndx) (pathQB[ndx].costSoFar + pathQB[ndx].costToGo)
#define XLOC(a) (a % MAP_WIDTH)
#define YLOC(a) (a / MAP_WIDTH)
#define LEGDISTANCE(a, b) (abs(XLOC(b) - XLOC(a)) + abs(YLOC(b) - YLOC(a)))
#define FARTHER(ndx, NDX)                            \
  (LEGDISTANCE(pathQB[ndx].location, sDestination) > \
   LEGDISTANCE(pathQB[NDX].location, sDestination))

#define FLAT_STRATEGIC_TRAVEL_TIME 60

#define QUESEARCH(ndx, NDX)                                                               \
  {                                                                                       \
    int32_t k = TOTALCOST(ndx);                                                           \
    NDX = pathQB[QHEADNDX].nextLink;                                                      \
    while (NDX && (k > TOTALCOST(NDX))) NDX = pathQB[NDX].nextLink;                       \
    while (NDX && (k == TOTALCOST(NDX)) && FARTHER(ndx, NDX)) NDX = pathQB[NDX].nextLink; \
  }

int32_t queRequests;

int16_t diStratDelta[8] = {
    -MAP_WIDTH,     // N
    1 - MAP_WIDTH,  // NE
    1,              // E
    1 + MAP_WIDTH,  // SE
    MAP_WIDTH,      // S
    MAP_WIDTH - 1,  // SW
    -1,             // W
    -MAP_WIDTH - 1  // NW
};

extern uint8_t GetTraversability(int16_t sStartSector, int16_t sEndSector);

// this will find if a shortest strategic path

int32_t FindStratPath(uint8_t startX, uint8_t startY, uint8_t destX, uint8_t destY,
                      int16_t sMvtGroupNumber, BOOLEAN fTacticalTraversal) {
  int32_t iCnt, ndx, insertNdx, qNewNdx;
  int32_t iDestX, iDestY, locX, locY, dx, dy;
  uint8_t sSectorX, sSectorY;
  uint16_t newLoc, curLoc;
  TRAILCELLTYPE curCost, newTotCost, nextCost;
  int16_t sOrigination;
  BOOLEAN fPlotDirectPath = FALSE;
  static BOOLEAN fPreviousPlotDirectPath = FALSE;  // don't save
  struct GROUP* pGroup;

  // ******** Fudge by Bret (for now), curAPcost is never initialized in this function, but should
  // be! so this is just to keep things happy!

  // for player groups only!
  pGroup = GetGroup((uint8_t)sMvtGroupNumber);
  if (pGroup->fPlayer) {
    // if player is holding down SHIFT key, find the shortest route instead of the quickest route!
    if (_KeyDown(SHIFT)) {
      fPlotDirectPath = TRUE;
    }

    if (fPlotDirectPath != fPreviousPlotDirectPath) {
      // must redraw map to erase the previous path...
      SetMapPanelDirty(true);
      fPreviousPlotDirectPath = fPlotDirectPath;
    }
  }

  queRequests = 2;

  // initialize the ai data structures
  memset(trailStratTreeB, 0, sizeof(trailStratTreeB));
  memset(trailCostB, 255, sizeof(trailCostB));

  // memset(trailCostB,255*PATHFACTOR,MAP_LENGTH);
  memset(pathQB, 0, sizeof(pathQB));

  SectorID16 sStart = GetSectorID16(startX, startY);
  SectorID16 sDestination = GetSectorID16(destX, destY);

  // FOLLOWING LINE COMMENTED OUT ON MARCH 7/97 BY IC
  memset(gusMapPathingData, ((uint16_t)sStart), sizeof(gusMapPathingData));
  trailStratTreedxB = 0;

  // set up common info
  sOrigination = sStart;

  iDestY = (sDestination / MAP_WIDTH);
  iDestX = (sDestination % MAP_WIDTH);

  // if origin and dest is water, then user wants to stay in water!
  // so, check and set waterToWater flag accordingly

  // setup Q
  pathQB[QHEADNDX].location = sOrigination;
  pathQB[QHEADNDX].nextLink = 1;
  pathQB[QHEADNDX].prevLink = 1;
  pathQB[QHEADNDX].costSoFar = MAXCOST;

  pathQB[QPOOLNDX].nextLink = QPOOLNDX;
  pathQB[QPOOLNDX].prevLink = QPOOLNDX;

  // setup first path record
  pathQB[1].nextLink = QHEADNDX;
  pathQB[1].prevLink = QHEADNDX;
  pathQB[1].location = sOrigination;
  pathQB[1].pathNdx = 0;
  pathQB[1].costSoFar = 0;
  pathQB[1].costToGo = REMAININGCOST(1);

  trailStratTreedxB = 0;
  trailCostB[sOrigination] = 0;
  ndx = pathQB[QHEADNDX].nextLink;
  pathQB[ndx].pathNdx = trailStratTreedxB;
  trailStratTreedxB++;

  do {
    // remove the first and best path so far from the que
    ndx = pathQB[QHEADNDX].nextLink;
    curLoc = pathQB[ndx].location;
    curCost = pathQB[ndx].costSoFar;
    // = totAPCostB[ndx];
    DELQUENODE((int16_t)ndx);

    if (trailCostB[curLoc] < curCost) continue;

    // contemplate a new path in each direction
    for (iCnt = 0; iCnt < 8; iCnt += 2) {
      newLoc = curLoc + diStratDelta[iCnt];

      // are we going off the map?
      if ((SectorID16_X(newLoc) == 0) || (SectorID16_X(newLoc) == MAP_WORLD_X - 1) ||
          (SectorID16_Y(newLoc) == 0) || (SectorID16_Y(newLoc) == MAP_WORLD_X - 1)) {
        // yeppers
        continue;
      }

      if (gfPlotToAvoidPlayerInfuencedSectors && newLoc != sDestination) {
        sSectorX = SectorID16_X(newLoc);
        sSectorY = SectorID16_Y(newLoc);

        if (IsThereASoldierInThisSector(sSectorX, sSectorY, 0)) {
          continue;
        }
        if (CountMilitiaInSector3D(sSectorX, sSectorY, 0)) {
          continue;
        }
        if (!OkayForEnemyToMoveThroughSector((uint8_t)GetSectorID8(sSectorX, sSectorY))) {
          continue;
        }
      }

      // are we plotting path or checking for existance of one?
      if (sMvtGroupNumber != 0) {
        if (iHelicopterVehicleId != -1) {
          nextCost = GetTravelTimeForGroup(
              (uint8_t)(GetSectorID8((SectorID16_X(curLoc)), (SectorID16_Y(curLoc)))),
              (uint8_t)(iCnt / 2), (uint8_t)sMvtGroupNumber);
          if (nextCost != 0xffffffff &&
              sMvtGroupNumber == pVehicleList[iHelicopterVehicleId].ubMovementGroup) {
            // is a heli, its pathing is determined not by time (it's always the same) but by total
            // cost Skyrider will avoid uncontrolled airspace as much as possible...
            if (IsSectorEnemyAirControlled(SectorID16_X(curLoc), SectorID16_Y(curLoc))) {
              nextCost = COST_AIRSPACE_UNSAFE;
            } else {
              nextCost = COST_AIRSPACE_SAFE;
            }
          }
        } else {
          nextCost = GetTravelTimeForGroup(
              (uint8_t)(GetSectorID8((SectorID16_X(curLoc)), (SectorID16_Y(curLoc)))),
              (uint8_t)(iCnt / 2), (uint8_t)sMvtGroupNumber);
        }
      } else {
        nextCost = GetTravelTimeForFootTeam(
            (uint8_t)(GetSectorID8(SectorID16_X(curLoc), SectorID16_Y(curLoc))),
            (uint8_t)(iCnt / 2));
      }

      if (nextCost == 0xffffffff) {
        continue;
      }

      // if we're building this path due to a tactical traversal exit, we have to force the path to
      // the next sector be in the same direction as the traversal, even if it's not the shortest
      // route, otherwise pathing can crash!  This can happen in places where the long way around to
      // next sector is actually shorter: e.g. D5 to D6.  ARM
      if (fTacticalTraversal) {
        // if it's the first sector only (no cost yet)
        if (curCost == 0 && (newLoc == sDestination)) {
          if (GetTraversability((int16_t)(GetSectorID8(curLoc % 18, curLoc / 18)),
                                (int16_t)(GetSectorID8(newLoc % 18, newLoc / 18))) !=
              GROUNDBARRIER) {
            nextCost = 0;
          }
        }
      } else {
        if (fPlotDirectPath) {
          // use shortest route instead of faster route
          nextCost = FLAT_STRATEGIC_TRAVEL_TIME;
        }
      }

      newTotCost = curCost + nextCost;
      if (newTotCost < trailCostB[newLoc]) {
        NEWQUENODE;

        if (qNewNdx == QHEADNDX) {
          return (0);
        }

        if (qNewNdx == QPOOLNDX) {
          return (0);
        }

        // make new path to current location
        trailStratTreeB[trailStratTreedxB].nextLink = pathQB[ndx].pathNdx;
        trailStratTreeB[trailStratTreedxB].diStratDelta = (int16_t)iCnt;
        pathQB[qNewNdx].pathNdx = trailStratTreedxB;
        trailStratTreedxB++;

        if (trailStratTreedxB >= MAXTRAILTREE) {
          return (0);
        }

        pathQB[qNewNdx].location = (int16_t)newLoc;
        pathQB[qNewNdx].costSoFar = newTotCost;
        pathQB[qNewNdx].costToGo = REMAININGCOST(qNewNdx);
        trailCostB[newLoc] = newTotCost;
        // do a sorted que insert of the new path
        QUESEARCH(qNewNdx, insertNdx);
        INSQUENODEPREV((int16_t)qNewNdx, (int16_t)insertNdx);
      }
    }
  } while (pathQNotEmpty && pathNotYetFound);
  // work finished. Did we find a path?
  if (pathFound) {
    int16_t z, _z, _nextLink;  //,tempgrid;

    _z = 0;
    z = pathQB[pathQB[QHEADNDX].nextLink].pathNdx;

    while (z) {
      _nextLink = trailStratTreeB[z].nextLink;
      trailStratTreeB[z].nextLink = _z;
      _z = z;
      z = _nextLink;
    }

    // if this function was called because a solider is about to embark on an actual route
    // (as opposed to "test" path finding (used by cursor, etc), then grab all pertinent
    // data and copy into soldier's database

    z = _z;

    for (iCnt = 0; z && (iCnt < MAX_PATH_LIST_SIZE); iCnt++) {
      gusMapPathingData[iCnt] = trailStratTreeB[z].diStratDelta;

      z = trailStratTreeB[z].nextLink;
    }

    gusPathDataSize = (uint16_t)iCnt;

    // return path length : serves as a "successful" flag and a path length counter
    return (iCnt);
  }
  // failed miserably, report...
  return (0);
}

struct path* BuildAStrategicPath(struct path* pPath, int16_t iStartSectorNum, int16_t iEndSectorNum,
                                 int16_t sMvtGroupNumber,
                                 BOOLEAN fTacticalTraversal /*, BOOLEAN fTempPath */) {
  int32_t iCurrentSectorNum;
  int32_t iDelta = 0;
  int32_t iPathLength;
  int32_t iCount = 0;
  struct path* pNode = NULL;
  struct path* pDeleteNode = NULL;
  struct path* pHeadOfPathList = pPath;
  iCurrentSectorNum = iStartSectorNum;

  if (pNode == NULL) {
    // start new path list
    pNode = (struct path*)MemAlloc(sizeof(struct path));
    pNode->fSpeed = NORMAL_MVT;
    pNode->uiSectorId = iStartSectorNum;
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    pNode->uiEta = GetGameTimeInMin();
    pHeadOfPathList = pNode;
  }

  if (iEndSectorNum < MAP_WORLD_X - 1) return NULL;

  iPathLength = ((int32_t)FindStratPath(
      SectorID16_X(iStartSectorNum), SectorID16_Y(iStartSectorNum), SectorID16_X(iEndSectorNum),
      SectorID16_Y(iEndSectorNum), sMvtGroupNumber, fTacticalTraversal));
  while (iPathLength > iCount) {
    switch (gusMapPathingData[iCount]) {
      case (NORTH):
        iDelta = NORTH_MOVE;
        break;
      case (SOUTH):
        iDelta = SOUTH_MOVE;
        break;
      case (EAST):
        iDelta = EAST_MOVE;
        break;
      case (WEST):
        iDelta = WEST_MOVE;
        break;
    }
    iCount++;
    // create new node
    iCurrentSectorNum += iDelta;

    if (!AddSectorToPathList(pHeadOfPathList, (uint16_t)iCurrentSectorNum)) {
      pNode = pHeadOfPathList;
      // intersected previous node, delete path to date
      if (!pNode) return NULL;
      while (pNode->pNext) pNode = pNode->pNext;
      // start backing up
      while (pNode->uiSectorId != (uint32_t)iStartSectorNum) {
        pDeleteNode = pNode;
        pNode = pNode->pPrev;
        pNode->pNext = NULL;
        MemFree(pDeleteNode);
      }
      return NULL;
    }

    pHeadOfPathList = pNode;
    if (!pNode) return NULL;
    while (pNode->pNext) pNode = pNode->pNext;
  }

  pNode = pHeadOfPathList;

  if (!pNode) return NULL;
  while (pNode->pNext) pNode = pNode->pNext;

  if (!pNode->pPrev) {
    MemFree(pNode);
    pHeadOfPathList = NULL;
    pPath = pHeadOfPathList;
    return FALSE;
  }

  /*
  // ok add last waypt
  if( fTempPath == FALSE )
  {
          // change in direction..add waypoint
          AddWaypointToGroup( ( uint8_t )sMvtGroupNumber, ( uint8_t )( iCurrentSectorNum%
MAP_WORLD_X ), ( uint8_t )( SectorID16_Y(iCurrentSectorNum) ) );
}
  */

  pPath = pHeadOfPathList;
  return pPath;
}

BOOLEAN AddSectorToPathList(struct path* pPath, uint16_t uiSectorNum) {
  struct path* pNode = NULL;
  struct path* pTempNode = NULL;
  struct path* pHeadOfList = pPath;
  pNode = pPath;

  if (uiSectorNum < MAP_WORLD_X - 1) return FALSE;

  if (pNode == NULL) {
    pNode = (struct path*)MemAlloc(sizeof(struct path));

    // Implement EtaCost Array as base EtaCosts of sectors
    // pNode->uiEtaCost=EtaCost[uiSectorNum];
    pNode->uiSectorId = uiSectorNum;
    pNode->uiEta = GetGameTimeInMin();
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    /*
         if ( _KeyDown( CTRL ))
                   pNode->fSpeed=SLOW_MVT;
               else
    */
    pNode->fSpeed = NORMAL_MVT;

    return TRUE;
  } else {
    // if (pNode->uiSectorId==uiSectorNum)
    //	  return FALSE;
    while (pNode->pNext) {
      //  if (pNode->uiSectorId==uiSectorNum)
      //	  return FALSE;
      pNode = pNode->pNext;
    }

    pTempNode = (struct path*)MemAlloc(sizeof(struct path));
    pTempNode->uiEta = 0;
    pNode->pNext = pTempNode;
    pTempNode->uiSectorId = uiSectorNum;
    pTempNode->pPrev = pNode;
    pTempNode->pNext = NULL;
    /*
          if ( _KeyDown( CTRL ))
           pTempNode->fSpeed=SLOW_MVT;
          else
    */
    pTempNode->fSpeed = NORMAL_MVT;
    pNode = pTempNode;
  }
  pPath = pHeadOfList;
  return TRUE;
}

struct path* AppendStrategicPath(struct path* pNewSection, struct path* pHeadOfPathList) {
  // will append a new section onto the end of the head of list, then return the head of the new
  // list

  struct path* pNode = pHeadOfPathList;
  // move to end of original section

  if (pNewSection == NULL) {
    return pHeadOfPathList;
  }

  // is there in fact a list to append to
  if (pNode) {
    // move to tail of old list
    while (pNode->pNext) {
      // next node in list
      pNode = pNode->pNext;
    }

    // make sure the 2 are not the same

    if (pNode->uiSectorId == pNewSection->uiSectorId) {
      // are the same, remove head of new list
      pNewSection = RemoveHeadFromStrategicPath(pNewSection);
    }

    // append onto old list
    pNode->pNext = pNewSection;
    pNewSection->pPrev = pNode;

  } else {
    // head of list becomes head of new section
    pHeadOfPathList = pNewSection;
  }

  // return head of new list
  return (pHeadOfPathList);
}

struct path* ClearStrategicPathList(struct path* pHeadOfPath, int16_t sMvtGroup) {
  // will clear out a strategic path and return head of list as NULL
  struct path* pNode = pHeadOfPath;
  struct path* pDeleteNode = pHeadOfPath;

  // is there in fact a path?
  if (pNode == NULL) {
    // no path, leave
    return (pNode);
  }

  // clear list
  while (pNode->pNext) {
    // set up delete node
    pDeleteNode = pNode;

    // move to next node
    pNode = pNode->pNext;

    // delete delete node
    MemFree(pDeleteNode);
  }

  // clear out last node
  MemFree(pNode);

  pNode = NULL;
  pDeleteNode = NULL;

  if ((sMvtGroup != -1) && (sMvtGroup != 0)) {
    // clear this groups mvt pathing
    RemoveGroupWaypoints((uint8_t)sMvtGroup);
  }

  return (pNode);
}

struct path* ClearStrategicPathListAfterThisSector(struct path* pHeadOfPath, uint8_t sX, uint8_t sY,
                                                   int16_t sMvtGroup) {
  // will clear out a strategic path and return head of list as NULL
  struct path* pNode = pHeadOfPath;
  struct path* pDeleteNode = pHeadOfPath;
  int16_t sSector = 0;
  int16_t sCurrentSector = -1;

  // is there in fact a path?
  if (pNode == NULL) {
    // no path, leave
    return (pNode);
  }

  // get sector value
  sSector = GetSectorID16(sX, sY);

  // go to end of list
  pNode = MoveToEndOfPathList(pNode);

  // get current sector value
  sCurrentSector = (int16_t)pNode->uiSectorId;

  // move through list
  while ((pNode) && (sSector != sCurrentSector)) {
    // next value
    pNode = pNode->pPrev;

    // get current sector value
    if (pNode != NULL) {
      sCurrentSector = (int16_t)pNode->uiSectorId;
    }
  }

  // did we find the target sector?
  if (pNode == NULL) {
    // nope, leave
    return (pHeadOfPath);
  }

  // we want to KEEP the target sector, not delete it, so advance to the next sector
  pNode = pNode->pNext;

  // is nothing left?
  if (pNode == NULL) {
    // that's it, leave
    return (pHeadOfPath);
  }

  // if we're NOT about to clear the head (there's a previous entry)
  if (pNode->pPrev) {
    // set next for tail to NULL
    pNode->pPrev->pNext = NULL;
  } else {
    // clear head, return NULL
    pHeadOfPath = ClearStrategicPathList(pHeadOfPath, sMvtGroup);

    return (NULL);
  }

  // clear list
  while (pNode->pNext) {
    // set up delete node
    pDeleteNode = pNode;

    // move to next node
    pNode = pNode->pNext;

    // check if we are clearing the head of the list
    if (pDeleteNode == pHeadOfPath) {
      // null out head
      pHeadOfPath = NULL;
    }

    // delete delete node
    MemFree(pDeleteNode);
  }

  // clear out last node
  MemFree(pNode);
  pNode = NULL;
  pDeleteNode = NULL;

  return (pHeadOfPath);
}

struct path* MoveToBeginningOfPathList(struct path* pList) {
  // move to beginning of this list

  // no list, return
  if (pList == NULL) {
    return (NULL);
  }

  // move to beginning of list
  while (pList->pPrev) {
    pList = pList->pPrev;
  }

  return (pList);
}

struct path* MoveToEndOfPathList(struct path* pList) {
  // move to end of list

  // no list, return
  if (pList == NULL) {
    return (NULL);
  }

  // move to beginning of list
  while (pList->pNext) {
    pList = pList->pNext;
  }

  return (pList);
}

struct path* RemoveTailFromStrategicPath(struct path* pHeadOfList) {
  // remove the tail section from the strategic path
  struct path* pNode = pHeadOfList;
  struct path* pLastNode = pHeadOfList;

  if (pNode == NULL) {
    // no list, leave
    return (NULL);
  }

  while (pNode->pNext) {
    pLastNode = pNode;
    pNode = pNode->pNext;
  }

  // end of list

  // set next to null
  pLastNode->pNext = NULL;

  // now remove old last node
  MemFree(pNode);

  // return head of new list
  return (pHeadOfList);
}

struct path* RemoveHeadFromStrategicPath(struct path* pList) {
  // move to head of list
  struct path* pNode = pList;
  struct path* pNewHead = pList;

  // check if there is a list
  if (pNode == NULL) {
    // no list, leave
    return (NULL);
  }

  // move to head of list
  while (pNode->pPrev) {
    // back one node
    pNode = pNode->pPrev;
  }

  // set up new head
  pNewHead = pNode->pNext;
  if (pNewHead) {
    pNewHead->pPrev = NULL;
  }

  // free old head
  MemFree(pNode);

  pNode = NULL;

  // return new head
  return (pNewHead);
}

struct path* RemoveSectorFromStrategicPathList(struct path* pList, uint8_t sX, uint8_t sY) {
  // find sector sX, sY ...then remove it
  int16_t sSector = 0;
  int16_t sCurrentSector = -1;
  struct path* pNode = pList;
  struct path* pPastNode = pList;

  // get sector value
  sSector = GetSectorID16(sX, sY);

  // check if there is a valid list
  if (pNode == NULL) {
    return (pNode);
  }

  // get current sector value
  sCurrentSector = (int16_t)pNode->uiSectorId;

  // move to end of list
  pNode = MoveToEndOfPathList(pNode);

  // move through list
  while ((pNode) && (sSector != sCurrentSector)) {
    // set past node up
    pPastNode = pNode;

    // next value
    pNode = pNode->pPrev;

    // get current sector value
    sCurrentSector = (int16_t)pNode->uiSectorId;
  }

  // no list left, sector not found
  if (pNode == NULL) {
    return (NULL);
  }

  // sector found...remove it
  pPastNode->pNext = pNode->pNext;

  // remove node
  MemFree(pNode);

  // set up prev for next
  pPastNode->pNext->pPrev = pPastNode;

  pPastNode = MoveToBeginningOfPathList(pPastNode);

  return (pPastNode);
}

int16_t GetLastSectorIdInCharactersPath(struct SOLDIERTYPE* pCharacter) {
  // will return the last sector of the current path, or the current sector if there's no path
  int16_t sLastSector = (GetSolSectorX(pCharacter)) + (GetSolSectorY(pCharacter)) * (MAP_WORLD_X);
  struct path* pNode = NULL;

  pNode = GetSoldierMercPathPtr(pCharacter);

  while (pNode) {
    sLastSector = (int16_t)(pNode->uiSectorId);
    pNode = pNode->pNext;
  }

  return sLastSector;
}

// get id of last sector in vehicle path list
int16_t GetLastSectorIdInVehiclePath(int32_t iId) {
  int16_t sLastSector = -1;
  struct path* pNode = NULL;

  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (sLastSector);
  }
  // now check if vehicle is valid
  if (pVehicleList[iId].fValid == FALSE) {
    return (sLastSector);
  }

  // get current last sector
  sLastSector = (pVehicleList[iId].sSectorX) + (pVehicleList[iId].sSectorY * MAP_WORLD_X);

  pNode = pVehicleList[iId].pMercPath;

  while (pNode) {
    sLastSector = (int16_t)(pNode->uiSectorId);
    pNode = pNode->pNext;
  }

  return sLastSector;
}

struct path* CopyPaths(struct path* pSourcePath, struct path* pDestPath) {
  struct path* pDestNode = pDestPath;
  struct path* pCurNode = pSourcePath;
  // copies path from source to dest

  // NULL out dest path
  pDestNode = ClearStrategicPathList(pDestNode, -1);
  Assert(pDestNode == NULL);

  // start list off
  if (pCurNode != NULL) {
    pDestNode = (struct path*)MemAlloc(sizeof(struct path));

    // set next and prev nodes
    pDestNode->pPrev = NULL;
    pDestNode->pNext = NULL;

    // copy sector value and times
    pDestNode->uiSectorId = pCurNode->uiSectorId;
    pDestNode->uiEta = pCurNode->uiEta;
    pDestNode->fSpeed = pCurNode->fSpeed;

    pCurNode = pCurNode->pNext;
  }

  while (pCurNode != NULL) {
    pDestNode->pNext = (struct path*)MemAlloc(sizeof(struct path));

    // set next's previous to current
    pDestNode->pNext->pPrev = pDestNode;

    // set next's next to null
    pDestNode->pNext->pNext = NULL;

    // increment ptr
    pDestNode = pDestNode->pNext;

    // copy sector value and times
    pDestNode->uiSectorId = pCurNode->uiSectorId;
    pDestNode->uiEta = pCurNode->uiEta;
    pDestNode->fSpeed = pCurNode->fSpeed;

    pCurNode = pCurNode->pNext;
  }

  // move back to beginning fo list
  pDestNode = MoveToBeginningOfPathList(pDestNode);

  // return to head of path
  return (pDestNode);
}

int32_t GetStrategicMvtSpeed(struct SOLDIERTYPE* pCharacter) {
  // will return the strategic speed of the character
  int32_t iSpeed;

  // avg of strength and agility * percentage health..very simple..replace later

  iSpeed = (int32_t)((pCharacter->bAgility + pCharacter->bStrength) / 2);
  iSpeed *= (int32_t)((pCharacter->bLife));
  iSpeed /= (int32_t)pCharacter->bLifeMax;

  return (iSpeed);
}

#ifdef BETA_VERSION
void VerifyAllMercsInGroupAreOnSameSquad(struct GROUP* pGroup) {
  PLAYERGROUP* pPlayer;
  struct SOLDIERTYPE* pSoldier;
  int8_t bSquad = -1;

  // Let's choose somebody in group.....
  pPlayer = pGroup->pPlayerList;

  while (pPlayer != NULL) {
    pSoldier = pPlayer->pSoldier;
    Assert(pSoldier);

    if (pSoldier->bAssignment < ON_DUTY) {
      if (bSquad == -1) {
        bSquad = pSoldier->bAssignment;
      } else {
        // better be the same squad!
        Assert(GetSolAssignment(pSoldier) == bSquad);
      }
    }

    pPlayer = pPlayer->next;
  }
}
#endif

void RebuildWayPointsForGroupPath(struct path* pHeadOfPath, int16_t sMvtGroup) {
  int32_t iDelta = 0;
  int32_t iOldDelta = 0;
  BOOLEAN fFirstNode = TRUE;
  struct path* pNode = pHeadOfPath;
  struct GROUP* pGroup = NULL;
  WAYPOINT* wp = NULL;

  if ((sMvtGroup == -1) || (sMvtGroup == 0)) {
    // invalid group...leave
    return;
  }

  pGroup = GetGroup((uint8_t)sMvtGroup);

  // KRIS!  Added this because it was possible to plot a new course to the same destination, and the
  //       group would add new arrival events without removing the existing one(s).
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, sMvtGroup);

  RemoveGroupWaypoints((uint8_t)sMvtGroup);

  if (pGroup->fPlayer) {
#ifdef BETA_VERSION
    VerifyAllMercsInGroupAreOnSameSquad(pGroup);
#endif

    // update the destination(s) in the team list
    fTeamPanelDirty = TRUE;

    // update the ETA in character info
    fCharacterInfoPanelDirty = TRUE;

    // allows assignments to flash right away if their subject moves away/returns (robot/vehicle
    // being repaired), or patient/doctor/student/trainer being automatically put on a squad via the
    // movement menu.
    gfReEvaluateEveryonesNothingToDo = TRUE;
  }

  // if group has no path planned at all
  if ((pNode == NULL) || (pNode->pNext == NULL)) {
    // and it's a player group, and it's between sectors
    // NOTE: AI groups never reverse direction between sectors, Kris cheats & teleports them back to
    // their current sector!
    if (pGroup->fPlayer && pGroup->fBetweenSectors) {
      // send the group right back to its current sector by reversing directions
      GroupReversingDirectionsBetweenSectors(pGroup, pGroup->ubSectorX, pGroup->ubSectorY, FALSE);
    }

    return;
  }

  // if we're currently between sectors
  if (pGroup->fBetweenSectors) {
    // figure out which direction we're already going in  (Otherwise iOldDelta starts at 0)
    iOldDelta = GetSectorID16(pGroup->ubNextX, pGroup->ubNextY) -
                GetSectorID16(pGroup->ubSectorX, pGroup->ubSectorY);
  }

  // build a brand new list of waypoints, one for initial direction, and another for every
  // "direction change" thereafter
  while (pNode->pNext) {
    iDelta = pNode->pNext->uiSectorId - pNode->uiSectorId;
    Assert(iDelta != 0);  // same sector should never repeat in the path list

    // Waypoints are only added at "pivot points" - whenever there is a change in orthogonal
    // direction. If we're NOT currently between sectors, iOldDelta will start off at 0, which means
    // that the first node can't be added as a waypoint.  This is what we want - for stationary
    // mercs, the first node in a path is the CURRENT sector.
    if ((iOldDelta != 0) && (iDelta != iOldDelta)) {
      // add this strategic sector as a waypoint
      AddWaypointStrategicIDToPGroup(pGroup, pNode->uiSectorId);
    }

    // remember this delta
    iOldDelta = iDelta;

    pNode = pNode->pNext;
    fFirstNode = FALSE;
  }

  // there must have been at least one next node, or we would have bailed out on "no path" earlier
  Assert(!fFirstNode);

  // the final destination sector - always add a waypoint for it
  AddWaypointStrategicIDToPGroup(pGroup, pNode->uiSectorId);

  // at this point, the final sector in the path must be identical to this group's last waypoint
  wp = GetFinalWaypoint(pGroup);
  AssertMsg(wp, "Path exists, but no waypoints were added!  AM-0");
  AssertMsg(pNode->uiSectorId == (uint32_t)GetSectorID16(wp->x, wp->y),
            "Last waypoint differs from final path sector!  AM-0");

  // see if we've already reached the first sector in the path (we never actually left the sector
  // and reversed back to it)
  if (pGroup->uiArrivalTime == GetGameTimeInMin()) {
    // never really left.  Must set check for battle TRUE in order for HandleNonCombatGroupArrival()
    // to run!
    GroupArrivedAtSector(pGroup->ubGroupID, TRUE, TRUE);
  }
}

// clear strategic movement (mercpaths and waypoints) for this soldier, and his group (including its
// vehicles)
void ClearMvtForThisSoldierAndGang(struct SOLDIERTYPE* pSoldier) {
  struct GROUP* pGroup = NULL;

  // check if valid grunt
  Assert(pSoldier);

  pGroup = GetGroup(pSoldier->ubGroupID);
  Assert(pGroup);

  // clear their strategic movement (mercpaths and waypoints)
  ClearMercPathsAndWaypointsForAllInGroup(pGroup);
}

BOOLEAN MoveGroupFromSectorToSector(uint8_t ubGroupID, uint8_t sStartX, uint8_t sStartY,
                                    uint8_t sDestX, uint8_t sDestY) {
  struct path* pNode = NULL;

  // build the path
  pNode = BuildAStrategicPath(pNode, GetSectorID16(sStartX, sStartY), GetSectorID16(sDestX, sDestY),
                              ubGroupID, FALSE /*, FALSE */);

  if (pNode == NULL) {
    return (FALSE);
  }

  pNode = MoveToBeginningOfPathList(pNode);

  // start movement to next sector
  RebuildWayPointsForGroupPath(pNode, ubGroupID);

  // now clear out the mess
  pNode = ClearStrategicPathList(pNode, -1);

  return (TRUE);
}

BOOLEAN MoveGroupFromSectorToSectorButAvoidLastSector(uint8_t ubGroupID, uint8_t sStartX,
                                                      uint8_t sStartY, uint8_t sDestX,
                                                      uint8_t sDestY) {
  struct path* pNode = NULL;

  // build the path
  pNode = BuildAStrategicPath(pNode, GetSectorID16(sStartX, sStartY), GetSectorID16(sDestX, sDestY),
                              ubGroupID, FALSE /*, FALSE*/);

  if (pNode == NULL) {
    return (FALSE);
  }

  // remove tail from path
  pNode = RemoveTailFromStrategicPath(pNode);

  pNode = MoveToBeginningOfPathList(pNode);

  // start movement to next sector
  RebuildWayPointsForGroupPath(pNode, ubGroupID);

  // now clear out the mess
  pNode = ClearStrategicPathList(pNode, -1);

  return (TRUE);
}

BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(uint8_t ubGroupID,
                                                                   uint8_t sStartX, uint8_t sStartY,
                                                                   uint8_t sDestX, uint8_t sDestY) {
  struct path* pNode = NULL;

  // init sectors with soldiers in them
  InitSectorsWithSoldiersList();

  // build the list of sectors with soldier in them
  BuildSectorsWithSoldiersList();

  // turn on the avoid flag
  gfPlotToAvoidPlayerInfuencedSectors = TRUE;

  // build the path
  pNode = BuildAStrategicPath(pNode, GetSectorID16(sStartX, sStartY), GetSectorID16(sDestX, sDestY),
                              ubGroupID, FALSE /*, FALSE */);

  // turn off the avoid flag
  gfPlotToAvoidPlayerInfuencedSectors = FALSE;

  if (pNode == NULL) {
    if (MoveGroupFromSectorToSector(ubGroupID, sStartX, sStartY, sDestX, sDestY) == FALSE) {
      return (FALSE);
    } else {
      return (TRUE);
    }
  }

  pNode = MoveToBeginningOfPathList(pNode);

  // start movement to next sector
  RebuildWayPointsForGroupPath(pNode, ubGroupID);

  // now clear out the mess
  pNode = ClearStrategicPathList(pNode, -1);

  return (TRUE);
}

BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(
    uint8_t ubGroupID, uint8_t sStartX, uint8_t sStartY, uint8_t sDestX, uint8_t sDestY) {
  struct path* pNode = NULL;

  // init sectors with soldiers in them
  InitSectorsWithSoldiersList();

  // build the list of sectors with soldier in them
  BuildSectorsWithSoldiersList();

  // turn on the avoid flag
  gfPlotToAvoidPlayerInfuencedSectors = TRUE;

  // build the path
  pNode = BuildAStrategicPath(pNode, GetSectorID16(sStartX, sStartY), GetSectorID16(sDestX, sDestY),
                              ubGroupID, FALSE /*, FALSE */);

  // turn off the avoid flag
  gfPlotToAvoidPlayerInfuencedSectors = FALSE;

  if (pNode == NULL) {
    if (MoveGroupFromSectorToSectorButAvoidLastSector(ubGroupID, sStartX, sStartY, sDestX,
                                                      sDestY) == FALSE) {
      return (FALSE);
    } else {
      return (TRUE);
    }
  }

  // remove tail from path
  pNode = RemoveTailFromStrategicPath(pNode);

  pNode = MoveToBeginningOfPathList(pNode);

  // start movement to next sector
  RebuildWayPointsForGroupPath(pNode, ubGroupID);

  // now clear out the mess
  pNode = ClearStrategicPathList(pNode, -1);

  return (TRUE);
}

/*
BOOLEAN MoveGroupToOriginalSector( uint8_t ubGroupID )
{
        struct GROUP *pGroup;
        uint8_t ubDestX, ubDestY;
        pGroup = GetGroup( ubGroupID );
        ubDestX = ( pGroup->ubOriginalSector % 16 ) + 1;
        ubDestY = ( pGroup->ubOriginalSector / 16 ) + 1;
        MoveGroupFromSectorToSector( ubGroupID, pGroup->ubSectorX, pGroup->ubSectorY, ubDestX,
ubDestY );

        return( TRUE );
}
*/

int32_t GetLengthOfPath(struct path* pHeadPath) {
  int32_t iLength = 0;
  struct path* pNode = pHeadPath;

  while (pNode) {
    pNode = pNode->pNext;
    iLength++;
  }

  return (iLength);
}

int32_t GetLengthOfMercPath(struct SOLDIERTYPE* pSoldier) {
  struct path* pNode = NULL;
  int32_t iLength = 0;

  pNode = GetSoldierMercPathPtr(pSoldier);
  iLength = GetLengthOfPath(pNode);
  return (iLength);
}

BOOLEAN CheckIfPathIsEmpty(struct path* pHeadPath) {
  // no path
  if (pHeadPath == NULL) {
    return (TRUE);
  }

  // nothing next either
  if (pHeadPath->pNext == NULL) {
    return (TRUE);
  }

  return (FALSE);
}

struct path* GetSoldierMercPathPtr(struct SOLDIERTYPE* pSoldier) {
  struct path* pMercPath = NULL;

  Assert(pSoldier);

  // IN a vehicle?
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    pMercPath = pVehicleList[pSoldier->iVehicleId].pMercPath;
  }
  // IS a vehicle?
  else if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    pMercPath = pVehicleList[pSoldier->bVehicleID].pMercPath;
  } else  // a person
  {
    pMercPath = pSoldier->pMercPath;
  }

  return (pMercPath);
}

struct path* GetGroupMercPathPtr(struct GROUP* pGroup) {
  struct path* pMercPath = NULL;
  int32_t iVehicledId = -1;

  Assert(pGroup);

  // must be a player group!
  Assert(pGroup->fPlayer);

  if (pGroup->fVehicle) {
    iVehicledId = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
    Assert(iVehicledId != -1);

    pMercPath = pVehicleList[iVehicledId].pMercPath;
  } else {
    // value returned will be NULL if there's nobody in the group!
    if (pGroup->pPlayerList && pGroup->pPlayerList->pSoldier) {
      pMercPath = pGroup->pPlayerList->pSoldier->pMercPath;
    }
  }

  return (pMercPath);
}

uint8_t GetSoldierGroupId(struct SOLDIERTYPE* pSoldier) {
  uint8_t ubGroupId = 0;

  // IN a vehicle?
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    ubGroupId = pVehicleList[pSoldier->iVehicleId].ubMovementGroup;
  }
  // IS a vehicle?
  else if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    ubGroupId = pVehicleList[pSoldier->bVehicleID].ubMovementGroup;
  } else  // a person
  {
    ubGroupId = pSoldier->ubGroupID;
  }

  return (ubGroupId);
}

// clears this groups strategic movement (mercpaths and waypoints), include those in the vehicle
// structs(!)
void ClearMercPathsAndWaypointsForAllInGroup(struct GROUP* pGroup) {
  PLAYERGROUP* pPlayer = NULL;
  struct SOLDIERTYPE* pSoldier = NULL;

  pPlayer = pGroup->pPlayerList;
  while (pPlayer) {
    pSoldier = pPlayer->pSoldier;

    if (pSoldier != NULL) {
      ClearPathForSoldier(pSoldier);
    }

    pPlayer = pPlayer->next;
  }

  // if it's a vehicle
  if (pGroup->fVehicle) {
    int32_t iVehicleId = -1;
    VEHICLETYPE* pVehicle = NULL;

    iVehicleId = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
    Assert(iVehicleId != -1);

    pVehicle = &(pVehicleList[iVehicleId]);

    // clear the path for that vehicle
    pVehicle->pMercPath = ClearStrategicPathList(pVehicle->pMercPath, pVehicle->ubMovementGroup);
  }

  // clear the waypoints for this group too - no mercpath = no waypoints!
  RemovePGroupWaypoints(pGroup);
  // not used anymore
  // SetWayPointsAsCanceled( pCurrentMerc->ubGroupID );
}

// clears the contents of the soldier's mercpPath, as well as his vehicle path if he is a / or is in
// a vehicle
void ClearPathForSoldier(struct SOLDIERTYPE* pSoldier) {
  VEHICLETYPE* pVehicle = NULL;

  // clear the soldier's mercpath
  pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, pSoldier->ubGroupID);

  // if a vehicle
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    pVehicle = &(pVehicleList[pSoldier->bVehicleID]);
  }
  // or in a vehicle
  else if (GetSolAssignment(pSoldier) == VEHICLE) {
    pVehicle = &(pVehicleList[pSoldier->iVehicleId]);
  }

  // if there's an associate vehicle structure
  if (pVehicle != NULL) {
    // clear its mercpath, too
    pVehicle->pMercPath = ClearStrategicPathList(pVehicle->pMercPath, pVehicle->ubMovementGroup);
  }
}

void AddSectorToFrontOfMercPathForAllSoldiersInGroup(struct GROUP* pGroup, uint8_t ubSectorX,
                                                     uint8_t ubSectorY) {
  PLAYERGROUP* pPlayer = NULL;
  struct SOLDIERTYPE* pSoldier = NULL;

  pPlayer = pGroup->pPlayerList;
  while (pPlayer) {
    pSoldier = pPlayer->pSoldier;

    if (pSoldier != NULL) {
      AddSectorToFrontOfMercPath(&(pSoldier->pMercPath), ubSectorX, ubSectorY);
    }

    pPlayer = pPlayer->next;
  }

  // if it's a vehicle
  if (pGroup->fVehicle) {
    int32_t iVehicleId = -1;
    VEHICLETYPE* pVehicle = NULL;

    iVehicleId = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
    Assert(iVehicleId != -1);

    pVehicle = &(pVehicleList[iVehicleId]);

    // add it to that vehicle's path
    AddSectorToFrontOfMercPath(&(pVehicle->pMercPath), ubSectorX, ubSectorY);
  }
}

void AddSectorToFrontOfMercPath(struct path** ppMercPath, uint8_t ubSectorX, uint8_t ubSectorY) {
  struct path* pNode = NULL;

  // allocate and hang a new node at the front of the path list
  pNode = (struct path*)MemAlloc(sizeof(struct path));

  pNode->uiSectorId = GetSectorID16(ubSectorX, ubSectorY);
  pNode->pNext = *ppMercPath;
  pNode->pPrev = NULL;
  pNode->uiEta = GetGameTimeInMin();
  pNode->fSpeed = NORMAL_MVT;

  // if path wasn't null
  if (*ppMercPath != NULL) {
    // hang the previous pointer of the old head to the new head
    (*ppMercPath)->pPrev = pNode;
  }

  *ppMercPath = pNode;
}
