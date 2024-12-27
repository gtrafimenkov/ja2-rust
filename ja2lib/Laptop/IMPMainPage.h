#ifndef __IMP_MAINPAGE_H
#define __IMP_MAINPAGE_H

#include "SGP/Types.h"

void RenderIMPMainPage(void);
void ExitIMPMainPage(void);
void EnterIMPMainPage(void);
void HandleIMPMainPage(void);
void NextProfilingMode(void);
void ShadeUnSelectableButtons(void);

extern int32_t iCurrentProfileMode;

#endif
