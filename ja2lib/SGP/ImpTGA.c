//**************************************************************************
//
// Filename :	impTGA.c
//
//	Purpose :	.tga file importer
//
// Modification history :
//
//		20nov96:HJH				- Creation
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "SGP/ImpTGA.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "rust_fileman.h"

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

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

BOOLEAN ReadUncompColMapImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                              uint16_t fContents);
BOOLEAN ReadUncompRGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                           uint16_t fContents);
BOOLEAN ReadRLEColMapImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                           uint16_t fContents);
BOOLEAN ReadRLERGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                        uint16_t fContents);
// BOOLEAN	ConvertTGAToSystemBPPFormat( struct Image* hImage );

//**************************************************************************
//
//				Function Definitions
//
//**************************************************************************

BOOLEAN LoadTGAFileToImage(struct Image* hImage, uint16_t fContents) {
  FileID hFile = FILE_ID_ERR;
  uint8_t uiImgID, uiColMap, uiType;
  uint32_t uiBytesRead;
  BOOLEAN fReturnVal = FALSE;

  Assert(hImage != NULL);

  if (!(File_Exists(hImage->ImageFile))) {
    return FALSE;
  }

  hFile = File_OpenForReading(hImage->ImageFile);
  if (!(hFile)) {
    return FALSE;
  }

  if (!File_Read(hFile, &uiImgID, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMap, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiType, sizeof(uint8_t), &uiBytesRead)) goto end;

  switch (uiType) {
    case 1:
      fReturnVal = ReadUncompColMapImage(hImage, hFile, uiImgID, uiColMap, fContents);
      break;
    case 2:
      fReturnVal = ReadUncompRGBImage(hImage, hFile, uiImgID, uiColMap, fContents);
      break;
    case 9:
      fReturnVal = ReadRLEColMapImage(hImage, hFile, uiImgID, uiColMap, fContents);
      break;
    case 10:
      fReturnVal = ReadRLERGBImage(hImage, hFile, uiImgID, uiColMap, fContents);
      break;
    default:
      break;
  }

  // Set remaining values

end:
  File_Close(hFile);
  return (fReturnVal);
}

//**************************************************************************
//
// ReadUncompColMapImage
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		20nov96:HJH		-> creation
//
//**************************************************************************

BOOLEAN ReadUncompColMapImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                              uint16_t fContents) {
  return (FALSE);
}

//**************************************************************************
//
// ReadUncompRGBImage
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		20nov96:HJH		-> creation
//
//**************************************************************************

