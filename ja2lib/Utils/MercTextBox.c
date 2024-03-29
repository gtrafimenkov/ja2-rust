#include "Utils/MercTextBox.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/VSurfaceInternal.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "rust_colors.h"

#define TEXT_POPUP_WINDOW_TEXT_OFFSET_X 8
#define TEXT_POPUP_WINDOW_TEXT_OFFSET_Y 8
#define TEXT_POPUP_STRING_WIDTH 296
#define TEXT_POPUP_GAP_BN_LINES 10
#define TEXT_POPUP_FONT FONT12ARIAL
#define TEXT_POPUP_COLOR FONT_MCOLOR_WHITE

#define MERC_TEXT_FONT FONT12ARIAL
#define MERC_TEXT_COLOR FONT_MCOLOR_WHITE

#define MERC_TEXT_MIN_WIDTH 10
#define MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X 10
#define MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y 10

#define MERC_BACKGROUND_WIDTH 350
#define MERC_BACKGROUND_HEIGHT 200

// the max number of pop up boxes availiable to user
#define MAX_NUMBER_OF_POPUP_BOXES 10

// attempt to add box to pop up box list
INT32 AddPopUpBoxToList(MercPopUpBox *pPopUpTextBox);

// grab box with this id value
MercPopUpBox *GetPopUpBoxIndex(INT32 iId);

// both of the below are index by the enum for thier types - background and border in
// MercTextBox.h

// filenames for border popup .sti's
STR8 zMercBorderPopupFilenames[] = {
    "INTERFACE\\TactPopUp.sti",     "INTERFACE\\TactRedPopUp.sti", "INTERFACE\\TactBluePopUp.sti",
    "INTERFACE\\TactPopUpMain.sti", "INTERFACE\\LaptopPopup.sti",

};

// filenames for background popup .pcx's
STR8 zMercBackgroundPopupFilenames[] = {
    "INTERFACE\\TactPopupBackground.pcx",     "INTERFACE\\TactPopupWhiteBackground.pcx",
    "INTERFACE\\TactPopupGreyBackground.pcx", "INTERFACE\\TactPopupBackgroundMain.pcx",
    "INTERFACE\\LaptopPopupBackground.pcx",   "INTERFACE\\imp_popup_background.pcx",
};

// the pop up box structure
MercPopUpBox gBasicPopUpTextBox;

// the current pop up box
MercPopUpBox *gPopUpTextBox = NULL;

// the old one
MercPopUpBox *gOldPopUpTextBox = NULL;

// the list of boxes
MercPopUpBox *gpPopUpBoxList[MAX_NUMBER_OF_POPUP_BOXES];

// the flags
UINT32 guiFlags = 0;
UINT32 guiBoxIcons;
UINT32 guiSkullIcons;

BOOLEAN SetCurrentPopUpBox(UINT32 uiId) {
  // given id of the box, find it in the list and set to current

  // make sure the box id is valid
  if (uiId == (UINT32)-1) {
    // ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Error: Trying to set Current Popup Box using
    // -1 as an ID" );
    return (FALSE);
  }

  // see if box inited
  if (gpPopUpBoxList[uiId] != NULL) {
    gPopUpTextBox = gpPopUpBoxList[uiId];
    return (TRUE);
  }
  return (FALSE);
}

BOOLEAN OverrideMercPopupBox(MercPopUpBox *pMercBox) {
  // store old box and set current this passed one
  gOldPopUpTextBox = gPopUpTextBox;

  gPopUpTextBox = pMercBox;

  return (TRUE);
}

BOOLEAN ResetOverrideMercPopupBox() {
  gPopUpTextBox = gOldPopUpTextBox;

  return (TRUE);
}

