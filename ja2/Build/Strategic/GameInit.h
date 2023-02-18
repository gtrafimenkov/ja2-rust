#ifndef __GAME_INIT_H
#define __GAME_INIT_H

#include "SGP/Types.h"

BOOLEAN InitNewGame(BOOLEAN fReset);
BOOLEAN AnyMercsHired();

void QuickStartGame();

void InitStrategicLayer();
void ShutdownStrategicLayer();

void ReStartingGame();

void InitBloodCatSectors();

#endif
