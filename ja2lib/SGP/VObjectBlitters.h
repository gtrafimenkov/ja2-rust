#ifndef __VOBJECT_BLITTERS
#define __VOBJECT_BLITTERS

#include "SGP/Types.h"

struct VSurface;
struct VObject;

extern SGPRect ClippingRect;
extern uint32_t guiTranslucentMask;

extern void SetClippingRect(SGPRect *clip);
void GetClippingRect(SGPRect *clip);

BOOLEAN BltIsClipped(struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex,
                     SGPRect *clipregion);
char BltIsClippedOrOffScreen(struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex,
                             SGPRect *clipregion);

uint16_t *InitZBuffer(uint32_t uiPitch, uint32_t uiHeight);
BOOLEAN ShutdownZBuffer(uint16_t *pBuffer);

// 8-Bit to 8-Bit Blitters

// pixelation blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelate(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                               uint16_t *pZBuffer, uint16_t usZValue,
                                               struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                               uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBPixelate(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                 uint16_t *pZBuffer, uint16_t usZValue,
                                                 struct VObject *hSrcVObject, int32_t iX,
                                                 int32_t iY, uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipPixelate(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                     uint16_t *pZBuffer, uint16_t usZValue,
                                                     struct VObject *hSrcVObject, int32_t iX,
                                                     int32_t iY, uint16_t usIndex,
                                                     SGPRect *clipregion);

// translucency blitters
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTranslucent(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                  uint16_t *pZBuffer, uint16_t usZValue,
                                                  struct VObject *hSrcVObject, int32_t iX,
                                                  int32_t iY, uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBTranslucent(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                    uint16_t *pZBuffer, uint16_t usZValue,
                                                    struct VObject *hSrcVObject, int32_t iX,
                                                    int32_t iY, uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPDataTo16BPPBufferMonoShadowClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                               struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                               uint16_t usIndex, SGPRect *clipregion,
                                               uint16_t usForeground, uint16_t usBackground,
                                               uint16_t usShadow);

BOOLEAN Blt8BPPDataTo16BPPBufferTransZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                       uint16_t *pZBuffer, uint16_t usZValue,
                                       struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                       uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNB(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                         uint16_t *pZBuffer, uint16_t usZValue,
                                         struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                         uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                           uint16_t *pZBuffer, uint16_t usZValue,
                                           struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                           uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZNBClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             uint16_t *pZBuffer, uint16_t usZValue,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             uint16_t *pZBuffer, uint16_t usZValue,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex, uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                                uint16_t usIndex, SGPRect *clipregion,
                                                uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNB(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                               uint16_t *pZBuffer, uint16_t usZValue,
                                               struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                               uint16_t usIndex, uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNB(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                          uint16_t *pZBuffer, uint16_t usZValue,
                                          struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                          uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZNBClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              uint16_t *pZBuffer, uint16_t usZValue,
                                              struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                              uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                        uint16_t *pZBuffer, uint16_t usZValue,
                                        struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                        uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferShadowZClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                            uint16_t *pZBuffer, uint16_t usZValue,
                                            struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                            uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                 uint16_t *pZBuffer, uint16_t usZValue,
                                                 struct VObject *hSrcVObject, int32_t iX,
                                                 int32_t iY, uint16_t usIndex, SGPRect *clipregion,
                                                 uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                   uint16_t *pZBuffer, uint16_t usZValue,
                                                   struct VObject *hSrcVObject, int32_t iX,
                                                   int32_t iY, uint16_t usIndex,
                                                   SGPRect *clipregion, uint16_t *p16BPPPalette);

// Next blitters are for blitting mask as intensity
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZNB(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             uint16_t *pZBuffer, uint16_t usZValue,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                           uint16_t *pZBuffer, uint16_t usZValue,
                                           struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                           uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityZClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                               uint16_t *pZBuffer, uint16_t usZValue,
                                               struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                               uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensityClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                              uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferIntensity(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                          struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                          uint16_t usIndex);

BOOLEAN Blt8BPPDataTo16BPPBufferTransparentClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                                uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransparent(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                            uint16_t usIndex);

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadow(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                            uint16_t usIndex, uint16_t *p16BPPPalette);

BOOLEAN Blt8BPPDataTo16BPPBufferShadowClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                           struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                           uint16_t usIndex, SGPRect *clipregion);

BOOLEAN Blt8BPPTo8BPP(uint8_t *pDest, uint32_t uiDestPitch, uint8_t *pSrc, uint32_t uiSrcPitch,
                      int32_t iDestXPos, int32_t iDestYPos, int32_t iSrcXPos, int32_t iSrcYPos,
                      uint32_t uiWidth, uint32_t uiHeight);
BOOLEAN Blt16BPPTo16BPP(uint16_t *pDest, uint32_t uiDestPitch, uint16_t *pSrc, uint32_t uiSrcPitch,
                        int32_t iDestXPos, int32_t iDestYPos, int32_t iSrcXPos, int32_t iSrcYPos,
                        uint32_t uiWidth, uint32_t uiHeight);
BOOLEAN Blt16BPPTo16BPPTrans(uint16_t *pDest, uint32_t uiDestPitch, uint16_t *pSrc,
                             uint32_t uiSrcPitch, int32_t iDestXPos, int32_t iDestYPos,
                             int32_t iSrcXPos, int32_t iSrcYPos, uint32_t uiWidth,
                             uint32_t uiHeight, uint16_t usTrans);
BOOLEAN Blt16BPPTo16BPPMirror(uint16_t *pDest, uint32_t uiDestPitch, uint16_t *pSrc,
                              uint32_t uiSrcPitch, int32_t iDestXPos, int32_t iDestYPos,
                              int32_t iSrcXPos, int32_t iSrcYPos, uint32_t uiWidth,
                              uint32_t uiHeight);

BOOLEAN Blt16BPPBufferPixelateRectWithColor(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                            SGPRect *area, uint8_t Pattern[8][8], uint16_t usColor);
BOOLEAN Blt16BPPBufferPixelateRect(uint16_t *pBuffer, uint32_t uiDestPitchBYTES, SGPRect *area,
                                   uint8_t Pattern[8][8]);

// A wrapper for the Blt16BPPBufferPixelateRect that automatically passes a hatch pattern.
BOOLEAN Blt16BPPBufferHatchRect(uint16_t *pBuffer, uint32_t uiDestPitchBYTES, SGPRect *area);
BOOLEAN Blt16BPPBufferLooseHatchRectWithColor(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              SGPRect *area, uint16_t usColor);

BOOLEAN Blt16BPPBufferShadowRect(uint16_t *pBuffer, uint32_t uiDestPitchBYTES, SGPRect *area);

BOOLEAN Blt8BPPDataTo16BPPBufferShadow(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                       struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                       uint16_t usIndex);

BOOLEAN Blt8BPPDataTo16BPPBuffer(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                 struct VSurface *hSrcVSurface, uint8_t *pSrcBuffer, int32_t iX,
                                 int32_t iY);
BOOLEAN Blt8BPPDataSubTo16BPPBuffer(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                    struct VSurface *hSrcVSurface, uint8_t *pSrcBuffer,
                                    uint32_t uiSrcPitch, int32_t iX, int32_t iY, SGPRect *pRect);

// Blits from flat 8bpp source, to 16bpp dest, divides in half
BOOLEAN Blt8BPPDataTo16BPPBufferHalf(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                     struct VSurface *hSrcVSurface, uint8_t *pSrcBuffer,
                                     uint32_t uiSrcPitch, int32_t iX, int32_t iY);
BOOLEAN Blt8BPPDataTo16BPPBufferHalfRect(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                         struct VSurface *hSrcVSurface, uint8_t *pSrcBuffer,
                                         uint32_t uiSrcPitch, int32_t iX, int32_t iY,
                                         SGPRect *pRect);

// New 16/16 blitters

// ATE: New blitters for showing an outline at color 254
BOOLEAN Blt8BPPDataTo16BPPBufferOutline(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                        struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                        uint16_t usIndex, int16_t s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                            struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                            uint16_t usIndex, int16_t s16BPPColor,
                                            BOOLEAN fDoOutline, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                         uint16_t *pZBuffer, uint16_t usZValue,
                                         struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                         uint16_t usIndex, int16_t s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadow(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                              uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineShadowClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                  struct VObject *hSrcVObject, int32_t iX,
                                                  int32_t iY, uint16_t usIndex,
                                                  SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZNB(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                           uint16_t *pZBuffer, uint16_t usZValue,
                                           struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                           uint16_t usIndex, int16_t s16BPPColor,
                                           BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(uint16_t *pBuffer,
                                                         uint32_t uiDestPitchBYTES,
                                                         uint16_t *pZBuffer, uint16_t usZValue,
                                                         struct VObject *hSrcVObject, int32_t iX,
                                                         int32_t iY, uint16_t usIndex,
                                                         int16_t s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, int16_t s16BPPColor,
    BOOLEAN fDoOutline, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferOutlineZClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             uint16_t *pZBuffer, uint16_t usZValue,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex, int16_t s16BPPColor,
                                             BOOLEAN fDoOutline, SGPRect *clipregion);

// ATE: New blitter for included shadow, but pixellate if obscured by z
BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                       uint16_t *pZBuffer, uint16_t usZValue,
                                                       struct VObject *hSrcVObject, int32_t iX,
                                                       int32_t iY, uint16_t usIndex,
                                                       uint16_t *p16BPPPalette);

BOOLEAN Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion,
    uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZPixelateObscured(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                       uint16_t *pZBuffer, uint16_t usZValue,
                                                       struct VObject *hSrcVObject, int32_t iX,
                                                       int32_t iY, uint16_t usIndex);

BOOLEAN FillRect16BPP(uint16_t *pBuffer, uint32_t uiDestPitchBYTES, int32_t x1, int32_t y1,
                      int32_t x2, int32_t y2, uint16_t color);

#endif
