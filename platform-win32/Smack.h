#ifndef SMACKH
#define SMACKH

#define SMACKVERSION "3.2f"

#ifndef __RADRES__

#include "RAD.h"

RADDEFSTART

typedef struct SmackTag {
  uint32_t Version;            // SMK2 only right now
  uint32_t Width;              // Width (1 based, 640 for example)
  uint32_t Height;             // Height (1 based, 480 for example)
  uint32_t Frames;             // Number of frames (1 based, 100 = 100 frames)
  uint32_t MSPerFrame;         // Frame Rate
  uint32_t SmackerType;        // bit 0 set=ring frame
  uint32_t LargestInTrack[7];  // Largest single size for each track
  uint32_t tablesize;          // Size of the init tables
  uint32_t codesize;           // Compression info
  uint32_t absize;             // ditto
  uint32_t detailsize;         // ditto
  uint32_t typesize;           // ditto
  uint32_t TrackType[7];       // high byte=0x80-Comp,0x40-PCM data,0x20-16 bit,0x10-stereo
  uint32_t extra;              // extra value (should be zero)
  uint32_t NewPalette;         // set to one if the palette changed
  uint8_t Palette[772];        // palette data
  uint32_t PalType;            // type of palette
  uint32_t FrameNum;           // Frame Number to be displayed
  uint32_t FrameSize;          // The current frame's size in bytes
  uint32_t SndSize;            // The current frame sound tracks' size in bytes
  s32 LastRectx;               // Rect set in from SmackToBufferRect (X coord)
  s32 LastRecty;               // Rect set in from SmackToBufferRect (Y coord)
  s32 LastRectw;               // Rect set in from SmackToBufferRect (Width)
  s32 LastRecth;               // Rect set in from SmackToBufferRect (Height)
  uint32_t OpenFlags;          // flags used on open
  uint32_t LeftOfs;            // Left Offset used in SmackTo
  uint32_t TopOfs;             // Top Offset used in SmackTo
  uint32_t LargestFrameSize;   // Largest frame size
  uint32_t Highest1SecRate;    // Highest 1 sec data rate
  uint32_t Highest1SecFrame;   // Highest 1 sec data rate starting frame
  uint32_t ReadError;          // Set to non-zero if a read error has ocurred
  uint32_t addr32;             // translated address for 16 bit interface
} Smack;

#define SmackHeaderSize(smk) ((((uint8_t*)&((smk)->extra)) - ((uint8_t*)(smk))) + 4)

typedef struct SmackSumTag {
  uint32_t TotalTime;          // total time
  uint32_t MS100PerFrame;      // MS*100 per frame (100000/MS100PerFrame=Frames/Sec)
  uint32_t TotalOpenTime;      // Time to open and prepare for decompression
  uint32_t TotalFrames;        // Total Frames displayed
  uint32_t SkippedFrames;      // Total number of skipped frames
  uint32_t SoundSkips;         // Total number of sound skips
  uint32_t TotalBlitTime;      // Total time spent blitting
  uint32_t TotalReadTime;      // Total time spent reading
  uint32_t TotalDecompTime;    // Total time spent decompressing
  uint32_t TotalBackReadTime;  // Total time spent reading in background
  uint32_t TotalReadSpeed;     // Total io speed (bytes/second)
  uint32_t SlowestFrameTime;   // Slowest single frame time
  uint32_t Slowest2FrameTime;  // Second slowest single frame time
  uint32_t SlowestFrameNum;    // Slowest single frame number
  uint32_t Slowest2FrameNum;   // Second slowest single frame number
  uint32_t AverageFrameSize;   // Average size of the frame
  uint32_t HighestMemAmount;   // Highest amount of memory allocated
  uint32_t TotalExtraMemory;   // Total extra memory allocated
  uint32_t HighestExtraUsed;   // Highest extra memory actually used
} SmackSum;

