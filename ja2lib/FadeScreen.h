#ifndef FADE_SCREEN_H
#define FADE_SCREEN_H

#include "SGP/Types.h"

#define FADE_OUT_REALFADE 5

#define FADE_IN_REALFADE 12

typedef void (*FADE_HOOK)(void);

extern FADE_HOOK gFadeInDoneCallback;
extern FADE_HOOK gFadeOutDoneCallback;

typedef void (*FADE_FUNCTION)(void);

extern BOOLEAN gfFadeInitialized;
extern BOOLEAN gfFadeIn;
extern int8_t gbFadeType;
extern FADE_FUNCTION gFadeFunction;
extern BOOLEAN gfFadeInVideo;

void BeginFade(uint32_t uiExitScreen, int8_t bFadeValue, int8_t bType, uint32_t uiDelay);

BOOLEAN HandleBeginFadeIn(uint32_t uiScreenExit);
BOOLEAN HandleBeginFadeOut(uint32_t uiScreenExit);

BOOLEAN HandleFadeOutCallback();
BOOLEAN HandleFadeInCallback();

void FadeInNextFrame();
void FadeOutNextFrame();

#endif
