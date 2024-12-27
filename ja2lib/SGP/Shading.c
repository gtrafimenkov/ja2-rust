// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Shading.h"

#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "rust_colors.h"
#include "rust_images.h"

// since some of the code is not complied on Linux
#ifdef __GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

BOOLEAN ShadesCalculateTables(struct SGPPaletteEntry *p8BPPPalette);
BOOLEAN ShadesCalculatePalette(struct SGPPaletteEntry *pSrcPalette,
                               struct SGPPaletteEntry *pDestPalette, uint16_t usRed,
                               uint16_t usGreen, uint16_t usBlue, BOOLEAN fMono);
void FindIndecies(struct SGPPaletteEntry *pSrcPalette, struct SGPPaletteEntry *pMapPalette,
                  uint8_t *pTable);
void FindMaskIndecies(uint8_t *, uint8_t *, uint8_t *);

struct SGPPaletteEntry Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES + 3][256];
uint8_t ubColorTables[HVOBJECT_SHADE_TABLES + 3][256];

uint16_t IntensityTable[65536];
uint16_t ShadeTable[65536];
uint16_t White16BPPPalette[256];
float guiShadePercent = (float)0.48;
float guiBrightPercent = (float)1.1;

BOOLEAN ShadesCalculateTables(struct SGPPaletteEntry *p8BPPPalette) {
  uint32_t uiCount;

  // Green palette
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[0], 0, 255, 0, TRUE);
  // Blue palette
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES], 0, 0, 255, TRUE);
  // Yellow palette
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES + 1], 255, 255, 0,
                         TRUE);
  // Red palette
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES + 2], 255, 0, 0,
                         TRUE);

  // these are the brightening tables, 115%-150% brighter than original
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[1], 293, 293, 293, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[2], 281, 281, 281, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[3], 268, 268, 268, FALSE);

  // palette 4 is the non-modified palette.
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[4], 255, 255, 255, FALSE);

  // the rest are darkening tables, right down to all-black.
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[5], 195, 195, 195, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[6], 165, 165, 165, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[7], 135, 135, 135, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[8], 105, 105, 105, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[9], 75, 75, 75, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[10], 45, 45, 45, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[11], 36, 36, 36, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[12], 27, 27, 27, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[13], 18, 18, 18, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[14], 9, 9, 9, FALSE);
  ShadesCalculatePalette(p8BPPPalette, Shaded8BPPPalettes[15], 0, 0, 0, FALSE);

  // Remap the shade colors to the original palette
  for (uiCount = 0; uiCount < (HVOBJECT_SHADE_TABLES + 3); uiCount++) {
    FindIndecies(Shaded8BPPPalettes[uiCount], p8BPPPalette, ubColorTables[uiCount]);
    ubColorTables[uiCount][0] = 0;
  }

  return (TRUE);
}

BOOLEAN ShadesCalculatePalette(struct SGPPaletteEntry *pSrcPalette,
                               struct SGPPaletteEntry *pDestPalette, uint16_t usRed,
                               uint16_t usGreen, uint16_t usBlue, BOOLEAN fMono) {
  uint32_t cnt, lumin;
  uint32_t rmod, gmod, bmod;

  Assert(pSrcPalette != NULL);
  Assert(pDestPalette != NULL);

  for (cnt = 0; cnt < 256; cnt++) {
    if (fMono) {
      lumin = (pSrcPalette[cnt].red * 299 / 1000) + (pSrcPalette[cnt].green * 587 / 1000) +
              (pSrcPalette[cnt].blue * 114 / 1000);
      rmod = usRed * lumin / 255;
      gmod = usGreen * lumin / 255;
      bmod = usBlue * lumin / 255;
    } else {
      rmod = (usRed * pSrcPalette[cnt].red / 255);
      gmod = (usGreen * pSrcPalette[cnt].green / 255);
      bmod = (usBlue * pSrcPalette[cnt].blue / 255);
    }

    pDestPalette[cnt].red = (uint8_t)min(rmod, 255);
    pDestPalette[cnt].green = (uint8_t)min(gmod, 255);
    pDestPalette[cnt].blue = (uint8_t)min(bmod, 255);
  }

  return (TRUE);
}

