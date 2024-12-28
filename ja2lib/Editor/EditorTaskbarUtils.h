// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __EDITOR_TASKBAR_UTILS_H
#define __EDITOR_TASKBAR_UTILS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

// These are utilities that are used within the editor.  This function absorbs the expensive
// compile time of the SGP dependencies, while allowing the other editor files to hook into it
// without paying, so to speak.

enum { GUI_CLEAR_EVENT, GUI_LCLICK_EVENT, GUI_RCLICK_EVENT, GUI_MOVE_EVENT };

// Taskbar manipulation functions
void DoTaskbar(void);
void ProcessEditorRendering();
void EnableEditorTaskbar();
void DisableEditorTaskbar();
void CreateEditorTaskbar();
void DeleteEditorTaskbar();

// Button manipulation functions
void ClickEditorButton(int32_t iEditorButtonID);
void UnclickEditorButton(int32_t iEditorButtonID);
void HideEditorButton(int32_t iEditorButtonID);
void ShowEditorButton(int32_t iEditorButtonID);
void DisableEditorButton(int32_t iEditorButtonID);
void EnableEditorButton(int32_t iEditorButtonID);

void ClickEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);
void UnclickEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);
void HideEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);
void ShowEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);
void DisableEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);
void EnableEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID);

// Region Utils
#define NUM_TERRAIN_TILE_REGIONS 9
enum {
  BASE_TERRAIN_TILE_REGION_ID,
  ITEM_REGION_ID = NUM_TERRAIN_TILE_REGIONS,
  MERC_REGION_ID,
};

void EnableEditorRegion(int8_t bRegionID);
void DisableEditorRegion(int8_t bRegionID);

// Rendering Utils
void mprintfEditor(int16_t x, int16_t y, wchar_t* pFontString, ...);
void ClearTaskbarRegion(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom);
void DrawEditorInfoBox(wchar_t* str, uint32_t uiFont, uint16_t x, uint16_t y, uint16_t w,
                       uint16_t h);

extern int32_t giEditMercDirectionIcons[2];
extern uint32_t guiMercInventoryPanel;
extern uint32_t guiOmertaMap;
extern uint32_t guiExclamation;
extern uint32_t guiKeyImage;
extern uint32_t guiMercInvPanelBuffers[9];
extern uint32_t guiMercTempBuffer;
extern int32_t giEditMercImage[2];

#endif
