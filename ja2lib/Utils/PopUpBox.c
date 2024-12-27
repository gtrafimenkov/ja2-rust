#include "Utils/PopUpBox.h"

#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "TileEngine/SysUtil.h"

#define BORDER_WIDTH 16
#define BORDER_HEIGHT 8
#define TOP_LEFT_CORNER 0
#define TOP_EDGE 4
#define TOP_RIGHT_CORNER 1
#define SIDE_EDGE 5
#define BOTTOM_LEFT_CORNER 2
#define BOTTOM_EDGE 4
#define BOTTOM_RIGHT_CORNER 3

static PopUpBoxPt PopUpBoxList[MAX_POPUP_BOX_COUNT];
static uint32_t guiCurrentBox;

struct popupstring popupstring;

BOOLEAN DrawBox(uint32_t uiCounter);
BOOLEAN DrawBoxText(uint32_t uiCounter);

void RemoveCurrentBoxPrimaryText(int32_t hStringHandle);
void RemoveCurrentBoxSecondaryText(int32_t hStringHandle);

void InitPopUpBoxList() {
  memset(&PopUpBoxList, 0, sizeof(PopUpBoxPt));
  return;
}

void InitPopUpBox(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);
  memset(PopUpBoxList[hBoxHandle], 0, sizeof(PopUpBo));
}

void SetLineSpace(int32_t hBoxHandle, uint32_t uiLineSpace) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);
  PopUpBoxList[hBoxHandle]->uiLineSpace = uiLineSpace;
  return;
}

uint32_t GetLineSpace(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (0);

  Assert(PopUpBoxList[hBoxHandle]);
  // return number of pixels between lines for this box
  return (PopUpBoxList[hBoxHandle]->uiLineSpace);
}

void SpecifyBoxMinWidth(int32_t hBoxHandle, int32_t iMinWidth) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->uiBoxMinWidth = iMinWidth;

  // check if the box is currently too small
  if (PopUpBoxList[hBoxHandle]->Dimensions.iRight < iMinWidth) {
    PopUpBoxList[hBoxHandle]->Dimensions.iRight = iMinWidth;
  }

  return;
}

BOOLEAN CreatePopUpBox(int32_t *phBoxHandle, SGPRect Dimensions, SGPPoint Position,
                       uint32_t uiFlags) {
  int32_t iCounter = 0;
  int32_t iCount = 0;
  PopUpBoxPt pBox = NULL;

  // find first free box
  for (iCounter = 0; (iCounter < MAX_POPUP_BOX_COUNT) && (PopUpBoxList[iCounter] != NULL);
       iCounter++);

  if (iCounter >= MAX_POPUP_BOX_COUNT) {
    // ran out of available popup boxes - probably not freeing them up right!
    Assert(0);
    return FALSE;
  }

  iCount = iCounter;
  *phBoxHandle = iCount;

  pBox = (PopUpBo *)MemAlloc(sizeof(PopUpBo));
  if (pBox == NULL) {
    return FALSE;
  }
  PopUpBoxList[iCount] = pBox;

  InitPopUpBox(iCount);
  SetBoxPosition(iCount, Position);
  SetBoxSize(iCount, Dimensions);
  SetBoxFlags(iCount, uiFlags);

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    PopUpBoxList[iCount]->Text[iCounter] = NULL;
    PopUpBoxList[iCount]->pSecondColumnString[iCounter] = NULL;
  }

  SetCurrentBox(iCount);
  SpecifyBoxMinWidth(iCount, 0);
  SetBoxSecondColumnMinimumOffset(iCount, 0);
  SetBoxSecondColumnCurrentOffset(iCount, 0);

  PopUpBoxList[iCount]->fUpdated = FALSE;

  return TRUE;
}

void SetBoxFlags(int32_t hBoxHandle, uint32_t uiFlags) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->uiFlags = uiFlags;
  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

void SetMargins(int32_t hBoxHandle, uint32_t uiLeft, uint32_t uiTop, uint32_t uiBottom,
                uint32_t uiRight) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->uiLeftMargin = uiLeft;
  PopUpBoxList[hBoxHandle]->uiRightMargin = uiRight;
  PopUpBoxList[hBoxHandle]->uiTopMargin = uiTop;
  PopUpBoxList[hBoxHandle]->uiBottomMargin = uiBottom;

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

uint32_t GetTopMarginSize(int32_t hBoxHandle) {
  // return size of top margin, for mouse region offsets

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (0);

  Assert(PopUpBoxList[hBoxHandle]);

  return (PopUpBoxList[hBoxHandle]->uiTopMargin);
}

void ShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber) {
  // shade iLineNumber Line in box indexed by hBoxHandle

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    // set current box
    SetCurrentBox(hBoxHandle);

    // shade line
    PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fShadeFlag = TRUE;
  }

  return;
}

void UnShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber) {
  // unshade iLineNumber in box indexed by hBoxHandle

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    // set current box
    SetCurrentBox(hBoxHandle);

    // shade line
    PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fShadeFlag = FALSE;
  }

  return;
}

void SecondaryShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber) {
  // shade iLineNumber Line in box indexed by hBoxHandle

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    // set current box
    SetCurrentBox(hBoxHandle);

    // shade line
    PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fSecondaryShadeFlag = TRUE;
  }

  return;
}

