// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __FLORIST_H
#define __FLORIST_H

#include "SGP/Types.h"

#define FLORIST_BUTTON_TEXT_FONT FONT14ARIAL
#define FLORIST_BUTTON_TEXT_UP_COLOR 2
#define FLORIST_BUTTON_TEXT_DOWN_COLOR 2
#define FLORIST_BUTTON_TEXT_SHADOW_COLOR 128

void GameInitFlorist();
BOOLEAN EnterFlorist();
void ExitFlorist();
void HandleFlorist();
void RenderFlorist();

void RemoveFloristDefaults();
void DisplayFloristDefaults();
BOOLEAN InitFloristDefaults();

#endif
