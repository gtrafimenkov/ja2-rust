#include "Utils/AnimatedProgressBar.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/FontControl.h"
#include "Utils/MusicControl.h"
#include "Utils/TimerControl.h"

double rStart, rEnd;
double rActual;

#define MAX_PROGRESSBARS 4

PROGRESSBAR *pBar[MAX_PROGRESSBARS];

BOOLEAN gfUseLoadScreenProgressBar = FALSE;
uint16_t gusLeftmostShaded = 0;

void CreateLoadingScreenProgressBar() {
  gusLeftmostShaded = 162;
  gfUseLoadScreenProgressBar = TRUE;
  CreateProgressBar(0, 162, 427, 480, 443);
}

void RemoveLoadingScreenProgressBar() {
  gfUseLoadScreenProgressBar = FALSE;
  RemoveProgressBar(0);
  SetFontShadow(DEFAULT_SHADOW);
}

// This creates a single progress bar given the coordinates without a panel (containing a title and
// background). A panel is automatically created if you specify a title using SetProgressBarTitle
BOOLEAN CreateProgressBar(uint8_t ubProgressBarID, uint16_t usLeft, uint16_t usTop,
                          uint16_t usRight, uint16_t usBottom) {
  PROGRESSBAR *pNew;
  // Allocate new progress bar
  pNew = (PROGRESSBAR *)MemAlloc(sizeof(PROGRESSBAR));
  Assert(pNew);

  if (pBar[ubProgressBarID]) RemoveProgressBar(ubProgressBarID);

  memset(pNew, 0, sizeof(PROGRESSBAR));

  pBar[ubProgressBarID] = pNew;
  pNew->ubProgressBarID = ubProgressBarID;
  // Assign coordinates
  pNew->usBarLeft = usLeft;
  pNew->usBarTop = usTop;
  pNew->usBarRight = usRight;
  pNew->usBarBottom = usBottom;
  // Init default data
  pNew->fPanel = FALSE;
  pNew->usMsgFont = (uint16_t)FONT12POINT1;
  pNew->ubMsgFontForeColor = FONT_BLACK;
  pNew->ubMsgFontShadowColor = 0;
  SetRelativeStartAndEndPercentage(pNew->ubProgressBarID, 0, 100, NULL);
  pNew->swzTitle = NULL;

  // Default the progress bar's color to be red
  pNew->ubColorFillRed = 150;
  pNew->ubColorFillGreen = 0;
  pNew->ubColorFillBlue = 0;

  pNew->fDisplayText = FALSE;

  return TRUE;
}

// You may also define a panel to go in behind the progress bar.  You can now assign a title to go
// with the panel.
void DefineProgressBarPanel(uint32_t ubID, uint8_t r, uint8_t g, uint8_t b, uint16_t usLeft,
                            uint16_t usTop, uint16_t usRight, uint16_t usBottom) {
  PROGRESSBAR *pCurr;
  Assert(ubID < MAX_PROGRESSBARS);
  pCurr = pBar[ubID];
  if (!pCurr) return;

  pCurr->fPanel = TRUE;
  pCurr->usPanelLeft = usLeft;
  pCurr->usPanelTop = usTop;
  pCurr->usPanelRight = usRight;
  pCurr->usPanelBottom = usBottom;
  pCurr->usColor = Get16BPPColor(FROMRGB(r, g, b));
  // Calculate the slightly lighter and darker versions of the same rgb color
  pCurr->usLtColor = Get16BPPColor(FROMRGB((uint8_t)min(255, (uint16_t)(r * 1.33)),
                                           (uint8_t)min(255, (uint16_t)(g * 1.33)),
                                           (uint8_t)min(255, (uint16_t)(b * 1.33))));
  pCurr->usDkColor =
      Get16BPPColor(FROMRGB((uint8_t)(r * 0.75), (uint8_t)(g * 0.75), (uint8_t)(b * 0.75)));
}

// Assigning a title for the panel will automatically position the text horizontally centered on the
// panel and vertically centered from the top of the panel, to the top of the progress bar.
void SetProgressBarTitle(uint32_t ubID, wchar_t *pString, uint32_t usFont, uint8_t ubForeColor,
                         uint8_t ubShadowColor) {
  PROGRESSBAR *pCurr;
  Assert(ubID < MAX_PROGRESSBARS);
  pCurr = pBar[ubID];
  if (!pCurr) return;
  if (pCurr->swzTitle) {
    MemFree(pCurr->swzTitle);
    pCurr->swzTitle = NULL;
  }
  if (pString && wcslen(pString)) {
    int bufSize = wcslen(pString) + 1;
    pCurr->swzTitle = (wchar_t *)MemAlloc(sizeof(wchar_t) * bufSize);
    swprintf(pCurr->swzTitle, bufSize, pString);
  }
  pCurr->usTitleFont = (uint16_t)usFont;
  pCurr->ubTitleFontForeColor = ubForeColor;
  pCurr->ubTitleFontShadowColor = ubShadowColor;
}

