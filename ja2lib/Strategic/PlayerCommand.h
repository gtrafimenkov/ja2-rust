#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(u8 sMapX, u8 sMapY, STR16 sFacilitiesString, size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(u8 sMapX, u8 sMapY, INT8 bMapZ, BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(u8 sMapX, u8 sMapY, INT8 bMapZ, BOOLEAN fContested);

void ReplaceSoldierProfileInPlayerGroup(UINT8 ubGroupID, UINT8 ubOldProfile, UINT8 ubNewProfile);

#endif
