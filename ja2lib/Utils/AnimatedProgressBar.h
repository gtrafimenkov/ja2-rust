#ifndef __ANIMATED_PROGRESSBAR_H
#define __ANIMATED_PROGRESSBAR_H

#include "SGP/Types.h"

#define MAX_PROGRESSBARS 4

typedef struct PROGRESSBAR {
  uint8_t ubProgressBarID;
  uint16_t usBarLeft, usBarTop, usBarRight, usBarBottom;
  BOOLEAN fPanel;
  uint16_t usPanelLeft, usPanelTop, usPanelRight, usPanelBottom;
  uint16_t usColor, usLtColor, usDkColor;
  wchar_t* swzTitle;
  uint16_t usTitleFont;
  uint8_t ubTitleFontForeColor, ubTitleFontShadowColor;
  uint16_t usMsgFont;
  uint8_t ubMsgFontForeColor, ubMsgFontShadowColor;
  uint8_t ubRelativeStartPercentage, ubRelativeEndPercentage;
  uint8_t ubColorFillRed;
  uint8_t ubColorFillGreen;
  uint8_t ubColorFillBlue;
  double rStart, rEnd;
  BOOLEAN fDisplayText;
  BOOLEAN fUseSaveBuffer;  // use the save buffer when display the text
  double rLastActual;
} PROGRESSBAR;

extern PROGRESSBAR* pBar[MAX_PROGRESSBARS];

void CreateLoadingScreenProgressBar();
void RemoveLoadingScreenProgressBar();

// This creates a single progress bar given the coordinates without a panel (containing a title and
// background). A panel is automatically created if you specify a title using SetProgressBarTitle
BOOLEAN CreateProgressBar(uint8_t ubProgressBarID, uint16_t usLeft, uint16_t usTop, uint16_t usRight,
                          uint16_t usBottom);

// You may also define a panel to go in behind the progress bar.  You can now assign a title to go
// with the panel.
void DefineProgressBarPanel(uint32_t ubID, uint8_t r, uint8_t g, uint8_t b, uint16_t usLeft, uint16_t usTop,
                            uint16_t usRight, uint16_t usBottom);

// Assigning a title for the panel will automatically position the text horizontally centered on the
// panel and vertically centered from the top of the panel, to the top of the progress bar.
void SetProgressBarTitle(uint32_t ubID, wchar_t* pString, uint32_t usFont, uint8_t ubForeColor,
                         uint8_t ubShadowColor);

// Unless you set up the attributes, any text you pass to SetRelativeStartAndEndPercentage will
// default to FONT12POINT1 in a black color.
void SetProgressBarMsgAttributes(uint32_t ubID, uint32_t usFont, uint8_t ubForeColor,
                                 uint8_t ubShadowColor);

// When finished, the progress bar needs to be removed.
void RemoveProgressBar(uint8_t ubID);

// An important setup function.  The best explanation is through example.  The example being the
// loading of a file -- there are many stages of the map loading.  In JA2, the first step is to load
// the tileset. Because it is a large chunk of the total loading of the map, we may gauge that it
// takes up 30% of the total load.  Because it is also at the beginning, we would pass in the
// arguments ( 0, 30, "text" ). As the process animates using UpdateProgressBar( 0 to 100 ), the
// total progress bar will only reach 30% at the 100% mark within UpdateProgressBar.  At that time,
// you would go onto the next step, resetting the relative start and end percentage from 30 to
// whatever, until your done.
void SetRelativeStartAndEndPercentage(uint8_t ubID, uint32_t uiRelStartPerc, uint32_t uiRelEndPerc,
                                      wchar_t* str);

// This part renders the progress bar at the percentage level that you specify.  If you have set
// relative percentage values in the above function, then the uiPercentage will be reflected based
// off of the relative percentages.
void RenderProgressBar(uint8_t ubID, uint32_t uiPercentage);

// Sets the color of the progress bars main color.
void SetProgressBarColor(uint8_t ubID, uint8_t ubColorFillRed, uint8_t ubColorFillGreen,
                         uint8_t ubColorFillBlue);

// Pass in TRUE to display the strings.
void SetProgressBarTextDisplayFlag(uint8_t ubID, BOOLEAN fDisplayText, BOOLEAN fUseSaveBuffer,
                                   BOOLEAN fSaveScreenToFrameBuffer);

#endif