// Unless you set up the attributes, any text you pass to SetRelativeStartAndEndPercentage will
// default to FONT12POINT1 in a black color.
void SetProgressBarMsgAttributes(uint32_t ubID, uint32_t usFont, uint8_t ubForeColor,
                                 uint8_t ubShadowColor) {
  PROGRESSBAR *pCurr;
  Assert(ubID < MAX_PROGRESSBARS);
  pCurr = pBar[ubID];
  if (!pCurr) return;
  pCurr->usMsgFont = (uint16_t)usFont;
  pCurr->ubMsgFontForeColor = ubForeColor;
  pCurr->ubMsgFontShadowColor = ubShadowColor;
}

// When finished, the progress bar needs to be removed.
void RemoveProgressBar(uint8_t ubID) {
  Assert(ubID < MAX_PROGRESSBARS);
  if (pBar[ubID]) {
    if (pBar[ubID]->swzTitle) MemFree(pBar[ubID]->swzTitle);
    MemFree(pBar[ubID]);
    pBar[ubID] = NULL;
    return;
  }
}

// An important setup function.  The best explanation is through example.  The example being the
// loading of a file -- there are many stages of the map loading.  In JA2, the first step is to load
// the tileset. Because it is a large chunk of the total loading of the map, we may gauge that it
// takes up 30% of the total load.  Because it is also at the beginning, we would pass in the
// arguments ( 0, 30, "text" ). As the process animates using UpdateProgressBar( 0 to 100 ), the
// total progress bar will only reach 30% at the 100% mark within UpdateProgressBar.  At that time,
// you would go onto the next step, resetting the relative start and end percentage from 30 to
// whatever, until your done.
void SetRelativeStartAndEndPercentage(uint8_t ubID, uint32_t uiRelStartPerc, uint32_t uiRelEndPerc,
                                      wchar_t *str) {
  PROGRESSBAR *pCurr;
  uint16_t usStartX, usStartY;

  Assert(ubID < MAX_PROGRESSBARS);
  pCurr = pBar[ubID];
  if (!pCurr) return;

  pCurr->rStart = uiRelStartPerc * 0.01;
  pCurr->rEnd = uiRelEndPerc * 0.01;

  // Render the entire panel now, as it doesn't need update during the normal rendering
  if (pCurr->fPanel) {
    // Draw panel
    VSurfaceColorFill(vsFB, pCurr->usPanelLeft, pCurr->usPanelTop, pCurr->usPanelRight,
                      pCurr->usPanelBottom, pCurr->usLtColor);
    VSurfaceColorFill(vsFB, pCurr->usPanelLeft + 1, pCurr->usPanelTop + 1, pCurr->usPanelRight,
                      pCurr->usPanelBottom, pCurr->usDkColor);
    VSurfaceColorFill(vsFB, pCurr->usPanelLeft + 1, pCurr->usPanelTop + 1, pCurr->usPanelRight - 1,
                      pCurr->usPanelBottom - 1, pCurr->usColor);
    InvalidateRegion(pCurr->usPanelLeft, pCurr->usPanelTop, pCurr->usPanelRight,
                     pCurr->usPanelBottom);
    // Draw title

    if (pCurr->swzTitle) {
      usStartX = pCurr->usPanelLeft +                                       // left position
                 (pCurr->usPanelRight - pCurr->usPanelLeft) / 2 -           // + half width
                 StringPixLength(pCurr->swzTitle, pCurr->usTitleFont) / 2;  // - half string width
      usStartY = pCurr->usPanelTop + 3;
      SetFont(pCurr->usTitleFont);
      SetFontForeground(pCurr->ubTitleFontForeColor);
      SetFontShadow(pCurr->ubTitleFontShadowColor);
      SetFontBackground(0);
      mprintf(usStartX, usStartY, pCurr->swzTitle);
    }
  }

  if (pCurr->fDisplayText) {
    // Draw message
    if (str) {
      if (pCurr->fUseSaveBuffer) {
        uint16_t usFontHeight = GetFontHeight(pCurr->usMsgFont);

        RestoreExternBackgroundRect(pCurr->usBarLeft, pCurr->usBarBottom,
                                    (int16_t)(pCurr->usBarRight - pCurr->usBarLeft),
                                    (int16_t)(usFontHeight + 3));
      }

      SetFont(pCurr->usMsgFont);
      SetFontForeground(pCurr->ubMsgFontForeColor);
      SetFontShadow(pCurr->ubMsgFontShadowColor);
      SetFontBackground(0);
      mprintf(pCurr->usBarLeft, pCurr->usBarBottom + 3, str);
    }
  }
}

