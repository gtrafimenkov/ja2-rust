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

extern UINT32 guiExitScreen;
BOOLEAN gfFadeInitialized = FALSE;
INT8 gbFadeValue;
INT16 gsFadeLimit;
UINT32 guiTime;
UINT32 guiFadeDelay;
BOOLEAN gfFirstTimeInFade = FALSE;
INT16 gsFadeCount;
INT8 gbFadeType;
INT32 giX1, giX2, giY1, giY2;
INT16 gsFadeRealCount;
BOOLEAN gfFadeInVideo;

UINT32 uiOldMusicMode;

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

BOOLEAN HandleBeginFadeIn(UINT32 uiScreenExit) {
  if (gfFadeIn) {
    BeginFade(uiScreenExit, 35, FADE_IN_REALFADE, 5);

    gfFadeIn = FALSE;

    gfFadeInDone = TRUE;

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN HandleBeginFadeOut(UINT32 uiScreenExit) {
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

void BeginFade(UINT32 uiExitScreen, INT8 bFadeValue, INT8 bType, UINT32 uiDelay) {
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

UINT32 FadeScreenInit() { return (TRUE); }

UINT32 FadeScreenHandle() {
  UINT32 uiTime;

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

UINT32 FadeScreenShutdown() { return (FALSE); }

void FadeFrameBufferRealFade() {
  if (gsFadeRealCount != gsFadeCount) {
    ShadowVideoSurfaceRectUsingLowPercentTable(vsFB, 0, 0, 640, 480);
    gsFadeRealCount = gsFadeCount;
  }
}

void FadeInFrameBufferRealFade() {
  INT32 cnt;

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
  UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
  UINT8 *pDestBuf, *pSrcBuf;
  UINT16 usWidth, usHeight;

  // Update saved buffer - do for the viewport size ony!
  GetCurrentVideoSettings(&usWidth, &usHeight);

  pSrcBuf = VSurfaceLockOld(vsFB, &uiSrcPitchBYTES);
  pDestBuf = VSurfaceLockOld(vsSB, &uiDestPitchBYTES);

  Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES, (UINT16 *)pSrcBuf, uiSrcPitchBYTES, 0, 0, 0,
                  0, 640, 480);

  VSurfaceUnlock(vsFB);
  VSurfaceUnlock(vsSB);

  return (TRUE);
}
