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
#include "TileEngine/SysUtil.h"
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
INT32 giX1, giX2, giY1, giY2;
int16_t gsFadeRealCount;
BOOLEAN gfFadeInVideo;

uint32_t uiOldMusicMode;

FADE_FUNCTION gFadeFunction = NULL;

FADE_HOOK gFadeInDoneCallback = NULL;
FADE_HOOK gFadeOutDoneCallback = NULL;

void FadeFrameBufferSquare();
void FadeFrameBufferVersionOne();
void FadeFrameBufferVersionFaster(int8_t bFadeValue);
void FadeFrameBufferSide();
void FadeFrameBufferRealFade();

void FadeInBackBufferVersionOne();
void FadeInBackBufferSquare();
void FadeInFrameBufferRealFade();

extern BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(uint32_t uiDestVSurface, INT32 X1,
                                                          INT32 Y1, INT32 X2, INT32 Y2);
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
      ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
      break;

    case FADE_OUT_REALFADE:

      gsFadeRealCount = -1;
      gsFadeLimit = 10;
      gFadeFunction = (FADE_FUNCTION)FadeFrameBufferRealFade;
      gfFadeInVideo = FALSE;

      // Clear framebuffer
      // ColorFillVideoSurfaceArea( FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor( FROMRGB( 0, 0, 0 )
      // ) );
      break;

    case FADE_OUT_VERSION_ONE:
      // gsFadeLimit = 255 / bFadeValue;
      // gFadeFunction = (FADE_FUNCTION)FadeFrameBufferVersionOne;
      // SetMusicFadeSpeed( 25 );
      // SetMusicMode( MUSIC_NONE );
      break;

    case FADE_OUT_SQUARE:
      gsFadeLimit = (640 / (SQUARE_STEP * 2));
      giX1 = 0;
      giX2 = 640;
      giY1 = 0;
      giY2 = 480;
      gFadeFunction = (FADE_FUNCTION)FadeFrameBufferSquare;

      // Zero frame buffer
      ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
      // ColorFillVideoSurfaceArea( guiSAVEBUFFER, 0, 0, 640,	480, Get16BPPColor( FROMRGB( 0, 0, 0
      // ) ) );

      //	SetMusicFadeSpeed( 25 );
      // SetMusicMode( MUSIC_NONE );
      break;

    case FADE_IN_VERSION_ONE:
      gsFadeLimit = 255 / bFadeValue;
      gFadeFunction = (FADE_FUNCTION)FadeInBackBufferVersionOne;
      break;

    case FADE_IN_SQUARE:
      gFadeFunction = (FADE_FUNCTION)FadeInBackBufferSquare;
      giX1 = 320;
      giX2 = 320;
      giY1 = 240;
      giY2 = 240;
      gsFadeLimit = (640 / (SQUARE_STEP * 2));
      gfFadeIn = TRUE;
      break;

    case FADE_OUT_VERSION_FASTER:
      gsFadeLimit = (255 / bFadeValue) * 2;
      gFadeFunction = (FADE_FUNCTION)FadeFrameBufferVersionFaster;

      // SetMusicFadeSpeed( 25 );
      // SetMusicMode( MUSIC_NONE );
      break;

    case FADE_OUT_VERSION_SIDE:
      // Copy frame buffer to save buffer
      gsFadeLimit = (640 / 8);
      gFadeFunction = (FADE_FUNCTION)FadeFrameBufferSide;

      // SetMusicFadeSpeed( 25 );
      // SetMusicMode( MUSIC_NONE );
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
    // Fade!
    if (!gfFadeIn) {
      // gFadeFunction( );
    }

    InvalidateScreen();

    if (!gfFadeInVideo) {
      gFadeFunction();
    }

    gsFadeCount++;

    if (gsFadeCount > gsFadeLimit) {
      switch (gbFadeType) {
        case FADE_OUT_REALFADE:

          // Clear framebuffer
          ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
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

void FadeFrameBufferVersionOne() {
  INT32 cX, cY;
  uint32_t uiDestPitchBYTES;
  uint16_t *pBuf;
  int16_t bR, bG, bB;
  uint32_t uiRGBColor;
  uint16_t s16BPPSrc;

  pBuf = (uint16_t *)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);

  // LOCK FRAME BUFFER
  for (cX = 0; cX < 640; cX++) {
    for (cY = 0; cY < 480; cY++) {
      s16BPPSrc = pBuf[(cY * 640) + cX];

      uiRGBColor = GetRGBColor(s16BPPSrc);

      bR = SGPGetRValue(uiRGBColor);
      bG = SGPGetGValue(uiRGBColor);
      bB = SGPGetBValue(uiRGBColor);

      // Fade down
      bR -= gbFadeValue;
      if (bR < 0) bR = 0;

      bG -= gbFadeValue;
      if (bG < 0) bG = 0;

      bB -= gbFadeValue;
      if (bB < 0) bB = 0;

      // Set back info buffer
      pBuf[(cY * 640) + cX] = Get16BPPColor(FROMRGB(bR, bG, bB));
    }
  }

  UnLockVideoSurface(FRAME_BUFFER);
}

void FadeInBackBufferVersionOne() {
  INT32 cX, cY;
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint16_t *pSrcBuf, *pDestBuf;
  int16_t bR, bG, bB;
  uint32_t uiRGBColor;
  uint16_t s16BPPSrc;
  int16_t bFadeVal = (gsFadeLimit - gsFadeCount) * gbFadeValue;

  pDestBuf = (uint16_t *)LockVideoSurface(BACKBUFFER, &uiDestPitchBYTES);
  pSrcBuf = (uint16_t *)LockVideoSurface(FRAME_BUFFER, &uiSrcPitchBYTES);

  // LOCK FRAME BUFFER
  for (cX = 0; cX < 640; cX++) {
    for (cY = 0; cY < 480; cY++) {
      s16BPPSrc = pSrcBuf[(cY * 640) + cX];

      uiRGBColor = GetRGBColor(s16BPPSrc);

      bR = SGPGetRValue(uiRGBColor);
      bG = SGPGetGValue(uiRGBColor);
      bB = SGPGetBValue(uiRGBColor);

      // Fade down
      bR -= bFadeVal;
      if (bR < 0) bR = 0;

      bG -= bFadeVal;
      if (bG < 0) bG = 0;

      bB -= bFadeVal;
      if (bB < 0) bB = 0;

      // Set back info dest buffer
      pDestBuf[(cY * 640) + cX] = Get16BPPColor(FROMRGB(bR, bG, bB));
    }
  }

  UnLockVideoSurface(FRAME_BUFFER);
  UnLockVideoSurface(BACKBUFFER);
}

void FadeFrameBufferVersionFaster(int8_t bFadeValue) {
  INT32 cX, cY, iStartX, iStartY;
  uint32_t uiDestPitchBYTES;
  uint16_t *pBuf;
  int16_t bR, bG, bB;
  uint32_t uiRGBColor;
  uint16_t s16BPPSrc;

  pBuf = (uint16_t *)LockVideoSurface(FRAME_BUFFER, &uiDestPitchBYTES);

  iStartX = gsFadeCount % 2;
  iStartY = 0;

  // LOCK FRAME BUFFER
  for (cX = iStartX; cX < 640; cX += 2) {
    if (iStartX == 1) {
      iStartX = 0;
    } else {
      iStartX = 1;
    }

    for (cY = iStartY; cY < 480; cY++) {
      s16BPPSrc = pBuf[(cY * 640) + cX];

      uiRGBColor = GetRGBColor(s16BPPSrc);

      bR = SGPGetRValue(uiRGBColor);
      bG = SGPGetGValue(uiRGBColor);
      bB = SGPGetBValue(uiRGBColor);

      // Fade down
      bR -= bFadeValue;
      if (bR < 0) bR = 0;

      bG -= bFadeValue;
      if (bG < 0) bG = 0;

      bB -= bFadeValue;
      if (bB < 0) bB = 0;

      // Set back info buffer
      pBuf[(cY * 640) + cX] = Get16BPPColor(FROMRGB(bR, bG, bB));
    }
  }

  UnLockVideoSurface(FRAME_BUFFER);
}

void FadeFrameBufferSide() {
  INT32 iX1, iX2;
  int16_t sFadeMove;

  sFadeMove = gsFadeCount * 4;

  iX1 = 0;
  iX2 = sFadeMove;

  ColorFillVideoSurfaceArea(FRAME_BUFFER, iX1, 0, iX2, 480, Get16BPPColor(FROMRGB(0, 0, 0)));

  iX1 = 640 - sFadeMove;
  iX2 = 640;

  ColorFillVideoSurfaceArea(FRAME_BUFFER, iX1, 0, iX2, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
}

void FadeFrameBufferSquare() {
  INT32 iX1, iX2, iY1, iY2;
  int16_t sFadeXMove, sFadeYMove;

  sFadeXMove = SQUARE_STEP;
  sFadeYMove = (int16_t)(sFadeXMove * .75);

  iX1 = giX1;
  iX2 = giX1 + sFadeXMove;
  iY1 = giY1;
  iY2 = giY1 + sFadeYMove;

  ColorFillVideoSurfaceArea(BACKBUFFER, iX1, 0, iX2, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  ColorFillVideoSurfaceArea(BACKBUFFER, 0, iY1, 640, iY2, Get16BPPColor(FROMRGB(0, 0, 0)));

  iX1 = giX2 - sFadeXMove;
  iX2 = giX2;
  iY1 = giY2 - sFadeYMove;
  iY2 = giY2;

  ColorFillVideoSurfaceArea(BACKBUFFER, iX1, 0, iX2, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  ColorFillVideoSurfaceArea(BACKBUFFER, 0, iY1, 640, iY2, Get16BPPColor(FROMRGB(0, 0, 0)));

  giX1 += sFadeXMove;
  giX2 -= sFadeXMove;
  giY1 += sFadeYMove;
  giY2 -= sFadeYMove;
}

void FadeInBackBufferSquare() {
  INT32 iX1, iX2, iY1, iY2;
  int16_t sFadeXMove, sFadeYMove;
  blt_vs_fx BltFx;

  sFadeXMove = SQUARE_STEP;
  sFadeYMove = (int16_t)(sFadeXMove * .75);

  if (gsFadeCount == 0) {
    ColorFillVideoSurfaceArea(BACKBUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  }

  iX1 = giX1 - sFadeXMove;
  iX2 = giX1;
  iY1 = giY1 - sFadeYMove;
  iY2 = giY2 + sFadeYMove;

  BltFx.SrcRect.iLeft = iX1;
  BltFx.SrcRect.iTop = iY1;
  BltFx.SrcRect.iRight = iX2;
  BltFx.SrcRect.iBottom = iY2;

  if (BltFx.SrcRect.iRight != BltFx.SrcRect.iLeft) {
    BltVideoSurface(BACKBUFFER, FRAME_BUFFER, 0, iX1, iY1, VS_BLT_SRCSUBRECT, &BltFx);
  }

  iX1 = giX2;
  iX2 = giX2 + sFadeXMove;
  iY1 = giY1 - sFadeYMove;
  iY2 = giY2 + sFadeYMove;

  BltFx.SrcRect.iLeft = iX1;
  BltFx.SrcRect.iTop = iY1;
  BltFx.SrcRect.iRight = iX2;
  BltFx.SrcRect.iBottom = iY2;

  if (BltFx.SrcRect.iRight != BltFx.SrcRect.iLeft) {
    BltVideoSurface(BACKBUFFER, FRAME_BUFFER, 0, iX1, iY1, VS_BLT_SRCSUBRECT, &BltFx);
  }

  iX1 = giX1;
  iX2 = giX2;
  iY1 = giY1 - sFadeYMove;
  iY2 = giY1;

  BltFx.SrcRect.iLeft = iX1;
  BltFx.SrcRect.iTop = iY1;
  BltFx.SrcRect.iRight = iX2;
  BltFx.SrcRect.iBottom = iY2;

  if (BltFx.SrcRect.iBottom != BltFx.SrcRect.iTop) {
    BltVideoSurface(BACKBUFFER, FRAME_BUFFER, 0, iX1, iY1, VS_BLT_SRCSUBRECT, &BltFx);
  }

  iX1 = giX1;
  iX2 = giX2;
  iY1 = giY2;
  iY2 = giY2 + sFadeYMove;

  BltFx.SrcRect.iLeft = iX1;
  BltFx.SrcRect.iTop = iY1;
  BltFx.SrcRect.iRight = iX2;
  BltFx.SrcRect.iBottom = iY2;

  if (BltFx.SrcRect.iBottom != BltFx.SrcRect.iTop) {
    BltVideoSurface(BACKBUFFER, FRAME_BUFFER, 0, iX1, iY1, VS_BLT_SRCSUBRECT, &BltFx);
  }

  giX1 -= sFadeXMove;
  giX2 += sFadeXMove;
  giY1 -= sFadeYMove;
  giY2 += sFadeYMove;
}

void FadeFrameBufferRealFade() {
  if (gsFadeRealCount != gsFadeCount) {
    ShadowVideoSurfaceRectUsingLowPercentTable(FRAME_BUFFER, 0, 0, 640, 480);

    gsFadeRealCount = gsFadeCount;
  }
}

void FadeInFrameBufferRealFade() {
  INT32 cnt;

  if (gsFadeRealCount != gsFadeCount) {
    for (cnt = 0; cnt < (gsFadeLimit - gsFadeCount); cnt++) {
      ShadowVideoSurfaceRectUsingLowPercentTable(FRAME_BUFFER, 0, 0, 640, 480);
    }

    // Refresh Screen
    RefreshScreen(NULL);

    // Copy save buffer back
    RestoreExternBackgroundRect(0, 0, 640, 480);

    gsFadeRealCount = gsFadeCount;
  }
}

BOOLEAN UpdateSaveBufferWithBackbuffer(void) {
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;
  uint16_t usWidth, usHeight;
  uint8_t ubBitDepth;

  // Update saved buffer - do for the viewport size ony!
  GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);

  pSrcBuf = LockVideoSurface(FRAME_BUFFER, &uiSrcPitchBYTES);
  pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

  Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES, 0,
                  0, 0, 0, 640, 480);

  UnLockVideoSurface(FRAME_BUFFER);
  UnLockVideoSurface(guiSAVEBUFFER);

  return (TRUE);
}
