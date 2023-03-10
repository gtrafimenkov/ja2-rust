#ifndef __MS_I_TOWNMINE_INFO_H
#define __MS_I_TOWNMINE_INFO_H

#include "SGP/Types.h"

extern BOOLEAN fShowTownInfo;

// display the box
void DisplayTownInfo(u8 sMapX, u8 sMapY, i8 bMapZ);

// create or destroy the town info box..should only be directly called the exit code for mapscreen
void CreateDestroyTownInfoBox(void);

#endif
