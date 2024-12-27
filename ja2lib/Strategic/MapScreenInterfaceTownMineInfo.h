// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __MS_I_TOWNMINE_INFO_H
#define __MS_I_TOWNMINE_INFO_H

#include "SGP/Types.h"

extern BOOLEAN fShowTownInfo;

// display the box
void DisplayTownInfo(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);

// create or destroy the town info box..should only be directly called the exit code for mapscreen
void CreateDestroyTownInfoBox(void);

#endif
