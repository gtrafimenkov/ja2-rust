#include "Laptop/IMPVideoObjects.h"

#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// globals

// video object handles
UINT32 guiBACKGROUND;
UINT32 guiIMPSYMBOL;
UINT32 guiBEGININDENT;
UINT32 guiACTIVATIONINDENT;
UINT32 guiFRONTPAGEINDENT;
UINT32 guiFULLNAMEINDENT;
UINT32 guiNAMEINDENT;
UINT32 guiNICKNAMEINDENT;
UINT32 guiGENDERINDENT;
UINT32 guiSMALLFRAME;
UINT32 guiANALYSE;
UINT32 guiATTRIBUTEGRAPH;
UINT32 guiATTRIBUTEGRAPHBAR;
UINT32 guiSMALLSILHOUETTE;
UINT32 guiLARGESILHOUETTE;
UINT32 guiPORTRAITFRAME;
UINT32 guiSLIDERBAR;
UINT32 guiATTRIBUTEFRAME;
UINT32 guiATTRIBUTESCREENINDENT1;
UINT32 guiATTRIBUTESCREENINDENT2;
UINT32 guiATTRIBUTEBAR;
UINT32 guiBUTTON2IMAGE;
UINT32 guiBUTTON1IMAGE;
UINT32 guiBUTTON4IMAGE;
UINT32 guiMAININDENT;
UINT32 guiLONGINDENT;
UINT32 guiSHORTINDENT;
UINT32 guiSHORTHINDENT;
UINT32 guiSHORT2INDENT;
UINT32 guiLONGHINDENT;
UINT32 guiQINDENT;
UINT32 guiA1INDENT;
UINT32 guiA2INDENT;
UINT32 guiAVGMERCINDENT;
UINT32 guiABOUTUSINDENT;
UINT32 guiSHORT2HINDENT;

// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining(void);

BOOLEAN LoadProfileBackGround(void) {
  VOBJECT_DESC VObjectDesc;

  // this procedure will load in the graphics for the generic background

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\MetalBackGround.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiBACKGROUND)) {
    return FALSE;
  }

  return (TRUE);
}

void RemoveProfileBackGround(void) {
  // remove background
  DeleteVideoObjectFromIndex(guiBACKGROUND);

  return;
}

void RenderProfileBackGround(void) {
  struct VObject* hHandle;
  INT32 iCounter = 0;

  // this procedure will render the generic backgound to the screen

  // get the video object
  GetVideoObject(&hHandle, guiBACKGROUND);

  // render each row 5 times wide, 5 tiles high
  for (iCounter = 0; iCounter < 4; iCounter++) {
    // blt background to screen from left to right
    BltVideoObject(FRAME_BUFFER, hHandle, 0,
                   LAPTOP_SCREEN_UL_X + 0 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT,
                   VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,
                   LAPTOP_SCREEN_UL_X + 1 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT,
                   VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,
                   LAPTOP_SCREEN_UL_X + 2 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT,
                   VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(FRAME_BUFFER, hHandle, 0,
                   LAPTOP_SCREEN_UL_X + 3 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT,
                   VO_BLT_SRCTRANSPARENCY, NULL);
  }

  // dirty buttons
  MarkButtonsDirty();

  // force refresh of screen
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, 640, 480);

  return;
}

BOOLEAN LoadIMPSymbol(void) {
  // this procedure will load the IMP main symbol into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  GetMLGFilename(VObjectDesc.ImageFile, MLG_IMPSYMBOL);
  if (!AddVideoObject(&VObjectDesc, &guiIMPSYMBOL)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteIMPSymbol(void) {
  // remove IMP symbol
  DeleteVideoObjectFromIndex(guiIMPSYMBOL);

  return;
}

void RenderIMPSymbol(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiIMPSYMBOL);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadBeginIndent(void) {
  // this procedure will load the indent main symbol into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\BeginScreenIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiBEGININDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteBeginIndent(void) {
  // remove indent symbol

  DeleteVideoObjectFromIndex(guiBEGININDENT);

  return;
}

void RenderBeginIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBEGININDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadActivationIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\ActivationIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiACTIVATIONINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteActivationIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiACTIVATIONINDENT);

  return;
}

void RenderActivationIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiACTIVATIONINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadFrontPageIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\FrontPageIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiFRONTPAGEINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteFrontPageIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiFRONTPAGEINDENT);

  return;
}

void RenderFrontPageIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFRONTPAGEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAnalyse(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\Analyze.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiANALYSE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAnalyse(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiANALYSE);

  return;
}

void RenderAnalyse(INT16 sX, INT16 sY, INT8 bImageNumber) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiANALYSE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, bImageNumber, LAPTOP_SCREEN_UL_X + sX,
                 LAPTOP_SCREEN_WEB_UL_Y + sY, VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAttributeGraph(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\Attributegraph.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiATTRIBUTEGRAPH)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAttributeGraph(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEGRAPH);

  return;
}

void RenderAttributeGraph(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPH);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAttributeGraphBar(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\AttributegraphBar.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiATTRIBUTEGRAPHBAR)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAttributeBarGraph(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEGRAPHBAR);

  return;
}

void RenderAttributeBarGraph(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPHBAR);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadFullNameIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\FullNameIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiFULLNAMEINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteFullNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiFULLNAMEINDENT);

  return;
}

void RenderFullNameIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFULLNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadNickNameIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\NickName.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiNICKNAMEINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteNickNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiNICKNAMEINDENT);

  return;
}

void RenderNickNameIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNICKNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadNameIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\NameIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiNAMEINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiNAMEINDENT);

  return;
}

void RenderNameIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadGenderIndent(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\GenderIndent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiGENDERINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteGenderIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiGENDERINDENT);

  return;
}

