#ifndef _INTRO__C_
#define _INTRO__C_

#include "SGP/Types.h"

uint32_t IntroScreenShutdown(void);
uint32_t IntroScreenHandle(void);
uint32_t IntroScreenInit(void);

// enums used for when the intro screen can come up, used with 'gbIntroScreenMode'
enum {
  INTRO_BEGINING,  // set when viewing the intro at the begining of the game
  INTRO_ENDING,    // set when viewing the end game video.

  INTRO_SPLASH,
};

extern uint32_t guiSmackerSurface;

void SetIntroType(INT8 bIntroType);

#endif
