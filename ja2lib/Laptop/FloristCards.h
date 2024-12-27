// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __FLORIST_CARDS_H
#define __FLORIST_CARDS_H

#include "SGP/Types.h"

#define FLOR_CARD_TEXT_FILE "BINARYDATA\\FlowerCard.edt"
#define FLOR_CARD_TEXT_TITLE_SIZE 5 * 80 * 2

void GameInitFloristCards();
BOOLEAN EnterFloristCards();
void ExitFloristCards();
void HandleFloristCards();
void RenderFloristCards();

extern int8_t gbCurrentlySelectedCard;

#endif
