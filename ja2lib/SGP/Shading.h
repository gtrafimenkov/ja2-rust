#ifndef _SHADING_H_
#define _SHADING_H_

#include "SGP/VObject.h"

struct VSurface;

BOOLEAN ShadesCalculateTables(struct SGPPaletteEntry *p8BPPPalette);

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent(FLOAT uiShadePercent);

void Init8BitTables(void);
BOOLEAN Set8BitModePalette(struct SGPPaletteEntry *pPal);

extern UINT8 ubColorTables[HVOBJECT_SHADE_TABLES + 3][256];

extern UINT16 IntensityTable[65536];
extern UINT16 ShadeTable[65536];
extern UINT16 White16BPPPalette[256];
extern FLOAT guiShadePercent;
extern FLOAT guiBrightPercent;

#define DEFAULT_SHADE_LEVEL 4

#endif