void UnSecondaryShadeStringInBox(int32_t hBoxHandle, int32_t iLineNumber) {
  // unshade iLineNumber in box indexed by hBoxHandle

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    // set current box
    SetCurrentBox(hBoxHandle);

    // shade line
    PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fSecondaryShadeFlag = FALSE;
  }

  return;
}

void SetBoxBuffer(int32_t hBoxHandle, uint32_t uiBuffer) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->uiBuffer = uiBuffer;

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

void SetBoxPosition(int32_t hBoxHandle, SGPPoint Position) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->Position.iX = Position.iX;
  PopUpBoxList[hBoxHandle]->Position.iY = Position.iY;

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

void GetBoxPosition(int32_t hBoxHandle, SGPPoint *Position) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  Position->iX = PopUpBoxList[hBoxHandle]->Position.iX;
  Position->iY = PopUpBoxList[hBoxHandle]->Position.iY;

  return;
}

void SetBoxSize(int32_t hBoxHandle, SGPRect Dimensions) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  PopUpBoxList[hBoxHandle]->Dimensions.iLeft = Dimensions.iLeft;
  PopUpBoxList[hBoxHandle]->Dimensions.iBottom = Dimensions.iBottom;
  PopUpBoxList[hBoxHandle]->Dimensions.iRight = Dimensions.iRight;
  PopUpBoxList[hBoxHandle]->Dimensions.iTop = Dimensions.iTop;

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

void GetBoxSize(int32_t hBoxHandle, SGPRect *Dimensions) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);

  Dimensions->iLeft = PopUpBoxList[hBoxHandle]->Dimensions.iLeft;
  Dimensions->iBottom = PopUpBoxList[hBoxHandle]->Dimensions.iBottom;
  Dimensions->iRight = PopUpBoxList[hBoxHandle]->Dimensions.iRight;
  Dimensions->iTop = PopUpBoxList[hBoxHandle]->Dimensions.iTop;

  return;
}

void SetBorderType(int32_t hBoxHandle, int32_t iBorderObjectIndex) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);
  PopUpBoxList[hBoxHandle]->iBorderObjectIndex = iBorderObjectIndex;
  return;
}

void SetBackGroundSurface(int32_t hBoxHandle, int32_t iBackGroundSurfaceIndex) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle]);
  PopUpBoxList[hBoxHandle]->iBackGroundSurface = iBackGroundSurfaceIndex;
  return;
}

// adds a FIRST column string to the CURRENT popup box
void AddMonoString(uint32_t *hStringHandle, wchar_t *pString) {
  wchar_t *pLocalString = NULL;
  POPUPSTRINGPTR pStringSt = NULL;
  uint32_t iCounter = 0;

  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  // find first free slot in list
  for (iCounter = 0; (iCounter < MAX_POPUP_BOX_STRING_COUNT) &&
                     (PopUpBoxList[guiCurrentBox]->Text[iCounter] != NULL);
       iCounter++);

  if (iCounter >= MAX_POPUP_BOX_STRING_COUNT) {
    // using too many text lines, or not freeing them up properly
    Assert(0);
    return;
  }

  pStringSt = (POPUPSTRING *)(MemAlloc(sizeof(POPUPSTRING)));
  if (pStringSt == NULL) return;

  pLocalString = (wchar_t *)(MemAlloc(wcslen(pString) * 2 + 2));
  if (pLocalString == NULL) return;

  wcscpy(pLocalString, pString);

  RemoveCurrentBoxPrimaryText(iCounter);

  PopUpBoxList[guiCurrentBox]->Text[iCounter] = pStringSt;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->fColorFlag = FALSE;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->pString = pLocalString;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->fShadeFlag = FALSE;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->fHighLightFlag = FALSE;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->fSecondaryShadeFlag = FALSE;

  *hStringHandle = iCounter;

  PopUpBoxList[guiCurrentBox]->fUpdated = FALSE;

  return;
}

// adds a SECOND column string to the CURRENT popup box
void AddSecondColumnMonoString(uint32_t *hStringHandle, wchar_t *pString) {
  wchar_t *pLocalString = NULL;
  POPUPSTRINGPTR pStringSt = NULL;
  uint32_t iCounter = 0;

  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  // find the LAST USED text string index
  for (iCounter = 0; (iCounter + 1 < MAX_POPUP_BOX_STRING_COUNT) &&
                     (PopUpBoxList[guiCurrentBox]->Text[iCounter + 1] != NULL);
       iCounter++);

  if (iCounter >= MAX_POPUP_BOX_STRING_COUNT) {
    // using too many text lines, or not freeing them up properly
    Assert(0);
    return;
  }

  pStringSt = (POPUPSTRING *)(MemAlloc(sizeof(POPUPSTRING)));
  if (pStringSt == NULL) return;

  pLocalString = (wchar_t *)(MemAlloc(wcslen(pString) * 2 + 2));
  if (pLocalString == NULL) return;

  wcscpy(pLocalString, pString);

  RemoveCurrentBoxSecondaryText(iCounter);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[iCounter] = pStringSt;
  PopUpBoxList[guiCurrentBox]->pSecondColumnString[iCounter]->fColorFlag = FALSE;
  PopUpBoxList[guiCurrentBox]->pSecondColumnString[iCounter]->pString = pLocalString;
  PopUpBoxList[guiCurrentBox]->pSecondColumnString[iCounter]->fShadeFlag = FALSE;
  PopUpBoxList[guiCurrentBox]->pSecondColumnString[iCounter]->fHighLightFlag = FALSE;

  *hStringHandle = iCounter;

  return;
}

