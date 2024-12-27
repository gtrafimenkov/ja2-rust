#ifndef _MAP_SCREEN_INTERFACE_MAP_H
#define _MAP_SCREEN_INTERFACE_MAP_H

#include "SGP/Types.h"
#include "Tactical/Vehicles.h"

struct GROUP;
struct VObject;

// functions
void DrawMapIndexBigMap(BOOLEAN fSelectedCursorIsYellow);
// void DrawMapIndexSmallMap( BOOLEAN fSelectedCursorIsYellow );

uint32_t DrawMap(void);

void GetScreenXYFromMapXY(INT16 sMapX, INT16 sMapY, INT16 *psX, INT16 *psY);
void GetScreenXYFromMapXYStationary(INT16 sMapX, INT16 sMapY, INT16 *psX, INT16 *psY);

BOOLEAN InitializePalettesForMap(void);
void ShutDownPalettesForMap(void);

// plot path for helicopter
void PlotPathForHelicopter(INT16 sX, INT16 sY);

// the temp path, where the helicopter could go
void PlotATemporaryPathForHelicopter(INT16 sX, INT16 sY);

// trace a route for a passed path...doesn't require dest char - most more general
BOOLEAN TracePathRoute(BOOLEAN fCheckFlag, BOOLEAN fForceUpDate, struct path *pPath);

// show arrows for this char
void DisplayPathArrows(uint16_t usCharNum, struct VObject *hMapHandle);

// restore backgrounds for arrows
void RestoreArrowBackgroundsForTrace(INT32 iArrow, INT32 iArrowX, INT32 iArrowY, BOOLEAN fZoom);

// build path for character
void PlotPathForCharacter(struct SOLDIERTYPE *pCharacter, INT16 sX, INT16 sY,
                          BOOLEAN fTacticalTraversal);

// build temp path for character
void PlotATemporaryPathForCharacter(struct SOLDIERTYPE *pCharacter, INT16 sX, INT16 sY);

// display current/temp paths
void DisplaySoldierPath(struct SOLDIERTYPE *pCharacter);
void DisplaySoldierTempPath(struct SOLDIERTYPE *pCharacter);
void DisplayHelicopterPath(void);
void DisplayHelicopterTempPath(void);

// clear path after this sector
uint32_t ClearPathAfterThisSectorForCharacter(struct SOLDIERTYPE *pCharacter, INT16 sX, INT16 sY);

// cancel path : clear the path completely and gives player feedback message that the route was
// canceled
void CancelPathForCharacter(struct SOLDIERTYPE *pCharacter);
void CancelPathForVehicle(VEHICLETYPE *pVehicle, BOOLEAN fAlreadyReversed);
void CancelPathForGroup(struct GROUP *pGroup);

// check if we have waited long enought o update temp path
void DisplayThePotentialPathForHelicopter(INT16 sMapX, INT16 sMapY);

// clear out helicopter list after this sector
uint32_t ClearPathAfterThisSectorForHelicopter(INT16 sX, INT16 sY);

// check to see if sector is highlightable
BOOLEAN IsTheCursorAllowedToHighLightThisSector(u8 sSectorX, u8 sSectorY);

// set bad sectors list
void SetUpBadSectorsList(void);

// restore background for map grids
void RestoreBackgroundForMapGrid(INT16 sMapX, INT16 sMapY);

// clip blits to map view region
void ClipBlitsToMapViewRegion(void);
void ClipBlitsToMapViewRegionForRectangleAndABit(uint32_t uiDestPitchBYTES);

// clip blits to full screen....restore after use of ClipBlitsToMapViewRegion( )
void RestoreClipRegionToFullScreen(void);
void RestoreClipRegionToFullScreenForRectangle(uint32_t uiDestPitchBYTES);

// show the icons for people in motion
void ShowPeopleInMotion(INT16 sX, INT16 sY);

// last sector in helicopter's path
INT16 GetLastSectorOfHelicoptersPath(void);

// display info about helicopter path
void DisplayDistancesForHelicopter(void);

// display where hei is
void DisplayPositionOfHelicopter(void);

// check for click
BOOLEAN CheckForClickOverHelicopterIcon(INT16 sX, INT16 sY);

// load and delete the graphics for the map screen map militia
BOOLEAN LoadMilitiaPopUpBox(void);
void RemoveMilitiaPopUpBox(void);

// check if anyone left behind, if not, move selected cursor along with movement group
// void CheckIfAnyoneLeftInSector( INT16 sX, INT16 sY, INT16 sNewX, INT16 sNewY, int8_t bZ );

// create destroy
void CreateDestroyMilitiaPopUPRegions(void);

