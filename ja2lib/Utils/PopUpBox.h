#ifndef __POPUP_BOX
#define __POPUP_BOX

#include "SGP/Types.h"
#include "rust_geometry.h"

#define MAX_POPUP_BOX_COUNT 20
#define MAX_POPUP_BOX_STRING_COUNT \
  50  // worst case = 45: move menu with 20 soldiers, each on different squad + overhead

// PopUpBox Flags
#define POPUP_BOX_FLAG_CLIP_TEXT 1
#define POPUP_BOX_FLAG_CENTER_TEXT 2
#define POPUP_BOX_FLAG_RESIZE 4
#define POPUP_BOX_FLAG_CAN_HIGHLIGHT_SHADED_LINES 8

struct popupstring {
  wchar_t *pString;
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
  struct GRect Dimensions;
  SGPPoint Position;
  uint32_t uiLeftMargin;
  uint32_t uiRightMargin;
  uint32_t uiBottomMargin;
  uint32_t uiTopMargin;
  uint32_t uiLineSpace;
  int32_t iBorderObjectIndex;
  struct Image *backgroundImage;
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
BOOLEAN CreatePopUpBox(int32_t *hBoxHandle, struct GRect Dimensions, SGPPoint Position,
                       uint32_t uiFlags);

void SetMargins(int32_t hBoxHandle, uint32_t uiLeft, uint32_t uiTop, uint32_t uiBottom,
                uint32_t uiRight);
uint32_t GetTopMarginSize(int32_t hBoxHandle);
void SetLineSpace(int32_t hBoxHandle, uint32_t uiLineSpace);
uint32_t GetLineSpace(int32_t hBoxHandle);
void SetBoxBuffer(int32_t hBoxHandle, uint32_t uiBuffer);
void SetBoxPosition(int32_t hBoxHandle, SGPPoint Position);
void GetBoxPosition(int32_t hBoxHandle, SGPPoint *Position);
uint32_t GetNumberOfLinesOfTextInBox(int32_t hBoxHandle);
void SetBoxSize(int32_t hBoxHandle, struct GRect Dimensions);
void GetBoxSize(int32_t hBoxHandle, struct GRect *Dimensions);
void SetBoxFlags(int32_t hBoxHandle, uint32_t uiFlags);
void SetBorderType(int32_t hBoxHandle, int32_t BorderObjectIndex);
void SetBackGroundSurface(int32_t hBoxHandle, struct Image *image);
void AddMonoString(uint32_t *hStringHandle, wchar_t *pString);
void AddColorString(int32_t *hStringHandle, wchar_t *pString);
void SetPopUpStringFont(int32_t hStringHandle, uint32_t uiFont);
void SetBoxFont(int32_t hBoxHandle, uint32_t uiFont);
uint32_t GetBoxFont(int32_t hBoxHandle);
void SetStringForeground(int32_t hStringHandle, uint8_t ubColor);
void SetStringBackground(int32_t hStringHandle, uint8_t ubColor);
void SetStringHighLight(int32_t hStringHandle, uint8_t ubColor);
void SetStringShade(int32_t hStringHandle, uint8_t ubShade);
void SetBoxForeground(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxBackground(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxHighLight(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxShade(int32_t hBoxHandle, uint8_t ubColor);
void ShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber);
void UnShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber);
void HighLightLine(int32_t hStringHandle);
void HighLightBoxLine(int32_t hBoxHandle, int32_t iLineNumber);
void UnHighLightLine(int32_t hStringHandle);
void UnHighLightBox(int32_t hBoxHandle);
void RemoveOneCurrentBoxString(int32_t hStringHandle, BOOLEAN fFillGaps);
void RemoveAllCurrentBoxStrings(void);
void RemoveBox(int32_t hBoxHandle);
void ShowBox(int32_t hBoxHandle);
void HideBox(int32_t hBoxHandle);
void DisplayBoxes(uint32_t uiBuffer);
void DisplayOnePopupBox(uint32_t uiIndex, uint32_t uiBuffer);
void SetCurrentBox(int32_t hBoxHandle);
void GetCurrentBox(int32_t *hBoxHandle);

// resize this box to the text it contains
void ResizeBoxToText(int32_t hBoxHandle);

// force update/redraw of this boxes background
void ForceUpDateOfBox(uint32_t uiIndex);

// force redraw of ALL boxes
void MarkAllBoxesAsAltered(void);

// is the box being displayed?
BOOLEAN IsBoxShown(uint32_t uiHandle);

// is this line int he current boxed in a shaded state?
BOOLEAN GetShadeFlag(int32_t hStringHandle);

// is this line in the current box set to a shaded state ?
BOOLEAN GetBoxShadeFlag(int32_t hBoxHandle, int32_t iLineNumber);

// set boxes foreground color
void SetBoxLineForeground(int32_t iBox, int32_t iStringValue, uint8_t ubColor);

// hide all visible boxes
void HideAllBoxes(void);

// add the second column monocrome string
void AddSecondColumnMonoString(uint32_t *hStringHandle, wchar_t *pString);

// set the 2nd column font for this box
void SetBoxSecondColumnFont(int32_t hBoxHandle, uint32_t uiFont);

// set the minimum offset
void SetBoxSecondColumnMinimumOffset(int32_t hBoxHandle, uint32_t uiWidth);
void SetBoxSecondColumnCurrentOffset(int32_t hBoxHandle, uint32_t uiCurrentOffset);
void ResizeBoxForSecondStrings(int32_t hBoxHandle);

// fore ground, background, highlight and shade.. for indivdual strings
void SetStringSecondColumnForeground(int32_t hStringHandle, uint8_t ubColor);
void SetStringSecondColumnBackground(int32_t hStringHandle, uint8_t ubColor);
void SetStringSecondColumnHighLight(int32_t hStringHandle, uint8_t ubColor);
void SetStringSecondColumnShade(int32_t hStringHandle, uint8_t ubShade);

// now on a box wide basis, one if recomened to use this function after adding all the
// strings..rather than on an individual basis
void SetBoxSecondColumnForeground(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnBackground(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnHighLight(int32_t hBoxHandle, uint8_t ubColor);
void SetBoxSecondColumnShade(int32_t hBoxHandle, uint8_t ubColor);

// secondary shades for boxes
void UnSecondaryShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber);
void SecondaryShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber);
void SetBoxSecondaryShade(int32_t iBox, uint8_t ubColor);

// min width for box
void SpecifyBoxMinWidth(int32_t hBoxHandle, int32_t iMinWidth);

#endif
