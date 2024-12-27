#ifndef __EDITORBUILDINGS_H
#define __EDITORBUILDINGS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

extern BOOLEAN fBuildingShowRoofs, fBuildingShowWalls, fBuildingShowRoomInfo;
extern uint8_t gubCurrRoomNumber;
extern uint8_t gubMaxRoomNumber;

void SetupTextInputForBuildings();
void ExtractAndUpdateBuildingInfo();

// Initialization routines
void GameInitEditorBuildingInfo();
void InitEditorBuildingsToolbar();

// Selection method callbacks
// Building utility functions
void UpdateBuildingsInfo();
void KillBuilding(uint32_t iMapIndex);

typedef struct BUILDINGLAYOUTNODE {
  struct BUILDINGLAYOUTNODE *next;
  int16_t sGridNo;
} BUILDINGLAYOUTNODE;

extern BUILDINGLAYOUTNODE *gpBuildingLayoutList;
extern int16_t gsBuildingLayoutAnchorGridNo;

// The first step is copying a building.  After that, it either must be pasted or moved.
void CopyBuilding(INT32 iMapIndex);
void MoveBuilding(INT32 iMapIndex);
void PasteBuilding(INT32 iMapIndex);
void DeleteBuildingLayout();

void ReplaceBuildingWithNewRoof(INT32 iMapIndex);
void UpdateWallsView();
void UpdateRoofsView();

void InitDoorEditing(INT32 iMapIndex);
void ExtractAndUpdateDoorInfo();
void KillDoorEditing();
void RenderDoorEditingWindow();

void AddLockedDoorCursors();
void RemoveLockedDoorCursors();
void FindNextLockedDoor();

extern BOOLEAN gfEditingDoor;

extern uint16_t usCurrentMode;

#endif
