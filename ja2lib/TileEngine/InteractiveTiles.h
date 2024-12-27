// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __INTERACTIVE_TILES_H
#define __INTERACTIVE_TILES_H

#include "SGP/Types.h"

struct GRect;
struct LEVELNODE;
struct SOLDIERTYPE;
struct STRUCTURE;

#define INTTILE_DOOR_TILE_ONE 1

#define INTTILE_DOOR_OPENSPEED 70

#define INTILE_CHECK_FULL 1
#define INTILE_CHECK_SELECTIVE 2

extern BOOLEAN gfOverIntTile;

void GetLevelNodeScreenRect(struct LEVELNODE *pNode, struct GRect *pRect, int16_t sXPos, int16_t sYPos,
                            int16_t sGridNo);
BOOLEAN InitInteractiveTileManagement();
void ShutdownInteractiveTileManagement();
BOOLEAN AddInteractiveTile(int16_t sGridNo, struct LEVELNODE *pLevelNode, uint32_t uiFlags,
                           uint16_t usType);
BOOLEAN StartInteractiveObject(int16_t sGridNo, uint16_t usStructureID, struct SOLDIERTYPE *pSoldier,
                               uint8_t ubDirection);
BOOLEAN StartInteractiveObjectFromMouse(struct SOLDIERTYPE *pSoldier, uint8_t ubDirection);
void CompileInteractiveTiles();
uint32_t GetInteractiveTileCursor(uint32_t uiOldCursor, BOOLEAN fConfirm);
BOOLEAN InteractWithInteractiveObject(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                      uint8_t ubDirection);
BOOLEAN SoldierHandleInteractiveObject(struct SOLDIERTYPE *pSoldier);

BOOLEAN CalcInteractiveObjectAPs(int16_t sGridNo, struct STRUCTURE *pStructure, int16_t *psAPCost,
                                 int16_t *psBPCost);

void HandleStructChangeFromGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);

void BeginCurInteractiveTileCheck(uint8_t bCheckFlags);
void EndCurInteractiveTileCheck();
void LogMouseOverInteractiveTile(int16_t sGridNo);
BOOLEAN ShouldCheckForMouseDetections();

void CycleIntTileFindStack(uint16_t usMapPos);
void SetActionModeDoorCursorText();

struct LEVELNODE *GetCurInteractiveTile();
struct LEVELNODE *GetCurInteractiveTileGridNo(int16_t *psGridNo);
struct LEVELNODE *GetCurInteractiveTileGridNoAndStructure(int16_t *psGridNo,
                                                          struct STRUCTURE **ppStructure);
struct LEVELNODE *ConditionalGetCurInteractiveTileGridNoAndStructure(int16_t *psGridNo,
                                                                     struct STRUCTURE **ppStructure,
                                                                     BOOLEAN fRejectOnTopItems);

#endif
