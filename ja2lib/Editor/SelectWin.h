// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SELECTION_WIN_H_
#define _SELECTION_WIN_H_

#include "BuildDefines.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Types.h"

struct VObject;

#define CANCEL_ICON 0
#define UP_ICON 1
#define DOWN_ICON 2
#define OK_ICON 3

extern int32_t iButtonIcons[4];
extern int32_t iSelectWin, iCancelWin, iScrollUp, iScrollDown;

extern BOOLEAN fAllDone;

extern void CreateJA2SelectionWindow(int16_t sWhat);
extern void InitJA2SelectionWindow(void);
extern void ShutdownJA2SelectionWindow(void);
extern void RemoveJA2SelectionWindow(void);
extern void RenderSelectionWindow(void);
extern void DrawSelections(void);

extern void SelWinClkCallback(GUI_BUTTON *button, int32_t reason);
extern void CnclClkCallback(GUI_BUTTON *button, int32_t reason);
extern void UpClkCallback(GUI_BUTTON *button, int32_t reason);
extern void DwnClkCallback(GUI_BUTTON *button, int32_t reason);
extern void OkClkCallback(GUI_BUTTON *button, int32_t reason);

extern void ScrollSelWinUp(void);
extern void ScrollSelWinDown(void);

// defines for DisplaySpec.ubType

#define DISPLAY_TEXT 1
#define DISPLAY_GRAPHIC 2

#define ONE_COLUMN 0x0001
#define ONE_ROW 0x0002
#define CLEAR_BACKGROUND 0x0004

#define DISPLAY_ALL_OBJECTS 0xffff

#define MAX_SELECTIONS 120

enum {
  SELWIN_SINGLEWALL,
  SELWIN_SINGLEDOOR,
  SELWIN_SINGLEWINDOW,
  SELWIN_SINGLEROOF,
  SELWIN_SINGLENEWROOF,
  SELWIN_SINGLEBROKENWALL,
  SELWIN_SINGLEDECOR,
  SELWIN_SINGLEDECAL,
  SELWIN_SINGLEFLOOR,
  SELWIN_SINGLETOILET,

  SELWIN_ROOM,
  SELWIN_BANKS,
  SELWIN_ROADS,
  SELWIN_DEBRIS,
  SELWIN_OSTRUCTS,
  SELWIN_OSTRUCTS1,
  SELWIN_OSTRUCTS2
};

typedef struct {
  uint8_t ubType;
  union {
    struct {
      struct VObject *hVObject;
      uint16_t usStart;
      uint16_t usEnd;
      uint32_t uiObjIndx;
    };
    struct {
      uint16_t *pString;
    };
  };
} DisplaySpec;

typedef struct _DisplayList {
  struct VObject *hObj;
  uint16_t uiIndex;
  int16_t iX;
  int16_t iY;
  int16_t iWidth;
  int16_t iHeight;
  uint32_t uiObjIndx;
  BOOLEAN fChosen;
  struct _DisplayList *pNext;
} DisplayList;

typedef struct _Selections {
  uint32_t uiObject;
  uint16_t usIndex;
  int16_t sCount;
} Selections;

int32_t FindInSelectionList(DisplayList *pNode);
BOOLEAN IsInSelectionList(DisplayList *pNode);
int32_t GetRandomSelection(void);
BOOLEAN RemoveFromSelectionList(DisplayList *pNode);
void AddToSelectionList(DisplayList *pNode);
void RestoreSelectionList(void);
void SaveSelectionList(void);
BOOLEAN ClearSelectionList(void);

void DisplaySelectionWindowGraphicalInformation();

extern int32_t iCurBank;
extern Selections *pSelList;
extern int32_t *pNumSelList;

extern Selections SelOStructs[MAX_SELECTIONS];
extern Selections SelOStructs1[MAX_SELECTIONS];
extern Selections SelOStructs2[MAX_SELECTIONS];
extern Selections SelBanks[MAX_SELECTIONS];
extern Selections SelRoads[MAX_SELECTIONS];
extern Selections SelDebris[MAX_SELECTIONS];

extern Selections SelSingleWall[MAX_SELECTIONS];
extern Selections SelSingleDoor[MAX_SELECTIONS];
extern Selections SelSingleWindow[MAX_SELECTIONS];
extern Selections SelSingleRoof[MAX_SELECTIONS];
extern Selections SelSingleNewRoof[MAX_SELECTIONS];
extern Selections SelSingleBrokenWall[MAX_SELECTIONS];
extern Selections SelSingleDecor[MAX_SELECTIONS];
extern Selections SelSingleDecal[MAX_SELECTIONS];
extern Selections SelSingleFloor[MAX_SELECTIONS];
extern Selections SelSingleToilet[MAX_SELECTIONS];

extern Selections SelRoom[MAX_SELECTIONS];

extern int32_t iNumOStructsSelected;
extern int32_t iNumOStructs1Selected;
extern int32_t iNumOStructs2Selected;
extern int32_t iNumBanksSelected;
extern int32_t iNumRoadsSelected;
extern int32_t iNumDebrisSelected;
extern int32_t iNumWallsSelected;
extern int32_t iNumDoorsSelected;
extern int32_t iNumWindowsSelected;
extern int32_t iNumDecorSelected;
extern int32_t iNumDecalsSelected;
extern int32_t iNumBrokenWallsSelected;
extern int32_t iNumFloorsSelected;
extern int32_t iNumToiletsSelected;
extern int32_t iNumRoofsSelected;
extern int32_t iNumNewRoofsSelected;
extern int32_t iNumRoomsSelected;

extern int32_t iDrawMode;
extern DisplayList *TrashList(DisplayList *pNode);

extern BOOLEAN BuildDisplayWindow(DisplaySpec *pDisplaySpecs, uint16_t usNumSpecs,
                                  DisplayList **pDisplayList, SGPPoint *pUpperLeft,
                                  SGPPoint *pBottomRight, SGPPoint *pSpacing, uint16_t fFlags);

extern BOOLEAN DisplayWindowFunc(DisplayList *pNode, int16_t iTopCutOff, int16_t iBottomCutOff,
                                 SGPPoint *pUpperLeft, uint16_t fFlags);

#endif
