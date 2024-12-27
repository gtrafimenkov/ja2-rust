// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(u8 sMapX, u8 sMapY, wchar_t* sFacilitiesString, size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(u8 sMapX, u8 sMapY, int8_t bMapZ, BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(u8 sMapX, u8 sMapY, int8_t bMapZ, BOOLEAN fContested);

void ReplaceSoldierProfileInPlayerGroup(uint8_t ubGroupID, uint8_t ubOldProfile, uint8_t ubNewProfile);

#endif
