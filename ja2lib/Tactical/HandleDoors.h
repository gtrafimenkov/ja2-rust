// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _DOORS_H
#define _DOORS_H

#include "SGP/Types.h"

struct SOLDIERTYPE;
struct STRUCTURE;

#define HANDLE_DOOR_OPEN 1
#define HANDLE_DOOR_EXAMINE 2
#define HANDLE_DOOR_LOCKPICK 3
#define HANDLE_DOOR_FORCE 4
#define HANDLE_DOOR_LOCK 5
#define HANDLE_DOOR_UNLOCK 6
#define HANDLE_DOOR_EXPLODE 7
#define HANDLE_DOOR_UNTRAP 8
#define HANDLE_DOOR_CROWBAR 9

extern BOOLEAN gfSetPerceivedDoorState;

BOOLEAN HandleOpenableStruct(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                             struct STRUCTURE *pStructure);

void InteractWithOpenableStruct(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                uint8_t ubDirection, BOOLEAN fDoor);

void InteractWithClosedDoor(struct SOLDIERTYPE *pSoldier, uint8_t ubHandleCode);

void SetDoorString(int16_t sGridNo);

void HandleDoorChangeFromGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                BOOLEAN fNoAnimations);

#endif
