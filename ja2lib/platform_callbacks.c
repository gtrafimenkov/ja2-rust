#include "platform_callbacks.h"

#include "FadeScreen.h"

bool PlatformCallback_IsInFade() { return gfFadeInitialized && gfFadeInVideo; }
void PlatformCallback_Fade() { gFadeFunction(); }
