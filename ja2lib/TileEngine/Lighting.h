// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _LIGHTING_H_
#define _LIGHTING_H_

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "rust_images.h"

struct SOLDIERTYPE;
struct VObject;

/****************************************************************************************
 * JA2 Lighting Module
 *
 *		Tile-based, ray-casted lighting system.
 *
 *		Lights are precalculated into linked lists containing offsets from 0,0, and a light
 * level to add at that tile. Lists are constructed by casting a ray from the origin of
 * the light, and each tile stopped at is stored as a node in the list. To draw the light
 * during runtime, you traverse the list, checking at each tile that it isn't of the type
 * that can obscure light. If it is, you keep traversing the list until you hit a node
 * with a marker LIGHT_NEW_RAY, which means you're back at the origin, and have skipped
 * the remainder of the last ray.
 *
 * Written by Derek Beland, April 14, 1997
 *
 ***************************************************************************************/

#define DISTANCE_SCALE 4
#define LIGHT_DUSK_CUTOFF 8
#define LIGHT_DECAY 0.9  // shade level decay per tile distance

// lightlist node flags
#define LIGHT_NODE_DRAWN 0x00000001    // light node duplicate marker
#define LIGHT_ROOF_ONLY 0x00001000     // light only rooftops
#define LIGHT_IGNORE_WALLS 0x00002000  // doesn't take walls into account
#define LIGHT_BACKLIGHT 0x00004000     // light does not light objs, trees
#define LIGHT_NEW_RAY 0x00008000       // start of new ray in linked list
#define LIGHT_EVERYTHING 0x00010000    // light up everything
#define LIGHT_FAKE 0x10000000          // "fake" light	for display only

// standard light file symbols

#define LIGHT_OMNI_R1 "LTO1.LHT"
#define LIGHT_OMNI_R2 "LTO2.LHT"
#define LIGHT_OMNI_R3 "LTO3.LHT"
#define LIGHT_OMNI_R4 "LTO4.LHT"
#define LIGHT_OMNI_R5 "LTO5.LHT"
#define LIGHT_OMNI_R6 "LTO6.LHT"
#define LIGHT_OMNI_R7 "LTO7.LHT"
#define LIGHT_OMNI_R8 "LTO8.LHT"

#define MAX_LIGHT_TEMPLATES 32  // maximum number of light types
#define MAX_LIGHT_SPRITES 256   // maximum number of light types
#define SHADE_MIN 15            // DARKEST shade value
#define SHADE_MAX 1             // LIGHTEST shade value

// light sprite flags
#define LIGHT_SPR_ACTIVE 0x0001
#define LIGHT_SPR_ON 0x0002
#define LIGHT_SPR_ANIMATE 0x0004
#define LIGHT_SPR_ERASE 0x0008
#define LIGHT_SPR_REDRAW 0x0010
#define LIGHT_SPR_ONROOF 0x0020
#define MERC_LIGHT 0x0040
#define LIGHT_PRIMETIME 0x0080  // light turns goes on in evening, turns off at bedtime.
#define LIGHT_NIGHTTIME 0x0100  // light stays on when dark outside

#define COLOR_RED 162
#define COLOR_BLUE 203
#define COLOR_YELLOW 144
#define COLOR_GREEN 184
#define COLOR_LTGREY 134
#define COLOR_DKGREY 136
#define COLOR_BROWN 80
#define COLOR_PURPLE 160
#define COLOR_ORANGE 76
#define COLOR_WHITE 208
#define COLOR_BLACK 72

// stucture of node in linked list for lights
typedef struct light_p {
  int16_t iDX, iDY;
  uint8_t uiFlags;
  uint8_t ubLight;
} LIGHT_NODE;

// structure of light instance, or sprite (a copy of the template)
typedef struct {
  int16_t iX, iY;
  int16_t iOldX, iOldY;
  int16_t iAnimSpeed;
  int32_t iTemplate;
  uint32_t uiFlags;
  uint32_t uiLightType;
} LIGHT_SPRITE;

// Initializes the lighting system
BOOLEAN InitLightingSystem(void);
// Shuts down, and frees up all lights/memory
BOOLEAN ShutdownLightingSystem(void);
// Resets all light sprites and deallocates templates
BOOLEAN LightReset(void);

// THIS MUST BE CALLED ONCE ALL SURFACE VIDEO OBJECTS HAVE BEEN LOADED!
BOOLEAN SetDefaultWorldLightingColors(void);

// Low-Level Template Interface

