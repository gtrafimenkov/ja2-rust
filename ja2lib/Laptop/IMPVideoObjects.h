// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __IMP_VIDEO_H
#define __IMP_VIDEO_H

#include "SGP/Types.h"

// the functions

// metal background
void RenderProfileBackGround(void);
void RemoveProfileBackGround(void);
BOOLEAN LoadProfileBackGround(void);

// imp symbol
void RenderIMPSymbol(int16_t sX, int16_t sY);
void DeleteIMPSymbol(void);
BOOLEAN LoadIMPSymbol(void);

BOOLEAN LoadBeginIndent(void);
void DeleteBeginIndent(void);
void RenderBeginIndent(int16_t sX, int16_t sY);

BOOLEAN LoadActivationIndent(void);
void DeleteActivationIndent(void);
void RenderActivationIndent(int16_t sX, int16_t sY);

BOOLEAN LoadFrontPageIndent(void);
void DeleteFrontPageIndent(void);
void RenderFrontPageIndent(int16_t sX, int16_t sY);

BOOLEAN LoadAnalyse(void);
void DeleteAnalyse(void);
void RenderAnalyse(int16_t sX, int16_t sY, int8_t bImageNumber);

BOOLEAN LoadAttributeGraph(void);
void DeleteAttributeGraph(void);
void RenderAttributeGraph(int16_t sX, int16_t sY);

BOOLEAN LoadAttributeGraphBar(void);
void DeleteAttributeBarGraph(void);
void RenderAttributeBarGraph(int16_t sX, int16_t sY);

BOOLEAN LoadFullNameIndent(void);
void DeleteFullNameIndent(void);
void RenderFullNameIndent(int16_t sX, int16_t sY);

BOOLEAN LoadNameIndent(void);
void DeleteNameIndent(void);
void RenderNameIndent(int16_t sX, int16_t sY);

BOOLEAN LoadNickNameIndent(void);
void DeleteNickNameIndent(void);
void RenderNickNameIndent(int16_t sX, int16_t sY);

BOOLEAN LoadGenderIndent(void);
void DeleteGenderIndent(void);
void RenderGenderIndent(int16_t sX, int16_t sY);

BOOLEAN LoadSmallFrame(void);
void DeleteSmallFrame(void);
void RenderSmallFrame(int16_t sX, int16_t sY);

BOOLEAN LoadSmallSilhouette(void);
void DeleteSmallSilhouette(void);
void RenderSmallSilhouette(int16_t sX, int16_t sY);

BOOLEAN LoadLargeSilhouette(void);
void DeleteLargeSilhouette(void);
void RenderLargeSilhouette(int16_t sX, int16_t sY);

BOOLEAN LoadAttributeFrame(void);
void DeleteAttributeFrame(void);
void RenderAttributeFrame(int16_t sX, int16_t sY);

BOOLEAN LoadSliderBar(void);
void DeleteSliderBar(void);
void RenderSliderBar(int16_t sX, int16_t sY);

BOOLEAN LoadButton2Image(void);
void DeleteButton2Image(void);
void RenderButton2Image(int16_t sX, int16_t sY);

BOOLEAN LoadButton4Image(void);
void DeleteButton4Image(void);
void RenderButton4Image(int16_t sX, int16_t sY);

BOOLEAN LoadButton1Image(void);
void DeleteButton1Image(void);
void RenderButton1Image(int16_t sX, int16_t sY);

BOOLEAN LoadPortraitFrame(void);
void DeletePortraitFrame(void);
void RenderPortraitFrame(int16_t sX, int16_t sY);

BOOLEAN LoadMainIndentFrame(void);
void DeleteMainIndentFrame(void);
void RenderMainIndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnLongIndentFrame(void);
void DeleteQtnLongIndentFrame(void);
void RenderQtnLongIndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnShortIndentFrame(void);
void DeleteQtnShortIndentFrame(void);
void RenderQtnShortIndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnLongIndentHighFrame(void);
void DeleteQtnLongIndentHighFrame(void);
void RenderQtnLongIndentHighFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnShortIndentHighFrame(void);
void DeleteQtnShortIndentHighFrame(void);
void RenderQtnShortIndentHighFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnShort2IndentFrame(void);
void DeleteQtnShort2IndentFrame(void);
void RenderQtnShort2IndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnShort2IndentHighFrame(void);
void DeleteQtnShort2IndentHighFrame(void);
void RenderQtnShort2IndentHighFrame(int16_t sX, int16_t sY);

BOOLEAN LoadQtnIndentFrame(void);
void DeleteQtnIndentFrame(void);
void RenderQtnIndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadAttrib1IndentFrame(void);
void DeleteAttrib1IndentFrame(void);
void RenderAttrib1IndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadAttrib2IndentFrame(void);
void DeleteAttrib2IndentFrame(void);
void RenderAttrib2IndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadAvgMercIndentFrame(void);
void DeleteAvgMercIndentFrame(void);
void RenderAvgMercIndentFrame(int16_t sX, int16_t sY);

BOOLEAN LoadAboutUsIndentFrame(void);
void DeleteAboutUsIndentFrame(void);
void RenderAboutUsIndentFrame(int16_t sX, int16_t sY);

void RenderAttributeFrameForIndex(int16_t sX, int16_t sY, int32_t iIndex);

// graphical handles

extern uint32_t guiBACKGROUND;
extern uint32_t guiIMPSYMBOL;
extern uint32_t guiBEGININDENT;
extern uint32_t guiACTIVATIONINDENT;
extern uint32_t guiFRONTPAGEINDENT;
extern uint32_t guiFULLNAMEINDENT;
extern uint32_t guiGENDERINDENT;
extern uint32_t guiSMALLFRAME;
extern uint32_t guiANALYSE;
extern uint32_t guiATTRIBUTEGRAPH;
extern uint32_t guiATTRIBUTEGRAPHBAR;
extern uint32_t guiSMALLSILHOUETTE;
extern uint32_t guiLARGESILHOUETTE;
extern uint32_t guiPORTRAITFRAME;
extern uint32_t guiATTRIBUTEFRAME;
extern uint32_t guiATTRIBUTESCREENINDENT1;
extern uint32_t guiATTRIBUTESCREENINDENT2;
extern uint32_t guiSLIDERBAR;

#endif
