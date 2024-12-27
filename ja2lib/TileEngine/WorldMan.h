#ifndef __WORLDMAN_H_
#define __WORLDMAN_H_

#include "SGP/Types.h"

struct LEVELNODE;
struct SOLDIERTYPE;
struct STRUCTURE;
struct VObject;

// memory-accounting function
void CountLevelNodes(void);

// Object manipulation functions
BOOLEAN RemoveObject(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddObjectToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddObjectToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusObjectIndex);
BOOLEAN RemoveAllObjectsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllObjectShadeLevels(uint32_t iMapIndex, UINT8 ubShadeLevel);
void AdjustAllObjectShadeLevels(uint32_t iMapIndex, INT8 bShadeDiff);
BOOLEAN TypeRangeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                     UINT16 *pusObjectIndex);

// Roof manipulation functions
BOOLEAN RemoveRoof(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddRoofToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddRoofToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusRoofIndex);
BOOLEAN RemoveAllRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllRoofShadeLevels(uint32_t iMapIndex, UINT8 ubShadeLevel);
void AdjustAllRoofShadeLevels(uint32_t iMapIndex, INT8 bShadeDiff);
void RemoveRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                       uint32_t uiFlags);
void SetRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                    uint32_t uiFlags);
BOOLEAN TypeRangeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   UINT16 *pusRoofIndex);
void SetWallLevelnodeFlags(UINT16 sGridNo, uint32_t uiFlags);
void RemoveWallLevelnodeFlags(UINT16 sGridNo, uint32_t uiFlags);
BOOLEAN IndexExistsInRoofLayer(INT16 sGridNo, UINT16 usIndex);

// OnRoof manipulation functions
BOOLEAN RemoveOnRoof(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddOnRoofToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddOnRoofToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInOnRoofLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusOnRoofIndex);
BOOLEAN RemoveAllOnRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllOnRoofShadeLevels(uint32_t iMapIndex, UINT8 ubShadeLevel);
void AdjustAllOnRoofShadeLevels(uint32_t iMapIndex, INT8 bShadeDiff);
BOOLEAN RemoveOnRoofFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

// Land manipulation functions
BOOLEAN RemoveLand(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddLandToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddLandToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInLandLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusLandIndex);
BOOLEAN RemoveAllLandsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN TypeRangeExistsInLandLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   UINT16 *pusLandIndex);
BOOLEAN TypeRangeExistsInLandHead(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                  UINT16 *pusLandIndex);
BOOLEAN ReplaceLandIndex(uint32_t iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex);
BOOLEAN DeleteAllLandLayers(uint32_t iMapIndex);
BOOLEAN InsertLandIndexAtLevel(uint32_t iMapIndex, UINT16 usIndex, UINT8 ubLevel);
BOOLEAN RemoveHigherLandLevels(uint32_t iMapIndex, uint32_t fSrcType, uint32_t **puiHigherTypes,
                               UINT8 *pubNumHigherTypes);
BOOLEAN SetLowerLandLevels(uint32_t iMapIndex, uint32_t fSrcType, UINT16 usIndex);
BOOLEAN AdjustForFullTile(uint32_t iMapIndex);
void SetAllLandShadeLevels(uint32_t iMapIndex, UINT8 ubShadeLevel);
void AdjustAllLandShadeLevels(uint32_t iMapIndex, INT8 bShadeDiff);
void AdjustAllLandDirtyCount(uint32_t iMapIndex, INT8 bDirtyDiff);
UINT8 GetTerrainType(INT16 sGridNo);
BOOLEAN Water(INT16 sGridNo);
BOOLEAN DeepWater(INT16 sGridNo);
BOOLEAN WaterTooDeepForAttacks(INT16 sGridNo);

