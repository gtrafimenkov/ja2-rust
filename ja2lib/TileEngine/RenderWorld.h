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
extern UINT8 gubCurScrollSpeedID;

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
extern INT16 gsScrollXIncrement;
extern INT16 gsScrollYIncrement;
extern INT32 guiScrollDirection;
extern BOOLEAN gfScrollStart;
extern INT16 gsRenderHeight;

// Distance around mercs to pixelate walls
#define REVEAL_WALLS_RADIUS 3

// GLOBAL VARIABLES
extern struct GRect gOldClipRect;
extern INT16 SCROLL_X_STEP;
extern INT16 SCROLL_Y_STEP;

extern INT16 gsVIEWPORT_START_X;
extern INT16 gsVIEWPORT_START_Y;
extern INT16 gsVIEWPORT_WINDOW_START_Y;
extern INT16 gsVIEWPORT_END_Y;
extern INT16 gsVIEWPORT_WINDOW_END_Y;
extern INT16 gsVIEWPORT_END_X;

extern INT16 gsRenderCenterX;
extern INT16 gsRenderCenterY;
extern INT16 gsRenderWorldOffsetX;
extern INT16 gsRenderWorldOffsetY;

// CURRENT VIEWPORT IN WORLD COORDS
extern INT16 gsTopLeftWorldX, gsTopLeftWorldY;
extern INT16 gsTopRightWorldX, gsTopRightWorldY;
extern INT16 gsBottomLeftWorldX, gsBottomLeftWorldY;
extern INT16 gsBottomRightWorldX, gsBottomRightWorldY;

extern struct GRect gSelectRegion;
extern SGPPoint gSelectAnchor;
extern UINT32 fSelectMode;

// GLOBAL COORDINATES
extern INT16 gTopLeftWorldLimitX, gTopLeftWorldLimitY;
extern INT16 gTopRightWorldLimitX, gTopRightWorldLimitY;
extern INT16 gBottomLeftWorldLimitX, gBottomLeftWorldLimitY;
extern INT16 gBottomRightWorldLimitX, gBottomRightWorldLimitY;
extern INT16 gCenterWorldX, gCenterWorldY;
extern INT16 gsTLX, gsTLY, gsTRX, gsTRY;
extern INT16 gsBLX, gsBLY, gsBRX, gsBRY;
extern INT16 gsCX, gsCY;
extern DOUBLE gdScaleX, gdScaleY;

extern BOOLEAN fLandLayerDirty;

extern BOOLEAN gfIgnoreScrollDueToCenterAdjust;

// FUNCTIONS
void ScrollWorld();
void InitRenderParams(UINT8 ubRestrictionID);
void RenderWorld();

void ResetLayerOptimizing(void);
void ResetSpecificLayerOptimizing(UINT32 uiRowFlag);

// Routines of RenderWorld
extern void RenderStaticWorld();
extern void RenderDynamicWorld();
void CopyRenderBuffer();

void SetRenderFlags(UINT32 uiFlags);
UINT32 GetRenderFlags(void);
void ClearRenderFlags(UINT32 uiFlags);

void RenderSetShadows(BOOLEAN fShadows);

extern UINT16 *gpZBuffer;
extern UINT32 gRenderFlags;
extern BOOLEAN gfIgnoreScrolling;

extern BOOLEAN gfScrollInertia;
extern BOOLEAN gfScrollPending;

// Definitions for dirty rectangle uploads
void ReRenderWorld(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);

BOOLEAN ConcealWalls(INT16 sX, INT16 sY, INT16 sRadius);
BOOLEAN RevealWalls(INT16 sX, INT16 sY, INT16 sRadius);
void ConcealAllWalls(void);

BOOLEAN ApplyScrolling(INT16 sTempRenderCenterX, INT16 sTempRenderCenterY, BOOLEAN fForceAdjust,
                       BOOLEAN fCheckOnly);

BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              UINT16 *pZBuffer, UINT16 usZValue,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex, struct GRect *clipregion);

void RenderStaticWorldRect(INT16, INT16, INT16, INT16, BOOLEAN);
void RenderMarkedWorld(void);
void RenderDynamicMercWorld(void);

void ExamineZBufferRect(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);

void InvalidateWorldRedundency(void);
void InvalidateWorldRedundencyRadius(INT16 sX, INT16 sY, INT16 sRadius);
void DirtyWorldRender();

// These two functions will setup the glow frame script to use then glowing enemy mercs....
void SetMercGlowFast();
void SetMercGlowNormal();

void SetRenderCenter(INT16 sNewX, INT16 sNewY);

#ifdef _DEBUG
void RenderFOVDebug();
#endif

#endif