void RenderGenderIndent(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiGENDERINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadSmallFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\SmallFrame.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSMALLFRAME)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteSmallFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSMALLFRAME);

  return;
}

void RenderSmallFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadSmallSilhouette(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\SmallSilhouette.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSMALLSILHOUETTE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteSmallSilhouette(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSMALLSILHOUETTE);

  return;
}

void RenderSmallSilhouette(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLSILHOUETTE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadLargeSilhouette(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\largesilhouette.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiLARGESILHOUETTE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteLargeSilhouette(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLARGESILHOUETTE);

  return;
}

void RenderLargeSilhouette(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLARGESILHOUETTE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAttributeFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\attributeframe.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiATTRIBUTEFRAME)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAttributeFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEFRAME);

  return;
}

void RenderAttributeFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;
  INT32 iCounter = 0;
  INT16 sCurrentY = 0;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  sCurrentY += 10;
  for (iCounter = 0; iCounter < 10; iCounter++) {
    // blt to sX, sY relative to upper left corner
    BltVideoObject(FRAME_BUFFER, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(FRAME_BUFFER, hHandle, 1, LAPTOP_SCREEN_UL_X + sX,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, VO_BLT_SRCTRANSPARENCY, NULL);
    BltVideoObject(FRAME_BUFFER, hHandle, 3, LAPTOP_SCREEN_UL_X + sX + 368,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, VO_BLT_SRCTRANSPARENCY, NULL);

    sCurrentY += 20;
  }

  BltVideoObject(FRAME_BUFFER, hHandle, 4, LAPTOP_SCREEN_UL_X + sX,
                 LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

void RenderAttributeFrameForIndex(INT16 sX, INT16 sY, INT32 iIndex) {
  INT16 sCurrentY = 0;
  struct VObject* hHandle;

  // valid index?
  if (iIndex == -1) {
    return;
  }

  sCurrentY = (INT16)(10 + (iIndex * 20));

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134,
                 LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY, VO_BLT_SRCTRANSPARENCY, NULL);

  RenderAttrib2IndentFrame(350, 42);

  // amt of bonus pts
  DrawBonusPointsRemaining();

  // render attribute boxes
  RenderAttributeBoxes();

  InvalidateRegion(LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY,
                   LAPTOP_SCREEN_UL_X + sX + 400, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY + 21);

  return;
}

BOOLEAN LoadSliderBar(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\attributeslider.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSLIDERBAR)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteSliderBar(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSLIDERBAR);

  return;
}

void RenderSliderBar(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSLIDERBAR);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadButton2Image(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\button_2.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiBUTTON2IMAGE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteButton2Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON2IMAGE);

  return;
}

void RenderButton2Image(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON2IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadButton4Image(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\button_4.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiBUTTON4IMAGE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteButton4Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON4IMAGE);

  return;
}

void RenderButton4Image(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON4IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadButton1Image(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\button_1.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiBUTTON1IMAGE)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteButton1Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON1IMAGE);

  return;
}

void RenderButton1Image(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON1IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadPortraitFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\Voice_PortraitFrame.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiPORTRAITFRAME)) {
    return FALSE;
  }

  return (TRUE);
}

void DeletePortraitFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiPORTRAITFRAME);

  return;
}

void RenderPortraitFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiPORTRAITFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadMainIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\mainprofilepageindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiMAININDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteMainIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiMAININDENT);

  return;
}

void RenderMainIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiMAININDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnLongIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\longindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiLONGINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnLongIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLONGINDENT);

  return;
}

void RenderQtnLongIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnShortIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\shortindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSHORTINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnShortIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORTINDENT);

  return;
}

void RenderQtnShortIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnLongIndentHighFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\longindenthigh.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiLONGHINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnLongIndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLONGHINDENT);

  return;
}

void RenderQtnLongIndentHighFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGHINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnShortIndentHighFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\shortindenthigh.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSHORTHINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnShortIndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORTHINDENT);

  return;
}

void RenderQtnShortIndentHighFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTHINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\questionindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiQINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiQINDENT);

  return;
}

void RenderQtnIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiQINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAttrib1IndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\attributescreenindent_1.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiA1INDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAttrib1IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiA1INDENT);

  return;
}

void RenderAttrib1IndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA1INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAttrib2IndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\attributescreenindent_2.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiA2INDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAttrib2IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiA2INDENT);

  return;
}

void RenderAttrib2IndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA2INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAvgMercIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\anaveragemercindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiAVGMERCINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAvgMercIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiAVGMERCINDENT);

  return;
}

void RenderAvgMercIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiAVGMERCINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadAboutUsIndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\aboutusindent.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiABOUTUSINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteAboutUsIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiABOUTUSINDENT);

  return;
}

void RenderAboutUsIndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiABOUTUSINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnShort2IndentFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\shortindent2.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSHORT2INDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnShort2IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORT2INDENT);

  return;
}

void RenderQtnShort2IndentFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}

BOOLEAN LoadQtnShort2IndentHighFrame(void) {
  // this procedure will load the activation indent into memory
  VOBJECT_DESC VObjectDesc;

  VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  CopyFilename("LAPTOP\\shortindent2High.sti", VObjectDesc.ImageFile);
  if (!AddVideoObject(&VObjectDesc, &guiSHORT2HINDENT)) {
    return FALSE;
  }

  return (TRUE);
}

void DeleteQtnShort2IndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORT2HINDENT);

  return;
}

void RenderQtnShort2IndentHighFrame(INT16 sX, INT16 sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2HINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(FRAME_BUFFER, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY,
                 VO_BLT_SRCTRANSPARENCY, NULL);

  return;
}
