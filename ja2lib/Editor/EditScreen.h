#ifndef EDITSCREEN_H
#define EDITSCREEN_H

#include "BuildDefines.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Types.h"

#define EDITOR_LIGHT_MAX (SHADE_MIN + SHADE_MAX)
#define EDITOR_LIGHT_FAKE (EDITOR_LIGHT_MAX - SHADE_MAX - 2)

extern BOOLEAN gfFakeLights;
extern GUI_BUTTON* gpPersistantButton;

extern uint16_t GenericButtonFillColors[40];

void DisplayWayPoints(void);

BOOLEAN CheckForSlantRoofs(void);
BOOLEAN CheckForFences(void);

void ShowLightPositionHandles(void);
void RemoveLightPositionHandles(void);

extern BOOLEAN DoWindowSelection(void);
extern void RemoveTempMouseCursorObject(void);
extern BOOLEAN DrawTempMouseCursorObject(void);
extern void ShowCurrentDrawingMode(void);
extern void ShowCurrentSlotImage(struct VObject* hVObj, INT32 iWindow);
extern void ShowCurrentSlotSurface(uint32_t hSurface, INT32 iWindow);

void CreateNewMap();

void DisplayTilesetPage(void);
void FindTilesetComments(void);
void GetMasterList(void);

void HandleJA2ToolbarSelection(void);
void HandleKeyboardShortcuts();
uint32_t PerformSelectedAction(void);
uint32_t WaitForSelectionWindowResponse(void);
uint32_t WaitForMessageBoxResponse(uint32_t uiCurrentScreen);
uint32_t WaitForHelpScreenResponse(void);

BOOLEAN PlaceLight(int16_t sRadius, int16_t iMapX, int16_t iMapY, int16_t sType);
BOOLEAN RemoveLight(int16_t iMapX, int16_t iMapY);
extern BOOLEAN gfMercResetUponEditorEntry;

void ShowEditorToolbar(INT32 iNewTaskMode);
void HideEditorToolbar(INT32 iOldTaskMode);

void ProcessSelectionArea();

void MapOptimize(void);

extern uint16_t GenericButtonFillColors[40];

// These go together.  The taskbar has a specific color scheme.
extern uint16_t gusEditorTaskbarColor;
extern uint16_t gusEditorTaskbarHiColor;
extern uint16_t gusEditorTaskbarLoColor;

extern INT32 iOldTaskMode;
extern INT32 iCurrentTaskbar;
extern INT32 iTaskMode;

void ProcessAreaSelection(BOOLEAN fWithLeftButton);

void ShowEntryPoints();
void HideEntryPoints();

extern BOOLEAN gfConfirmExitFirst;
extern BOOLEAN gfConfirmExitPending;
extern BOOLEAN gfIntendOnEnteringEditor;

#endif