// Adds a COLORED first column string to the CURRENT box
void AddColorString(int32_t *hStringHandle, wchar_t *pString) {
  wchar_t *pLocalString;
  POPUPSTRINGPTR pStringSt = NULL;
  int32_t iCounter = 0;

  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  // find first free slot in list
  for (iCounter = 0; (iCounter < MAX_POPUP_BOX_STRING_COUNT) &&
                     (PopUpBoxList[guiCurrentBox]->Text[iCounter] != NULL);
       iCounter++);

  if (iCounter >= MAX_POPUP_BOX_STRING_COUNT) {
    // using too many text lines, or not freeing them up properly
    Assert(0);
    return;
  }

  pStringSt = (POPUPSTRING *)(MemAlloc(sizeof(POPUPSTRING)));
  if (pStringSt == NULL) return;

  pLocalString = (wchar_t *)(MemAlloc(wcslen(pString) * 2 + 2));
  if (pLocalString == NULL) return;

  wcscpy(pLocalString, pString);

  RemoveCurrentBoxPrimaryText(iCounter);

  PopUpBoxList[guiCurrentBox]->Text[iCounter] = pStringSt;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->fColorFlag = TRUE;
  PopUpBoxList[guiCurrentBox]->Text[iCounter]->pString = pLocalString;

  *hStringHandle = iCounter;

  PopUpBoxList[guiCurrentBox]->fUpdated = FALSE;

  return;
}

void ResizeBoxForSecondStrings(int32_t hBoxHandle) {
  int32_t iCounter = 0;
  PopUpBoxPt pBox;
  uint32_t uiBaseWidth, uiThisWidth;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  pBox = (PopUpBoxList[hBoxHandle]);
  Assert(pBox);

  uiBaseWidth = pBox->uiLeftMargin + pBox->uiSecondColumnMinimunOffset;

  // check string sizes
  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (pBox->Text[iCounter]) {
      uiThisWidth = uiBaseWidth +
                    StringPixLength(pBox->Text[iCounter]->pString, pBox->Text[iCounter]->uiFont);

      if (uiThisWidth > pBox->uiSecondColumnCurrentOffset) {
        pBox->uiSecondColumnCurrentOffset = uiThisWidth;
      }
    }
  }
}

uint32_t GetNumberOfLinesOfTextInBox(int32_t hBoxHandle) {
  int32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (0);

  // count number of lines
  // check string size
  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[iCounter] == NULL) {
      break;
    }
  }

  return (iCounter);
}

void SetBoxFont(int32_t hBoxHandle, uint32_t uiFont) {
  uint32_t uiCounter;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[uiCounter] != NULL) {
      PopUpBoxList[hBoxHandle]->Text[uiCounter]->uiFont = uiFont;
    }
  }

  // set up the 2nd column font
  SetBoxSecondColumnFont(hBoxHandle, uiFont);

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

void SetBoxSecondColumnMinimumOffset(int32_t hBoxHandle, uint32_t uiWidth) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  PopUpBoxList[hBoxHandle]->uiSecondColumnMinimunOffset = uiWidth;
  return;
}

void SetBoxSecondColumnCurrentOffset(int32_t hBoxHandle, uint32_t uiCurrentOffset) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  PopUpBoxList[hBoxHandle]->uiSecondColumnCurrentOffset = uiCurrentOffset;
  return;
}

void SetBoxSecondColumnFont(int32_t hBoxHandle, uint32_t uiFont) {
  uint32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]) {
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->uiFont = uiFont;
    }
  }

  PopUpBoxList[hBoxHandle]->fUpdated = FALSE;

  return;
}

uint32_t GetBoxFont(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (0);

  Assert(PopUpBoxList[hBoxHandle]);
  Assert(PopUpBoxList[hBoxHandle]->Text[0]);

  // return font id of first line of text of box
  return (PopUpBoxList[hBoxHandle]->Text[0]->uiFont);
}

// set the foreground color of this string in this pop up box
void SetBoxLineForeground(int32_t iBox, int32_t iStringValue, uint8_t ubColor) {
  if ((iBox < 0) || (iBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[iBox]);
  Assert(PopUpBoxList[iBox]->Text[iStringValue]);

  PopUpBoxList[iBox]->Text[iStringValue]->ubForegroundColor = ubColor;
  return;
}

void SetBoxSecondaryShade(int32_t iBox, uint8_t ubColor) {
  uint32_t uiCounter;

  if ((iBox < 0) || (iBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[iBox]);

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[iBox]->Text[uiCounter] != NULL) {
      PopUpBoxList[iBox]->Text[uiCounter]->ubSecondaryShade = ubColor;
    }
  }
  return;
}

// The following functions operate on the CURRENT box

void SetPopUpStringFont(int32_t hStringHandle, uint32_t uiFont) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->uiFont = uiFont;
  return;
}

void SetPopUpSecondColumnStringFont(int32_t hStringHandle, uint32_t uiFont) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->uiFont = uiFont;
  return;
}

void SetStringSecondaryShade(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->ubSecondaryShade = ubColor;
  return;
}

void SetStringForeground(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->ubForegroundColor = ubColor;
  return;
}

void SetStringBackground(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->ubBackgroundColor = ubColor;
  return;
}

void SetStringHighLight(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->ubHighLight = ubColor;
  return;
}

void SetStringShade(int32_t hStringHandle, uint8_t ubShade) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->ubShade = ubShade;
  return;
}

void SetStringSecondColumnForeground(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->ubForegroundColor = ubColor;
  return;
}