//=======================================================================
#define SMACKNEEDPAN 0x00020L     // Will be setting the pan
#define SMACKNEEDVOLUME 0x00040L  // Will be setting the volume
#define SMACKFRAMERATE 0x00080L   // Override fr (call SmackFrameRate first)
#define SMACKLOADEXTRA 0x00100L   // Load the extra buffer during SmackOpen
#define SMACKPRELOADALL 0x00200L  // Preload the entire animation
#define SMACKNOSKIP 0x00400L      // Don't skip frames if falling behind
#define SMACKSIMULATE 0x00800L    // Simulate the speed (call SmackSim first)
#define SMACKFILEHANDLE 0x01000L  // Use when passing in a file handle
#define SMACKTRACK1 0x02000L      // Play audio track 1
#define SMACKTRACK2 0x04000L      // Play audio track 2
#define SMACKTRACK3 0x08000L      // Play audio track 3
#define SMACKTRACK4 0x10000L      // Play audio track 4
#define SMACKTRACK5 0x20000L      // Play audio track 5
#define SMACKTRACK6 0x40000L      // Play audio track 6
#define SMACKTRACK7 0x80000L      // Play audio track 7
#define SMACKTRACKS \
  (SMACKTRACK1 | SMACKTRACK2 | SMACKTRACK3 | SMACKTRACK4 | SMACKTRACK5 | SMACKTRACK6 | SMACKTRACK7)

#define SMACKBUFFERREVERSED 0x00000001
#define SMACKBUFFER555 0x80000000
#define SMACKBUFFER565 0xc0000000
#define SMACKBUFFER16 (SMACKBUFFER555 | SMACKBUFFER565)

#define SMACKYINTERLACE 0x100000L                    // Force interleaving Y scaling
#define SMACKYDOUBLE 0x200000L                       // Force doubling Y scaling
#define SMACKYNONE (SMACKYINTERLACE | SMACKYDOUBLE)  // Force normal Y scaling
#define SMACKFILEISSMK 0x2000000L                    // Internal flag for 16 to 32 bit thunking

#define SMACKAUTOEXTRA 0xffffffffL  // NOT A FLAG! - Use as extrabuf param
//=======================================================================

#define SMACKSURFACEFAST 0
#define SMACKSURFACESLOW 1
#define SMACKSURFACEDIRECT 2

RADEXPFUNC Smack PTR4* RADEXPLINK SmackOpen(const char PTR4* name, uint32_t flags,
                                            uint32_t extrabuf);

#ifdef __RADMAC__
#include <files.h>

RADEXPFUNC Smack PTR4* RADEXPLINK SmackMacOpen(FSSpec* fsp, uint32_t flags, uint32_t extrabuf);
#endif

RADEXPFUNC uint32_t RADEXPLINK SmackDoFrame(Smack PTR4* smk);
RADEXPFUNC void RADEXPLINK SmackNextFrame(Smack PTR4* smk);
RADEXPFUNC uint32_t RADEXPLINK SmackWait(Smack PTR4* smk);
RADEXPFUNC void RADEXPLINK SmackClose(Smack PTR4* smk);

RADEXPFUNC void RADEXPLINK SmackVolumePan(Smack PTR4* smk, uint32_t trackflag, uint32_t volume,
                                          uint32_t pan);

RADEXPFUNC void RADEXPLINK SmackSummary(Smack PTR4* smk, SmackSum PTR4* sum);

RADEXPFUNC uint32_t RADEXPLINK SmackSoundInTrack(Smack PTR4* smk, uint32_t trackflags);
RADEXPFUNC uint32_t RADEXPLINK SmackSoundOnOff(Smack PTR4* smk, uint32_t on);

#ifndef __RADMAC__
RADEXPFUNC void RADEXPLINK SmackToScreen(Smack PTR4* smk, uint32_t left, uint32_t top,
                                         uint32_t BytePS, const uint16_t PTR4* WinTbl,
                                         void* SetBank, uint32_t Flags);
#endif

RADEXPFUNC void RADEXPLINK SmackToBuffer(Smack PTR4* smk, uint32_t left, uint32_t top,
                                         uint32_t Pitch, uint32_t destheight, const void PTR4* buf,
                                         uint32_t Flags);
RADEXPFUNC uint32_t RADEXPLINK SmackToBufferRect(Smack PTR4* smk, uint32_t SmackSurface);

RADEXPFUNC void RADEXPLINK SmackGoto(Smack PTR4* smk, uint32_t frame);
RADEXPFUNC void RADEXPLINK SmackColorRemapWithTrans(Smack PTR4* smk, const void PTR4* remappal,
                                                    uint32_t numcolors, uint32_t paltype,
                                                    uint32_t transindex);