// draw the militia box
BOOLEAN DrawMilitiaPopUpBox(void);

// Returns true if the player knows how many enemies are in the sector if that number is greater
// than 0. Returns false for all other cases.
uint32_t WhatPlayerKnowsAboutEnemiesInSector(u8 sSectorX, u8 sSectorY);

// There is a special case flag used when players encounter enemies in a sector, then retreat.  The
// number of enemies will display on mapscreen until time is compressed.  When time is compressed,
// the flag is cleared, and a question mark is displayed to reflect that the player no longer knows.
// This is the function that clears that flag.
void ClearAnySectorsFlashingNumberOfEnemies();

void CopyPathToCharactersSquadIfInOne(struct SOLDIERTYPE *pCharacter);

void InitMapSecrets(void);

enum {
  ABORT_PLOTTING = 0,
  PATH_CLEARED,
  PATH_SHORTENED,
};

// what the player knows about the enemies in a given sector
enum {
  KNOWS_NOTHING = 0,
  KNOWS_THEYRE_THERE,
  KNOWS_HOW_MANY,
};

// size of squares on the map
#define MAP_GRID_X 21
#define MAP_GRID_Y 18

// scroll bounds
#define EAST_ZOOM_BOUND 378
#define WEST_ZOOM_BOUND 42
#define SOUTH_ZOOM_BOUND 324
#define NORTH_ZOOM_BOUND 36

// map view region
#define MAP_VIEW_START_X 270
#define MAP_VIEW_START_Y 10
#define MAP_VIEW_WIDTH 336
#define MAP_VIEW_HEIGHT 298

// zoomed in grid sizes
#define MAP_GRID_ZOOM_X MAP_GRID_X * 2
#define MAP_GRID_ZOOM_Y MAP_GRID_Y * 2

// number of units wide
#define WORLD_MAP_X 18

// dirty regions for the map
#define DMAP_GRID_X (MAP_GRID_X + 1)
#define DMAP_GRID_Y (MAP_GRID_Y + 1)
#define DMAP_GRID_ZOOM_X (MAP_GRID_ZOOM_X + 1)
#define DMAP_GRID_ZOOM_Y (MAP_GRID_ZOOM_Y + 1)

// Orta position on the map
#define ORTA_SECTOR_X 4
#define ORTA_SECTOR_Y 11

#define TIXA_SECTOR_X 9
#define TIXA_SECTOR_Y 10

// what are we showing?..teams/vehicles
// Show values
#define SHOW_TEAMMATES 1
#define SHOW_VEHICLES 2

// wait time until temp path is drawn, from placing cursor on a map grid
#define MIN_WAIT_TIME_FOR_TEMP_PATH 200

// zoom UL coords
extern INT32 iZoomX;
extern INT32 iZoomY;

// the number of militia on the cursor
extern INT16 sGreensOnCursor;
extern INT16 sRegularsOnCursor;
extern INT16 sElitesOnCursor;

// highlighted sectors
extern INT16 gsHighlightSectorX;
extern INT16 gsHighlightSectorY;

// the big map
extern uint32_t guiBIGMAP;

// the orta icon
extern uint32_t guiORTAICON;

extern uint32_t guiTIXAICON;

// the character icons
extern uint32_t guiCHARICONS;

// the merc arrival sector landing zone icon
extern uint32_t guiBULLSEYE;

// character between sector icons
extern uint32_t guiCHARBETWEENSECTORICONS;
extern uint32_t guiCHARBETWEENSECTORICONSCLOSE;

// the viewable map bound region
extern SGPRect MapScreenRect;

// draw temp path
extern BOOLEAN fDrawTempHeliPath;

// selected destination char
extern int8_t bSelectedDestChar;

// current assignment character
extern int8_t bSelectedAssignChar;

// the info character
extern int8_t bSelectedInfoChar;

// the contract char
extern int8_t bSelectedContractChar;

// map arrows graphical index value
extern uint32_t guiMAPCURSORS;

// has temp path for character path or helicopter been already drawn
extern BOOLEAN fTempPathAlreadyDrawn;

// map view region clipping rect
extern SGPRect MapScreenRect;

// the map border eta pop up
extern uint32_t guiMapBorderEtaPopUp;

// heli pop up
extern uint32_t guiMapBorderHeliSectors;

// the currently selected town militia
extern INT16 sSelectedMilitiaTown;

// the selected sectors
extern uint16_t sSelMapX;
extern uint16_t sSelMapY;

extern BOOLEAN fFoundTixa;

extern uint32_t guiSubLevel1, guiSubLevel2, guiSubLevel3;

#endif
