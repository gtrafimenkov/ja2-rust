#ifndef __CURSOR_DATABASE_
#define __CURSOR_DATABASE_

#include "SGP/Types.h"

struct VObject;

extern uint32_t GetCursorHandle(uint32_t uiCursorIndex);
extern void UnloadCursorData(uint32_t uiCursorIndex);
extern BOOLEAN LoadCursorData(uint32_t uiCursorIndex);
extern void CursorDatabaseClear(void);
extern uint16_t GetCursorSubIndex(uint32_t uiCursorIndex);
extern BOOLEAN SetCurrentCursorFromDatabase(uint32_t uiCursorIndex);

#define ANIMATED_CURSOR 0x02
#define USE_EXTERN_VO_CURSOR 0x04
#define USE_OUTLINE_BLITTER 0x08

#define EXTERN_CURSOR 0xFFF0
#define EXTERN2_CURSOR 0xFFE0
#define MAX_COMPOSITES 5
#define CENTER_SUBCURSOR 31000
#define HIDE_SUBCURSOR 32000

#define CENTER_CURSOR 32000
#define RIGHT_CURSOR 32001
#define LEFT_CURSOR 32002
#define TOP_CURSOR 32003
#define BOTTOM_CURSOR 32004

#define CURSOR_TO_FLASH 0x01
#define CURSOR_TO_FLASH2 0x02
#define CURSOR_TO_SUB_CONDITIONALLY 0x04
#define DELAY_START_CURSOR 0x08
#define CURSOR_TO_PLAY_SOUND 0x10

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cursor Database
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct CursorFileData;

typedef struct {
  uint32_t uiFileIndex;
  uint16_t uiSubIndex;
  uint32_t uiCurrentFrame;
  int16_t usPosX;
  int16_t usPosY;

} CursorImage;

typedef struct {
  CursorImage Composites[MAX_COMPOSITES];
  uint16_t usNumComposites;
  int16_t sOffsetX;
  int16_t sOffsetY;
  uint16_t usHeight;
  uint16_t usWidth;
  uint8_t bFlags;
  uint8_t bFlashIndex;

} CursorData;

extern int16_t gsGlobalCursorYOffset;

// Globals for cursor database offset values
extern int16_t gsCurMouseOffsetX;
extern int16_t gsCurMouseOffsetY;
extern uint16_t gsCurMouseHeight;
extern uint16_t gsCurMouseWidth;

extern uint32_t guiExternVo;
extern uint16_t gusExternVoSubIndex;
extern uint32_t guiExtern2Vo;
extern uint16_t gusExtern2VoSubIndex;
extern BOOLEAN gfExternUse2nd;

typedef void (*MOUSEBLT_HOOK)(void);

void InitCursorDatabase(struct CursorFileData *pCursorFileData, CursorData *pCursorData,
                        uint16_t suNumDataFiles);
void SetMouseBltHook(MOUSEBLT_HOOK pMouseBltOverride);

void SetExternVOData(uint32_t uiCursorIndex, struct VObject *hVObject, uint16_t usSubIndex);
void RemoveExternVOData(uint32_t uiCursorIndex);

#endif