#define SmackColorRemap(smk, remappal, numcolors, paltype) \
  SmackColorRemapWithTrans(smk, remappal, numcolors, paltype, 1000)
RADEXPFUNC void RADEXPLINK SmackColorTrans(Smack PTR4* smk, const void PTR4* trans);
RADEXPFUNC void RADEXPLINK SmackFrameRate(uint32_t forcerate);
RADEXPFUNC void RADEXPLINK SmackSimulate(uint32_t sim);

RADEXPFUNC uint32_t RADEXPLINK SmackGetTrackData(Smack PTR4* smk, void PTR4* dest,
                                                 uint32_t trackflag);

RADEXPFUNC void RADEXPLINK SmackSoundCheck(void);

//======================================================================

// the functions for the new SmackBlit API

typedef struct _SMACKBLIT PTR4* HSMACKBLIT;

typedef struct _SMACKBLIT {
  uint32_t Flags;
  uint8_t PTR4* Palette;
  uint32_t PalType;
  uint16_t PTR4* SmoothTable;
  uint16_t PTR4* Conv8to16Table;
  uint32_t whichmode;
  uint32_t palindex;
  uint32_t t16index;
  uint32_t smoothindex;
  uint32_t smoothtype;
  uint32_t firstpalette;
} SMACKBLIT;

#define SMACKBLIT1X 1
#define SMACKBLIT2X 2
#define SMACKBLIT2XSMOOTHING 4
#define SMACKBLIT2XINTERLACE 8

RADEXPFUNC HSMACKBLIT RADEXPLINK SmackBlitOpen(uint32_t flags);
RADEXPFUNC void RADEXPLINK SmackBlitSetPalette(HSMACKBLIT sblit, void PTR4* Palette,
                                               uint32_t PalType);
RADEXPFUNC uint32_t RADEXPLINK SmackBlitSetFlags(HSMACKBLIT sblit, uint32_t flags);
RADEXPFUNC void RADEXPLINK SmackBlit(HSMACKBLIT sblit, void PTR4* dest, uint32_t destpitch,
                                     uint32_t destx, uint32_t desty, void PTR4* src,
                                     uint32_t srcpitch, uint32_t srcx, uint32_t srcy, uint32_t srcw,
                                     uint32_t srch);
RADEXPFUNC void RADEXPLINK SmackBlitClear(HSMACKBLIT sblit, void PTR4* dest, uint32_t destpitch,
                                          uint32_t destx, uint32_t desty, uint32_t destw,
                                          uint32_t desth, s32 color);
RADEXPFUNC void RADEXPLINK SmackBlitClose(HSMACKBLIT sblit);
RADEXPFUNC void RADEXPLINK SmackBlitTrans(HSMACKBLIT sblit, void PTR4* dest, uint32_t destpitch,
                                          uint32_t destx, uint32_t desty, void PTR4* src,
                                          uint32_t srcpitch, uint32_t srcx, uint32_t srcy,
                                          uint32_t srcw, uint32_t srch, uint32_t trans);
RADEXPFUNC void RADEXPLINK SmackBlitMask(HSMACKBLIT sblit, void PTR4* dest, uint32_t destpitch,
                                         uint32_t destx, uint32_t desty, void PTR4* src,
                                         uint32_t srcpitch, uint32_t srcx, uint32_t srcy,
                                         uint32_t srcw, uint32_t srch, uint32_t trans,
                                         void PTR4* mask);
RADEXPFUNC void RADEXPLINK SmackBlitMerge(HSMACKBLIT sblit, void PTR4* dest, uint32_t destpitch,
                                          uint32_t destx, uint32_t desty, void PTR4* src,
                                          uint32_t srcpitch, uint32_t srcx, uint32_t srcy,
                                          uint32_t srcw, uint32_t srch, uint32_t trans,
                                          void PTR4* back);
RADEXPFUNC char PTR4* RADEXPLINK SmackBlitString(HSMACKBLIT sblit, char PTR4* dest);

#ifndef __RADMAC__
RADEXPFUNC uint32_t RADEXPLINK SmackUseMMX(uint32_t flag);  // 0=off, 1=on, 2=query current
#endif

