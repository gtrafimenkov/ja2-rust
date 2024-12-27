// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __RENDERWORLD_H
#define __RENDERWORLD_H

#include "SGP/Types.h"

struct VObject;

// TEMP SELECT STUFF
#define NO_SELECT 0
#define FULL_SELECT 1
#define SELECT_WIDTH 2
#define SELECT_HEIGHT 3

extern BOOLEAN gfDoVideoScroll;
extern BOOLEAN gfDoSubtileScroll;
extern uint8_t gubCurScrollSpeedID;

// RENDERING FLAGS
#define RENDER_FLAG_FULL 0x00000001
#define RENDER_FLAG_SHADOWS 0x00000002
#define RENDER_FLAG_MARKED 0x00000004
#define RENDER_FLAG_SAVEOFF 0x00000008
#define RENDER_FLAG_NOZ 0x00000010
#define RENDER_FLAG_ROOMIDS 0x00000020
#define RENDER_FLAG_CHECKZ 0x00000040
#define RENDER_FLAG_ONLYLAND 0x00000080
#define RENDER_FLAG_ONLYSTRUCT 0x00000100
#define RENDER_FLAG_FOVDEBUG 0x00000200

#define SCROLL_UP 0x00000001
#define SCROLL_DOWN 0x00000002
#define SCROLL_RIGHT 0x00000004
#define SCROLL_LEFT 0x00000008
#define SCROLL_UPLEFT 0x00000020
#define SCROLL_UPRIGHT 0x00000040
#define SCROLL_DOWNLEFT 0x00000080
#define SCROLL_DOWNRIGHT 0x00000200

#define Z_SUBLAYERS 8
#define LAND_Z_LEVEL 0
#define OBJECT_Z_LEVEL 1
#define SHADOW_Z_LEVEL 2
#define MERC_Z_LEVEL 3
#define STRUCT_Z_LEVEL 4
#define ROOF_Z_LEVEL 5
#define ONROOF_Z_LEVEL 6
#define FOG_Z_LEVEL 7
#define TOPMOST_Z_LEVEL 32767

// highest bit value is rendered first!
#define TILES_STATIC_LAND 0x00040000
#define TILES_STATIC_OBJECTS 0x00020000
#define TILES_STATIC_SHADOWS 0x00008000
#define TILES_STATIC_STRUCTURES 0x00004000
#define TILES_STATIC_ROOF 0x00002000
#define TILES_STATIC_ONROOF 0x00001000
#define TILES_STATIC_TOPMOST 0x00000800

// highest bit value is rendered first!
#define TILES_ALL_DYNAMICS 0x00000fff
#define TILES_DYNAMIC_CHECKFOR_INT_TILE 0x00000400
#define TILES_DYNAMIC_LAND 0x00000200
#define TILES_DYNAMIC_OBJECTS 0x00000100
#define TILES_DYNAMIC_SHADOWS 0x00000080
#define TILES_DYNAMIC_STRUCT_MERCS 0x00000040
#define TILES_DYNAMIC_MERCS 0x00000020
#define TILES_DYNAMIC_STRUCTURES 0x00000010
#define TILES_DYNAMIC_ROOF 0x00000008
#define TILES_DYNAMIC_HIGHMERCS 0x00000004
#define TILES_DYNAMIC_ONROOF 0x00000002
#define TILES_DYNAMIC_TOPMOST 0x00000001

extern BOOLEAN gfRenderScroll;
extern int16_t gsScrollXIncrement;
extern int16_t gsScrollYIncrement;
extern int32_t guiScrollDirection;
extern BOOLEAN gfScrollStart;
extern int16_t gsRenderHeight;

// Distance around mercs to pixelate walls
#define REVEAL_WALLS_RADIUS 3

// GLOBAL VARIABLES
extern struct GRect gOldClipRect;
extern int16_t SCROLL_X_STEP;
extern int16_t SCROLL_Y_STEP;

