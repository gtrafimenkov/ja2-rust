#ifndef _SHADING_H_
#define _SHADING_H_

#include "SGP/PaletteEntry.h"
#include "SGP/VObject.h"

struct VSurface;

BOOLEAN ShadesCalculateTables(struct SGPPaletteEntry *p8BPPPalette);

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent(float uiShadePercent);

void Init8BitTables(void);
BOOLEAN Set8BitModePalette(struct SGPPaletteEntry *pPal);

extern struct SGPPaletteEntry Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES + 3][256];
extern uint8_t ubColorTables[HVOBJECT_SHADE_TABLES + 3][256];

extern uint16_t IntensityTable[65536];
extern uint16_t ShadeTable[65536];
extern uint16_t White16BPPPalette[256];
extern float guiShadePercent;
extern float guiBrightPercent;

#define DEFAULT_SHADE_LEVEL 4

#endif
