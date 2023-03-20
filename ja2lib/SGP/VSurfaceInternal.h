#ifndef __VSURFACE_INTERNAL_H
#define __VSURFACE_INTERNAL_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"

struct VSurface {
  UINT16 usHeight;         // Height of Video Surface
  UINT16 usWidth;          // Width of Video Surface
  UINT8 ubBitDepth;        // BPP ALWAYS 16!
  PTR pSurfaceData;        // A void pointer, but for this implementation, is really a
                           // lpDirectDrawSurface;
  PTR pSurfaceData1;       // Direct Draw One Interface
  PTR pSavedSurfaceData1;  // A void pointer, but for this implementation, is really a
                           // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video
                           // memory Surfaces so that they my be restored
  PTR pSavedSurfaceData;   // A void pointer, but for this implementation, is really a
                          // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video memory
                          // Surfaces so that they my be restored
  UINT32 fFlags;              // Used to describe memory usage, etc
  PTR pPalette;               // A void pointer, but for this implementation a DDPalette
  UINT16 *p16BPPPalette;      // A 16BPP palette used for 8->16 blits
  COLORVAL TransparentColor;  // Defaults to 0,0,0
  PTR pClipper;               // A void pointer encapsolated as a clipper Surface
  HLIST RegionList;           // A List of regions within the video Surface
};

#endif