extern int16_t gsVIEWPORT_START_X;
extern int16_t gsVIEWPORT_START_Y;
extern int16_t gsVIEWPORT_WINDOW_START_Y;
extern int16_t gsVIEWPORT_END_Y;
extern int16_t gsVIEWPORT_WINDOW_END_Y;
extern int16_t gsVIEWPORT_END_X;

extern int16_t gsRenderCenterX;
extern int16_t gsRenderCenterY;
extern int16_t gsRenderWorldOffsetX;
extern int16_t gsRenderWorldOffsetY;

// CURRENT VIEWPORT IN WORLD COORDS
extern int16_t gsTopLeftWorldX, gsTopLeftWorldY;
extern int16_t gsTopRightWorldX, gsTopRightWorldY;
extern int16_t gsBottomLeftWorldX, gsBottomLeftWorldY;
extern int16_t gsBottomRightWorldX, gsBottomRightWorldY;

extern struct GRect gSelectRegion;
extern SGPPoint gSelectAnchor;
extern uint32_t fSelectMode;

// GLOBAL COORDINATES
extern int16_t gTopLeftWorldLimitX, gTopLeftWorldLimitY;
extern int16_t gTopRightWorldLimitX, gTopRightWorldLimitY;
extern int16_t gBottomLeftWorldLimitX, gBottomLeftWorldLimitY;
extern int16_t gBottomRightWorldLimitX, gBottomRightWorldLimitY;
extern int16_t gCenterWorldX, gCenterWorldY;
extern int16_t gsTLX, gsTLY, gsTRX, gsTRY;
extern int16_t gsBLX, gsBLY, gsBRX, gsBRY;
extern int16_t gsCX, gsCY;
extern double gdScaleX, gdScaleY;

extern BOOLEAN fLandLayerDirty;

extern BOOLEAN gfIgnoreScrollDueToCenterAdjust;

// FUNCTIONS
void ScrollWorld();
void InitRenderParams(uint8_t ubRestrictionID);
void RenderWorld();

void ResetLayerOptimizing(void);
void ResetSpecificLayerOptimizing(uint32_t uiRowFlag);

// Routines of RenderWorld
extern void RenderStaticWorld();
extern void RenderDynamicWorld();
void CopyRenderBuffer();

void SetRenderFlags(uint32_t uiFlags);
uint32_t GetRenderFlags(void);
void ClearRenderFlags(uint32_t uiFlags);

void RenderSetShadows(BOOLEAN fShadows);

extern uint16_t *gpZBuffer;
extern uint32_t gRenderFlags;
extern BOOLEAN gfIgnoreScrolling;

extern BOOLEAN gfScrollInertia;
extern BOOLEAN gfScrollPending;

// Definitions for dirty rectangle uploads
void ReRenderWorld(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom);

BOOLEAN ConcealWalls(int16_t sX, int16_t sY, int16_t sRadius);
BOOLEAN RevealWalls(int16_t sX, int16_t sY, int16_t sRadius);
void ConcealAllWalls(void);

BOOLEAN ApplyScrolling(int16_t sTempRenderCenterX, int16_t sTempRenderCenterY, BOOLEAN fForceAdjust,
                       BOOLEAN fCheckOnly);

BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              uint16_t *pZBuffer, uint16_t usZValue,
                                              struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                              uint16_t usIndex, struct GRect *clipregion);

void RenderStaticWorldRect(int16_t, int16_t, int16_t, int16_t, BOOLEAN);
void RenderMarkedWorld(void);
void RenderDynamicMercWorld(void);

void ExamineZBufferRect(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom);

void InvalidateWorldRedundency(void);
void InvalidateWorldRedundencyRadius(int16_t sX, int16_t sY, int16_t sRadius);
void DirtyWorldRender();

// These two functions will setup the glow frame script to use then glowing enemy mercs....
void SetMercGlowFast();
void SetMercGlowNormal();

void SetRenderCenter(int16_t sNewX, int16_t sNewY);

#ifdef _DEBUG
void RenderFOVDebug();
#endif

#endif