BOOLEAN InitMercPopupBox() {
  INT32 iCounter = 0;

  // init the pop up box list
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    // set ptr to null
    gpPopUpBoxList[iCounter] = NULL;
  }

  // LOAD STOP ICON...
  if (!AddVObjectFromFile("INTERFACE\\msgboxicons.sti", &guiBoxIcons))
    AssertMsg(0, "Missing INTERFACE\\msgboxicons.sti");

  // LOAD SKULL ICON...
  if (!AddVObjectFromFile("INTERFACE\\msgboxiconskull.sti", &guiSkullIcons))
    AssertMsg(0, "Missing INTERFACE\\msgboxiconskull.sti");

  return (TRUE);
}

BOOLEAN ShutDownPopUpBoxes() {
  INT32 iCounter = 0;
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    // now attempt to remove this box
    RemoveMercPopupBoxFromIndex(iCounter);
  }

  return (TRUE);
}

// Pass in the background index, and pointers to the font and shadow color
void GetMercPopupBoxFontColor(UINT8 ubBackgroundIndex, UINT8 *pubFontColor,
                              UINT8 *pubFontShadowColor);

// Tactical Popup
BOOLEAN LoadTextMercPopupImages(UINT8 ubBackgroundIndex, UINT8 ubBorderIndex) {
  // this function will load the graphics associated with the background and border index values

  // the background
  gPopUpTextBox->backgroundImage =
      CreateImage(zMercBackgroundPopupFilenames[ubBackgroundIndex], false);
  if (!gPopUpTextBox->backgroundImage) {
    return FALSE;
  }

  // border
  if (!AddVObjectFromFile(zMercBorderPopupFilenames[ubBorderIndex],
                          &gPopUpTextBox->uiMercTextPopUpBorder)) {
    return FALSE;
  }

  gPopUpTextBox->fMercTextPopupInitialized = TRUE;

  // so far so good, return successful
  gPopUpTextBox->ubBackgroundIndex = ubBackgroundIndex;
  gPopUpTextBox->ubBorderIndex = ubBorderIndex;

  return (TRUE);
}

void RemoveTextMercPopupImages() {
  // this procedure will remove the background and border video surface/object from the indecies
  if (gPopUpTextBox) {
    if (gPopUpTextBox->fMercTextPopupInitialized) {
      DestroyImage(gPopUpTextBox->backgroundImage);
      DeleteVideoObjectFromIndex(gPopUpTextBox->uiMercTextPopUpBorder);
      gPopUpTextBox->fMercTextPopupInitialized = FALSE;
    }
  }

  // done
  return;
}

BOOLEAN RenderMercPopUpBoxFromIndex(INT32 iBoxId, INT16 sDestX, INT16 sDestY, UINT32 uiBuffer) {
  // set the current box
  if (SetCurrentPopUpBox(iBoxId) == FALSE) {
    return (FALSE);
  }

  // now attempt to render the box
  return (RenderMercPopupBox(sDestX, sDestY, uiBuffer));
}

BOOLEAN RenderMercPopupBox(INT16 sDestX, INT16 sDestY, UINT32 uiBuffer) {
  // will render/transfer the image from the buffer in the data structure to the buffer specified by
  // user
  BOOLEAN fReturnValue = TRUE;

  // check to see if we are wanting to blit a transparent background
  if (gPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_TRANS_BACK)
    BltVideoSurface(GetVSByID(uiBuffer), GetVSByID(gPopUpTextBox->uiSourceBufferIndex), sDestX,
                    sDestY, VS_BLT_FAST | VS_BLT_USECOLORKEY, NULL);
  else
    BltVideoSurface(GetVSByID(uiBuffer), GetVSByID(gPopUpTextBox->uiSourceBufferIndex), sDestX,
                    sDestY, VS_BLT_FAST, NULL);

  // Invalidate!
  if (uiBuffer == FRAME_BUFFER) {
    InvalidateRegion(sDestX, sDestY, (INT16)(sDestX + gPopUpTextBox->sWidth),
                     (INT16)(sDestY + gPopUpTextBox->sHeight));
  }

  return fReturnValue;
}

