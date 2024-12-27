#ifndef __MS_I_TOWNMINE_INFO_H
#define __MS_I_TOWNMINE_INFO_H

#include "SGP/Types.h"

extern BOOLEAN fShowTownInfo;

// display the box
void DisplayTownInfo(INT16 sMapX, INT16 sMapY, int8_t bMapZ);

// create or destroy the town info box..should only be directly called the exit code for mapscreen
void CreateDestroyTownInfoBox(void);

#endif
