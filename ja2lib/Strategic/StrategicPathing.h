#ifndef __STRATPATH_H
#define __STRATPATH_H

#include "Strategic/MapScreenInterfaceMap.h"

struct GROUP;

// directions of movement for badsector determination ( blocking off of a sector exit from foot or
// vehicle travel)

// Shortest Path Defines
#define NORTH_MOVE -18
#define EAST_MOVE 1
#define WEST_MOVE -1
#define SOUTH_MOVE 18

// Movement speed defines
#define NORMAL_MVT 1
#define SLOW_MVT 0

// movment modes
enum {
  MVT_MODE_AIR,
  MVT_MODE_VEHICLE,
  MVT_MODE_FOOT,
};

INT32 FindStratPath(INT16 sStart, INT16 sDestination, INT16 sMvtGroupNumber,
                    BOOLEAN fTacticalTraversal);

/*
BOOLEAN SectorIsBlockedFromVehicleExit( UINT16 sSectorDest, INT8 bToDirection  );
BOOLEAN SectorIsBlockedFromFootExit( UINT16 sSector, INT8 bToDirection );
BOOLEAN TravelBetweenSectorsIsBlockedFromVehicle( UINT16 sSourceSector, UINT16 sDestSector );
BOOLEAN TravelBetweenSectorsIsBlockedFromFoot( UINT16 sSourceSector, UINT16 sDestSector );
BOOLEAN CanThisMercMoveToThisSector( struct SOLDIERTYPE *pSoldier ,INT16 sX, INT16 sY );
void SetThisMercsSectorXYToTheseValues( struct SOLDIERTYPE *pSoldier ,INT16 sX, INT16 sY, UINT8
ubFromDirection);
*/
BOOLEAN AddSectorToPathList(struct path* pPath, UINT16 uiSectorNum);

// build a stategic path
struct path* BuildAStrategicPath(struct path* pPath, INT16 iStartSectorNum, INT16 iEndSectorNum,
                                 INT16 sMvtGroupNumber,
                                 BOOLEAN fTacticalTraversal /*, BOOLEAN fTempPath */);

// append onto path list
struct path* AppendStrategicPath(struct path* pNewSection, struct path* pHeadOfPathList);

// clear out strategic path list
struct path* ClearStrategicPathList(struct path* pHeadOfPath, INT16 sMvtGroup);

// move to beginning of list
struct path* MoveToBeginningOfPathList(struct path* pList);

// move to end of path list
struct path* MoveToEndOfPathList(struct path* pList);

// remove tail of list
struct path* RemoveTailFromStrategicPath(struct path* pHeadOfList);

// remove head of list
struct path* RemoveHeadFromStrategicPath(struct path* pList);

// remove node with this value.. starting at end and working it's way back
struct path* RemoveSectorFromStrategicPathList(struct path* pList, INT16 sX, INT16 sY);

// clear out path list after/including this sector sX, sY..will start at end of path and work it's
// way back till sector is found...removes most recent sectors first
struct path* ClearStrategicPathListAfterThisSector(struct path* pHeadOfPath, INT16 sX, INT16 sY,
                                                   INT16 sMvtGroup);

// get id of last sector in mercs path list
INT16 GetLastSectorIdInCharactersPath(struct SOLDIERTYPE* pCharacter);

// get id of last sector in mercs path list
INT16 GetLastSectorIdInVehiclePath(INT32 iId);

// copy paths
struct path* CopyPaths(struct path* pSourcePath, struct path* pDestPath);

// build eta's for characters path - no longer used
// void CalculateEtaForCharacterPath( struct SOLDIERTYPE *pCharacter );
/*
// move character along path
void MoveCharacterOnPath( struct SOLDIERTYPE *pCharacter );
// move the whole team
void MoveTeamOnFoot( void );

// get the final eta of this path to the last sector in it's list
UINT32 GetEtaGivenRoute( struct path* pPath );
*/

// rebuild way points for strategic mapscreen path changes
void RebuildWayPointsForGroupPath(struct path* pHeadOfPath, INT16 sMvtGroup);

// clear strategic movement (mercpaths and waypoints) for this soldier, and his group (including its
// vehicles)
void ClearMvtForThisSoldierAndGang(struct SOLDIERTYPE* pSoldier);

// start movement of this group to this sector...not to be used by the player merc groups.
BOOLEAN MoveGroupFromSectorToSector(UINT8 ubGroupID, INT16 sStartX, INT16 sStartY, INT16 sDestX,
                                    INT16 sDestY);

BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(UINT8 ubGroupID, INT16 sStartX,
                                                                   INT16 sStartY, INT16 sDestX,
                                                                   INT16 sDestY);
BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(
    UINT8 ubGroupID, INT16 sStartX, INT16 sStartY, INT16 sDestX, INT16 sDestY);

/*
BOOLEAN MoveGroupToOriginalSector( UINT8 ubGroupID );
*/

// get length of path
INT32 GetLengthOfPath(struct path* pHeadPath);
INT32 GetLengthOfMercPath(struct SOLDIERTYPE* pSoldier);

// is the path empty?
BOOLEAN CheckIfPathIsEmpty(struct path* pHeadPath);

struct path* GetSoldierMercPathPtr(struct SOLDIERTYPE* pSoldier);
struct path* GetGroupMercPathPtr(struct GROUP* pGroup);

UINT8 GetSoldierGroupId(struct SOLDIERTYPE* pSoldier);

// clears this groups strategic movement (mercpaths and waypoints), include those in the vehicle
// structs(!)
void ClearMercPathsAndWaypointsForAllInGroup(struct GROUP* pGroup);

void ClearPathForSoldier(struct SOLDIERTYPE* pSoldier);

void AddSectorToFrontOfMercPathForAllSoldiersInGroup(struct GROUP* pGroup, UINT8 ubSectorX,
                                                     UINT8 ubSectorY);

#endif
