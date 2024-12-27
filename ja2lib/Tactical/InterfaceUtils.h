#ifndef __INTERFACE_UTILS_H
#define __INTERFACE_UTILS_H

#include "SGP/Types.h"

struct OBJECTTYPE;
struct SOLDIERTYPE;

#define DRAW_ITEM_STATUS_ATTACHMENT1 200
#define DRAW_ITEM_STATUS_ATTACHMENT2 201
#define DRAW_ITEM_STATUS_ATTACHMENT3 202
#define DRAW_ITEM_STATUS_ATTACHMENT4 203

void DrawMoraleUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                       int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer);
void DrawBreathUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                       int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer);
void DrawLifeUIBarEx(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                     int16_t sHeight, BOOLEAN fErase, uint32_t uiBuffer);

void DrawItemUIBarEx(struct OBJECTTYPE *pObject, uint8_t ubStatus, int16_t sXPos, int16_t sYPos,
                     int16_t sWidth, int16_t sHeight, int16_t sColor1, int16_t sColor2,
                     BOOLEAN fErase, uint32_t uiBuffer);

void RenderSoldierFace(struct SOLDIERTYPE *pSoldier, int16_t sFaceX, int16_t sFaceY,
                       BOOLEAN fAutoFace);

// load portraits for cars
BOOLEAN LoadCarPortraitValues(void);

// get rid of the loaded portraits for cars
void UnLoadCarPortraits(void);

#endif
