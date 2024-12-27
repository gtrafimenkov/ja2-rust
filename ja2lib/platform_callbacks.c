// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "platform_callbacks.h"

#include "FadeScreen.h"

bool PlatformCallback_IsInFade() { return gfFadeInitialized && gfFadeInVideo; }
void PlatformCallback_Fade() { gFadeFunction(); }
