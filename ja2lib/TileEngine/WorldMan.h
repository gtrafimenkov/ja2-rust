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
BOOLEAN RemoveObject(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddObjectToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddObjectToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusObjectIndex);
BOOLEAN RemoveAllObjectsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllObjectShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel);
void AdjustAllObjectShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff);
BOOLEAN TypeRangeExistsInObjectLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                     uint16_t *pusObjectIndex);

// Roof manipulation functions
BOOLEAN RemoveRoof(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddRoofToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddRoofToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusRoofIndex);
BOOLEAN RemoveAllRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllRoofShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel);
void AdjustAllRoofShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff);
void RemoveRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                       uint32_t uiFlags);
void SetRoofIndexFlagsFromTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                    uint32_t uiFlags);
BOOLEAN TypeRangeExistsInRoofLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   uint16_t *pusRoofIndex);
void SetWallLevelnodeFlags(uint16_t sGridNo, uint32_t uiFlags);
void RemoveWallLevelnodeFlags(uint16_t sGridNo, uint32_t uiFlags);
BOOLEAN IndexExistsInRoofLayer(INT16 sGridNo, uint16_t usIndex);

// OnRoof manipulation functions
BOOLEAN RemoveOnRoof(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddOnRoofToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddOnRoofToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInOnRoofLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusOnRoofIndex);
BOOLEAN RemoveAllOnRoofsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
void SetAllOnRoofShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel);
void AdjustAllOnRoofShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff);
BOOLEAN RemoveOnRoofFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

// Land manipulation functions
BOOLEAN RemoveLand(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddLandToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddLandToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInLandLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusLandIndex);
BOOLEAN RemoveAllLandsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN TypeRangeExistsInLandLayer(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                   uint16_t *pusLandIndex);
BOOLEAN TypeRangeExistsInLandHead(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType,
                                  uint16_t *pusLandIndex);
BOOLEAN ReplaceLandIndex(uint32_t iMapIndex, uint16_t usOldIndex, uint16_t usNewIndex);
BOOLEAN DeleteAllLandLayers(uint32_t iMapIndex);
BOOLEAN InsertLandIndexAtLevel(uint32_t iMapIndex, uint16_t usIndex, uint8_t ubLevel);
BOOLEAN RemoveHigherLandLevels(uint32_t iMapIndex, uint32_t fSrcType, uint32_t **puiHigherTypes,
                               uint8_t *pubNumHigherTypes);
BOOLEAN SetLowerLandLevels(uint32_t iMapIndex, uint32_t fSrcType, uint16_t usIndex);
BOOLEAN AdjustForFullTile(uint32_t iMapIndex);
void SetAllLandShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel);
void AdjustAllLandShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff);
void AdjustAllLandDirtyCount(uint32_t iMapIndex, int8_t bDirtyDiff);
uint8_t GetTerrainType(INT16 sGridNo);
BOOLEAN Water(INT16 sGridNo);
BOOLEAN DeepWater(INT16 sGridNo);
BOOLEAN WaterTooDeepForAttacks(INT16 sGridNo);

// Structure manipulation routines
BOOLEAN RemoveStruct(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddStructToTail(uint32_t iMapIndex, uint16_t usIndex);
struct LEVELNODE *AddStructToTailCommon(uint32_t iMapIndex, uint16_t usIndex,
                                        BOOLEAN fAddStructDBInfo);
struct LEVELNODE *ForceStructToTail(uint32_t iMapIndex, uint16_t usIndex);

BOOLEAN AddStructToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInStructLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusStructIndex);
BOOLEAN RemoveAllStructsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN AddWallToStructLayer(INT32 iMapIndex, uint16_t usIndex, BOOLEAN fReplace);
BOOLEAN ReplaceStructIndex(uint32_t iMapIndex, uint16_t usOldIndex, uint16_t usNewIndex);
BOOLEAN HideStructOfGivenType(uint32_t iMapIndex, uint32_t fType, BOOLEAN fHide);
BOOLEAN InsertStructIndex(uint32_t iMapIndex, uint16_t usIndex, uint8_t ubLevel);
void SetAllStructShadeLevels(uint32_t iMapIndex, uint8_t ubShadeLevel);
void AdjustAllStructShadeLevels(uint32_t iMapIndex, int8_t bShadeDiff);
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
                                     uint16_t *pusStructIndex);

// Shadow manipulation routines
BOOLEAN RemoveShadow(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddShadowToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddShadowToHead(uint32_t iMapIndex, uint16_t usIndex);
void AddExclusiveShadow(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInShadowLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusShadowIndex);
BOOLEAN RemoveAllShadowsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN RemoveAllShadows(uint32_t iMapIndex);
BOOLEAN RemoveShadowFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

// Merc manipulation routines
// #################################################################

BOOLEAN AddMercToHead(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fAddStructInfo);
BOOLEAN RemoveMerc(uint32_t iMapIndex, struct SOLDIERTYPE *pSoldier, BOOLEAN fPlaceHolder);
uint8_t WhoIsThere2(INT16 sGridNo, int8_t bLevel);
BOOLEAN AddMercStructureInfo(INT16 sGridNo, struct SOLDIERTYPE *pSoldier);
BOOLEAN AddMercStructureInfoFromAnimSurface(INT16 sGridNo, struct SOLDIERTYPE *pSoldier,
                                            uint16_t usAnimSurface, uint16_t usAnimState);
BOOLEAN UpdateMercStructureInfo(struct SOLDIERTYPE *pSoldier);
BOOLEAN OKToAddMercToWorld(struct SOLDIERTYPE *pSoldier, int8_t bDirection);

// TOPMOST manipulation functions
struct LEVELNODE *AddTopmostToTail(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN AddTopmostToHead(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN RemoveTopmost(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN TypeExistsInTopmostLayer(uint32_t iMapIndex, uint32_t fType, uint16_t *pusTopmostIndex);
BOOLEAN RemoveAllTopmostsOfTypeRange(uint32_t iMapIndex, uint32_t fStartType, uint32_t fEndType);
BOOLEAN SetMapElementShadeLevel(uint32_t uiMapIndex, uint8_t ubShadeLevel);
void SetTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, uint16_t usIndex);
void RemoveTopmostFlags(uint32_t iMapIndex, uint32_t uiFlags, uint16_t usIndex);
BOOLEAN AddUIElem(uint32_t iMapIndex, uint16_t usIndex, int8_t sRelativeX, int8_t sRelativeY,
                  struct LEVELNODE **ppNewNode);
void RemoveUIElem(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN RemoveTopmostFromLevelNode(uint32_t iMapIndex, struct LEVELNODE *pNode);

BOOLEAN IsLowerLevel(INT16 sGridNo);
BOOLEAN IsHeigherLevel(INT16 sGridNo);
BOOLEAN IsRoofVisible(INT16 sMapPos);
BOOLEAN IsRoofVisible2(INT16 sMapPos);

struct LEVELNODE *FindLevelNodeBasedOnStructure(INT16 sGridNo, struct STRUCTURE *pStructure);
struct LEVELNODE *FindShadow(INT16 sGridNo, uint16_t usStructIndex);

void WorldHideTrees();
void WorldShowTrees();

BOOLEAN IsTileRedundent(uint16_t *pZBuffer, uint16_t usZValue, struct VObject *hSrcVObject,
                        INT32 iX, INT32 iY, uint16_t usIndex);

// this is found in editscreen.c
// Andrew, you had worldman.c checked out at the time, so I stuck it here.
// The best thing to do is toast it here, and include editscreen.h in worldman.c.
extern uint32_t gCurrentBackground;

void SetTreeTopStateForMap();

#endif