INT32 AddPopUpBoxToList(MercPopUpBox *pPopUpTextBox) {
  INT32 iCounter = 0;

  // make sure is a valid box
  if (pPopUpTextBox == NULL) {
    return (-1);
  }

  // attempt to add box to list
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    if (gpPopUpBoxList[iCounter] == NULL) {
      // found a spot, inset
      gpPopUpBoxList[iCounter] = pPopUpTextBox;

      // set as current
      SetCurrentPopUpBox(iCounter);

      // return index value
      return (iCounter);
    }
  }

  // return failure
  return (-1);
}

// get box with this id
MercPopUpBox *GetPopUpBoxIndex(INT32 iId) { return (gpPopUpBoxList[iId]); }

INT32 PrepareMercPopupBox(INT32 iBoxId, UINT8 ubBackgroundIndex, UINT8 ubBorderIndex, STR16 pString,
                          UINT16 usWidth, UINT16 usMarginX, UINT16 usMarginTopY,
                          UINT16 usMarginBottomY, UINT16 *pActualWidth, UINT16 *pActualHeight) {
  UINT16 usNumberVerticalPixels;
  UINT16 usTextWidth, usHeight;
  UINT16 i;
  struct VObject *hImageHandle;
  UINT16 usPosY, usPosX;
  UINT16 usStringPixLength;
  struct GRect DestRect;
  UINT32 uiDestPitchBYTES;
  UINT16 *pDestBuf;
  UINT8 ubFontColor, ubFontShadowColor;
  UINT16 usColorVal;
  UINT16 usLoopEnd;
  INT16 sDispTextXPos;
  MercPopUpBox *pPopUpTextBox = NULL;

  if (usWidth >= 640) return (-1);

  if (usWidth <= MERC_TEXT_MIN_WIDTH) usWidth = MERC_TEXT_MIN_WIDTH;

  // check id value, if -1, box has not been inited yet
  if (iBoxId == -1) {
    // no box yet

    // create box
    pPopUpTextBox = (MercPopUpBox *)MemAlloc(sizeof(MercPopUpBox));

    // copy over ptr
    gPopUpTextBox = pPopUpTextBox;

    // Load appropriate images
    if (LoadTextMercPopupImages(ubBackgroundIndex, ubBorderIndex) == FALSE) {
      MemFree(pPopUpTextBox);
      return (-1);
    }

  } else {
    // has been created already,
    // Check if these images are different

    // grab box
    pPopUpTextBox = GetPopUpBoxIndex(iBoxId);

    // box has valid id and no instance?..error
    Assert(pPopUpTextBox);

    // copy over ptr
    gPopUpTextBox = pPopUpTextBox;

    if (ubBackgroundIndex != pPopUpTextBox->ubBackgroundIndex ||
        ubBorderIndex != pPopUpTextBox->ubBorderIndex ||
        !pPopUpTextBox->fMercTextPopupInitialized) {
      // Remove old, set new
      RemoveTextMercPopupImages();
      if (LoadTextMercPopupImages(ubBackgroundIndex, ubBorderIndex) == FALSE) {
        return (-1);
      }
    }
  }

  gPopUpTextBox->uiFlags = guiFlags;
  // reset flags
  guiFlags = 0;

  usStringPixLength = WFStringPixLength(pString, TEXT_POPUP_FONT);

  if (usStringPixLength < (usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X)*2)) {
    usWidth = usStringPixLength + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2;
    usTextWidth = usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X)*2 + 1;
  } else {
    usTextWidth = usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X)*2 + 1 - usMarginX;
  }

  usNumberVerticalPixels =
      IanWrappedStringHeight(0, 0, usTextWidth, 2, TEXT_POPUP_FONT, MERC_TEXT_COLOR, pString,
                             FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  usHeight = usNumberVerticalPixels + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2;

  // Add height for margins
  usHeight += usMarginTopY + usMarginBottomY;

  // Add width for margins
  usWidth += (usMarginX * 2);

  // Add width for iconic...
  if ((pPopUpTextBox->uiFlags &
       (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON))) {
    // Make minimun height for box...
    if (usHeight < 45) {
      usHeight = 45;
    }
    usWidth += 35;
  }

  if (usWidth >= MERC_BACKGROUND_WIDTH) usWidth = MERC_BACKGROUND_WIDTH - 1;
  // make sure the area isnt bigger then the background texture
  if ((usWidth >= MERC_BACKGROUND_WIDTH) || usHeight >= MERC_BACKGROUND_HEIGHT) {
    if (iBoxId == -1) {
      MemFree(pPopUpTextBox);
    }

    return (-1);
  }
  // Create a background video surface to blt the face onto
  VSURFACE_DESC vs_desc;
  vs_desc.usWidth = usWidth;
  vs_desc.usHeight = usHeight;
  if (!(AddVideoSurface(&vs_desc, &pPopUpTextBox->uiSourceBufferIndex))) {
    return FALSE;
  }
  pPopUpTextBox->fMercTextPopupSurfaceInitialized = TRUE;

  pPopUpTextBox->sWidth = usWidth;
  pPopUpTextBox->sHeight = usHeight;

  *pActualWidth = usWidth;
  *pActualHeight = usHeight;

  DestRect.iLeft = 0;
  DestRect.iTop = 0;
  DestRect.iRight = DestRect.iLeft + usWidth;
  DestRect.iBottom = DestRect.iTop + usHeight;

  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_TRANS_BACK) {
    // Zero with yellow,
    // Set source transparcenty
    SetVideoSurfaceTransparency(pPopUpTextBox->uiSourceBufferIndex, FROMRGB(255, 255, 0));

    pDestBuf =
        (UINT16 *)VSurfaceLockOld(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), &uiDestPitchBYTES);

    usColorVal = rgb32_to_rgb565(FROMRGB(255, 255, 0));
    usLoopEnd = (usWidth * usHeight);

    for (i = 0; i < usLoopEnd; i++) {
      pDestBuf[i] = usColorVal;
    }

    VSurfaceUnlock(GetVSByID(pPopUpTextBox->uiSourceBufferIndex));

  } else {
    BlitImageToSurfaceRect(pPopUpTextBox->backgroundImage,
                           GetVSByID(pPopUpTextBox->uiSourceBufferIndex), 0, 0, DestRect);
  }

  GetVideoObject(&hImageHandle, pPopUpTextBox->uiMercTextPopUpBorder);

  usPosX = usPosY = 0;
  // blit top row of images
  for (i = TEXT_POPUP_GAP_BN_LINES; i < usWidth - TEXT_POPUP_GAP_BN_LINES;
       i += TEXT_POPUP_GAP_BN_LINES) {
    // TOP ROW
    BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 1, i, usPosY);
    // BOTTOM ROW
    BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 6, i,
               usHeight - TEXT_POPUP_GAP_BN_LINES + 6);
  }

  // blit the left and right row of images
  usPosX = 0;
  for (i = TEXT_POPUP_GAP_BN_LINES; i < usHeight - TEXT_POPUP_GAP_BN_LINES;
       i += TEXT_POPUP_GAP_BN_LINES) {
    BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 3, usPosX, i);
    BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 4, usPosX + usWidth - 4,
               i);
  }

  // blt the corner images for the row
  // top left
  BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 0, 0, usPosY);
  // top right
  BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 2,
             usWidth - TEXT_POPUP_GAP_BN_LINES, usPosY);
  // bottom left
  BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 5, 0,
             usHeight - TEXT_POPUP_GAP_BN_LINES);
  // bottom right
  BltVObject(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), hImageHandle, 7,
             usWidth - TEXT_POPUP_GAP_BN_LINES, usHeight - TEXT_POPUP_GAP_BN_LINES);

  // Icon if ness....
  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_STOPICON) {
    BltVObjectFromIndex(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), guiBoxIcons, 0, 5, 4);
  }
  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_SKULLICON) {
    BltVObjectFromIndex(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), guiSkullIcons, 0, 9, 4);
  }

  // Get the font and shadow colors
  GetMercPopupBoxFontColor(ubBackgroundIndex, &ubFontColor, &ubFontShadowColor);

  SetFontShadow(ubFontShadowColor);
  SetFontDest(GetVSByID(pPopUpTextBox->uiSourceBufferIndex), 0, 0, usWidth, usHeight, FALSE);

  // Display the text
  sDispTextXPos = (INT16)((MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X + usMarginX));

  if (pPopUpTextBox->uiFlags &
      (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON)) {
    sDispTextXPos += 30;
  }

  DisplayWrappedString(
      sDispTextXPos, (INT16)((MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y + usMarginTopY)), usTextWidth, 2,
      MERC_TEXT_FONT, ubFontColor, pString, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  SetFontDest(vsFB, 0, 0, 640, 480, FALSE);
  SetFontShadow(DEFAULT_SHADOW);

  if (iBoxId == -1) {
    // now return attemp to add to pop up box list, if successful will
    // return index
    return (AddPopUpBoxToList(pPopUpTextBox));
  } else {
    // set as current box
    SetCurrentPopUpBox(iBoxId);

    return (iBoxId);
  }
}

