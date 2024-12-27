#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(INT16 sMapX, INT16 sMapY, STR16 sFacilitiesString, size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(INT16 sMapX, INT16 sMapY, int8_t bMapZ, BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(INT16 sMapX, INT16 sMapY, int8_t bMapZ, BOOLEAN fContested);

#ifdef JA2TESTVERSION
void ClearMapControlledFlags(void);
#endif

/*
// is this sector under player control
BOOLEAN IsTheSectorPerceivedToBeUnderEnemyControl( INT16 sMapX, INT16 sMapY, int8_t bMapZ );

// make player's perceived control over the sector reflect reality
void MakePlayerPerceptionOfSectorControlCorrect( INT16 sMapX, INT16 sMapY, int8_t bMapZ );
*/

void ReplaceSoldierProfileInPlayerGroup(uint8_t ubGroupID, uint8_t ubOldProfile,
                                        uint8_t ubNewProfile);

#endif