BOOLEAN ReadUncompRGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                           uint16_t fContents) {
  uint8_t* pBMData;
  uint8_t* pBMPtr;

  uint16_t uiColMapOrigin;
  uint16_t uiColMapLength;
  uint8_t uiColMapEntrySize;
  uint32_t uiBytesRead;
  uint16_t uiXOrg;
  uint16_t uiYOrg;
  uint16_t uiWidth;
  uint16_t uiHeight;
  uint8_t uiImagePixelSize;
  uint8_t uiImageDescriptor;
  uint32_t iNumValues;
  uint16_t cnt;

  uint32_t i;
  uint8_t r;
  uint8_t g;
  uint8_t b;

  if (!File_Read(hFile, &uiColMapOrigin, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMapLength, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMapEntrySize, sizeof(uint8_t), &uiBytesRead)) goto end;

  if (!File_Read(hFile, &uiXOrg, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiYOrg, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiWidth, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiHeight, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiImagePixelSize, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiImageDescriptor, sizeof(uint8_t), &uiBytesRead)) goto end;

  // skip the id
  File_Seek(hFile, uiImgID, FILE_SEEK_CURRENT);

  // skip the colour map
  if (uiColMap != 0) {
    File_Seek(hFile, uiColMapLength * (uiImagePixelSize / 8), FILE_SEEK_CURRENT);
  }

  // Set some struct Image* data values
  hImage->usWidth = uiWidth;
  hImage->usHeight = uiHeight;
  hImage->ubBitDepth = uiImagePixelSize;

  // Allocate memory based on bpp, height, width

  // Only do if contents flag is appropriate
  if (fContents & IMAGE_BITMAPDATA) {
    if (uiImagePixelSize == 16) {
      iNumValues = uiWidth * uiHeight;

      hImage->p16BPPData = (uint16_t*)MemAlloc(iNumValues * (uiImagePixelSize / 8));

      if (hImage->p16BPPData == NULL) goto end;

      // Get data pointer
      pBMData = hImage->p8BPPData;

      // Start at end
      pBMData += uiWidth * (uiHeight - 1) * (uiImagePixelSize / 8);

      // Data is stored top-bottom - reverse for SGP struct Image* format
      for (cnt = 0; cnt < uiHeight - 1; cnt++) {
        if (!File_Read(hFile, pBMData, uiWidth * 2, &uiBytesRead)) goto freeEnd;

        pBMData -= uiWidth * 2;
      }
      // Do first row
      if (!File_Read(hFile, pBMData, uiWidth * 2, &uiBytesRead)) goto freeEnd;

      // Convert TGA 5,5,5 16 BPP data into current system 16 BPP Data
      // ConvertTGAToSystemBPPFormat( hImage );

      hImage->fFlags |= IMAGE_BITMAPDATA;
    }

    if (uiImagePixelSize == 24) {
      hImage->p8BPPData = (uint8_t*)MemAlloc(uiWidth * uiHeight * (uiImagePixelSize / 8));

      if (hImage->p8BPPData == NULL) goto end;

      // Get data pointer
      pBMData = (uint8_t*)hImage->p8BPPData;

      // Start at end
      pBMPtr = pBMData + uiWidth * (uiHeight - 1) * 3;

      iNumValues = uiWidth * uiHeight;

      for (cnt = 0; cnt < uiHeight; cnt++) {
        for (i = 0; i < uiWidth; i++) {
          if (!File_Read(hFile, &b, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;
          if (!File_Read(hFile, &g, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;
          if (!File_Read(hFile, &r, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;

          pBMPtr[i * 3] = r;
          pBMPtr[i * 3 + 1] = g;
          pBMPtr[i * 3 + 2] = b;
        }
        pBMPtr -= uiWidth * 3;
      }
      hImage->fFlags |= IMAGE_BITMAPDATA;
    }

#if 0
		// 32 bit not yet allowed in SGP
		else if ( uiImagePixelSize == 32 )
		{
			iNumValues = uiWidth * uiHeight;

			for ( i=0 ; i<iNumValues; i++ )
			{
				if ( !File_Read( hFile, &b, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &g, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &r, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &a, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;

				pBMData[ i*3   ] = r;
				pBMData[ i*3+1 ] = g;
				pBMData[ i*3+2 ] = b;
			}
		}
#endif
  }
  return (TRUE);

end:
  return (FALSE);

freeEnd:
  MemFree(pBMData);
  return (FALSE);
}

//**************************************************************************
//
// ReadRLEColMapImage
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		20nov96:HJH		-> creation
//
//**************************************************************************

BOOLEAN ReadRLEColMapImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                           uint16_t fContents) {
  return (FALSE);
}

//**************************************************************************
//
// ReadRLERGBImage
//
//
//
// Parameter List :
// Return Value :
// Modification history :
//
//		20nov96:HJH		-> creation
//
//**************************************************************************

BOOLEAN ReadRLERGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap,
                        uint16_t fContents) {
  return (FALSE);
}

/*
BOOLEAN	ConvertTGAToSystemBPPFormat( struct Image* hImage )
{
        uint16_t		usX, usY;
        uint16_t		Old16BPPValue;
        uint16_t		*pData;
        uint16_t		usR, usG, usB;
        float			scale_val;
        uint32_t		uiRBitMask;
        uint32_t		uiGBitMask;
        uint32_t		uiBBitMask;
        uint8_t			ubRNewShift;
        uint8_t			ubGNewShift;
        uint8_t			ubBNewShift;
        uint8_t			ubScaleR;
        uint8_t			ubScaleB;
        uint8_t			ubScaleG;

        // Basic algorithm for coonverting to different rgb distributions

        // Get current Pixel Format from DirectDraw
        if (!( GetPrimaryRGBDistributionMasks( &uiRBitMask, &uiGBitMask, &uiBBitMask ) )) { return
FALSE; }

        // Only convert if different
        if ( uiRBitMask == 0x7c00 && uiGBitMask == 0x3e0 && uiBBitMask == 0x1f )
        {
                return( TRUE );
        }

        // Default values
        ubScaleR			= 0;
        ubScaleG			= 0;
        ubScaleB			= 0;
        ubRNewShift   = 10;
        ubGNewShift   = 5;
        ubBNewShift   = 0;

        // Determine values
  switch( uiBBitMask )
  {
                case 0x3f: // 0000000000111111 pixel mask for blue

                        // 5-5-6
                        ubRNewShift = 11;
                        ubGNewShift = 6;
                        ubScaleB		= 1;
                        break;

    case 0x1f: // 0000000000011111 pixel mask for blue
                        switch( uiGBitMask )
      {
                                case 0x7e0: // 0000011111100000 pixel mask for green

                // 5-6-5
                                        ubRNewShift = 11;
                                        ubScaleG    = 1;
                                        break;

        case 0x3e0: // 0000001111100000 pixel mask for green

                                        switch( uiRBitMask )
          {
                                                case 0xfc00: // 1111110000000000 pixel mask for red

                                                        // 6-5-5
                                                        ubScaleR	= 1;
                                                        break;
          }
          break;
      }
      break;
  }

        pData = hImage->pui16BPPPalette;
        usX = 0;
        do
        {

                usY = 0;

                do
                {

                        // Get Old 5,5,5 value
                        Old16BPPValue = hImage->p16BPPData[ usX * hImage->usWidth + usY ];

                        // Get component r,g,b values AT 5 5 5
                        usR = ( Old16BPPValue & 0x7c00 ) >> 10;
                        usG = ( Old16BPPValue & 0x3e0 ) >> 5;
                        usB = Old16BPPValue & 0x1f;

                        // Scale accordingly
                        usR = usR << ubScaleR;
                        usG = usG << ubScaleG;
                        usB = usB << ubScaleB;

                        hImage->p16BPPData[ usX * hImage->usWidth + usY ] = ((uint16_t) ( ( usR <<
ubRNewShift | usG << ubGNewShift ) | usB  ) );

                        usY++;

                } while( usY < hImage->usWidth );

                usX++;

        } while( usX < hImage->usHeight );

        return( TRUE );

}
*/