// This part renders the progress bar at the percentage level that you specify.  If you have set
// relative percentage values in the above function, then the uiPercentage will be reflected based
// off of the relative percentages.
void RenderProgressBar(uint8_t ubID, uint32_t uiPercentage) {
  static uint32_t uiLastTime = 0;
  uint32_t uiCurTime = GetJA2Clock();
  double rActual;
  PROGRESSBAR *pCurr = NULL;
  // uint32_t r, g;
  int32_t end;

  Assert(ubID < MAX_PROGRESSBARS);
  pCurr = pBar[ubID];

  if (pCurr == NULL) return;

  if (pCurr) {
    rActual = pCurr->rStart + (pCurr->rEnd - pCurr->rStart) * uiPercentage * 0.01;

    if (rActual - pCurr->rLastActual < 0.01) {
      return;
    }

    pCurr->rLastActual = (double)((int32_t)(rActual * 100) * 0.01);

    end = (int32_t)(pCurr->usBarLeft + 2.0 + rActual * (pCurr->usBarRight - pCurr->usBarLeft - 4));
    if (end < pCurr->usBarLeft + 2 || end > pCurr->usBarRight - 2) {
      return;
    }
    if (gfUseLoadScreenProgressBar) {
      VSurfaceColorFill(vsFB, pCurr->usBarLeft, pCurr->usBarTop, end, pCurr->usBarBottom,
                        Get16BPPColor(FROMRGB(pCurr->ubColorFillRed, pCurr->ubColorFillGreen,
                                              pCurr->ubColorFillBlue)));
    } else {
      // Border edge of the progress bar itself in gray
      VSurfaceColorFill(vsFB, pCurr->usBarLeft, pCurr->usBarTop, pCurr->usBarRight,
                        pCurr->usBarBottom, Get16BPPColor(FROMRGB(160, 160, 160)));
      // Interior of progress bar in black
      VSurfaceColorFill(vsFB, pCurr->usBarLeft + 2, pCurr->usBarTop + 2, pCurr->usBarRight - 2,
                        pCurr->usBarBottom - 2, Get16BPPColor(FROMRGB(0, 0, 0)));
      VSurfaceColorFill(vsFB, pCurr->usBarLeft + 2, pCurr->usBarTop + 2, end,
                        pCurr->usBarBottom - 2, Get16BPPColor(FROMRGB(72, 155, 24)));
    }
    InvalidateRegion(pCurr->usBarLeft, pCurr->usBarTop, pCurr->usBarRight, pCurr->usBarBottom);
    ExecuteBaseDirtyRectQueue();
    EndFrameBufferRender();
    RefreshScreen();
  }

  // update music here
  if (uiCurTime > (uiLastTime + 200)) {
    MusicPoll(TRUE);
    uiLastTime = GetJA2Clock();
  }
}

void SetProgressBarColor(uint8_t ubID, uint8_t ubColorFillRed, uint8_t ubColorFillGreen,
                         uint8_t ubColorFillBlue) {
  PROGRESSBAR *pCurr = NULL;

  Assert(ubID < MAX_PROGRESSBARS);

  pCurr = pBar[ubID];
  if (pCurr == NULL) return;

  pCurr->ubColorFillRed = ubColorFillRed;
  pCurr->ubColorFillGreen = ubColorFillGreen;
  pCurr->ubColorFillBlue = ubColorFillBlue;
}

void SetProgressBarTextDisplayFlag(uint8_t ubID, BOOLEAN fDisplayText, BOOLEAN fUseSaveBuffer,
                                   BOOLEAN fSaveScreenToFrameBuffer) {
  PROGRESSBAR *pCurr = NULL;

  Assert(ubID < MAX_PROGRESSBARS);

  pCurr = pBar[ubID];
  if (pCurr == NULL) return;

  pCurr->fDisplayText = fDisplayText;

  pCurr->fUseSaveBuffer = fUseSaveBuffer;

  // if we are to use the save buffer, blit the portion of the screen to the save buffer
  if (fSaveScreenToFrameBuffer) {
    uint16_t usFontHeight = GetFontHeight(pCurr->usMsgFont) + 3;

    // blit everything to the save buffer ( cause the save buffer can bleed through )
    VSurfaceBlitBufToBuf(vsFB, vsSB, pCurr->usBarLeft, pCurr->usBarBottom,
                         (uint16_t)(pCurr->usBarRight - pCurr->usBarLeft), usFontHeight);
  }
}
