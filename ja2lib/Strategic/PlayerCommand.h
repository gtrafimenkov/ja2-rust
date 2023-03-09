#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(INT16 sMapX, INT16 sMapY, STR16 sFacilitiesString, size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(u8 sMapX, u8 sMapY, INT8 bMapZ, BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fContested);

void ReplaceSoldierProfileInPlayerGroup(UINT8 ubGroupID, UINT8 ubOldProfile, UINT8 ubNewProfile);

#endif
