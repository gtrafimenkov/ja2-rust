#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(int16_t sMapX, int16_t sMapY, wchar_t* sFacilitiesString,
                              size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(int16_t sMapX, int16_t sMapY, int8_t bMapZ,
                                       BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(int16_t sMapX, int16_t sMapY, int8_t bMapZ,
                                        BOOLEAN fContested);

#ifdef JA2TESTVERSION
void ClearMapControlledFlags(void);
#endif

/*
// is this sector under player control
BOOLEAN IsTheSectorPerceivedToBeUnderEnemyControl( int16_t sMapX, int16_t sMapY, int8_t bMapZ );

// make player's perceived control over the sector reflect reality
void MakePlayerPerceptionOfSectorControlCorrect( int16_t sMapX, int16_t sMapY, int8_t bMapZ );
*/

void ReplaceSoldierProfileInPlayerGroup(uint8_t ubGroupID, uint8_t ubOldProfile,
                                        uint8_t ubNewProfile);

#endif
