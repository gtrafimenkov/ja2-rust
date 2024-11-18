#ifndef _MERC_ENTRING_H
#define _MERC_ENTRING_H

#include "SGP/Types.h"

void ResetHeliSeats();
void AddMercToHeli(uint8_t ubID);

void StartHelicopterRun(int16_t sGridNoSweetSpot);

void HandleHeliDrop();

extern BOOLEAN gfIngagedInDrop;

#endif
