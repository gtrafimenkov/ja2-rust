#ifndef __POPUP_BOX
#define __POPUP_BOX

#include "SGP/Types.h"

#define MAX_POPUP_BOX_COUNT 20
#define MAX_POPUP_BOX_STRING_COUNT \
  50  // worst case = 45: move menu with 20 soldiers, each on different squad + overhead

// PopUpBox Flags
#define POPUP_BOX_FLAG_CLIP_TEXT 1
#define POPUP_BOX_FLAG_CENTER_TEXT 2
#define POPUP_BOX_FLAG_RESIZE 4
#define POPUP_BOX_FLAG_CAN_HIGHLIGHT_SHADED_LINES 8

struct popupstring {
  STR16 pString;
  uint8_t ubForegroundColor;
  uint8_t ubBackgroundColor;
  uint8_t ubHighLight;
  uint8_t ubShade;
  uint8_t ubSecondaryShade;
  uint32_t uiFont;
  BOOLEAN fColorFlag;
  BOOLEAN fHighLightFlag;
  BOOLEAN fShadeFlag;
  BOOLEAN fSecondaryShadeFlag;
};

extern struct popupstring popupstring;

typedef struct popupstring POPUPSTRING;
typedef POPUPSTRING *POPUPSTRINGPTR;

struct popupbox {
  SGPRect Dimensions;
  SGPPoint Position;
  uint32_t uiLeftMargin;
  uint32_t uiRightMargin;
  uint32_t uiBottomMargin;
  uint32_t uiTopMargin;
  uint32_t uiLineSpace;
  INT32 iBorderObjectIndex;
  INT32 iBackGroundSurface;
  uint32_t uiFlags;
  uint32_t uiBuffer;
  uint32_t uiSecondColumnMinimunOffset;
  uint32_t uiSecondColumnCurrentOffset;
  uint32_t uiBoxMinWidth;
  BOOLEAN fUpdated;
  BOOLEAN fShowBox;

  POPUPSTRINGPTR Text[MAX_POPUP_BOX_STRING_COUNT];
  POPUPSTRINGPTR pSecondColumnString[MAX_POPUP_BOX_STRING_COUNT];
};

typedef struct popupbox PopUpBo;
typedef PopUpBo *PopUpBoxPt;

// functions
void InitPopUpBoxList();
BOOLEAN CreatePopUpBox(INT32 *hBoxHandle, SGPRect Dimensions, SGPPoint Position, uint32_t uiFlags);

void SetMargins(INT32 hBoxHandle, uint32_t uiLeft, uint32_t uiTop, uint32_t uiBottom,
                uint32_t uiRight);
