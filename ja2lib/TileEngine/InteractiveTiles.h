#ifndef __INTERACTIVE_TILES_H
#define __INTERACTIVE_TILES_H

#include "SGP/Types.h"

struct LEVELNODE;
struct SOLDIERTYPE;
struct STRUCTURE;

#define INTTILE_DOOR_TILE_ONE 1

#define INTTILE_DOOR_OPENSPEED 70

#define INTILE_CHECK_FULL 1
#define INTILE_CHECK_SELECTIVE 2

extern BOOLEAN gfOverIntTile;

void GetLevelNodeScreenRect(struct LEVELNODE *pNode, struct GRect *pRect, INT16 sXPos, INT16 sYPos,
                            INT16 sGridNo);
BOOLEAN InitInteractiveTileManagement();
void ShutdownInteractiveTileManagement();
BOOLEAN AddInteractiveTile(INT16 sGridNo, struct LEVELNODE *pLevelNode, UINT32 uiFlags,
                           UINT16 usType);
BOOLEAN StartInteractiveObject(INT16 sGridNo, UINT16 usStructureID, struct SOLDIERTYPE *pSoldier,
                               UINT8 ubDirection);
BOOLEAN StartInteractiveObjectFromMouse(struct SOLDIERTYPE *pSoldier, UINT8 ubDirection);
void CompileInteractiveTiles();
UINT32 GetInteractiveTileCursor(UINT32 uiOldCursor, BOOLEAN fConfirm);
BOOLEAN InteractWithInteractiveObject(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                      UINT8 ubDirection);
BOOLEAN SoldierHandleInteractiveObject(struct SOLDIERTYPE *pSoldier);

BOOLEAN CalcInteractiveObjectAPs(INT16 sGridNo, struct STRUCTURE *pStructure, INT16 *psAPCost,
                                 INT16 *psBPCost);

void HandleStructChangeFromGridNo(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

void BeginCurInteractiveTileCheck(UINT8 bCheckFlags);
void EndCurInteractiveTileCheck();
void LogMouseOverInteractiveTile(INT16 sGridNo);
BOOLEAN ShouldCheckForMouseDetections();

void CycleIntTileFindStack(UINT16 usMapPos);
void SetActionModeDoorCursorText();

struct LEVELNODE *GetCurInteractiveTile();
struct LEVELNODE *GetCurInteractiveTileGridNo(INT16 *psGridNo);
struct LEVELNODE *GetCurInteractiveTileGridNoAndStructure(INT16 *psGridNo,
                                                          struct STRUCTURE **ppStructure);
struct LEVELNODE *ConditionalGetCurInteractiveTileGridNoAndStructure(INT16 *psGridNo,
                                                                     struct STRUCTURE **ppStructure,
                                                                     BOOLEAN fRejectOnTopItems);

#endif