void SetStringSecondColumnBackground(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->ubBackgroundColor = ubColor;
  return;
}

void SetStringSecondColumnHighLight(int32_t hStringHandle, uint8_t ubColor) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->ubHighLight = ubColor;
  return;
}

void SetStringSecondColumnShade(int32_t hStringHandle, uint8_t ubShade) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->ubShade = ubShade;
  return;
}

void SetBoxForeground(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t uiCounter;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[uiCounter] != NULL) {
      PopUpBoxList[hBoxHandle]->Text[uiCounter]->ubForegroundColor = ubColor;
    }
  }
  return;
}

void SetBoxBackground(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t uiCounter;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[uiCounter] != NULL) {
      PopUpBoxList[hBoxHandle]->Text[uiCounter]->ubBackgroundColor = ubColor;
    }
  }
  return;
}

void SetBoxHighLight(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t uiCounter;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[uiCounter] != NULL) {
      PopUpBoxList[hBoxHandle]->Text[uiCounter]->ubHighLight = ubColor;
    }
  }
  return;
}

void SetBoxShade(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t uiCounter;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[uiCounter] != NULL) {
      PopUpBoxList[hBoxHandle]->Text[uiCounter]->ubShade = ubColor;
    }
  }
  return;
}

void SetBoxSecondColumnForeground(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]) {
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->ubForegroundColor = ubColor;
    }
  }

  return;
}

void SetBoxSecondColumnBackground(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]) {
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->ubBackgroundColor = ubColor;
    }
  }

  return;
}

void SetBoxSecondColumnHighLight(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]) {
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->ubHighLight = ubColor;
    }
  }

  return;
}

void SetBoxSecondColumnShade(int32_t hBoxHandle, uint8_t ubColor) {
  uint32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[hBoxHandle] != NULL);

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]) {
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->ubShade = ubColor;
    }
  }
  return;
}

void HighLightLine(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  if (!PopUpBoxList[guiCurrentBox]->Text[hStringHandle]) return;
  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->fHighLightFlag = TRUE;
  return;
}

BOOLEAN GetShadeFlag(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  if (!PopUpBoxList[guiCurrentBox]->Text[hStringHandle]) return (FALSE);

  return (PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->fShadeFlag);
}

BOOLEAN GetSecondaryShadeFlag(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  if (!PopUpBoxList[guiCurrentBox]->Text[hStringHandle]) return (FALSE);

  return (PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->fSecondaryShadeFlag);
}

void HighLightBoxLine(int32_t hBoxHandle, int32_t iLineNumber) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  // highlight iLineNumber Line in box indexed by hBoxHandle

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    // set current box
    SetCurrentBox(hBoxHandle);

    // highlight line
    HighLightLine(iLineNumber);
  }

  return;
}

BOOLEAN GetBoxShadeFlag(int32_t hBoxHandle, int32_t iLineNumber) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    return (PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fShadeFlag);
  }

  return (FALSE);
}

BOOLEAN GetBoxSecondaryShadeFlag(int32_t hBoxHandle, int32_t iLineNumber) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  if (PopUpBoxList[hBoxHandle]->Text[iLineNumber] != NULL) {
    return (PopUpBoxList[hBoxHandle]->Text[iLineNumber]->fSecondaryShadeFlag);
  }

  return (FALSE);
}

void UnHighLightLine(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  if (!PopUpBoxList[guiCurrentBox]->Text[hStringHandle]) return;
  PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->fHighLightFlag = FALSE;
  return;
}

void UnHighLightBox(int32_t hBoxHandle) {
  int32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->Text[iCounter])
      PopUpBoxList[hBoxHandle]->Text[iCounter]->fHighLightFlag = FALSE;
  }
}

void UnHighLightSecondColumnLine(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);

  if (!PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]) return;

  PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->fHighLightFlag = FALSE;
  return;
}

void UnHighLightSecondColumnBox(int32_t hBoxHandle) {
  int32_t iCounter = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  for (iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++) {
    if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter])
      PopUpBoxList[hBoxHandle]->pSecondColumnString[iCounter]->fHighLightFlag = FALSE;
  }
}

void RemoveOneCurrentBoxString(int32_t hStringHandle, BOOLEAN fFillGaps) {
  uint32_t uiCounter = 0;

  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(hStringHandle < MAX_POPUP_BOX_STRING_COUNT);

  RemoveCurrentBoxPrimaryText(hStringHandle);
  RemoveCurrentBoxSecondaryText(hStringHandle);

  if (fFillGaps) {
    // shuffle all strings down a slot to fill in the gap
    for (uiCounter = hStringHandle; uiCounter < (MAX_POPUP_BOX_STRING_COUNT - 1); uiCounter++) {
      PopUpBoxList[guiCurrentBox]->Text[uiCounter] =
          PopUpBoxList[guiCurrentBox]->Text[uiCounter + 1];
      PopUpBoxList[guiCurrentBox]->pSecondColumnString[uiCounter] =
          PopUpBoxList[guiCurrentBox]->pSecondColumnString[uiCounter + 1];
    }
  }

  PopUpBoxList[guiCurrentBox]->fUpdated = FALSE;
}

void RemoveAllCurrentBoxStrings(void) {
  uint32_t uiCounter;

  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_STRING_COUNT; uiCounter++)
    RemoveOneCurrentBoxString(uiCounter, FALSE);
}

