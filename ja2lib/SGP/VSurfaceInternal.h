#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  UINT16 usHeight;   // Height of Video Surface
  UINT16 usWidth;    // Width of Video Surface
  UINT8 ubBitDepth;  // BPP ALWAYS 16!

  // A void pointer, but for this implementation, is really a
  // lpDirectDrawSurface;
  PTR pSurfaceData;

  // Direct Draw One Interface
  PTR pSurfaceData1;

  // A void pointer, but for this implementation, is really a
  // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video
  // memory Surfaces so that they my be restored
  PTR pSavedSurfaceData1;

  // A void pointer, but for this implementation, is really a
  // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video memory
  // Surfaces so that they my be restored
  PTR pSavedSurfaceData;

  // A void pointer, but for this implementation a DDPalette
  PTR pPalette;

  UINT32 fFlags;              // Used to describe memory usage, etc
  UINT16 *p16BPPPalette;      // A 16BPP palette used for 8->16 blits
  COLORVAL TransparentColor;  // Defaults to 0,0,0
  PTR pClipper;               // A void pointer encapsolated as a clipper Surface
  HLIST RegionList;           // A List of regions within the video Surface
};

#endif
