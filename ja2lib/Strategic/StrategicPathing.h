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

int32_t FindStratPath(int16_t sStart, int16_t sDestination, int16_t sMvtGroupNumber,
                      BOOLEAN fTacticalTraversal);

BOOLEAN AddSectorToPathList(struct path* pPath, uint16_t uiSectorNum);

// build a stategic path
struct path* BuildAStrategicPath(struct path* pPath, int16_t iStartSectorNum, int16_t iEndSectorNum,
                                 int16_t sMvtGroupNumber,
                                 BOOLEAN fTacticalTraversal /*, BOOLEAN fTempPath */);

// append onto path list
struct path* AppendStrategicPath(struct path* pNewSection, struct path* pHeadOfPathList);

// clear out strategic path list
struct path* ClearStrategicPathList(struct path* pHeadOfPath, int16_t sMvtGroup);

// move to beginning of list
struct path* MoveToBeginningOfPathList(struct path* pList);

// move to end of path list
struct path* MoveToEndOfPathList(struct path* pList);

// remove tail of list
struct path* RemoveTailFromStrategicPath(struct path* pHeadOfList);

// remove head of list
struct path* RemoveHeadFromStrategicPath(struct path* pList);

// remove node with this value.. starting at end and working it's way back
struct path* RemoveSectorFromStrategicPathList(struct path* pList, uint8_t sX, uint8_t sY);

// clear out path list after/including this sector sX, sY..will start at end of path and work it's
// way back till sector is found...removes most recent sectors first
struct path* ClearStrategicPathListAfterThisSector(struct path* pHeadOfPath, uint8_t sX, uint8_t sY,
                                                   int16_t sMvtGroup);

// get id of last sector in mercs path list
int16_t GetLastSectorIdInCharactersPath(struct SOLDIERTYPE* pCharacter);

// get id of last sector in mercs path list
int16_t GetLastSectorIdInVehiclePath(int32_t iId);

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
uint32_t GetEtaGivenRoute( struct path* pPath );
*/

// rebuild way points for strategic mapscreen path changes
void RebuildWayPointsForGroupPath(struct path* pHeadOfPath, int16_t sMvtGroup);

// clear strategic movement (mercpaths and waypoints) for this soldier, and his group (including its
// vehicles)
void ClearMvtForThisSoldierAndGang(struct SOLDIERTYPE* pSoldier);

// start movement of this group to this sector...not to be used by the player merc groups.
BOOLEAN MoveGroupFromSectorToSector(uint8_t ubGroupID, uint8_t sStartX, uint8_t sStartY,
                                    uint8_t sDestX, uint8_t sDestY);

BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(uint8_t ubGroupID,
                                                                   uint8_t sStartX, uint8_t sStartY,
                                                                   uint8_t sDestX, uint8_t sDestY);
BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(
    uint8_t ubGroupID, uint8_t sStartX, uint8_t sStartY, uint8_t sDestX, uint8_t sDestY);

// get length of path
int32_t GetLengthOfPath(struct path* pHeadPath);
int32_t GetLengthOfMercPath(struct SOLDIERTYPE* pSoldier);

// is the path empty?
BOOLEAN CheckIfPathIsEmpty(struct path* pHeadPath);

struct path* GetSoldierMercPathPtr(struct SOLDIERTYPE* pSoldier);
struct path* GetGroupMercPathPtr(struct GROUP* pGroup);

uint8_t GetSoldierGroupId(struct SOLDIERTYPE* pSoldier);

// clears this groups strategic movement (mercpaths and waypoints), include those in the vehicle
// structs(!)
void ClearMercPathsAndWaypointsForAllInGroup(struct GROUP* pGroup);

void ClearPathForSoldier(struct SOLDIERTYPE* pSoldier);

void AddSectorToFrontOfMercPathForAllSoldiersInGroup(struct GROUP* pGroup, uint8_t ubSectorX,
                                                     uint8_t ubSectorY);

#endif