void RemoveBox(int32_t hBoxHandle) {
  int32_t hOldBoxHandle;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  GetCurrentBox(&hOldBoxHandle);
  SetCurrentBox(hBoxHandle);

  RemoveAllCurrentBoxStrings();

  MemFree(PopUpBoxList[hBoxHandle]);
  PopUpBoxList[hBoxHandle] = NULL;

  if (hOldBoxHandle != hBoxHandle) SetCurrentBox(hOldBoxHandle);

  return;
}

void ShowBox(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  if (PopUpBoxList[hBoxHandle] != NULL) {
    if (PopUpBoxList[hBoxHandle]->fShowBox == FALSE) {
      PopUpBoxList[hBoxHandle]->fShowBox = TRUE;
      PopUpBoxList[hBoxHandle]->fUpdated = FALSE;
    }
  }
}

void HideBox(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  if (PopUpBoxList[hBoxHandle] != NULL) {
    if (PopUpBoxList[hBoxHandle]->fShowBox == TRUE) {
      PopUpBoxList[hBoxHandle]->fShowBox = FALSE;
      PopUpBoxList[hBoxHandle]->fUpdated = FALSE;
    }
  }
}

void SetCurrentBox(int32_t hBoxHandle) {
  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  guiCurrentBox = hBoxHandle;
}

void GetCurrentBox(int32_t *hBoxHandle) { *hBoxHandle = guiCurrentBox; }

void DisplayBoxes(uint32_t uiBuffer) {
  uint32_t uiCounter;

  for (uiCounter = 0; uiCounter < MAX_POPUP_BOX_COUNT; uiCounter++) {
    DisplayOnePopupBox(uiCounter, uiBuffer);
  }
  return;
}

void DisplayOnePopupBox(uint32_t uiIndex, uint32_t uiBuffer) {
  if ((uiIndex < 0) || (uiIndex >= MAX_POPUP_BOX_COUNT)) return;

  if (PopUpBoxList[uiIndex] != NULL) {
    if ((PopUpBoxList[uiIndex]->uiBuffer == uiBuffer) && (PopUpBoxList[uiIndex]->fShowBox)) {
      DrawBox(uiIndex);
      DrawBoxText(uiIndex);
    }
  }
}

// force an update of this box
void ForceUpDateOfBox(uint32_t uiIndex) {
  if ((uiIndex < 0) || (uiIndex >= MAX_POPUP_BOX_COUNT)) return;

  if (PopUpBoxList[uiIndex] != NULL) {
    PopUpBoxList[uiIndex]->fUpdated = FALSE;
  }
}

