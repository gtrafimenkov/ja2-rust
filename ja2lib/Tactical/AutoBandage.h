// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _AUTO_BANDAGE_H
#define _AUTO_BANDAGE_H

#include "SGP/Types.h"

// #define VISIBLE_AUTO_BANDAGE

void AutoBandage(BOOLEAN fStart);
void BeginAutoBandage();

BOOLEAN HandleAutoBandage();

void ShouldBeginAutoBandage();

void SetAutoBandagePending(BOOLEAN fSet);
void HandleAutoBandagePending();

// ste the autobandage as complete
void SetAutoBandageComplete(void);

#endif