// Sets the normal light level for all tiles in the world
BOOLEAN LightSetBaseLevel(uint8_t iIntensity);
// Adds a light value to all tiles
BOOLEAN LightAddBaseLevel(uint32_t uiLightType, uint8_t iIntensity);
// Subtracts a light value from all tiles
BOOLEAN LightSubtractBaseLevel(uint32_t uiLightType, uint8_t iIntensity);
// Creates an omni (circular) light
int32_t LightCreateOmni(uint8_t ubIntensity, int16_t iRadius);
// Creates an oval-shaped light (two separate radii)
int32_t LightCreateElliptical(uint8_t ubIntensity, int16_t iRadius1, int16_t iRadius2);
// Creates a square light
int32_t LightCreateSquare(uint8_t ubIntensity, int16_t iRadius1, int16_t iRadius2);
// Draws a light into the scene at X,Y
BOOLEAN LightDraw(uint32_t uiLightType, int32_t iLight, int16_t iX, int16_t iY, uint32_t uiSprite);
// Reverts the tiles a light has affected back to normal
BOOLEAN LightErase(uint32_t uiLightType, int32_t iLight, int16_t iX, int16_t iY, uint32_t uiSprite);
// Save a light list into a file
BOOLEAN LightSave(int32_t uiLight, char *pFilename);
// Load a light list from a file
int32_t LightLoad(char *pFilename);

// Sets the RGB values and number of light colors (1/2)
BOOLEAN LightSetColors(struct SGPPaletteEntry *pPal, uint8_t ubNumColors);
// Returns the number of colors active (1/2) and the palette colors
uint8_t LightGetColors(struct SGPPaletteEntry *pPal);

// High-Level Sprite Interface

// Creates a new light sprite from a given filename/predefined symbol
int32_t LightSpriteCreate(char *pName, uint32_t uiLightType);
// Destroys the instance of that light
BOOLEAN LightSpriteDestroy(int32_t iSprite);
// Sets the tile position of the light instance
BOOLEAN LightSpritePosition(int32_t iSprite, int16_t iX, int16_t iY);
// Makes a light "fake"
BOOLEAN LightSpriteFake(int32_t iSprite);
// Updates any change in position in lights
BOOLEAN LightSpriteRender();
// Renders all lights
BOOLEAN LightSpriteRenderAll(void);
// Turns on/off power to a light
BOOLEAN LightSpritePower(int32_t iSprite, BOOLEAN fOn);
// Moves light to/from roof position
BOOLEAN LightSpriteRoofStatus(int32_t iSprite, BOOLEAN fOnRoof);

// Reveals translucent walls
BOOLEAN CalcTranslucentWalls(int16_t iX, int16_t iY);
BOOLEAN ApplyTranslucencyToWalls(int16_t iX, int16_t iY);
// Makes trees translucent
BOOLEAN LightTranslucentTrees(int16_t iX, int16_t iY);
BOOLEAN LightHideTrees(int16_t iX, int16_t iY);
BOOLEAN LightShowRays(int16_t iX, int16_t iY, BOOLEAN fReset);
BOOLEAN LightHideRays(int16_t iX, int16_t iY);

// makes the 16-bit palettes
uint16_t CreateTilePaletteTables(struct VObject *pObj, uint32_t uiType, BOOLEAN fForce);
BOOLEAN CreateSoldierShadedPalette(struct SOLDIERTYPE *pSoldier, uint32_t uiBase,
                                   struct SGPPaletteEntry *pShadePal);
uint16_t CreateSoldierPaletteTables(struct SOLDIERTYPE *pSoldier, uint32_t uiType);

// returns the true light value at a tile (ignoring fake/merc lights)
uint8_t LightTrueLevel(int16_t sGridNo, int8_t bLevel);

// system variables
extern LIGHT_NODE *pLightList[MAX_LIGHT_TEMPLATES];
extern uint16_t usTemplateSize[MAX_LIGHT_TEMPLATES];
extern uint16_t *pLightRayList[MAX_LIGHT_TEMPLATES];
extern uint16_t usRaySize[MAX_LIGHT_TEMPLATES];
extern int16_t LightHeight[MAX_LIGHT_TEMPLATES];
extern int16_t LightWidth[MAX_LIGHT_TEMPLATES];
extern int16_t LightXOffset[MAX_LIGHT_TEMPLATES];
extern int16_t LightYOffset[MAX_LIGHT_TEMPLATES];
extern int16_t LightMapLeft[MAX_LIGHT_TEMPLATES];
extern int16_t LightMapTop[MAX_LIGHT_TEMPLATES];
extern int16_t LightMapRight[MAX_LIGHT_TEMPLATES];
extern int16_t LightMapBottom[MAX_LIGHT_TEMPLATES];
extern char *pLightNames[MAX_LIGHT_TEMPLATES];

// Sprite data
extern LIGHT_SPRITE LightSprites[MAX_LIGHT_SPRITES];

// Lighting system general data
extern uint8_t ubAmbientLightLevel;
extern uint8_t gubNumLightColors;

// Lighting colors
extern uint8_t gubNumLightColors;
extern struct SGPPaletteEntry gpLightColors[3];

// macros
#define LightSpriteGetType(x) (LightSprites[x].uiLightType)
#define LightSpriteGetTypeName(x) (pLightNames[LightSprites[x].iTemplate])
#define LightGetAmbient() (ubAmbientLightLevel)

#endif
