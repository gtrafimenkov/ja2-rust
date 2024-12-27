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

void GetLevelNodeScreenRect(struct LEVELNODE *pNode, SGPRect *pRect, INT16 sXPos, INT16 sYPos,
                            INT16 sGridNo);
BOOLEAN InitInteractiveTileManagement();
void ShutdownInteractiveTileManagement();
BOOLEAN AddInteractiveTile(INT16 sGridNo, struct LEVELNODE *pLevelNode, uint32_t uiFlags,
                           uint16_t usType);
BOOLEAN StartInteractiveObject(INT16 sGridNo, uint16_t usStructureID, struct SOLDIERTYPE *pSoldier,
                               uint8_t ubDirection);
BOOLEAN StartInteractiveObjectFromMouse(struct SOLDIERTYPE *pSoldier, uint8_t ubDirection);
void CompileInteractiveTiles();
uint32_t GetInteractiveTileCursor(uint32_t uiOldCursor, BOOLEAN fConfirm);
BOOLEAN InteractWithInteractiveObject(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                      uint8_t ubDirection);
BOOLEAN SoldierHandleInteractiveObject(struct SOLDIERTYPE *pSoldier);

BOOLEAN CalcInteractiveObjectAPs(INT16 sGridNo, struct STRUCTURE *pStructure, INT16 *psAPCost,
                                 INT16 *psBPCost);

void HandleStructChangeFromGridNo(struct SOLDIERTYPE *pSoldier, INT16 sGridNo);

void BeginCurInteractiveTileCheck(uint8_t bCheckFlags);
void EndCurInteractiveTileCheck();
void LogMouseOverInteractiveTile(INT16 sGridNo);
BOOLEAN ShouldCheckForMouseDetections();

void CycleIntTileFindStack(uint16_t usMapPos);
void SetActionModeDoorCursorText();

struct LEVELNODE *GetCurInteractiveTile();
struct LEVELNODE *GetCurInteractiveTileGridNo(INT16 *psGridNo);
struct LEVELNODE *GetCurInteractiveTileGridNoAndStructure(INT16 *psGridNo,
                                                          struct STRUCTURE **ppStructure);
struct LEVELNODE *ConditionalGetCurInteractiveTileGridNoAndStructure(INT16 *psGridNo,
                                                                     struct STRUCTURE **ppStructure,
                                                                     BOOLEAN fRejectOnTopItems);

#endif
