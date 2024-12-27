#ifndef __MERCTEXTBOX_H_
#define __MERCTEXTBOX_H_

#include "SGP/Types.h"

#define MERC_POPUP_PREPARE_FLAGS_TRANS_BACK 0x00000001
#define MERC_POPUP_PREPARE_FLAGS_MARGINS 0x00000002
#define MERC_POPUP_PREPARE_FLAGS_STOPICON 0x00000004
#define MERC_POPUP_PREPARE_FLAGS_SKULLICON 0x00000008

BOOLEAN InitMercPopupBox();

// create a pop up box if needed, return id of box..a -1 means couldn't be added
INT32 PrepareMercPopupBox(INT32 iBoxId, uint8_t ubBackgroundIndex, uint8_t ubBorderIndex,
                          STR16 pString, uint16_t usWidth, uint16_t usMarginX,
                          uint16_t usMarginTopY, uint16_t usMarginBottomY, uint16_t *pActualWidth,
                          uint16_t *pActualHeight);

// remove the current box
BOOLEAN RemoveMercPopupBox();

// remove this box from the index
BOOLEAN RemoveMercPopupBoxFromIndex(uint32_t uiId);

// render the current pop up box
BOOLEAN RenderMercPopupBox(INT16 sDestX, INT16 sDestY, uint32_t uiBuffer);

// render pop up box with this index value
BOOLEAN RenderMercPopUpBoxFromIndex(INT32 iBoxId, INT16 sDestX, INT16 sDestY, uint32_t uiBuffer);

void RemoveTextMercPopupImages();

typedef struct {
  uint32_t uiSourceBufferIndex;
  uint16_t sWidth;
  uint16_t sHeight;
  uint8_t ubBackgroundIndex;
  uint8_t ubBorderIndex;
  uint32_t uiMercTextPopUpBackground;
  uint32_t uiMercTextPopUpBorder;
  BOOLEAN fMercTextPopupInitialized;
  BOOLEAN fMercTextPopupSurfaceInitialized;
  uint32_t uiFlags;

} MercPopUpBox;

BOOLEAN OverrideMercPopupBox(MercPopUpBox *pMercBox);
BOOLEAN ResetOverrideMercPopupBox();
BOOLEAN SetPrepareMercPopupFlags(uint32_t uiFlags);

// background enumeration
enum {
  BASIC_MERC_POPUP_BACKGROUND = 0,
  WHITE_MERC_POPUP_BACKGROUND,
  GREY_MERC_POPUP_BACKGROUND,
  DIALOG_MERC_POPUP_BACKGROUND,
  LAPTOP_POPUP_BACKGROUND,
  IMP_POPUP_BACKGROUND,
};

// border enumeration
enum {
  BASIC_MERC_POPUP_BORDER = 0,
  RED_MERC_POPUP_BORDER,
  BLUE_MERC_POPUP_BORDER,
  DIALOG_MERC_POPUP_BORDER,
  LAPTOP_POP_BORDER
};

#endif