// Deletes the surface thats contains the border, background and the text.
BOOLEAN RemoveMercPopupBox() {
  INT32 iCounter = 0;

  // make sure the current box does in fact exist
  if (gPopUpTextBox == NULL) {
    // failed..
    return (FALSE);
  }

  // now check to see if inited...
  if (gPopUpTextBox->fMercTextPopupSurfaceInitialized) {
    // now find this box in the list
    for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
      if (gpPopUpBoxList[iCounter] == gPopUpTextBox) {
        gpPopUpBoxList[iCounter] = NULL;
        iCounter = MAX_NUMBER_OF_POPUP_BOXES;
      }
    }
    // yep, get rid of the bloody...
    DeleteVideoSurfaceFromIndex(gPopUpTextBox->uiSourceBufferIndex);

    // DEF Added 5/26
    // Delete the background and the border
    RemoveTextMercPopupImages();

    MemFree(gPopUpTextBox);

    // reset current ptr
    gPopUpTextBox = NULL;
  }

  return (TRUE);
}

BOOLEAN RemoveMercPopupBoxFromIndex(UINT32 uiId) {
  // find this box, set it to current, and delete it
  if (SetCurrentPopUpBox(uiId) == FALSE) {
    // failed
    return (FALSE);
  }

  // now try to remove it
  return (RemoveMercPopupBox());
}

// Pass in the background index, and pointers to the font and shadow color
void GetMercPopupBoxFontColor(UINT8 ubBackgroundIndex, UINT8 *pubFontColor,
                              UINT8 *pubFontShadowColor) {
  switch (ubBackgroundIndex) {
    case BASIC_MERC_POPUP_BACKGROUND:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;

    case WHITE_MERC_POPUP_BACKGROUND:
      *pubFontColor = 2;
      *pubFontShadowColor = FONT_MCOLOR_WHITE;
      break;

    case GREY_MERC_POPUP_BACKGROUND:
      *pubFontColor = 2;
      *pubFontShadowColor = NO_SHADOW;
      break;

    case LAPTOP_POPUP_BACKGROUND:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;

    default:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;
  }
}

BOOLEAN SetPrepareMercPopupFlags(UINT32 uiFlags) {
  guiFlags |= uiFlags;
  return (TRUE);
}

BOOLEAN SetPrepareMercPopUpFlagsFromIndex(UINT32 uiFlags, UINT32 uiId) {
  // find this box, set it to current, and delete it
  if (SetCurrentPopUpBox(uiId) == FALSE) {
    // failed
    return (FALSE);
  }

  // now try to remove it
  return (SetPrepareMercPopupFlags(uiFlags));
}
