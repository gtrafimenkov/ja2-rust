#include <stdio.h>
#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/Shading.h"
#include "SGP/VObject.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurfaceInternal.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "rust_debug.h"
#include "rust_images.h"

#ifdef __GCC
// since some of the code is not complied on Linux
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

SGPRect ClippingRect = {0, 0, 640, 480};
UINT32 guiTranslucentMask = 0x3def;  // 0x7bef;		// mask for halving 5,6,5

// GLOBALS for pre-calculating skip values
INT32 gLeftSkip, gRightSkip, gTopSkip, gBottomSkip;
BOOLEAN gfUsePreCalcSkips = FALSE;

//*Experimental**********************************************************************

/***********************************************************************************/

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16
bit, and must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                        UINT16 *pZBuffer, UINT16 usZValue,
                                                        struct VObject *hSrcVObject, INT32 iX,
                                                        INT32 iY, UINT16 usIndex,
                                                        SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset, uiLineFlag;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		and		eax, [guiTranslucentMask]

		xor		edx, edx
		mov		dx, [edi]
		shr		edx, 1
		and		edx, [guiTranslucentMask]

		add		eax, edx

		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZTranslucent

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                  UINT16 *pZBuffer, UINT16 usZValue,
                                                  struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                  UINT16 usIndex) {
  UINT32 usHeight, usWidth, uiOffset, LineSkip;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 uiLineFlag;
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
        //		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		and		eax, guiTranslucentMask

		xor		edx, edx
		mov		dx, [edi]
		shr		edx, 1
		and		edx, guiTranslucentMask

		add		eax, edx
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBTranslucent

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16
bit, and must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("Translucents").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                    UINT16 *pZBuffer, UINT16 usZValue,
                                                    struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                    UINT16 usIndex) {
  UINT32 usHeight, usWidth, uiOffset, LineSkip;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 uiLineFlag;
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		edx, edx
		xor		eax, eax
		mov		edx, p16BPPPalette
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		shr		eax, 1
		mov		dx, [edi]
		and		eax, [guiTranslucentMask]

		shr		edx, 1
		and		edx, [guiTranslucentMask]

		add		eax, edx
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 InitZBuffer

        Allocates and initializes a Z buffer for use with the Z buffer blitters. Doesn't really do
        much except allocate a chunk of memory, and zero it.

**********************************************************************************************/
UINT16 *InitZBuffer(UINT32 uiPitch, UINT32 uiHeight) {
  UINT16 *pBuffer;

  if ((pBuffer = (UINT16 *)MemAlloc(uiPitch * uiHeight)) == NULL) return (NULL);

  memset(pBuffer, 0, (uiPitch * uiHeight));
  return (pBuffer);
}

/**********************************************************************************************
 ShutdownZBuffer

        Frees up the memory allocated for the Z buffer.

**********************************************************************************************/
BOOLEAN ShutdownZBuffer(UINT16 *pBuffer) {
  MemFree(pBuffer);
  return (TRUE);
}

//*****************************************************************************
//** 16 Bit Blitters
//**
//*****************************************************************************

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferMonoShadowClip

        Uses a bitmap an 8BPP template for blitting. Anywhere a 1 appears in the bitmap, a shadow
        is blitted to the destination (a black pixel). Any other value above zero is considered a
        forground color, and zero is background. If the parameter for the background color is zero,
        transparency is used for the background.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, SGPRect *clipregion,
                                               UINT16 usForeground, UINT16 usBackground,
                                               UINT16 usShadow) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax
		mov		al, [esi]
		cmp		al, 1
		jne		BlitNTL3

        // write shadow pixel
		mov		ax, usShadow

        // only write if not zero
		cmp		ax, 0
		je		BlitNTL2

		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		or		al, al
		jz		BlitNTL4

        // write foreground pixel
		mov		ax, usForeground
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL4:
		cmp		usBackground, 0
		je		BlitNTL2

		mov		ax, usBackground
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:
		sub		LSCount, ecx

		mov		ax, usBackground
		or		ax, ax
		jz		BTrans2

		rep		stosw
		jmp		BlitDispatch

BTrans2:
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
        Blt16BPPTo16BPP

        Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
        in the data area, for later blitting. Used to copy background information for mercs
        etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
