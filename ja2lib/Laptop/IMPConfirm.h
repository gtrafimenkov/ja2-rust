#ifndef __IMP_CONFIRM_H
#define __IMP_CONFIRM_H

#include "SGP/Types.h"

void EnterIMPConfirm(void);
void RenderIMPConfirm(void);
void ExitIMPConfirm(void);
void HandleIMPConfirm(void);

BOOLEAN AddCharacterToPlayersTeam(void);
void LoadInCurrentImpCharacter(void);
void WriteOutCurrentImpCharacter(INT32 iProfileId);

void ResetIMPCharactersEyesAndMouthOffsets(uint8_t ubMercProfileID);

#endif