BOOLEAN DrawBox(uint32_t uiCounter) {
  // will build pop up box in usTopX, usTopY with dimensions usWidth and usHeight
  uint32_t uiNumTilesWide;
  uint32_t uiNumTilesHigh;
  uint32_t uiCount = 0;
  struct VObject *hBoxHandle;
  struct VSurface *hSrcVSurface;
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  uint8_t *pSrcBuf;
  SGPRect clip;
  uint16_t usTopX, usTopY;
  uint16_t usWidth, usHeight;

  if ((uiCounter < 0) || (uiCounter >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  Assert(PopUpBoxList[uiCounter] != NULL);

  // only update if we need to

  if (PopUpBoxList[uiCounter]->fUpdated == TRUE) {
    return (FALSE);
  }

  PopUpBoxList[uiCounter]->fUpdated = TRUE;

  if (PopUpBoxList[uiCounter]->uiFlags & POPUP_BOX_FLAG_RESIZE) {
    ResizeBoxToText(uiCounter);
  }

  usTopX = (uint16_t)PopUpBoxList[uiCounter]->Position.iX;
  usTopY = (uint16_t)PopUpBoxList[uiCounter]->Position.iY;
  usWidth = ((uint16_t)(PopUpBoxList[uiCounter]->Dimensions.iRight -
                        PopUpBoxList[uiCounter]->Dimensions.iLeft));
  usHeight = ((uint16_t)(PopUpBoxList[uiCounter]->Dimensions.iBottom -
                         PopUpBoxList[uiCounter]->Dimensions.iTop));

  // check if we have a min width, if so then update box for such
  if ((PopUpBoxList[uiCounter]->uiBoxMinWidth) &&
      (usWidth < PopUpBoxList[uiCounter]->uiBoxMinWidth)) {
    usWidth = (int16_t)(PopUpBoxList[uiCounter]->uiBoxMinWidth);
  }

  // make sure it will fit on screen!
  Assert(usTopX + usWidth <= 639);
  Assert(usTopY + usHeight <= 479);

  // subtract 4 because the 2 2-pixel corners are handled separately
  uiNumTilesWide = ((usWidth - 4) / BORDER_WIDTH);
  uiNumTilesHigh = ((usHeight - 4) / BORDER_HEIGHT);

  clip.iLeft = 0;
  clip.iRight = clip.iLeft + usWidth;
  clip.iTop = 0;
  clip.iBottom = clip.iTop + usHeight;

  // blit in texture first, then borders
  // blit in surface
  pDestBuf = (uint16_t *)LockVideoSurface(PopUpBoxList[uiCounter]->uiBuffer, &uiDestPitchBYTES);
  CHECKF(GetVideoSurface(&hSrcVSurface, PopUpBoxList[uiCounter]->iBackGroundSurface));
  pSrcBuf = LockVideoSurface(PopUpBoxList[uiCounter]->iBackGroundSurface, &uiSrcPitchBYTES);
  Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, hSrcVSurface, pSrcBuf, uiSrcPitchBYTES,
                              usTopX, usTopY, &clip);
  UnLockVideoSurface(PopUpBoxList[uiCounter]->iBackGroundSurface);
  UnLockVideoSurface(PopUpBoxList[uiCounter]->uiBuffer);
  GetVideoObject(&hBoxHandle, PopUpBoxList[uiCounter]->iBorderObjectIndex);

  // blit in 4 corners (they're 2x2 pixels)
  BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, TOP_LEFT_CORNER, usTopX, usTopY,
                 VO_BLT_SRCTRANSPARENCY, NULL);
  BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, TOP_RIGHT_CORNER,
                 usTopX + usWidth - 2, usTopY, VO_BLT_SRCTRANSPARENCY, NULL);
  BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, BOTTOM_RIGHT_CORNER,
                 usTopX + usWidth - 2, usTopY + usHeight - 2, VO_BLT_SRCTRANSPARENCY, NULL);
  BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, BOTTOM_LEFT_CORNER, usTopX,
                 usTopY + usHeight - 2, VO_BLT_SRCTRANSPARENCY, NULL);

  // blit in edges
  if (uiNumTilesWide > 0) {
    // full pieces
    for (uiCount = 0; uiCount < uiNumTilesWide; uiCount++) {
      BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, TOP_EDGE,
                     usTopX + 2 + (uiCount * BORDER_WIDTH), usTopY, VO_BLT_SRCTRANSPARENCY, NULL);
      BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, BOTTOM_EDGE,
                     usTopX + 2 + (uiCount * BORDER_WIDTH), usTopY + usHeight - 2,
                     VO_BLT_SRCTRANSPARENCY, NULL);
    }

    // partial pieces
    BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, TOP_EDGE,
                   usTopX + usWidth - 2 - BORDER_WIDTH, usTopY, VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, BOTTOM_EDGE,
                   usTopX + usWidth - 2 - BORDER_WIDTH, usTopY + usHeight - 2,
                   VO_BLT_SRCTRANSPARENCY, NULL);
  }
  if (uiNumTilesHigh > 0) {
    // full pieces
    for (uiCount = 0; uiCount < uiNumTilesHigh; uiCount++) {
      BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX,
                     usTopY + 2 + (uiCount * BORDER_HEIGHT), VO_BLT_SRCTRANSPARENCY, NULL);
      BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX + usWidth - 2,
                     usTopY + 2 + (uiCount * BORDER_HEIGHT), VO_BLT_SRCTRANSPARENCY, NULL);
    }

    // partial pieces
    BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX,
                   usTopY + usHeight - 2 - BORDER_HEIGHT, VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(PopUpBoxList[uiCounter]->uiBuffer, hBoxHandle, SIDE_EDGE, usTopX + usWidth - 2,
                   usTopY + usHeight - 2 - BORDER_HEIGHT, VO_BLT_SRCTRANSPARENCY, NULL);
  }

  InvalidateRegion(usTopX, usTopY, usTopX + usWidth, usTopY + usHeight);
  return TRUE;
}