BOOLEAN Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch,
                        INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos,
                        UINT32 uiWidth, UINT32 uiHeight) {
  UINT16 *pSrcPtr, *pDestPtr;
  UINT32 uiLineSkipDest, uiLineSkipSrc;

  Assert(pDest != NULL);
  Assert(pSrc != NULL);

  if (!pSrc || !pDest) {
    return FALSE;
  }

  pSrcPtr = (UINT16 *)((UINT8 *)pSrc + (iSrcYPos * uiSrcPitch) + (iSrcXPos * 2));
  pDestPtr = (UINT16 *)((UINT8 *)pDest + (iDestYPos * uiDestPitch) + (iDestXPos * 2));
  uiLineSkipDest = uiDestPitch - (uiWidth * 2);
  uiLineSkipSrc = uiSrcPitch - (uiWidth * 2);

#ifdef _WINDOWS
  __asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	cld

	mov		ecx, uiWidth
	test	ecx, 1
	jz		BlitDwords

BlitNewLine:

	mov		ecx, uiWidth
	shr		ecx, 1
	movsw

        // BlitNL2:

	rep		movsd

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

	jmp		BlitDone


BlitDwords:
	mov		ecx, uiWidth
	shr		ecx, 1
	rep		movsd

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitDwords

BlitDone:

  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
        Blt16BPPTo16BPPTrans

        Copies a rect of 16 bit data from a video buffer to a buffer position of the brush
        in the data area, for later blitting. Used to copy background information for mercs
        etc. to their unblit buffer, for later reblitting. Does NOT clip. Transparent color is
        not copied.

**********************************************************************************************/
BOOLEAN Blt16BPPTo16BPPTrans(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch,
                             INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos,
                             UINT32 uiWidth, UINT32 uiHeight, UINT16 usTrans) {
  UINT16 *pSrcPtr, *pDestPtr;
  UINT32 uiLineSkipDest, uiLineSkipSrc;

  Assert(pDest != NULL);
  Assert(pSrc != NULL);

  pSrcPtr = (UINT16 *)((UINT8 *)pSrc + (iSrcYPos * uiSrcPitch) + (iSrcXPos * 2));
  pDestPtr = (UINT16 *)((UINT8 *)pDest + (iDestYPos * uiDestPitch) + (iDestXPos * 2));
  uiLineSkipDest = uiDestPitch - (uiWidth * 2);
  uiLineSkipSrc = uiSrcPitch - (uiWidth * 2);

#ifdef _WINDOWS
  __asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	mov		dx, usTrans

BlitNewLine:
	mov		ecx, uiWidth

Blit2:
	mov		ax, [esi]
	cmp		ax, dx
	je		Blit3

	mov		[edi], ax

Blit3:
	add		esi, 2
	add		edi, 2
	dec		ecx
	jnz		Blit2

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/***********************************************************************************************
        Blt8BPPTo8BPP

        Copies a rect of an 8 bit data from a video buffer to a buffer position of the brush
        in the data area, for later blitting. Used to copy background information for mercs
        etc. to their unblit buffer, for later reblitting. Does NOT clip.

**********************************************************************************************/
BOOLEAN Blt8BPPTo8BPP(UINT8 *pDest, UINT32 uiDestPitch, UINT8 *pSrc, UINT32 uiSrcPitch,
                      INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos,
                      UINT32 uiWidth, UINT32 uiHeight) {
  UINT8 *pSrcPtr, *pDestPtr;
  UINT32 uiLineSkipDest, uiLineSkipSrc;

  Assert(pDest != NULL);
  Assert(pSrc != NULL);

  pSrcPtr = pSrc + (iSrcYPos * uiSrcPitch) + (iSrcXPos);
  pDestPtr = pDest + (iDestYPos * uiDestPitch) + (iDestXPos);
  uiLineSkipDest = uiDestPitch - (uiWidth);
  uiLineSkipSrc = uiSrcPitch - (uiWidth);

#ifdef _WINDOWS
  __asm {
	mov		esi, pSrcPtr
	mov		edi, pDestPtr
	mov		ebx, uiHeight
	cld

BlitNewLine:
	mov		ecx, uiWidth

	clc
	rcr		ecx, 1
	jnc		Blit2
	movsb

Blit2:
	clc
	rcr		ecx, 1
	jnc		Blit3

	movsw

Blit3:
	or		ecx, ecx
	jz		BlitLineDone

	rep		movsd

BlitLineDone:

	add		edi, uiLineSkipDest
	add		esi, uiLineSkipSrc
	dec		ebx
	jnz		BlitNewLine

  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZPixelate

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex) {
  UINT32 usHeight, usWidth, uiOffset, LineSkip;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 uiLineFlag;
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZPixelateObscured

        // OK LIKE NORMAL PIXELATE BUT ONLY PIXELATES STUFF BELOW Z level

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                       UINT16 *pZBuffer, UINT16 usZValue,
                                                       struct VObject *hSrcVObject, INT32 iX,
                                                       INT32 iY, UINT16 usIndex) {
  UINT32 usHeight, usWidth, uiOffset, LineSkip;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 uiLineFlag;
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

        // TEST FOR Z FIRST!
		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL8

        // Write it NOW!
		jmp		BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL9

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

        // Write normal z value
		mov		ax, usZValue
		mov		[ebx], ax
        // jmp   BlitNTL10

BlitNTL9:

        // Write no z
        // mov		ax, 32767
        // mov		[ebx], ax

        // BlitNTL10:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBPixelate

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16
bit, and must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                 UINT16 *pZBuffer, UINT16 usZValue,
                                                 struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                 UINT16 usIndex) {
  UINT32 usHeight, usWidth, uiOffset, LineSkip;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 uiLineFlag;
  struct Subimage *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx
		mov		edx, p16BPPPalette

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

        // ATE: DONOT WRITE Z VALUE
        // mov		ax, usZValue
        // mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClipPixelate

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16
bit, and must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                     UINT16 *pZBuffer, UINT16 usZValue,
                                                     struct VObject *hSrcVObject, INT32 iX,
                                                     INT32 iY, UINT16 usIndex,
                                                     SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset, uiLineFlag;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:
		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

        // mov		ax, usZValue
        // mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZ

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                       UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                       INT32 iY, UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

		mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNB

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on. The Z value is
        NOT updated by this version. The Z-buffer is 16 bit, and	must be the same dimensions
        (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                         UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                         INT32 iY, UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadow

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
        it is considered a shadow, and the destination buffer is darkened rather than blitted on.
        The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZ

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. If the source pixel is 254,
        it is considered a shadow, and the destination buffer is darkened rather than blitted on.
        The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNB

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
        updated. If the source pixel is 254, it is considered a shadow, and the destination
        buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
        dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL5

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBObscured

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
        updated. If the source pixel is 254, it is considered a shadow, and the destination
        buffer is darkened rather than blitted on. The Z-buffer is 16 bit, and must be the same
        dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                       UINT16 *pZBuffer, UINT16 usZValue,
                                                       struct VObject *hSrcVObject, INT32 iX,
                                                       INT32 iY, UINT16 usIndex,
                                                       UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  UINT32 uiLineFlag;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:


		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL8

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL8:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL5

		test	uiLineFlag, 1
		jz		BlitNTL9

		test	edi, 2
		jz		BlitNTL5

		jmp		BlitNTL6

BlitNTL9:
		test	edi, 2
		jnz		BlitNTL5


BlitNTL6:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination. Pixels with a value of
        254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                 UINT16 *pZBuffer, UINT16 usZValue,
                                                 struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                 UINT16 usIndex, SGPRect *clipregion,
                                                 UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination. Pixels with a value of
        254 are shaded instead of blitted.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                UINT16 usIndex, SGPRect *clipregion,
                                                UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on.
        The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
        destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
        NOT updated.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                   UINT16 *pZBuffer, UINT16 usZValue,
                                                   struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                   UINT16 usIndex, SGPRect *clipregion,
                                                   UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransShadowZNBClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on.
        The Z-buffer is 16 bit, and	must be the same dimensions (including Pitch) as the
        destination. Pixels with a value of	254 are shaded instead of blitted. The Z buffer is
        NOT updated.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                           UINT16 *pZBuffer, UINT16 usZValue,
                                                           struct VObject *hSrcVObject, INT32 iX,
                                                           INT32 iY, UINT16 usIndex,
                                                           SGPRect *clipregion,
                                                           UINT16 *p16BPPPalette) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted, uiLineFlag;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:

		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitPixellate

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitPixellate:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL2

		test	uiLineFlag, 1
		jz		BlitNTL9

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL3

BlitNTL9:
		test	edi, 2
		jnz		BlitNTL2


BlitNTL3:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZ

        Creates a shadow using a brush, but modifies the destination buffer only if the current
        Z level is equal to higher than what's in the Z buffer at that pixel location. It
        updates the Z buffer with the new Z level.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                        UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                        INT32 iY, UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		mov		ax, usZValue
		mov		[ebx], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            UINT16 *pZBuffer, UINT16 usZValue,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZNB

        Creates a shadow using a brush, but modifies the destination buffer only if the current
        Z level is equal to higher than what's in the Z buffer at that pixel location. It does
        NOT update the Z buffer with the new Z value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                          UINT16 *pZBuffer, UINT16 usZValue,
                                          struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                          UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowZNBClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, the Z value is
        not updated,	for any non-transparent pixels. The Z-buffer is 16 bit, and	must be the
        same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              UINT16 *pZBuffer, UINT16 usZValue,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZNBClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on. The Z value is NOT
        updated in this version. The Z-buffer is 16 bit, and must be the same dimensions (including
Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		ja		BlitNTL2

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// Blits a subrect from a flat 8 bit surface to a 16-bit buffer.
// This is the same as Blt8bppTo16bp with support of source rect.
bool Blt8bppTo16bppRect(const struct ImageDataParams *source, UINT16 *dest, u32 destPitch, INT32 iX,
                        INT32 iY, SGPRect *sourceRect) {
  UINT16 *p16BPPPalette;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip, LeftSkip, RightSkip, TopSkip, BlitLength, SrcSkip, BlitHeight;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(source != NULL);
  Assert(dest != NULL);

  // Get Offsets from Index into structure
  usHeight = (UINT32)source->height;
  usWidth = (UINT32)source->width;

  // Add to start position of dest buffer
  iTempX = iX;
  iTempY = iY;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  LeftSkip = sourceRect->iLeft;
  RightSkip = usWidth - sourceRect->iRight;
  TopSkip = sourceRect->iTop * source->pitch;
  BlitLength = sourceRect->iRight - sourceRect->iLeft;
  BlitHeight = sourceRect->iBottom - sourceRect->iTop;
  SrcSkip = source->pitch - BlitLength;

  SrcPtr = (UINT8 *)source->data + TopSkip + LeftSkip;
  DestPtr = ((UINT8 *)dest + (destPitch * iTempY) + (iTempX * 2));
  p16BPPPalette = source->palette16bpp;
  LineSkip = (destPitch - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr  // pointer to current line start address in source
		mov		edi, DestPtr  // pointer to current line start address in destination
		mov		ebx, BlitHeight  // line counter (goes top to bottom)
		mov		edx, p16BPPPalette  // conversion table

		sub		eax, eax
		sub		ecx, ecx

NewRow:
		mov		ecx, BlitLength  // pixels to blit count

BlitLoop:
		mov		al, [esi]
		xor		ah, ah

		shl		eax, 1  // make it into a word index
		mov		ax, [edx+eax]  // get 16-bit version of 8-bit pixel
		mov		[edi], ax  // store it in destination buffer

		inc		edi
		inc		esi
		inc		edi
		dec		ecx
		jnz		BlitLoop

		add		esi, SrcSkip  // move line pointers down one line
		add		edi, LineSkip

		dec		ebx  // check line counter
		jnz		NewRow  // done blitting, exit

    // DoneBlit: // finished blit
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// Blits from 8bpp to 16bpp.
// This function is only used for drawing strategic map.
bool Blt8bppTo16bp(struct ImageDataParams *source, u16 *destBuffer, u32 destPitch, INT32 x,
                   INT32 y) {
  if (x < 0 || y < 0) {
    return false;
  }

  u16 height = source->height;
  u16 width = source->width;

  if (destPitch / 2 < width) {
    return false;
  }

  u8 *srcPtr = (UINT8 *)source->data;
  u16 *destPtr = destBuffer + (destPitch / 2 * y) + x;
  UINT16 *p16BPPPalette = source->palette16bpp;

  u32 destSkip = destPitch / 2 - width;
  u32 srcSkip = source->pitch - width;

  for (u16 y = 0; y < height; y++) {
    for (u16 x = 0; x < width; x++) {
      u8 index = *srcPtr++;
      u16 color = p16BPPPalette[index];
      *destPtr++ = color;
    }
    srcPtr += srcSkip;
    destPtr += destSkip;
  }

  return true;
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferHalfRect

        Blits from a flat surface to a 16-bit buffer, dividing the source image into
exactly half the size, from a sub-region.
        - Source rect is in source units.
        - In order to make sure the same pixels are skipped, always align the top and
                left coordinates to the same factor of two.
        - A rect specifying an odd number of pixels will divide out to an even
                number of pixels blitted to the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferHalfRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                         struct VSurface *hSrcVSurface, UINT8 *pSrcBuffer,
                                         UINT32 uiSrcPitch, INT32 iX, INT32 iY, SGPRect *pRect) {
  UINT16 *p16BPPPalette;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  INT32 iTempX, iTempY;
  UINT32 uiSrcSkip;

  // Assertions
  Assert(hSrcVSurface != NULL);
  Assert(pSrcBuffer != NULL);
  Assert(pBuffer != NULL);
  Assert(pRect != NULL);

  // Get Offsets from Index into structure
  usWidth = (UINT32)(pRect->iRight - pRect->iLeft);
  usHeight = (UINT32)(pRect->iBottom - pRect->iTop);

  // Add to start position of dest buffer
  iTempX = iX;
  iTempY = iY;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }
  if (!(usWidth > 0)) {
    return FALSE;
  }
  if (!(usHeight > 0)) {
    return FALSE;
  }
  if (!(usHeight <= hSrcVSurface->usHeight)) {
    return FALSE;
  }
  if (!(usWidth <= hSrcVSurface->usWidth)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)pSrcBuffer + (uiSrcPitch * pRect->iTop) + (pRect->iLeft);
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVSurface->p16BPPPalette;
  LineSkip = (uiDestPitchBYTES - (usWidth & 0xfffffffe));
  uiSrcSkip = (uiSrcPitch * 2) - (usWidth & 0xfffffffe);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr  // pointer to current line start address in source
		mov		edi, DestPtr  // pointer to current line start address in destination
		mov		ebx, usHeight  // line counter (goes top to bottom)
		shr		ebx, 1  // half the rows
		mov		edx, p16BPPPalette

		xor		eax, eax

BlitSetup:
		mov		ecx, usWidth
		shr		ecx, 1  // divide the width by 2

ReadMask:
		mov		al, [esi]
		xor		ah, ah
		inc		esi  // skip one source byte
		inc		esi

		shl		eax, 1  // make it into a word index
		mov		ax, [edx+eax]  // get 16-bit version of 8-bit pixel
		mov		[edi], ax  // store it in destination buffer
		inc		edi  // next pixel
		inc		edi

		dec		ecx
		jnz		ReadMask

                    // DoneRow:

		add		esi, uiSrcSkip  // move source pointer down one line
		add		edi, LineSkip

		dec		ebx  // check line counter
		jnz		BlitSetup  // done blitting, exit

    // DoneBlit: // finished blit
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/****************************INCOMPLETE***********************************************/

void SetClippingRect(SGPRect *clip) {
  Assert(clip != NULL);
  Assert(clip->iLeft < clip->iRight);
  Assert(clip->iTop < clip->iBottom);

  memcpy(&ClippingRect, clip, sizeof(SGPRect));
}

void GetClippingRect(SGPRect *clip) {
  Assert(clip != NULL);

  memcpy(clip, &ClippingRect, sizeof(SGPRect));
}

/**********************************************************************************************
        Blt16BPPBufferPixelateRectWithColor

                Given an 8x8 pattern and a color, pixelates an area by repeatedly "applying the
color" to pixels whereever there is a non-zero value in the pattern.

                KM:  Added Nov. 23, 1998
                This is all the code that I moved from Blt16BPPBufferPixelateRect().
                This function now takes a color field (which previously was
                always black.  The 3rd assembler line in this function:

                                mov		ax, usColor				// color of
pixel

                used to be:

                                xor   eax, eax					// color of pixel
(black or 0)

          This was the only internal modification I made other than adding the usColor argument.

*********************************************************************************************/
BOOLEAN Blt16BPPBufferPixelateRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area,
                                            UINT8 Pattern[8][8], UINT16 usColor) {
  INT32 width, height;
  UINT32 LineSkip;
  UINT16 *DestPtr;
  INT32 iLeft, iTop, iRight, iBottom;

  // Assertions
  Assert(pBuffer != NULL);
  Assert(Pattern != NULL);

  iLeft = max(ClippingRect.iLeft, area->iLeft);
  iTop = max(ClippingRect.iTop, area->iTop);
  iRight = min(ClippingRect.iRight - 1, area->iRight);
  iBottom = min(ClippingRect.iBottom - 1, area->iBottom);

  DestPtr = (pBuffer + (iTop * (uiDestPitchBYTES / 2)) + iLeft);
  width = iRight - iLeft + 1;
  height = iBottom - iTop + 1;
  LineSkip = (uiDestPitchBYTES - (width * 2));

  if (!(width >= 1)) {
    return FALSE;
  }
  if (!(height >= 1)) {
    return FALSE;
  }

#ifdef _WINDOWS
  __asm {
		mov		esi, Pattern  // Pointer to pixel pattern
		mov		edi, DestPtr  // Pointer to top left of rect area
		mov		ax, usColor  // color of pixel
		xor		ebx, ebx  // pattern column index
		xor		edx, edx  // pattern row index


BlitNewLine:
		mov		ecx, width

BlitLine:
		cmp	[esi+ebx], 0
		je	BlitLine2

		mov		[edi], ax

BlitLine2:
		add		edi, 2
		inc		ebx
		and		ebx, 07H
		or		ebx, edx
		dec		ecx
		jnz		BlitLine

		add		edi, LineSkip
		xor		ebx, ebx
		add		edx, 08H
		and		edx, 38H
		dec		height
		jnz		BlitNewLine
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// KM:  Modified Nov. 23, 1998
// Original prototype (this function) didn't have a color field.  I've added the color field to
// Blt16BPPBufferPixelateRectWithColor(), moved the previous implementation of this function there,
// and added the modification to allow a specific color.
BOOLEAN Blt16BPPBufferPixelateRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area,
                                   UINT8 Pattern[8][8]) {
  return Blt16BPPBufferPixelateRectWithColor(pBuffer, uiDestPitchBYTES, area, Pattern, 0);
}

// Uses black hatch color
BOOLEAN Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area) {
  UINT8 Pattern[8][8] = {{1, 0, 1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1, 0, 1},
                         {1, 0, 1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1, 0, 1},
                         {1, 0, 1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1, 0, 1},
                         {1, 0, 1, 0, 1, 0, 1, 0}, {0, 1, 0, 1, 0, 1, 0, 1}};
  return Blt16BPPBufferPixelateRectWithColor(pBuffer, uiDestPitchBYTES, area, Pattern, 0);
}

BOOLEAN Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              SGPRect *area, UINT16 usColor) {
  UINT8 Pattern[8][8] = {
      {1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 1, 0, 0, 0, 1, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0, 0},
  };
  return Blt16BPPBufferPixelateRectWithColor(pBuffer, uiDestPitchBYTES, area, Pattern, usColor);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadow

        Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
        image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                       struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                       UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, usHeight
		xor		ecx, ecx
		mov		edx, OFFSET ShadeTable

BlitDispatch:


		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

		add		esi, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		ebx
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransparent

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination.

**********************************************************************************************/

BOOLEAN Blt8BPPDataTo16BPPBufferTransparent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		mov		bl, [esi]
		mov		ax, [edx+ebx*2]
		mov		[edi], ax

		mov		bl, [esi+1]
		mov		ax, [edx+ebx*2]
		mov		[edi+2], ax

		mov		bl, [esi+2]
		mov		ax, [edx+ebx*2]
		mov		[edi+4], ax

		mov		bl, [esi+3]
		mov		ax, [edx+ebx*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransparentClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. Clips the brush.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransparentClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx  // check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip  // check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, ebx  // skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, ebx  // skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx  // Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		xor		eax, eax
		mov		al, [esi+1]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		xor		eax, eax
		mov		al, [esi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		xor		eax, eax
		mov		al, [esi+3]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 BltIsClipped

        Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
BOOLEAN BltIsClipped(struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex,
                     SGPRect *clipregion) {
  UINT32 usHeight, usWidth;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  if (min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth)) return (TRUE);

  if (min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth)) return (TRUE);

  if (min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight)) return (TRUE);

  if (min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight)) return (TRUE);

  return (FALSE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferShadowClip

        Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
        image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
        clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx  // check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip  // check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, ebx  // skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, ebx  // skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx  // Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
         //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
        Blt16BPPBufferShadowRect

                Darkens a rectangular area by 25%. This blitter is used by ShadowVideoObjectRect.

        pBuffer						Pointer to a 16BPP buffer
        uiDestPitchBytes	Pitch of the destination surface
        area							An SGPRect, the area to darken

*********************************************************************************************/
BOOLEAN Blt16BPPBufferShadowRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area) {
  INT32 width, height;
  UINT32 LineSkip;
  UINT16 *DestPtr;

  // Assertions
  Assert(pBuffer != NULL);

  // Clipping
  if (area->iLeft < ClippingRect.iLeft) area->iLeft = ClippingRect.iLeft;
  if (area->iTop < ClippingRect.iTop) area->iTop = ClippingRect.iTop;
  if (area->iRight >= ClippingRect.iRight) area->iRight = ClippingRect.iRight - 1;
  if (area->iBottom >= ClippingRect.iBottom) area->iBottom = ClippingRect.iBottom - 1;
  // if (!(area->iLeft >= ClippingRect.iLeft )) { return FALSE; }
  // if (!(area->iTop >= ClippingRect.iTop )) { return FALSE; }
  // if (!(area->iRight <= ClippingRect.iRight )) { return FALSE; }
  // if (!(area->iBottom <= ClippingRect.iBottom )) { return FALSE; }

  DestPtr = (pBuffer + (area->iTop * (uiDestPitchBYTES / 2)) + area->iLeft);
  width = area->iRight - area->iLeft + 1;
  height = area->iBottom - area->iTop + 1;
  LineSkip = (uiDestPitchBYTES - (width * 2));

  if (!(width >= 1)) {
    return FALSE;
  }
  if (!(height >= 1)) {
    return FALSE;
  }

#ifdef _WINDOWS
  __asm {
		mov		esi, OFFSET ShadeTable
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, LineSkip
		mov		edx, height

BlitNewLine:
		mov		ecx, width

BlitLine:
		mov		ax, [edi]
		mov		ax, [esi+eax*2]
		mov		[edi], ax
		add		edi, 2
		dec		ecx
		jnz		BlitLine

		add		edi, ebx
		dec		edx
		jnz		BlitNewLine
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
        Blt16BPPBufferShadowRect

                Darkens a rectangular area by 25%. This blitter is used by ShadowVideoObjectRect.

        pBuffer						Pointer to a 16BPP buffer
        uiDestPitchBytes	Pitch of the destination surface
        area							An SGPRect, the area to darken

*********************************************************************************************/
BOOLEAN Blt16BPPBufferShadowRectAlternateTable(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               SGPRect *area) {
  INT32 width, height;
  UINT32 LineSkip;
  UINT16 *DestPtr;

  // Assertions
  Assert(pBuffer != NULL);

  // Clipping
  if (area->iLeft < ClippingRect.iLeft) area->iLeft = ClippingRect.iLeft;
  if (area->iTop < ClippingRect.iTop) area->iTop = ClippingRect.iTop;
  if (area->iRight >= ClippingRect.iRight) area->iRight = ClippingRect.iRight - 1;
  if (area->iBottom >= ClippingRect.iBottom) area->iBottom = ClippingRect.iBottom - 1;
  // if (!(area->iLeft >= ClippingRect.iLeft )) { return FALSE; }
  // if (!(area->iTop >= ClippingRect.iTop )) { return FALSE; }
  // if (!(area->iRight <= ClippingRect.iRight )) { return FALSE; }
  // if (!(area->iBottom <= ClippingRect.iBottom )) { return FALSE; }

  DestPtr = (pBuffer + (area->iTop * (uiDestPitchBYTES / 2)) + area->iLeft);
  width = area->iRight - area->iLeft + 1;
  height = area->iBottom - area->iTop + 1;
  LineSkip = (uiDestPitchBYTES - (width * 2));

  if (!(width >= 1)) {
    return FALSE;
  }
  if (!(height >= 1)) {
    return FALSE;
  }

#ifdef _WINDOWS
  __asm {
		mov		esi, OFFSET IntensityTable
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, LineSkip
		mov		edx, height

BlitNewLine:
		mov		ecx, width

BlitLine:
		mov		ax, [edi]
		mov		ax, [esi+eax*2]
		mov		[edi], ax
		add		edi, 2
		dec		ecx
		jnz		BlitLine

		add		edi, ebx
		dec		edx
		jnz		BlitNewLine
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 BltIsClippedOrOffScreen

        Determines whether a given blit will need clipping or not. Returns TRUE/FALSE.

**********************************************************************************************/
CHAR8 BltIsClippedOrOffScreen(struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex,
                              SGPRect *clipregion) {
  UINT32 usHeight, usWidth;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  gLeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  gRightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  gTopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  gBottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  gfUsePreCalcSkips = TRUE;

  // check if whole thing is clipped
  if ((gLeftSkip >= (INT32)usWidth) || (gRightSkip >= (INT32)usWidth)) return (-1);

  // check if whole thing is clipped
  if ((gTopSkip >= (INT32)usHeight) || (gBottomSkip >= (INT32)usHeight)) return (-1);

  if (gLeftSkip) return (TRUE);

  if (gRightSkip) return (TRUE);

  if (gTopSkip) return (TRUE);

  if (gBottomSkip) return (TRUE);

  return (FALSE);
}

// Blt8BPPDataTo16BPPBufferOutline
// ATE New blitter for rendering a differrent color for value 254. Can be transparent if fDoOutline
// is FALSE
BOOLEAN Blt8BPPDataTo16BPPBufferOutline(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                        struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                        UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  p16BPPPalette = hSrcVObject->pShadeCurrent;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5


BlitNTL6:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// ATE New blitter for rendering a differrent color for value 254. Can be transparent if fDoOutline
// is FALSE
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline,
                                            SGPRect *clipregion) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;
  UINT16 *p16BPPPalette;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  p16BPPPalette = hSrcVObject->pShadeCurrent;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:
		xor		eax, eax

		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline,
                                             SGPRect *clipregion) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;
  UINT16 *p16BPPPalette;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);

  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  p16BPPPalette = hSrcVObject->pShadeCurrent;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL2

        // CHECK FOR OUTLINE...
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:

        // Write to z-buffer
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		inc		ebx
		inc		ebx
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(
    UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue,
    struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor,
    BOOLEAN fDoOutline, SGPRect *clipregion) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;
  UINT16 *p16BPPPalette;
  UINT32 uiLineFlag;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);

  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL8

        // Write it now!
		jmp BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL9


BlitNTL6:

		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		mov		[ebx], ax

BlitNTL9:

        // CHECK FOR OUTLINE...
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL3

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL2

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL3:

        // Write to z-buffer
		mov		[ebx], ax

		xor		eax, eax

		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		inc		ebx
		inc		ebx
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex) {
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  UINT16 *p16BPPPalette;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  p16BPPPalette = hSrcVObject->pShadeCurrent;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		je		BlitNTL5

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax


BlitNTL5:
		inc		esi
		add		edi, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                  struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                  UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET ShadeTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx  // check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

        // Check for outline as well
		mov		cl, [esi]
		cmp		cl, 254
		je		TopSkipLoop
        //

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip  // check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, ebx  // skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, ebx  // skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx  // Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
         //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                         UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                         INT32 iY, UINT16 usIndex, INT16 s16BPPColor,
                                         BOOLEAN fDoOutline) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

        // CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL6:

        // Donot write to z-buffer
		mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                         UINT16 *pZBuffer, UINT16 usZValue,
                                                         struct VObject *hSrcVObject, INT32 iX,
                                                         INT32 iY, UINT16 usIndex,
                                                         INT16 s16BPPColor, BOOLEAN fDoOutline) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;
  UINT32 uiLineFlag;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jbe		BlitNTL8

        // Write it now!
		jmp BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL5
		jmp		BlitNTL9


BlitNTL6:

		test	edi, 2
		jnz		BlitNTL5

BlitNTL7:

		mov		[ebx], ax

BlitNTL9:

        // CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL12

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL12:

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		xor		uiLineFlag, 1
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// This is the same as above, but DONOT WRITE to Z!
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jb		BlitNTL5

        // CHECK FOR OUTLINE, BLIT DIFFERENTLY IF WE WANT IT TO!
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL6

        //		DO OUTLINE
        //		ONLY IF WE WANT IT!
		mov		al, fDoOutline;
		cmp		al,	1
		jne		BlitNTL5

		mov		ax, s16BPPColor
		mov		[edi], ax
		jmp		BlitNTL5

BlitNTL6:

        // Donot write to z-buffer
        // mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZ

        Creates a shadow using a brush, but modifies the destination buffer only if the current
        Z level is equal to higher than what's in the Z buffer at that pixel location. It
        updates the Z buffer with the new Z level.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax
		mov		ax, usZValue
		mov		[ebx], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL2

		mov		ax, usZValue
		mov		[ebx], ax

		xor		eax, eax

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityZNB

        Creates a shadow using a brush, but modifies the destination buffer only if the current
        Z level is equal to higher than what's in the Z buffer at that pixel location. It does
        NOT update the Z buffer with the new Z value.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

BlitNTL4:

		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL5

		xor		eax, eax
		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensityClip

        Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
        image as a mask. Any Non-zero index pixels are used to darken destination pixels. Blitter
        clips brush if it doesn't fit on the viewport.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex, SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, OFFSET IntensityTable
		xor		eax, eax
		mov		ebx, TopSkip
		xor		ecx, ecx

		or		ebx, ebx  // check for nothing clipped on top
		jz		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		ebx
		jnz		TopSkipLoop




LeftSkipSetup:

		mov		Unblitted, 0
		mov		ebx, LeftSkip  // check for nothing clipped on the left
		or		ebx, ebx
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, ebx
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, ebx  // skip partial run, jump into normal loop for rest
		sub		ecx, ebx
		mov		ebx, BlitLength
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		ebx, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, ebx
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, ebx  // skip partial run, jump into normal loop for rest
		mov		ebx, BlitLength
		jmp		BlitTransparent


LSTrans1:
		sub		ebx, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop




BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		ebx, BlitLength
		mov		Unblitted, 0

BlitDispatch:

		or		ebx, ebx  // Check to see if we're done blitting
		jz		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:

		cmp		ecx, ebx
		jbe		BNTrans1

		sub		ecx, ebx
		mov		Unblitted, ecx
		mov		ecx, ebx

BNTrans1:
		sub		ebx, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitLineEnd

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
		cmp		ecx, ebx
		jbe		BTrans1

		mov		ecx, ebx

BTrans1:

		sub		ebx, ecx
         //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferIntensity

        Modifies the destination buffer. Darkens the destination pixels by 25%, using the source
        image as a mask. Any Non-zero index pixels are used to darken destination pixels.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferIntensity(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                          struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                          UINT16 usIndex) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset;
  UINT32 usHeight, usWidth;
  UINT8 *SrcPtr, *DestPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  // Validations
  if (!(iTempX >= 0)) {
    return FALSE;
  }
  if (!(iTempY >= 0)) {
    return FALSE;
  }

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		mov		ebx, usHeight
		xor		ecx, ecx
		mov		edx, OFFSET IntensityTable

BlitDispatch:


		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

		add		esi, ecx

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

BlitNTL4:

		mov		ax, [edi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

		mov		ax, [edi+2]
		mov		ax, [edx+eax*2]
		mov		[edi+2], ax

		mov		ax, [edi+4]
		mov		ax, [edx+eax*2]
		mov		[edi+4], ax

		mov		ax, [edi+6]
		mov		ax, [edx+eax*2]
		mov		[edi+6], ax

		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		ebx
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        NOT updated to the current value,	for any non-transparent pixels. The Z-buffer is 16
bit, and must be the same dimensions (including Pitch) as the destination.

        Blits every second pixel ("pixelates").

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                           UINT16 *pZBuffer, UINT16 usZValue,
                                                           struct VObject *hSrcVObject, INT32 iX,
                                                           INT32 iY, UINT16 usIndex,
                                                           SGPRect *clipregion) {
  UINT16 *p16BPPPalette;
  UINT32 uiOffset, uiLineFlag;
  UINT32 usHeight, usWidth, Unblitted;
  UINT8 *SrcPtr, *DestPtr, *ZPtr;
  UINT32 LineSkip;
  struct Subimage *pTrav;
  INT32 iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  INT32 ClipX1, ClipY1, ClipX2, ClipY2;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->subimages[usIndex]);
  usHeight = (UINT32)pTrav->height;
  usWidth = (UINT32)pTrav->width;
  uiOffset = pTrav->data_offset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->x_offset;
  iTempY = iY + pTrav->y_offset;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (INT32)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (INT32)usWidth)) - ClipX2, (INT32)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (INT32)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (INT32)usHeight)) - ClipY2, (INT32)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((INT32)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((INT32)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (INT32)usWidth) || (RightSkip >= (INT32)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (INT32)usHeight) || (BottomSkip >= (INT32)usHeight)) return (TRUE);

  SrcPtr = (UINT8 *)hSrcVObject->image_data + uiOffset;
  DestPtr = (UINT8 *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (UINT8 *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));
  uiLineFlag = (iTempY & 1);

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