// Structure manipulation routines
BOOLEAN RemoveStruct(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddStructToTail(uint32_t iMapIndex, UINT16 usIndex);
struct LEVELNODE *AddStructToTailCommon(uint32_t iMapIndex, UINT16 usIndex,
                                        BOOLEAN fAddStructDBInfo);
struct LEVELNODE *ForceStructToTail(uint32_t iMapIndex, UINT16 usIndex);

BOOLEAN AddStructToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInStructLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusStructIndex);
BOOLEAN RemoveAllStructsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN AddWallToStructLayer(INT32 iMapIndex, UINT16 usIndex, BOOLEAN fReplace);
BOOLEAN ReplaceStructIndex(uint32_t iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex);
BOOLEAN HideStructOfGivenType(uint32_t iMapIndex, uint32_t fType, BOOLEAN fHide);
BOOLEAN InsertStructIndex(uint32_t iMapIndex, UINT16 usIndex, UINT8 ubLevel);
void SetAllStructShadeLevels(uint32_t iMapIndex, UINT8 ubShadeLevel);
void AdjustAllStructShadeLevels(uint32_t iMapIndex, INT8 bShadeDiff);
void SetStructIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                      uint32_t uiFlags);
void RemoveStructIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                         uint32_t uiFlags);
void SetStructAframeFlags(uint32_t iMapIndex, uint32_t uiFlags);
void RemoveStructAframeFlags(uint32_t iMapIndex, uint32_t uiFlags);
BOOLEAN RemoveStructFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

BOOLEAN RemoveStructFromTail(uint32_t iMapIndex);
BOOLEAN RemoveStructFromTailCommon(uint32_t iMapIndex, BOOLEAN fRemoveStructDBInfo);
BOOLEAN ForceRemoveStructFromTail(uint32_t iMapIndex);

BOOLEAN TypeRangeExistsInStructLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                     UINT16 *pusStructIndex);

// Shadow manipulation routines
BOOLEAN RemoveShadow(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddShadowToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddShadowToHead(uint32_t iMapIndex, UINT16 usIndex);
void AddExclusiveShadow(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInShadowLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusShadowIndex);
BOOLEAN RemoveAllShadowsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN RemoveAllShadows(uint32_t iMapIndex);
BOOLEAN RemoveShadowFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

// Merc manipulation routines
// #################################################################

BOOLEAN AddMercToHead(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fAddStructInfo);
BOOLEAN RemoveMerc(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fPlaceHolder);
UINT8 WhoIsThere2(INT16 sGridNo, INT8 bLevel);
BOOLEAN AddMercStructureInfo(INT16 sGridNo, struct SOLDIERTYPE *pSoldier);
BOOLEAN AddMercStructureInfoFromAnimSurface(INT16 sGridNo, struct SOLDIERTYPE *pSoldier,
                                            UINT16 usAnimSurface, UINT16 usAnimState);
BOOLEAN UpdateMercStructureInfo(struct SOLDIERTYPE *pSoldier);
BOOLEAN OKToAddMercToWorld(struct SOLDIERTYPE *pSoldier, INT8 bDirection);

// TOPMOST manipulation functions
struct LEVELNODE *AddTopmostToTail(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN AddTopmostToHead(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN RemoveTopmost(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN TypeExistsInTopmostLayer(uint32_t iMapIndex, uint32_t fType, UINT16 *pusTopmostIndex);
BOOLEAN RemoveAllTopmostsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN SetMapElementShadeLevel(uint32_t uiMapIndex, UINT8 ubShadeLevel);
void SetTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, UINT16 usIndex);
void RemoveTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, UINT16 usIndex);
BOOLEAN AddUIElem(uint32_t iMapIndex, UINT16 usIndex, INT8 sRelativeX, INT8 sRelativeY,
                  struct LEVELNODE **ppNewNode);
void RemoveUIElem(uint32_t iMapIndex, UINT16 usIndex);
BOOLEAN RemoveTopmostFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

BOOLEAN IsLowerLevel(INT16 sGridNo);
BOOLEAN IsHeigherLevel(INT16 sGridNo);
BOOLEAN IsRoofVisible(INT16 sMapPos);
BOOLEAN IsRoofVisible2(INT16 sMapPos);

struct LEVELNODE *FindLevelNodeBasedOnStructure(INT16 sGridNo, struct STRUCTURE *pStructure);
struct LEVELNODE *FindShadow(INT16 sGridNo, UINT16 usStructIndex);

void WorldHideTrees();
void WorldShowTrees();

BOOLEAN IsTileRedundent(UINT16 *pZBuffer, UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                        INT32 iY, UINT16 usIndex);

// this is found in editscreen.c
// Andrew, you had worldman.c checked out at the time, so I stuck it here.
// The best thing to do is toast it here, and include editscreen.h in worldman.c.
extern uint32_t gCurrentBackground;

void SetTreeTopStateForMap();

#endif