//======================================================================
#ifdef __RADDOS__

#define SMACKSOUNDNONE -1

extern void* SmackTimerSetupAddr;
extern void* SmackTimerReadAddr;
extern void* SmackTimerDoneAddr;

typedef void RADEXPLINK (*SmackTimerSetupType)(void);
typedef uint32_t RADEXPLINK (*SmackTimerReadType)(void);
typedef void RADEXPLINK (*SmackTimerDoneType)(void);

#define SmackTimerSetup() ((SmackTimerSetupType)(SmackTimerSetupAddr))()
#define SmackTimerRead() ((SmackTimerReadType)(SmackTimerReadAddr))()
#define SmackTimerDone() ((SmackTimerDoneType)(SmackTimerDoneAddr))()

RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseMSS(void* DigDriver);

#ifndef AIL_startup
#ifdef __SW_3R
extern s32 cdecl AIL_startup_reg(void);
#define AIL_startup AIL_startup_reg
#else
extern s32 cdecl AIL_startup_stack(void);
#define AIL_startup AIL_startup_stack
#endif
#endif
#define SmackSoundMSSLiteInit() SmackSoundMSSLiteInitWithStart(&AIL_startup);
RADEXPFUNC void RADEXPLINK SmackSoundMSSLiteInitWithStart(void* start);
RADEXPFUNC void RADEXPLINK SmackSoundMSSLiteDone(void);

RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseSOS3r(uint32_t SOSDriver, uint32_t MaxTimerSpeed);
RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseSOS3s(uint32_t SOSDriver, uint32_t MaxTimerSpeed);
RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseSOS4r(uint32_t SOSDriver, uint32_t MaxTimerSpeed);
RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseSOS4s(uint32_t SOSDriver, uint32_t MaxTimerSpeed);

#ifdef __SW_3R
#define SmackSoundUseSOS3 SmackSoundUseSOS3r
#define SmackSoundUseSOS4 SmackSoundUseSOS4r
#else
#define SmackSoundUseSOS3 SmackSoundUseSOS3s
#define SmackSoundUseSOS4 SmackSoundUseSOS4s
#endif

#else

#define SMACKRESRESET 0
#define SMACKRES640X400 1
#define SMACKRES640X480 2
#define SMACKRES800X600 3
#define SMACKRES1024X768 4

RADEXPFUNC uint32_t RADEXPLINK SmackSetSystemRes(uint32_t mode);  // use SMACKRES* values

#define SMACKNOCUSTOMBLIT 128
#define SMACKSMOOTHBLIT 256
#define SMACKINTERLACEBLIT 512

#ifdef __RADMAC__

#include <palettes.h>
#include <qdoffscreen.h>
#include <windows.h>

#define SmackTimerSetup()
#define SmackTimerDone()
RADEXPFUNC uint32_t RADEXPLINK SmackTimerRead(void);

RADEXPFUNC s32 RADEXPLINK SmackGDSurfaceType(GDHandle gd);

#define SMACKAUTOBLIT 0
#define SMACKDIRECTBLIT 1
#define SMACKGWORLDBLIT 2

typedef struct SmackBufTag {
  uint32_t Reversed;
  uint32_t SurfaceType;  // SMACKSURFACExxxxxx
  uint32_t BlitType;     // SMACKxxxxxBLIT
  uint32_t Width;
  uint32_t Height;
  uint32_t Pitch;
  uint32_t Zoomed;
  uint32_t ZWidth;
  uint32_t ZHeight;
  uint32_t DispColors;  // colors on screen
  uint32_t MaxPalColors;
  uint32_t PalColorsInUse;
  uint32_t StartPalColor;
  uint32_t EndPalColor;
  void* Buffer;
  void* Palette;
  uint32_t PalType;
  uint32_t SoftwareCursor;

  WindowPtr wp;
  GWorldPtr gwp;
  CTabHandle cth;
  PaletteHandle palh;

  GDHandle gd;
  uint32_t gdSurfaceType;
  HSMACKBLIT sblit;
  void* ScreenAddr;
  uint32_t ScreenPitch;

  s32 manyblits;
  s32 PTR4* blitrects;
  s32 PTR4* rectsptr;
  s32 maxrects;
  s32 numrects;

} SmackBuf;

