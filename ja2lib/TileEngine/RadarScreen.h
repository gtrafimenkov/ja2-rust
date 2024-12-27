#ifndef __RADAR_SCREEN_H
#define __RADAR_SCREEN_H

#include "Tactical/Interface.h"

// RADAR MOUSE REGION STUFF
void RadarRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void RadarRegionButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

BOOLEAN LoadRadarScreenBitmap(char *aFilename);

// RADAR WINDOW DEFINES
#define RADAR_WINDOW_X 543
#define RADAR_WINDOW_TM_Y INTERFACE_START_Y + 13
#define RADAR_WINDOW_SM_Y INV_INTERFACE_START_Y + 13
#define RADAR_WINDOW_WIDTH 88
#define RADAR_WINDOW_HEIGHT 44

BOOLEAN InitRadarScreen();
void RenderRadarScreen();
void MoveRadarScreen();

// disable rendering of radar screen
void DisableRadarScreenRender(void);

// enable rendering of radar screen
void EnableRadarScreenRender(void);

// toggle rendering flag of radar screen
void ToggleRadarScreenRender(void);

// create destroy squad list regions as needed
BOOLEAN CreateDestroyMouseRegionsForSquadList(void);

// clear out the video object for the radar map
void ClearOutRadarMapImage(void);

// do we render the radar screen?..or the squad list?
extern BOOLEAN fRenderRadarScreen;

#endif
