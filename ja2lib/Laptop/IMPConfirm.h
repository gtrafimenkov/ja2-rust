// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __IMP_CONFIRM_H
#define __IMP_CONFIRM_H

#include "SGP/Types.h"

void EnterIMPConfirm(void);
void RenderIMPConfirm(void);
void ExitIMPConfirm(void);
void HandleIMPConfirm(void);

BOOLEAN AddCharacterToPlayersTeam(void);
void LoadInCurrentImpCharacter(void);
void WriteOutCurrentImpCharacter(int32_t iProfileId);

void ResetIMPCharactersEyesAndMouthOffsets(uint8_t ubMercProfileID);

#endif
