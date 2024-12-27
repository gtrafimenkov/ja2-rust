// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

//**************************************************************************
//
// Filename :	bitmap.h
//
//	Purpose :	bitmap format
//
// Modification history :
//
//		20nov96:HJH				- Creation
//
//**************************************************************************

#ifndef _bitmap_h
#define _bitmap_h

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "SGP/Types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

typedef struct sgpBmHeadertag {
  uint32_t uiNumBytes;      //	number of bytes of the bitmap, including the
                          // memory for all variables in this structure plus
                          // all the memory in bitmap
  uint32_t uiWidth;         // width of bitmap in pixels
  uint32_t uiHeight;        // height of bitmap in pixels
  uint8_t uiBitDepth;       // 8, 16, 24, or 32
  uint8_t uiNumPalEntries;  // if uiBitDepth is 8, non-zero, else 0
} SGPBmHeader;

typedef struct sgpBitmaptag {
  SGPBmHeader header;
  uint8_t uiData[1];  // if uiNumPalEntries != 0
                    //		uiNumPalEntries*3 (rgb) bytes for palette
                    // if uiBitDepth == 8
                    //		uiWidth * uiHeight bytes
                    // else if uiBitDepth == 16
                    //		uiWidth * uiHeight * 2 bytes
                    // else if uiBitDepth == 24
                    //		uiWidth * uiHeight * 3 bytes
                    // else if uiBitDepth == 32
                    //		uiWidth * uiHeight * 4 bytes
} SGPBitmap;

#endif
