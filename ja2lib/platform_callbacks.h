#ifndef __PLATFORM_CALLBACKS_H
#define __PLATFORM_CALLBACKS_H

#include <stdbool.h>

// Callbacks from the platfrom code to application

bool PlatformCallback_IsInFade();
void PlatformCallback_Fade();

#endif
