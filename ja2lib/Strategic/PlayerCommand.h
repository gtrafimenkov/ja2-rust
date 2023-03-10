#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(uint8_t sMapX, uint8_t sMapY, wchar_t* sFacilitiesString,
                              size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                       BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                        BOOLEAN fContested);

void ReplaceSoldierProfileInPlayerGroup(uint8_t ubGroupID, uint8_t ubOldProfile,
                                        uint8_t ubNewProfile);

#endif
