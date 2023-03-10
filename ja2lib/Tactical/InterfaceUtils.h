#ifndef __INTERFACE_UTILS_H
#define __INTERFACE_UTILS_H

#include "SGP/Types.h"

struct OBJECTTYPE;
struct SOLDIERTYPE;

#define DRAW_ITEM_STATUS_ATTACHMENT1 200
#define DRAW_ITEM_STATUS_ATTACHMENT2 201
#define DRAW_ITEM_STATUS_ATTACHMENT3 202
#define DRAW_ITEM_STATUS_ATTACHMENT4 203

void DrawMoraleUIBarEx(struct SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth,
                       INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer);
void DrawBreathUIBarEx(struct SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth,
                       INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer);
void DrawLifeUIBarEx(struct SOLDIERTYPE *pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth,
                     INT16 sHeight, BOOLEAN fErase, UINT32 uiBuffer);

void DrawItemUIBarEx(struct OBJECTTYPE *pObject, UINT8 ubStatus, INT16 sXPos, INT16 sYPos,
                     INT16 sWidth, INT16 sHeight, INT16 sColor1, INT16 sColor2, BOOLEAN fErase,
                     UINT32 uiBuffer);

void RenderSoldierFace(struct SOLDIERTYPE *pSoldier, INT16 sFaceX, INT16 sFaceY, BOOLEAN fAutoFace);

// load portraits for cars
BOOLEAN LoadCarPortraitValues(void);

// get rid of the loaded portraits for cars
void UnLoadCarPortraits(void);

#endif