TopSkipLoop:  // Skips the number of lines clipped at the top

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

LeftSkipSetup:

		mov		Unblitted, 0
		mov		eax, LeftSkip
		mov		LSCount, eax
		or		eax, eax
		jz		BlitLineSetup

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNonTransLoop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop


BlitLineSetup:  // Does any actual blitting (trans/non) for the line
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent

BlitNonTransLoop:  // blit non-transparent pixels

		cmp		ecx, LSCount
		jbe		BNTrans1

		sub		ecx, LSCount
		mov		Unblitted, ecx
		mov		ecx, LSCount

BNTrans1:
		sub		LSCount, ecx

BlitNTL1:

        // OK, DO CHECK FOR Z FIRST!
		mov		ax, [ebx]
		cmp		ax, usZValue
		jae		BlitNTL8

        // ONLY WRITE DATA IF WE REALLY SHOULD
		mov		ax, usZValue
		mov		[ebx], ax
		jmp   BlitNTL7

BlitNTL8:

		test	uiLineFlag, 1
		jz		BlitNTL6

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitNTL7

BlitNTL6:
		test	edi, 2
		jnz		BlitNTL2

BlitNTL7:

		xor		eax, eax
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2
		dec		cl
		jnz		BlitNTL1

        // BlitLineEnd:
		add		esi, Unblitted
		jmp		BlitDispatch

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH
		cmp		ecx, LSCount
		jbe		BTrans1

		mov		ecx, LSCount

BTrans1:

		sub		LSCount, ecx
        //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


RightSkipLoop:    // skip along until we hit and end-of-line marker


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

#ifdef __GCC
#pragma GCC diagnostic pop
#endif
