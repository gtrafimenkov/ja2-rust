#include "FadeScreen.h"

#include "GameLoop.h"
#include "Globals.h"
#include "SGP/CursorControl.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "SysGlobals.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/MusicControl.h"
#include "Utils/TimerControl.h"

#define SQUARE_STEP 8

extern uint32_t guiExitScreen;
BOOLEAN gfFadeInitialized = FALSE;
int8_t gbFadeValue;
int16_t gsFadeLimit;
uint32_t guiTime;
uint32_t guiFadeDelay;
BOOLEAN gfFirstTimeInFade = FALSE;
int16_t gsFadeCount;
int8_t gbFadeType;
int32_t giX1, giX2, giY1, giY2;
int16_t gsFadeRealCount;
BOOLEAN gfFadeInVideo;

uint32_t uiOldMusicMode;

FADE_FUNCTION gFadeFunction = NULL;

FADE_HOOK gFadeInDoneCallback = NULL;
FADE_HOOK gFadeOutDoneCallback = NULL;

void FadeFrameBufferRealFade();

void FadeInFrameBufferRealFade();

BOOLEAN UpdateSaveBufferWithBackbuffer(void);

BOOLEAN gfFadeIn = FALSE;
BOOLEAN gfFadeOut = FALSE;
BOOLEAN gfFadeOutDone = FALSE;
BOOLEAN gfFadeInDone = FALSE;

void FadeInNextFrame() {
  gfFadeIn = TRUE;
  gfFadeInDone = FALSE;
}

void FadeOutNextFrame() {
  gfFadeOut = TRUE;
  gfFadeOutDone = FALSE;
}

BOOLEAN HandleBeginFadeIn(uint32_t uiScreenExit) {
  if (gfFadeIn) {
    BeginFade(uiScreenExit, 35, FADE_IN_REALFADE, 5);

    gfFadeIn = FALSE;

    gfFadeInDone = TRUE;

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN HandleBeginFadeOut(uint32_t uiScreenExit) {
  if (gfFadeOut) {
    BeginFade(uiScreenExit, 35, FADE_OUT_REALFADE, 5);

    gfFadeOut = FALSE;

    gfFadeOutDone = TRUE;

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN HandleFadeOutCallback() {
  if (gfFadeOutDone) {
    gfFadeOutDone = FALSE;

    if (gFadeOutDoneCallback != NULL) {
      gFadeOutDoneCallback();

      gFadeOutDoneCallback = NULL;

      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN HandleFadeInCallback() {
  if (gfFadeInDone) {
    gfFadeInDone = FALSE;

    if (gFadeInDoneCallback != NULL) {
      gFadeInDoneCallback();
    }

    gFadeInDoneCallback = NULL;

    return (TRUE);
  }

  return (FALSE);
}

void BeginFade(uint32_t uiExitScreen, int8_t bFadeValue, int8_t bType, uint32_t uiDelay) {
  // Init some paramters
  guiExitScreen = uiExitScreen;
  gbFadeValue = bFadeValue;
  guiFadeDelay = uiDelay;
  gfFadeIn = FALSE;
  gfFadeInVideo = TRUE;

  uiOldMusicMode = uiMusicHandle;

  // Calculate step;
  switch (bType) {
    case FADE_IN_REALFADE:

      gsFadeRealCount = -1;
      gsFadeLimit = 8;
      gFadeFunction = (FADE_FUNCTION)FadeInFrameBufferRealFade;
      gfFadeInVideo = FALSE;

      // Copy backbuffer to savebuffer
      UpdateSaveBufferWithBackbuffer();

      // Clear framebuffer
      VSurfaceColorFill(vsFB, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
      break;

    case FADE_OUT_REALFADE:
      gsFadeRealCount = -1;
      gsFadeLimit = 10;
      gFadeFunction = (FADE_FUNCTION)FadeFrameBufferRealFade;
      gfFadeInVideo = FALSE;
      break;
  }

  gfFadeInitialized = TRUE;
  gfFirstTimeInFade = TRUE;
  gsFadeCount = 0;
  gbFadeType = bType;

  SetPendingNewScreen(FADE_SCREEN);
}

uint32_t FadeScreenInit() { return (TRUE); }

uint32_t FadeScreenHandle() {
  uint32_t uiTime;

  if (!gfFadeInitialized) {
    SET_ERROR("Fade Screen called but not intialized ");
    return (ERROR_SCREEN);
  }

  // ATE: Remove cursor
  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  if (gfFirstTimeInFade) {
    gfFirstTimeInFade = FALSE;

    // Calcuate delay
    guiTime = GetJA2Clock();
  }

  // Get time
  uiTime = GetJA2Clock();

  MusicPoll(TRUE);

  if ((uiTime - guiTime) > guiFadeDelay) {
    InvalidateScreen();
    if (!gfFadeInVideo) {
      gFadeFunction();
    }

    gsFadeCount++;

    if (gsFadeCount > gsFadeLimit) {
      switch (gbFadeType) {
        case FADE_OUT_REALFADE:

          // Clear framebuffer
          VSurfaceColorFill(vsFB, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
          break;
      }

      // End!
      gfFadeInitialized = FALSE;
      gfFadeIn = FALSE;

      return (guiExitScreen);
    }
  }

  return (FADE_SCREEN);
}

uint32_t FadeScreenShutdown() { return (FALSE); }

void FadeFrameBufferRealFade() {
  if (gsFadeRealCount != gsFadeCount) {
    ShadowVideoSurfaceRectUsingLowPercentTable(vsFB, 0, 0, 640, 480);
    gsFadeRealCount = gsFadeCount;
  }
}

void FadeInFrameBufferRealFade() {
  int32_t cnt;

  if (gsFadeRealCount != gsFadeCount) {
    for (cnt = 0; cnt < (gsFadeLimit - gsFadeCount); cnt++) {
      ShadowVideoSurfaceRectUsingLowPercentTable(vsFB, 0, 0, 640, 480);
    }

    // Refresh Screen
    RefreshScreen();

    // Copy save buffer back
    RestoreExternBackgroundRect(0, 0, 640, 480);

    gsFadeRealCount = gsFadeCount;
  }
}

BOOLEAN UpdateSaveBufferWithBackbuffer(void) {
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;
  uint16_t usWidth, usHeight;

  // Update saved buffer - do for the viewport size ony!
  GetCurrentVideoSettings(&usWidth, &usHeight);

  pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);
  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES, 0,
                  0, 0, 0, 640, 480);

  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  return (TRUE);
}
