#ifndef __VOBJECT_BLITTERS
#define __VOBJECT_BLITTERS

#include "SGP/Types.h"

struct ImageDataParams;
struct VObject;
struct VSurface;

extern struct GRect ClippingRect;
extern UINT32 guiTranslucentMask;

void GetClippingRect(struct GRect *clip);

BOOLEAN BltIsClipped(struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex,
                     struct GRect *clipregion);
CHAR8 BltIsClippedOrOffScreen(struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex,
                              struct GRect *clipregion);

UINT16 *InitZBuffer(UINT32 uiPitch, UINT32 uiHeight);
BOOLEAN ShutdownZBuffer(UINT16 *pBuffer);

// 8-Bit to 8-Bit Blitters

// pixelation blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                 UINT16 *pZBuffer, UINT16 usZValue,
                                                 struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                 UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipPixelate(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                     UINT16 *pZBuffer, UINT16 usZValue,
                                                     struct VObject *hSrcVObject, INT32 iX,
                                                     INT32 iY, UINT16 usIndex,
                                                     struct GRect *clipregion);

// translucency blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                  UINT16 *pZBuffer, UINT16 usZValue,
                                                  struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                  UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                    UINT16 *pZBuffer, UINT16 usZValue,
                                                    struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                    UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                        UINT16 *pZBuffer, UINT16 usZValue,
                                                        struct VObject *hSrcVObject, INT32 iX,
                                                        INT32 iY, UINT16 usIndex,
                                                        struct GRect *clipregion);

BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, struct GRect *clipregion,
                                               UINT16 usForeground, UINT16 usBackground,
                                               UINT16 usShadow);

BOOLEAN Blt8BPPDataTo16BPPBufferTransZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                       UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                       INT32 iY, UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                         UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                         INT32 iY, UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                UINT16 usIndex, struct GRect *clipregion,
                                                UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                          UINT16 *pZBuffer, UINT16 usZValue,
                                          struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                          UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              UINT16 *pZBuffer, UINT16 usZValue,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                        UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                        INT32 iY, UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            UINT16 *pZBuffer, UINT16 usZValue,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                 UINT16 *pZBuffer, UINT16 usZValue,
                                                 struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                 UINT16 usIndex, struct GRect *clipregion,
                                                 UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                   UINT16 *pZBuffer, UINT16 usZValue,
                                                   struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                   UINT16 usIndex, struct GRect *clipregion,
                                                   UINT16 *p16BPPPalette);

// Next blitters are for blitting mask as intensity
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                               UINT16 *pZBuffer, UINT16 usZValue,
                                               struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                               UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensity(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                          struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                          UINT16 usIndex);

BOOLEAN Blt8BPPDataTo16BPPBufferTransparentClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransparent(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex);

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, UINT16 *p16BPPPalette);

BOOLEAN Blt8BPPDataTo16BPPBufferShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, struct GRect *clipregion);

BOOLEAN Blt8BPPTo8BPP(UINT8 *pDest, UINT32 uiDestPitch, UINT8 *pSrc, UINT32 uiSrcPitch,
                      INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos,
                      UINT32 uiWidth, UINT32 uiHeight);
BOOLEAN Blt16BPPTo16BPP(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch,
                        INT32 iDestXPos, INT32 iDestYPos, struct GRect sourceRect);
BOOLEAN Blt16BPPTo16BPPTrans(UINT16 *pDest, UINT32 uiDestPitch, UINT16 *pSrc, UINT32 uiSrcPitch,
                             INT32 iDestXPos, INT32 iDestYPos, INT32 iSrcXPos, INT32 iSrcYPos,
                             UINT32 uiWidth, UINT32 uiHeight, UINT16 usTrans);

BOOLEAN Blt16BPPBufferPixelateRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct GRect *area, UINT8 Pattern[8][8],
                                            UINT16 usColor);
BOOLEAN Blt16BPPBufferPixelateRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, struct GRect *area,
                                   UINT8 Pattern[8][8]);

// A wrapper for the Blt16BPPBufferPixelateRect that automatically passes a hatch pattern.
BOOLEAN Blt16BPPBufferHatchRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, struct GRect *area);
BOOLEAN Blt16BPPBufferLooseHatchRectWithColor(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              struct GRect *area, UINT16 usColor);

BOOLEAN Blt16BPPBufferShadowRect(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, struct GRect *area);

BOOLEAN Blt8BPPDataTo16BPPBufferShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                       struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                       UINT16 usIndex);

// Blits from 8bpp to 16bpp.
// This function is only used for drawing strategic map.
bool Blt8bppTo16bp(struct ImageDataParams *source, u16 *dest, u32 destPitch, INT32 x, INT32 y);

void Blt16bppTo16bppRect(const struct ImageDataParams *source, u16 *dest, u32 destPitch, i32 x,
                         i32 y, struct GRect sourceRect);

// Blits a subrect from a flat 8 bit surface to a 16-bit buffer.
// This is the same as Blt8bppTo16bp with support of source rect.
bool Blt8bppTo16bppRect(const struct ImageDataParams *source, u16 *dest, u32 destPitch, INT32 iX,
                        INT32 iY, struct GRect sourceRect);

// New 16/16 blitters

// ATE: New blitters for showing an outline at color 254
BOOLEAN Blt8BPPDataTo16BPPBufferOutline(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                        struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                        UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                            UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline,
                                            struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZ(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer,
                                         UINT16 usZValue, struct VObject *hSrcVObject, INT32 iX,
                                         INT32 iY, UINT16 usIndex, INT16 s16BPPColor,
                                         BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadow(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                              UINT16 usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadowClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                  struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                                  UINT16 usIndex, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZNB(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                           UINT16 *pZBuffer, UINT16 usZValue,
                                           struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                           UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                         UINT16 *pZBuffer, UINT16 usZValue,
                                                         struct VObject *hSrcVObject, INT32 iX,
                                                         INT32 iY, UINT16 usIndex,
                                                         INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(
    UINT16 *pBuffer, UINT32 uiDestPitchBYTES, UINT16 *pZBuffer, UINT16 usZValue,
    struct VObject *hSrcVObject, INT32 iX, INT32 iY, UINT16 usIndex, INT16 s16BPPColor,
    BOOLEAN fDoOutline, struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                             UINT16 *pZBuffer, UINT16 usZValue,
                                             struct VObject *hSrcVObject, INT32 iX, INT32 iY,
                                             UINT16 usIndex, INT16 s16BPPColor, BOOLEAN fDoOutline,
                                             struct GRect *clipregion);

// ATE: New blitter for included shadow, but pixellate if obscured by z
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                       UINT16 *pZBuffer, UINT16 usZValue,
                                                       struct VObject *hSrcVObject, INT32 iX,
                                                       INT32 iY, UINT16 usIndex,
                                                       UINT16 *p16BPPPalette);

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                           UINT16 *pZBuffer, UINT16 usZValue,
                                                           struct VObject *hSrcVObject, INT32 iX,
                                                           INT32 iY, UINT16 usIndex,
                                                           struct GRect *clipregion,
                                                           UINT16 *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                           UINT16 *pZBuffer, UINT16 usZValue,
                                                           struct VObject *hSrcVObject, INT32 iX,
                                                           INT32 iY, UINT16 usIndex,
                                                           struct GRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelateObscured(UINT16 *pBuffer, UINT32 uiDestPitchBYTES,
                                                       UINT16 *pZBuffer, UINT16 usZValue,
                                                       struct VObject *hSrcVObject, INT32 iX,
                                                       INT32 iY, UINT16 usIndex);

#endif