uint32_t GetTopMarginSize(INT32 hBoxHandle);
void SetLineSpace(INT32 hBoxHandle, uint32_t uiLineSpace);
uint32_t GetLineSpace(INT32 hBoxHandle);
void SetBoxBuffer(INT32 hBoxHandle, uint32_t uiBuffer);
void SetBoxPosition(INT32 hBoxHandle, SGPPoint Position);
void GetBoxPosition(INT32 hBoxHandle, SGPPoint *Position);
uint32_t GetNumberOfLinesOfTextInBox(INT32 hBoxHandle);
void SetBoxSize(INT32 hBoxHandle, SGPRect Dimensions);
void GetBoxSize(INT32 hBoxHandle, SGPRect *Dimensions);
void SetBoxFlags(INT32 hBoxHandle, uint32_t uiFlags);
void SetBorderType(INT32 hBoxHandle, INT32 BorderObjectIndex);
void SetBackGroundSurface(INT32 hBoxHandle, INT32 BackGroundSurfaceIndex);
void AddMonoString(uint32_t *hStringHandle, STR16 pString);
void AddColorString(INT32 *hStringHandle, STR16 pString);
void SetPopUpStringFont(INT32 hStringHandle, uint32_t uiFont);
void SetBoxFont(INT32 hBoxHandle, uint32_t uiFont);
uint32_t GetBoxFont(INT32 hBoxHandle);
void SetStringForeground(INT32 hStringHandle, uint8_t ubColor);
void SetStringBackground(INT32 hStringHandle, uint8_t ubColor);
void SetStringHighLight(INT32 hStringHandle, uint8_t ubColor);
void SetStringShade(INT32 hStringHandle, uint8_t ubShade);
void SetBoxForeground(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxBackground(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxHighLight(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxShade(INT32 hBoxHandle, uint8_t ubColor);
void ShadeStringInBox(INT32 hBoxHandle, INT32 iLineNumber);
void UnShadeStringInBox(INT32 hBoxHandle, INT32 iLineNumber);
void HighLightLine(INT32 hStringHandle);
void HighLightBoxLine(INT32 hBoxHandle, INT32 iLineNumber);
void UnHighLightLine(INT32 hStringHandle);
void UnHighLightBox(INT32 hBoxHandle);
void RemoveOneCurrentBoxString(INT32 hStringHandle, BOOLEAN fFillGaps);
void RemoveAllCurrentBoxStrings(void);
void RemoveBox(INT32 hBoxHandle);
void ShowBox(INT32 hBoxHandle);
void HideBox(INT32 hBoxHandle);
void DisplayBoxes(uint32_t uiBuffer);
void DisplayOnePopupBox(uint32_t uiIndex, uint32_t uiBuffer);
void SetCurrentBox(INT32 hBoxHandle);
void GetCurrentBox(INT32 *hBoxHandle);

// resize this box to the text it contains
void ResizeBoxToText(INT32 hBoxHandle);

// force update/redraw of this boxes background
void ForceUpDateOfBox(uint32_t uiIndex);

// force redraw of ALL boxes
void MarkAllBoxesAsAltered(void);

// is the box being displayed?
BOOLEAN IsBoxShown(uint32_t uiHandle);

// is this line int he current boxed in a shaded state?
BOOLEAN GetShadeFlag(INT32 hStringHandle);

// is this line in the current box set to a shaded state ?
BOOLEAN GetBoxShadeFlag(INT32 hBoxHandle, INT32 iLineNumber);

// set boxes foreground color
void SetBoxLineForeground(INT32 iBox, INT32 iStringValue, uint8_t ubColor);

// hide all visible boxes
void HideAllBoxes(void);

// add the second column monocrome string
void AddSecondColumnMonoString(uint32_t *hStringHandle, STR16 pString);

// set the 2nd column font for this box
void SetBoxSecondColumnFont(INT32 hBoxHandle, uint32_t uiFont);

// set the minimum offset
void SetBoxSecondColumnMinimumOffset(INT32 hBoxHandle, uint32_t uiWidth);
void SetBoxSecondColumnCurrentOffset(INT32 hBoxHandle, uint32_t uiCurrentOffset);
void ResizeBoxForSecondStrings(INT32 hBoxHandle);

// fore ground, background, highlight and shade.. for indivdual strings
void SetStringSecondColumnForeground(INT32 hStringHandle, uint8_t ubColor);
void SetStringSecondColumnBackground(INT32 hStringHandle, uint8_t ubColor);
void SetStringSecondColumnHighLight(INT32 hStringHandle, uint8_t ubColor);
void SetStringSecondColumnShade(INT32 hStringHandle, uint8_t ubShade);

// now on a box wide basis, one if recomened to use this function after adding all the
// strings..rather than on an individual basis
void SetBoxSecondColumnForeground(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnBackground(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnHighLight(INT32 hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnShade(INT32 hBoxHandle, uint8_t ubColor);

// secondary shades for boxes
void UnSecondaryShadeStringInBox(INT32 hBoxHandle, INT32 iLineNumber);
void SecondaryShadeStringInBox(INT32 hBoxHandle, INT32 iLineNumber);
void SetBoxSecondaryShade(INT32 iBox, uint8_t ubColor);

// min width for box
void SpecifyBoxMinWidth(INT32 hBoxHandle, INT32 iMinWidth);

#endif