#else

#ifdef __RADWIN__

#define INCLUDE_MMSYSTEM_H
#include <windows.h>
#include <windowsx.h>

#ifdef __RADNT__  // to combat WIN32_LEAN_AND_MEAN

#include <mmsystem.h>

RADEXPFUNC s32 RADEXPLINK SmackDDSurfaceType(void* lpDDS);

#endif

#define SMACKAUTOBLIT 0
#define SMACKFULL320X240BLIT 1
#define SMACKFULL320X200BLIT 2
#define SMACKFULL320X200DIRECTBLIT 3
#define SMACKSTANDARDBLIT 4
#define SMACKWINGBLIT 5
#define SMACKDIBSECTIONBLIT 5

#define WM_SMACKACTIVATE WM_USER + 0x5678

typedef struct SmackBufTag {
  uint32_t Reversed;     // 1 if the buffer is upside down
  uint32_t SurfaceType;  // SMACKSURFACExxxx defines
  uint32_t BlitType;     // SMACKxxxxBLIT defines
  uint32_t FullScreen;   // 1 if full-screen
  uint32_t Width;
  uint32_t Height;
  uint32_t Pitch;
  uint32_t Zoomed;
  uint32_t ZWidth;
  uint32_t ZHeight;
  uint32_t DispColors;      // colors on the screen
  uint32_t MaxPalColors;    // total possible colors in palette (usually 256)
  uint32_t PalColorsInUse;  // Used colors in palette (usually 236)
  uint32_t StartPalColor;   // first usable color index (usually 10)
  uint32_t EndPalColor;     // last usable color index (usually 246)
  RGBQUAD Palette[256];
  uint32_t PalType;
  uint32_t forceredraw;  // force a complete redraw on next blit (for >8bit)
  uint32_t didapalette;  // force an invalidate on the next palette change

  void PTR4* Buffer;
  void PTR4* DIBRestore;
  uint32_t OurBitmap;
  uint32_t OrigBitmap;
  uint32_t OurPalette;
  uint32_t WinGDC;
  uint32_t FullFocused;
  uint32_t ParentHwnd;
  uint32_t OldParWndProc;
  uint32_t OldDispWndProc;
  uint32_t DispHwnd;
  uint32_t WinGBufHandle;
  void PTR4* lpDD;
  void PTR4* lpDDSP;
  uint32_t DDSurfaceType;
  HSMACKBLIT DDblit;
  s32 ddSoftwarecur;
  s32 didaddblit;
  s32 lastwasdd;
  RECT ddscreen;
  s32 manyblits;
  s32 PTR4* blitrects;
  s32 PTR4* rectsptr;
  s32 maxrects;
  s32 numrects;
  HDC lastdc;
} SmackBuf;

RADEXPFUNC void RADEXPLINK SmackGet(Smack PTR4* smk, void PTR4* dest);
RADEXPFUNC void RADEXPLINK SmackBufferGet(SmackBuf PTR4* sbuf, void PTR4* dest);

RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseMSS(void PTR4* dd);
RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseDirectSound(void PTR4* dd);  // NULL=Create
RADEXPFUNC void RADEXPLINK SmackSoundSetDirectSoundHWND(HWND hw);
RADEXPFUNC uint8_t RADEXPLINK SmackSoundUseDW(uint32_t openfreq, uint32_t openbits,
                                              uint32_t openchans);

#define SmackTimerSetup()
#define SmackTimerDone()
#define SmackTimerRead timeGetTime

#endif

#endif

#ifdef __RADMAC__
RADEXPFUNC SmackBuf PTR4* RADEXPLINK SmackBufferOpen(WindowPtr wp, uint32_t BlitType,
                                                     uint32_t width, uint32_t height,
                                                     uint32_t ZoomW, uint32_t ZoomH);
RADEXPFUNC uint32_t RADEXPLINK SmackBufferBlit(SmackBuf PTR4* sbuf, s32 hwndx, s32 hwndy, s32 subx,
                                               s32 suby, s32 subw, s32 subh);
RADEXPFUNC void RADEXPLINK SmackBufferFromScreen(SmackBuf PTR4* destbuf, s32 x, s32 y);

