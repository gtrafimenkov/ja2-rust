// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _MERC_ENTRING_H
#define _MERC_ENTRING_H

#include "SGP/Types.h"

void ResetHeliSeats();
void AddMercToHeli(uint8_t ubID);

void StartHelicopterRun(int16_t sGridNoSweetSpot);

void HandleHeliDrop();

extern BOOLEAN gfIngagedInDrop;

#endif