void FindIndecies(struct SGPPaletteEntry *pSrcPalette, struct SGPPaletteEntry *pMapPalette,
                  uint8_t *pTable) {
  uint16_t usCurIndex, usCurDelta, usCurCount;
  uint32_t *pSavedPtr;

#ifdef _WINDOWS
__asm {

// Assumes:
//	ESI = Pointer to source palette (shaded values)
//	EDI = Pointer to original palette (palette we'll end up using!)
//	EBX = Pointer to array of indecies

		mov		esi, pSrcPalette
		mov		edi, pMapPalette
		mov		ebx, pTable

		mov		BYTE PTR [ebx],0						; Index 0 is always 0, for trans col
		inc		ebx

		add		esi,4												; Goto next color entry
		add		edi,4
		mov		pSavedPtr, edi							; Save pointer to original pal

		mov		usCurCount, 255							; We will check cols 1-255

DoNextIndex:

		mov		edi, pSavedPtr							; Restore saved ptr
		mov		usCurIndex, 256							; Set found index & delta to some
		mov		usCurDelta, 0ffffh					; val so we get at least 1 col.

		mov		ecx,255											; Check cols 1-255 of orig pal
		push	ebx
		xor		bx,bx

NextColor:
		xor		ah,ah                       ; Calc delta between shaded color
		mov		al,[edi]										; and a color in the orig palette.
		mov		bl,[esi]										; Formula:
		sub		ax,bx                       ;  Delta = abs(red-origred) +
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC1
		neg		ax
NC1:mov		dx,ax                       ;          abs(blue-origblue)
		xor		ah,ah
		mov		al,[edi+1]
		mov		bl,[esi+1]
		sub		ax,bx
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC2
		neg		ax
NC2:add		dx,ax
		xor		ah,ah
		mov		al,[edi+2]
		mov		bl,[esi+2]
		sub		ax,bx
		or		ax,ax												;          abs(green-origgreen) +
		jns		NC3
		neg		ax
NC3:add		dx,ax

		cmp		dx,usCurDelta								; If delta < old delta
		jae		NotThisCol									;	Save this delta and its
		mov		ax,256                      ;	palette index
		mov		[usCurDelta],dx
		sub   ax,cx
		mov		[usCurIndex],ax
NotThisCol:
		add		edi,4												; Try next color in orginal pal
		dec		cx
		jnz   NextColor

		pop		ebx
		mov		ax,usCurIndex								; By now, usCurIndex holds pal index
		mov		[ebx],al										; of closest color in orig pal
		inc		ebx													; so save it, then repeat above
		add		esi,4												; for the other cols in shade pal
		dec		usCurCount
		jnz		DoNextIndex
	}
#else
  // Linux: NOT IMPLEMENTED
#endif
}

/**********************************************************************************************
 BuildShadeTable

        Builds a 16-bit color shading table. This function should be called only after the current
        video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
        globals for masks and shifts have been initialized by that function), and before any
        blitting is done.

        Using the table is a straight lookup. The pixel to be shaded down is used as the index into
        the table and the entry at that point will be a pixel that is 25% darker.

**********************************************************************************************/
void BuildShadeTable(void) {
  uint16_t red, green, blue;
  uint16_t index;

  for (red = 0; red < 256; red += 4)
    for (green = 0; green < 256; green += 4)
      for (blue = 0; blue < 256; blue += 4) {
        index = rgb32_to_rgb565(FROMRGB(red, green, blue));
        ShadeTable[index] = rgb32_to_rgb565(
            FROMRGB(red * guiShadePercent, green * guiShadePercent, blue * guiShadePercent));
      }

  memset(White16BPPPalette, 65535, sizeof(White16BPPPalette));
}

/**********************************************************************************************
 BuildIntensityTable

        Builds a 16-bit color shading table. This function should be called only after the current
        video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
        globals for masks and shifts have been initialized by that function), and before any
        blitting is done.



**********************************************************************************************/
void BuildIntensityTable(void) {
  uint16_t red, green, blue;
  uint16_t index;
  float dShadedPercent = (float)0.80;

#if 0

	uint32_t lumin;
	uint32_t rmod, gmod, bmod;

	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=rgb32_to_rgb565(FROMRGB(red, green, blue));

				lumin=( red*299/1000)+ ( green*587/1000 ) + ( blue*114/1000 );

				//lumin = min(lumin, 255);
				rmod=(255*lumin)/256;
				gmod=(100*lumin)/256;
				bmod=(100*lumin)/256;

				//rmod = __m( 255, rmod );

				IntensityTable[index]=rgb32_to_rgb565( FROMRGB( rmod, gmod , bmod ) );
			}
#endif

  for (red = 0; red < 256; red += 4)
    for (green = 0; green < 256; green += 4)
      for (blue = 0; blue < 256; blue += 4) {
        index = rgb32_to_rgb565(FROMRGB(red, green, blue));
        IntensityTable[index] = rgb32_to_rgb565(
            FROMRGB(red * dShadedPercent, green * dShadedPercent, blue * dShadedPercent));
      }
}

void SetShadeTablePercent(float uiShadePercent) {
  guiShadePercent = uiShadePercent;
  BuildShadeTable();
}

void Init8BitTables(void) {
  struct SGPPaletteEntry Pal[256];
  uint32_t uiCount;

  // calculate a grey-scale table for the default palette
  for (uiCount = 0; uiCount < 256; uiCount++) {
    Pal[uiCount].red = (uint8_t)(uiCount % 128) + 128;
    Pal[uiCount].green = (uint8_t)(uiCount % 128) + 128;
    Pal[uiCount].blue = (uint8_t)(uiCount % 128) + 128;
  }

  Pal[0].red = 0;
  Pal[0].green = 0;
  Pal[0].blue = 0;

  Set8BPPPalette(Shaded8BPPPalettes[4]);
}

BOOLEAN Set8BitModePalette(struct SGPPaletteEntry *pPal) {
  ShadesCalculateTables(pPal);
  Set8BPPPalette(pPal);
  return (TRUE);
}

#ifdef __GCC
#pragma GCC diagnostic pop
#endif