RADEXPFUNC s32 RADEXPLINK SmackIsSoftwareCursor(GDHandle gd);
RADEXPFUNC s32 RADEXPLINK SmackCheckCursor(WindowPtr wp, s32 x, s32 y, s32 w, s32 h);
RADEXPFUNC void RADEXPLINK SmackRestoreCursor(s32 checkcount);
#else
RADEXPFUNC SmackBuf PTR4* RADEXPLINK SmackBufferOpen(HWND wnd, uint32_t BlitType, uint32_t width,
                                                     uint32_t height, uint32_t ZoomW,
                                                     uint32_t ZoomH);
RADEXPFUNC uint32_t RADEXPLINK SmackBufferBlit(SmackBuf PTR4* sbuf, HDC dc, s32 hwndx, s32 hwndy,
                                               s32 subx, s32 suby, s32 subw, s32 subh);
RADEXPFUNC void RADEXPLINK SmackBufferFromScreen(SmackBuf PTR4* destbuf, HWND hw, s32 x, s32 y);

RADEXPFUNC s32 RADEXPLINK SmackIsSoftwareCursor(void* lpDDSP, HCURSOR cur);
RADEXPFUNC s32 RADEXPLINK SmackCheckCursor(HWND wnd, s32 x, s32 y, s32 w, s32 h);
RADEXPFUNC void RADEXPLINK SmackRestoreCursor(s32 checkcount);
#endif

RADEXPFUNC void RADEXPLINK SmackBufferStartMultipleBlits(SmackBuf PTR4* sbuf);
RADEXPFUNC void RADEXPLINK SmackBufferEndMultipleBlits(SmackBuf PTR4* sbuf);

RADEXPFUNC char PTR4* RADEXPLINK SmackBufferString(SmackBuf PTR4* sb, char PTR4* dest);

RADEXPFUNC void RADEXPLINK SmackBufferNewPalette(SmackBuf PTR4* sbuf, const void PTR4* pal,
                                                 uint32_t paltype);
RADEXPFUNC uint32_t RADEXPLINK SmackBufferSetPalette(SmackBuf PTR4* sbuf);
RADEXPFUNC void RADEXPLINK SmackBufferClose(SmackBuf PTR4* sbuf);

RADEXPFUNC void RADEXPLINK SmackBufferClear(SmackBuf PTR4* destbuf, uint32_t color);

RADEXPFUNC void RADEXPLINK SmackBufferToBuffer(SmackBuf PTR4* destbuf, s32 destx, s32 desty,
                                               const SmackBuf PTR4* sourcebuf, s32 sourcex,
                                               s32 sourcey, s32 sourcew, s32 sourceh);
RADEXPFUNC void RADEXPLINK SmackBufferToBufferTrans(SmackBuf PTR4* destbuf, s32 destx, s32 desty,
                                                    const SmackBuf PTR4* sourcebuf, s32 sourcex,
                                                    s32 sourcey, s32 sourcew, s32 sourceh,
                                                    uint32_t TransColor);
RADEXPFUNC void RADEXPLINK SmackBufferToBufferMask(SmackBuf PTR4* destbuf, s32 destx, s32 desty,
                                                   const SmackBuf PTR4* sourcebuf, s32 sourcex,
                                                   s32 sourcey, s32 sourcew, s32 sourceh,
                                                   uint32_t TransColor,
                                                   const SmackBuf PTR4* maskbuf);
RADEXPFUNC void RADEXPLINK SmackBufferToBufferMerge(SmackBuf PTR4* destbuf, s32 destx, s32 desty,
                                                    const SmackBuf PTR4* sourcebuf, s32 sourcex,
                                                    s32 sourcey, s32 sourcew, s32 sourceh,
                                                    uint32_t TransColor,
                                                    const SmackBuf PTR4* mergebuf);
RADEXPFUNC void RADEXPLINK SmackBufferCopyPalette(SmackBuf PTR4* destbuf, SmackBuf PTR4* sourcebuf,
                                                  uint32_t remap);

RADEXPFUNC uint32_t RADEXPLINK SmackBufferFocused(SmackBuf PTR4* sbuf);

#endif

RADDEFEND

#endif

#endif
