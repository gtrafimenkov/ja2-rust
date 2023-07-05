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

void GetScreenXYFromMapXY(uint8_t sMapX, uint8_t sMapY, int16_t *psX, int16_t *psY);

BOOLEAN InitializePalettesForMap(void);
void ShutDownPalettesForMap(void);

// plot path for helicopter
void PlotPathForHelicopter(uint8_t sX, uint8_t sY);

// the temp path, where the helicopter could go
void PlotATemporaryPathForHelicopter(uint8_t sX, uint8_t sY);

// trace a route for a passed path...doesn't require dest char - most more general
BOOLEAN TracePathRoute(BOOLEAN fCheckFlag, BOOLEAN fForceUpDate, struct path *pPath);

// show arrows for this char
void DisplayPathArrows(uint16_t usCharNum, struct VObject *hMapHandle);

// restore backgrounds for arrows
void RestoreArrowBackgroundsForTrace(int32_t iArrow, int32_t iArrowX, int32_t iArrowY,
                                     BOOLEAN fZoom);

// build path for character
void PlotPathForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX, uint8_t sY,
                          BOOLEAN fTacticalTraversal);

// build temp path for character
void PlotATemporaryPathForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX, uint8_t sY);

// display current/temp paths
void DisplaySoldierPath(struct SOLDIERTYPE *pCharacter);
void DisplaySoldierTempPath(struct SOLDIERTYPE *pCharacter);
void DisplayHelicopterPath(void);
void DisplayHelicopterTempPath(void);

// clear path after this sector
uint32_t ClearPathAfterThisSectorForCharacter(struct SOLDIERTYPE *pCharacter, uint8_t sX,
                                              uint8_t sY);

// cancel path : clear the path completely and gives player feedback message that the route was
// canceled
void CancelPathForCharacter(struct SOLDIERTYPE *pCharacter);
void CancelPathForVehicle(VEHICLETYPE *pVehicle, BOOLEAN fAlreadyReversed);
void CancelPathForGroup(struct GROUP *pGroup);

// check if we have waited long enought o update temp path
void DisplayThePotentialPathForHelicopter(uint8_t sMapX, uint8_t sMapY);

// clear out helicopter list after this sector
uint32_t ClearPathAfterThisSectorForHelicopter(uint8_t sX, uint8_t sY);

// check to see if sector is highlightable
BOOLEAN IsTheCursorAllowedToHighLightThisSector(uint8_t sSectorX, uint8_t sSectorY);

// set bad sectors list
void SetUpBadSectorsList(void);

// restore background for map grids
void RestoreBackgroundForMapGrid(uint8_t sMapX, uint8_t sMapY);

// clip blits to map view region
void ClipBlitsToMapViewRegion(void);
void ClipBlitsToMapViewRegionForRectangleAndABit(uint32_t uiDestPitchBYTES);

// clip blits to full screen....restore after use of ClipBlitsToMapViewRegion( )
void RestoreClipRegionToFullScreen(void);
void RestoreClipRegionToFullScreenForRectangle(uint32_t uiDestPitchBYTES);

// show the icons for people in motion
void ShowPeopleInMotion(uint8_t sX, uint8_t sY);

// last sector in helicopter's path
int16_t GetLastSectorOfHelicoptersPath(void);

// display info about helicopter path
void DisplayDistancesForHelicopter(void);

// display where hei is
void DisplayPositionOfHelicopter(void);

// check for click
BOOLEAN CheckForClickOverHelicopterIcon(uint8_t sClickedSectorX, uint8_t sClickedSectorY);

// load and delete the graphics for the map screen map militia
BOOLEAN LoadMilitiaPopUpBox(void);
void RemoveMilitiaPopUpBox(void);

// check if anyone left behind, if not, move selected cursor along with movement group
// void CheckIfAnyoneLeftInSector( int16_t sX, int16_t sY, int16_t sNewX, int16_t sNewY, int8_t bZ
// );

// create destroy
void CreateDestroyMilitiaPopUPRegions(void);

// draw the militia box
BOOLEAN DrawMilitiaPopUpBox(void);

// Returns true if the player knows how many enemies are in the sector if that number is greater
// than 0. Returns false for all other cases.
uint32_t WhatPlayerKnowsAboutEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY);

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
extern int32_t iZoomX;
extern int32_t iZoomY;

// the number of militia on the cursor
extern int16_t sGreensOnCursor;
extern int16_t sRegularsOnCursor;
extern int16_t sElitesOnCursor;

// highlighted sectors
extern uint8_t gsHighlightSectorX;
extern uint8_t gsHighlightSectorY;

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
extern struct GRect MapScreenRect;

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
extern struct GRect MapScreenRect;

// the map border eta pop up
extern uint32_t guiMapBorderEtaPopUp;

// heli pop up
extern uint32_t guiMapBorderHeliSectors;

// the currently selected town militia
extern int16_t sSelectedMilitiaTown;

// the selected sectors
extern uint8_t sSelMapX;
extern uint8_t sSelMapY;

extern BOOLEAN fFoundTixa;

extern uint32_t guiSubLevel1, guiSubLevel2, guiSubLevel3;

#endif