BOOLEAN DrawBoxText(uint32_t uiCounter) {
  uint32_t uiCount = 0;
  int16_t uX, uY;
  wchar_t sString[100];

  if ((uiCounter < 0) || (uiCounter >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  Assert(PopUpBoxList[uiCounter] != NULL);

  // clip text?
  if (PopUpBoxList[uiCounter]->uiFlags & POPUP_BOX_FLAG_CLIP_TEXT) {
    SetFontDestBuffer(
        PopUpBoxList[uiCounter]->uiBuffer,
        PopUpBoxList[uiCounter]->Position.iX + PopUpBoxList[uiCounter]->uiLeftMargin - 1,
        PopUpBoxList[uiCounter]->Position.iY + PopUpBoxList[uiCounter]->uiTopMargin,
        PopUpBoxList[uiCounter]->Position.iX + PopUpBoxList[uiCounter]->Dimensions.iRight -
            PopUpBoxList[uiCounter]->uiRightMargin,
        PopUpBoxList[uiCounter]->Position.iY + PopUpBoxList[uiCounter]->Dimensions.iBottom -
            PopUpBoxList[uiCounter]->uiBottomMargin,
        FALSE);
  }

  for (uiCount = 0; uiCount < MAX_POPUP_BOX_STRING_COUNT; uiCount++) {
    // there is text in this line?
    if (PopUpBoxList[uiCounter]->Text[uiCount]) {
      // set font
      SetFont(PopUpBoxList[uiCounter]->Text[uiCount]->uiFont);

      // are we highlighting?...shading?..or neither
      if ((PopUpBoxList[uiCounter]->Text[uiCount]->fHighLightFlag == FALSE) &&
          (PopUpBoxList[uiCounter]->Text[uiCount]->fShadeFlag == FALSE) &&
          (PopUpBoxList[uiCounter]->Text[uiCount]->fSecondaryShadeFlag == FALSE)) {
        // neither
        SetFontForeground(PopUpBoxList[uiCounter]->Text[uiCount]->ubForegroundColor);
      } else if ((PopUpBoxList[uiCounter]->Text[uiCount]->fHighLightFlag == TRUE)) {
        // highlight
        SetFontForeground(PopUpBoxList[uiCounter]->Text[uiCount]->ubHighLight);
      } else if ((PopUpBoxList[uiCounter]->Text[uiCount]->fSecondaryShadeFlag == TRUE)) {
        SetFontForeground(PopUpBoxList[uiCounter]->Text[uiCount]->ubSecondaryShade);
      } else {
        // shading
        SetFontForeground(PopUpBoxList[uiCounter]->Text[uiCount]->ubShade);
      }

      // set background
      SetFontBackground(PopUpBoxList[uiCounter]->Text[uiCount]->ubBackgroundColor);

      // copy string
      wcsncpy(sString, PopUpBoxList[uiCounter]->Text[uiCount]->pString,
              wcslen(PopUpBoxList[uiCounter]->Text[uiCount]->pString) + 1);

      // cnetering?
      if (PopUpBoxList[uiCounter]->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT) {
        FindFontCenterCoordinates(
            ((int16_t)(PopUpBoxList[uiCounter]->Position.iX +
                       PopUpBoxList[uiCounter]->uiLeftMargin)),
            ((int16_t)(PopUpBoxList[uiCounter]->Position.iY +
                       uiCount * GetFontHeight(PopUpBoxList[uiCounter]->Text[uiCount]->uiFont) +
                       PopUpBoxList[uiCounter]->uiTopMargin +
                       uiCount * PopUpBoxList[uiCounter]->uiLineSpace)),
            ((int16_t)(PopUpBoxList[uiCounter]->Dimensions.iRight -
                       (PopUpBoxList[uiCounter]->uiRightMargin +
                        PopUpBoxList[uiCounter]->uiLeftMargin + 2))),
            ((int16_t)GetFontHeight(PopUpBoxList[uiCounter]->Text[uiCount]->uiFont)), (sString),
            ((int32_t)PopUpBoxList[uiCounter]->Text[uiCount]->uiFont), &uX, &uY);
      } else {
        uX = ((int16_t)(PopUpBoxList[uiCounter]->Position.iX +
                        PopUpBoxList[uiCounter]->uiLeftMargin));
        uY = ((int16_t)(PopUpBoxList[uiCounter]->Position.iY +
                        uiCount * GetFontHeight(PopUpBoxList[uiCounter]->Text[uiCount]->uiFont) +
                        PopUpBoxList[uiCounter]->uiTopMargin +
                        uiCount * PopUpBoxList[uiCounter]->uiLineSpace));
      }

      // print
      // gprintfdirty(uX,uY,PopUpBoxList[uiCounter]->Text[uiCount]->pString );
      mprintf(uX, uY, PopUpBoxList[uiCounter]->Text[uiCount]->pString);
    }

    // there is secondary text in this line?
    if (PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]) {
      // set font
      SetFont(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->uiFont);

      // are we highlighting?...shading?..or neither
      if ((PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->fHighLightFlag == FALSE) &&
          (PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->fShadeFlag == FALSE)) {
        // neither
        SetFontForeground(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->ubForegroundColor);
      } else if ((PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->fHighLightFlag == TRUE)) {
        // highlight
        SetFontForeground(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->ubHighLight);
      } else {
        // shading
        SetFontForeground(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->ubShade);
      }

      // set background
      SetFontBackground(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->ubBackgroundColor);

      // copy string
      wcsncpy(sString, PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->pString,
              wcslen(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->pString) + 1);

      // cnetering?
      if (PopUpBoxList[uiCounter]->uiFlags & POPUP_BOX_FLAG_CENTER_TEXT) {
        FindFontCenterCoordinates(
            ((int16_t)(PopUpBoxList[uiCounter]->Position.iX +
                       PopUpBoxList[uiCounter]->uiLeftMargin)),
            ((int16_t)(PopUpBoxList[uiCounter]->Position.iY +
                       uiCount *
                           GetFontHeight(
                               PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->uiFont) +
                       PopUpBoxList[uiCounter]->uiTopMargin +
                       uiCount * PopUpBoxList[uiCounter]->uiLineSpace)),
            ((int16_t)(PopUpBoxList[uiCounter]->Dimensions.iRight -
                       (PopUpBoxList[uiCounter]->uiRightMargin +
                        PopUpBoxList[uiCounter]->uiLeftMargin + 2))),
            ((int16_t)GetFontHeight(PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->uiFont)),
            (sString), ((int32_t)PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->uiFont),
            &uX, &uY);
      } else {
        uX = ((int16_t)(PopUpBoxList[uiCounter]->Position.iX +
                        PopUpBoxList[uiCounter]->uiLeftMargin +
                        PopUpBoxList[uiCounter]->uiSecondColumnCurrentOffset));
        uY = ((int16_t)(PopUpBoxList[uiCounter]->Position.iY +
                        uiCount *
                            GetFontHeight(
                                PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->uiFont) +
                        PopUpBoxList[uiCounter]->uiTopMargin +
                        uiCount * PopUpBoxList[uiCounter]->uiLineSpace));
      }

      // print
      // gprintfdirty(uX,uY,PopUpBoxList[uiCounter]->Text[uiCount]->pString );
      mprintf(uX, uY, PopUpBoxList[uiCounter]->pSecondColumnString[uiCount]->pString);
    }
  }

  if (PopUpBoxList[uiCounter]->uiBuffer != guiSAVEBUFFER) {
    InvalidateRegion(
        PopUpBoxList[uiCounter]->Position.iX + PopUpBoxList[uiCounter]->uiLeftMargin - 1,
        PopUpBoxList[uiCounter]->Position.iY + PopUpBoxList[uiCounter]->uiTopMargin,
        PopUpBoxList[uiCounter]->Position.iX + PopUpBoxList[uiCounter]->Dimensions.iRight -
            PopUpBoxList[uiCounter]->uiRightMargin,
        PopUpBoxList[uiCounter]->Position.iY + PopUpBoxList[uiCounter]->Dimensions.iBottom -
            PopUpBoxList[uiCounter]->uiBottomMargin);
  }

  SetFontDestBuffer(FRAME_BUFFER, 0, 0, 640, 480, FALSE);

  return TRUE;
}

void ResizeBoxToText(int32_t hBoxHandle) {
  // run through lines of text in box and size box width to longest line plus margins
  // height is sum of getfontheight of each line+ spacing
  int32_t iWidth = 0;
  int32_t iHeight = 0;
  int32_t iCurrString = 0;
  int32_t iSecondColumnLength = 0;

  if ((hBoxHandle < 0) || (hBoxHandle >= MAX_POPUP_BOX_COUNT)) return;

  if (!PopUpBoxList[hBoxHandle]) return;

  ResizeBoxForSecondStrings(hBoxHandle);

  iHeight = PopUpBoxList[hBoxHandle]->uiTopMargin + PopUpBoxList[hBoxHandle]->uiBottomMargin;

  for (iCurrString = 0; iCurrString < MAX_POPUP_BOX_STRING_COUNT; iCurrString++) {
    if (PopUpBoxList[hBoxHandle]->Text[iCurrString] != NULL) {
      if (PopUpBoxList[hBoxHandle]->pSecondColumnString[iCurrString] != NULL) {
        iSecondColumnLength =
            StringPixLength(PopUpBoxList[hBoxHandle]->pSecondColumnString[iCurrString]->pString,
                            PopUpBoxList[hBoxHandle]->pSecondColumnString[iCurrString]->uiFont);
        if (PopUpBoxList[hBoxHandle]->uiSecondColumnCurrentOffset + iSecondColumnLength +
                PopUpBoxList[hBoxHandle]->uiLeftMargin + PopUpBoxList[hBoxHandle]->uiRightMargin >
            ((uint32_t)iWidth)) {
          iWidth = PopUpBoxList[hBoxHandle]->uiSecondColumnCurrentOffset + iSecondColumnLength +
                   PopUpBoxList[hBoxHandle]->uiLeftMargin + PopUpBoxList[hBoxHandle]->uiRightMargin;
        }
      }

      if ((StringPixLength(PopUpBoxList[hBoxHandle]->Text[iCurrString]->pString,
                           PopUpBoxList[hBoxHandle]->Text[iCurrString]->uiFont) +
           PopUpBoxList[hBoxHandle]->uiLeftMargin + PopUpBoxList[hBoxHandle]->uiRightMargin) >
          ((uint32_t)iWidth))
        iWidth = StringPixLength(PopUpBoxList[hBoxHandle]->Text[iCurrString]->pString,
                                 PopUpBoxList[hBoxHandle]->Text[iCurrString]->uiFont) +
                 PopUpBoxList[hBoxHandle]->uiLeftMargin + PopUpBoxList[hBoxHandle]->uiRightMargin;

      // vertical
      iHeight += GetFontHeight(PopUpBoxList[hBoxHandle]->Text[iCurrString]->uiFont) +
                 PopUpBoxList[hBoxHandle]->uiLineSpace;
    } else {
      // doesn't support gaps in text array...
      break;
    }
  }
  PopUpBoxList[hBoxHandle]->Dimensions.iBottom = iHeight;
  PopUpBoxList[hBoxHandle]->Dimensions.iRight = iWidth;
}

BOOLEAN IsBoxShown(uint32_t uiHandle) {
  if ((uiHandle < 0) || (uiHandle >= MAX_POPUP_BOX_COUNT)) return (FALSE);

  if (PopUpBoxList[uiHandle] == NULL) {
    return (FALSE);
  }

  return (PopUpBoxList[uiHandle]->fShowBox);
}

void MarkAllBoxesAsAltered(void) {
  int32_t iCounter = 0;

  // mark all boxes as altered
  for (iCounter = 0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++) {
    ForceUpDateOfBox(iCounter);
  }

  return;
}

void HideAllBoxes(void) {
  int32_t iCounter = 0;

  // hide all the boxes that are shown
  for (iCounter = 0; iCounter < MAX_POPUP_BOX_COUNT; iCounter++) {
    HideBox(iCounter);
  }
}

void RemoveCurrentBoxPrimaryText(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(hStringHandle < MAX_POPUP_BOX_STRING_COUNT);

  // remove & release primary text
  if (PopUpBoxList[guiCurrentBox]->Text[hStringHandle] != NULL) {
    if (PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->pString) {
      MemFree(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]->pString);
    }

    MemFree(PopUpBoxList[guiCurrentBox]->Text[hStringHandle]);
    PopUpBoxList[guiCurrentBox]->Text[hStringHandle] = NULL;
  }
}

void RemoveCurrentBoxSecondaryText(int32_t hStringHandle) {
  if ((guiCurrentBox < 0) || (guiCurrentBox >= MAX_POPUP_BOX_COUNT)) return;

  Assert(PopUpBoxList[guiCurrentBox] != NULL);
  Assert(hStringHandle < MAX_POPUP_BOX_STRING_COUNT);

  // remove & release secondary strings
  if (PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle] != NULL) {
    if (PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->pString) {
      MemFree(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]->pString);
    }

    MemFree(PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle]);
    PopUpBoxList[guiCurrentBox]->pSecondColumnString[hStringHandle] = NULL;
  }
}
