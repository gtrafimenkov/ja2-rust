#include <process.h>
#include <stdio.h>
#include <stdlib.h>

#include "Globals.h"
#include "Local.h"
#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/Input.h"
#include "SGP/PaletteEntry.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VObjectInternal.h"
#include "SGP/VSurface.h"
#include "SGP/VSurfaceInternal.h"
#include "SGP/Video.h"
#include "SGP/VideoInternal.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/TimerControl.h"
#include "platform.h"
#include "platform_callbacks.h"
#include "platform_strings.h"
#include "rust_debug.h"

#define INITGUID
#include <ddraw.h>
#include <windows.h>

#include "Smack.h"
#include "platform_win.h"

extern struct VSurface *vsPrimary;
extern struct VSurface *vsBackBuffer;
extern struct VSurface *vsMouseCursor;
extern struct VSurface *vsMouseCursor;
extern struct VSurface *vsMouseCursorOriginal;

bool BltFastSurfaceWithFlags(struct VSurface *dest, u32 x, u32 y, struct VSurface *src,
                             LPRECT pSrcRect, u32 flags);

static bool DDBltFastSurfaceWithFlags(LPDIRECTDRAWSURFACE2 dest, UINT32 uiX, UINT32 uiY,
                                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect, u32 ddFlags);

#define MAX_CURSOR_WIDTH 64
#define MAX_CURSOR_HEIGHT 64
#define VIDEO_NO_CURSOR 0xFFFF

extern UINT32 guiMouseBufferState;  // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface);

static BOOLEAN GetRGBDistribution(void);

// Surface Functions

void DDCreateSurface(LPDIRECTDRAW2 pExistingDirectDraw, DDSURFACEDESC *pNewSurfaceDesc,
                     LPDIRECTDRAWSURFACE *ppNewSurface1, LPDIRECTDRAWSURFACE2 *ppNewSurface2);
void DDGetSurfaceDescription(LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc);
void DDReleaseSurface(LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2);
static struct BufferLockInfo DDLockSurface(LPDIRECTDRAWSURFACE2 pSurface);
void DDRestoreSurface(LPDIRECTDRAWSURFACE2 pSurface);
bool DDBltFastSurface(LPDIRECTDRAWSURFACE2 dest, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 src,
                      LPRECT pSrcRect);
void DDBltSurface(LPDIRECTDRAWSURFACE2 dest, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 src,
                  LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx);
void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, UINT32 uiFlags, LPDDCOLORKEY pDDColorKey);

// Palette Functions
void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, UINT32 uiFlags, LPPALETTEENTRY pColorTable,
                     LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter);
void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiStartingEntry,
                         UINT32 uiCount, LPPALETTEENTRY pEntries);
void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette);
void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiBase,
                         UINT32 uiNumEntries, LPPALETTEENTRY pEntries);

// Clipper functions
void DDCreateClipper(LPDIRECTDRAW2 pDirectDraw, UINT32 fFlags, LPDIRECTDRAWCLIPPER *pDDClipper);
void DDSetClipper(LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper);
void DDReleaseClipper(LPDIRECTDRAWCLIPPER pDDClipper);
void DDSetClipperList(LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, UINT32 uiFlags);

// local functions
void DirectXAssert(BOOLEAN fValue, INT32 nLine, char *szFilename);
void DirectXZeroMem(void *pMemory, int nSize);

#undef ZEROMEM
#define ZEROMEM(x) DirectXZeroMem((void *)&(x), sizeof(x))

#define VIDEO_OFF 0x00
#define VIDEO_ON 0x01
#define VIDEO_SHUTTING_DOWN 0x02
#define VIDEO_SUSPENDED 0x04

#define THREAD_OFF 0x00
#define THREAD_ON 0x01
#define THREAD_SUSPENDED 0x02

#define CURRENT_MOUSE_DATA 0
#define PREVIOUS_MOUSE_DATA 1

typedef struct {
  BOOLEAN fRestore;
  UINT16 usMouseXPos, usMouseYPos;
  UINT16 usLeft, usTop, usRight, usBottom;
  RECT Region;
  LPDIRECTDRAWSURFACE _pSurface;
  LPDIRECTDRAWSURFACE2 pSurface;
} MouseCursorBackground;

//
// Video state variables
//

#define MAX_NUM_FRAMES 25

UINT32 guiFramePeriod = (1000 / 15);
UINT32 guiLastFrame;
UINT16 *gpFrameData[MAX_NUM_FRAMES];
INT32 giNumFrames = 0;

//
// Direct Draw objects for both the Primary and Backbuffer surfaces
//

static LPDIRECTDRAW _gpDirectDrawObject = NULL;
static LPDIRECTDRAW2 gpDirectDrawObject = NULL;

static LPDIRECTDRAWSURFACE _gpPrimarySurface = NULL;
static LPDIRECTDRAWSURFACE2 gpPrimarySurface = NULL;
static LPDIRECTDRAWSURFACE2 gpBackBuffer = NULL;

//
// Direct Draw Objects for the frame buffer
//

static LPDIRECTDRAWSURFACE _gpFrameBuffer = NULL;
static LPDIRECTDRAWSURFACE2 gpFrameBuffer = NULL;

#ifdef WINDOWED_MODE

static LPDIRECTDRAWSURFACE _gpBackBuffer = NULL;

extern RECT rcWindow;

#endif

//
// Globals for mouse cursor
//

static UINT16 gusMouseCursorWidth;
static UINT16 gusMouseCursorHeight;
static INT16 gsMouseCursorXOffset;
static INT16 gsMouseCursorYOffset;

static LPDIRECTDRAWSURFACE _gpMouseCursor = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursor = NULL;

static LPDIRECTDRAWSURFACE _gpMouseCursorOriginal = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursorOriginal = NULL;

static MouseCursorBackground gMouseCursorBackground[2];

static struct VObject *gpCursorStore;

BOOLEAN gfFatalError = FALSE;
char gFatalErrorString[512];

// 8-bit palette stuff

struct SGPPaletteEntry gSgpPalette[256];
LPDIRECTDRAWPALETTE gpDirectDrawPalette;

//
// Refresh thread based variables
//

extern UINT32 guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
extern UINT32 guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
extern UINT32 guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
extern UINT32 guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

//
// Dirty rectangle management variables
//

extern SGPRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
extern UINT32 guiDirtyRegionCount;
extern BOOLEAN gfForceFullScreenRefresh;

extern SGPRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
extern UINT32 gDirtyRegionsFlagsEx[MAX_DIRTY_REGIONS];
extern UINT32 guiDirtyRegionExCount;

//
// Screen output stuff
//

BOOLEAN gfPrintFrameBuffer;
UINT32 guiPrintFrameBufferIndex;

extern UINT16 gusRedMask;
extern UINT16 gusGreenMask;
extern UINT16 gusBlueMask;
extern INT16 gusRedShift;
extern INT16 gusBlueShift;
extern INT16 gusGreenShift;

BOOLEAN InitializeVideoManager(struct PlatformInitParams *params) {
  UINT32 uiIndex;
  HRESULT ReturnCode;
  HWND hWindow;
  WNDCLASS WindowClass;
  CHAR8 ClassName[] = APPLICATION_NAME;
  DDSURFACEDESC SurfaceDescription;
  DDCOLORKEY ColorKey;

#ifndef WINDOWED_MODE
  DDSCAPS SurfaceCaps;
#endif

  //
  // Register debug topics
  //

  DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Initializing the video manager");

  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = (WNDPROC)params->WindowProc;
  WindowClass.cbClsExtra = 0;
  WindowClass.cbWndExtra = 0;
  WindowClass.hInstance = params->hInstance;
  WindowClass.hIcon = LoadIcon(params->hInstance, MAKEINTRESOURCE(params->iconID));
  WindowClass.hCursor = NULL;
  WindowClass.hbrBackground = NULL;
  WindowClass.lpszMenuName = NULL;
  WindowClass.lpszClassName = ClassName;
  RegisterClass(&WindowClass);

  //
  // Get a window handle for our application (gotta have on of those)
  // Don't change this
  //
#ifdef WINDOWED_MODE
  hWindow = CreateWindowEx(0, ClassName, "Windowed JA2 !!", WS_POPUP, 0, 0, SCREEN_WIDTH,
                           SCREEN_HEIGHT, NULL, NULL, params->hInstance, NULL);
#else
  hWindow = CreateWindowEx(WS_EX_TOPMOST, ClassName, ClassName, WS_POPUP | WS_VISIBLE, 0, 0,
                           GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL,
                           params->hInstance, NULL);
#endif
  if (hWindow == NULL) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to create window frame for Direct Draw");
    return FALSE;
  }

  //
  // Excellent. Now we record the hWindow variable for posterity (not)
  //

  memset(gpFrameData, 0, sizeof(gpFrameData));

  ghWindow = hWindow;

  //
  // Display our full screen window
  //

  ShowCursor(FALSE);
  ShowWindow(hWindow, params->usCommandShow);
  UpdateWindow(hWindow);
  SetFocus(hWindow);

  ReturnCode = DirectDrawCreate(NULL, &_gpDirectDrawObject, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  IID tmpID = IID_IDirectDraw2;
  ReturnCode =
      IDirectDraw_QueryInterface(_gpDirectDrawObject, &tmpID, (LPVOID *)&gpDirectDrawObject);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Set the exclusive mode
  //
#ifdef WINDOWED_MODE
  ReturnCode = IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow, DDSCL_NORMAL);
#else
  ReturnCode = IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow,
                                                DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
#endif
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Set the display mode
  //
#ifndef WINDOWED_MODE
  ReturnCode =
      IDirectDraw2_SetDisplayMode(gpDirectDrawObject, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0, 0);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }
#endif

  gusScreenWidth = SCREEN_WIDTH;
  gusScreenHeight = SCREEN_HEIGHT;

  //
  // Initialize Primary Surface along with BackBuffer
  //

  ZEROMEM(SurfaceDescription);
#ifdef WINDOWED_MODE

  // Create a primary surface and a backbuffer in system memory
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpPrimarySurface, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode =
      IDirectDrawSurface_QueryInterface(_gpPrimarySurface, &tmpID, (LPVOID *)&gpPrimarySurface);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  // Backbuffer
  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = SCREEN_WIDTH;
  SurfaceDescription.dwHeight = SCREEN_HEIGHT;
  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpBackBuffer, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpBackBuffer, &tmpID, (LPVOID *)&gpBackBuffer);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

#else
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  SurfaceDescription.dwBackBufferCount = 1;

  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpPrimarySurface, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode =
      IDirectDrawSurface_QueryInterface(_gpPrimarySurface, &tmpID, (LPVOID *)&gpPrimarySurface);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;
  ReturnCode =
      IDirectDrawSurface2_GetAttachedSurface(gpPrimarySurface, &SurfaceCaps, &gpBackBuffer);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

#endif

  //
  // Initialize the frame buffer
  //

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = SCREEN_WIDTH;
  SurfaceDescription.dwHeight = SCREEN_HEIGHT;
  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpFrameBuffer, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpFrameBuffer, &tmpID, (LPVOID *)&gpFrameBuffer);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Blank out the frame buffer
  //
  {
    struct BufferLockInfo lock = VSurfaceLock(vsFrameBuffer);
    memset(lock.dest, 0, 480 * lock.pitch);
    VSurfaceUnlock(vsFrameBuffer);
  }

  //
  // Initialize the main mouse surfaces
  //

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  // SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
  SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpMouseCursor, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR,
             String("Failed to create MouseCursor witd %ld", ReturnCode & 0x0f));
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpMouseCursor, &tmpID, (LPVOID *)&gpMouseCursor);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  ColorKey.dwColorSpaceLowValue = 0;
  ColorKey.dwColorSpaceHighValue = 0;
  ReturnCode = IDirectDrawSurface2_SetColorKey(gpMouseCursor, DDCKEY_SRCBLT, &ColorKey);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Initialize the main mouse original surface
  //

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
  SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
  ReturnCode = IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription,
                                          &_gpMouseCursorOriginal, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to create MouseCursorOriginal");
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpMouseCursorOriginal, &tmpID,
                                                 (LPVOID *)&gpMouseCursorOriginal);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Initialize the main mouse background surfaces. There are two of them (one for each of the
  // Primary and Backbuffer surfaces
  //

  for (uiIndex = 0; uiIndex < 1; uiIndex++) {
    //
    // Initialize various mouse background variables
    //

    gMouseCursorBackground[uiIndex].fRestore = FALSE;

    //
    // Initialize the direct draw surfaces for the mouse background
    //

    ZEROMEM(SurfaceDescription);
    SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    // SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
    SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
    ReturnCode = IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription,
                                            &(gMouseCursorBackground[uiIndex]._pSurface), NULL);
    if (ReturnCode != DD_OK) {
      DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to create MouseCursorBackground");
      return FALSE;
    }

    IID tmpID = IID_IDirectDrawSurface2;
    ReturnCode = IDirectDrawSurface_QueryInterface(
        (IDirectDrawSurface *)gMouseCursorBackground[uiIndex]._pSurface, &tmpID,
        (LPVOID *)&(gMouseCursorBackground[uiIndex].pSurface));
    if (ReturnCode != DD_OK) {
      return FALSE;
    }
  }

  //
  // Initialize state variables
  //

  guiFrameBufferState = BUFFER_DIRTY;
  guiMouseBufferState = BUFFER_DISABLED;
  guiVideoManagerState = VIDEO_ON;
  guiRefreshThreadState = THREAD_OFF;
  guiDirtyRegionCount = 0;
  gfForceFullScreenRefresh = TRUE;
  gpCursorStore = NULL;
  gfPrintFrameBuffer = FALSE;
  guiPrintFrameBufferIndex = 0;

  //
  // This function must be called to setup RGB information
  //

  GetRGBDistribution();

  // create video surfaces from DD surfaces
  vsPrimary = CreateVideoSurfaceFromDDSurface(gpPrimarySurface);
  vsBackBuffer = CreateVideoSurfaceFromDDSurface(gpBackBuffer);
  vsMouseCursor = CreateVideoSurfaceFromDDSurface(gpMouseCursor);
  vsMouseCursorOriginal = CreateVideoSurfaceFromDDSurface(gpMouseCursorOriginal);
  vsFrameBuffer = CreateVideoSurfaceFromDDSurface(gpFrameBuffer);
  if (!vsPrimary || !vsBackBuffer || !vsMouseCursor || !vsMouseCursorOriginal || !vsFrameBuffer) {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_ERROR, String("Could not create primary surfaces"));
    return FALSE;
  }

  return TRUE;
}

void ShutdownVideoManager(void) {
  DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Shutting down the video manager");

  DeleteVideoSurface(vsPrimary);
  DeleteVideoSurface(vsBackBuffer);
  DeleteVideoSurface(vsFrameBuffer);
  DeleteVideoSurface(vsMouseCursor);
  DeleteVideoSurface(vsMouseCursorOriginal);

  //
  // Toggle the state of the video manager to indicate to the refresh thread that it needs to shut
  // itself down
  //

  IDirectDrawSurface2_Release(gpMouseCursorOriginal);
  IDirectDrawSurface2_Release(gpMouseCursor);
  IDirectDrawSurface2_Release(gMouseCursorBackground[0].pSurface);
  IDirectDrawSurface2_Release(gpBackBuffer);
  IDirectDrawSurface2_Release(gpPrimarySurface);

  IDirectDraw2_RestoreDisplayMode(gpDirectDrawObject);
  IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow, DDSCL_NORMAL);
  IDirectDraw2_Release(gpDirectDrawObject);

  // destroy the window
  // DestroyWindow( ghWindow );

  guiVideoManagerState = VIDEO_OFF;

  if (gpCursorStore != NULL) {
    DeleteVideoObject(gpCursorStore);
    gpCursorStore = NULL;
  }

  // ATE: Release mouse cursor!
  FreeMouseCursor();
}

void SuspendVideoManager(void) { guiVideoManagerState = VIDEO_SUSPENDED; }

void DoTester() {
  IDirectDraw2_RestoreDisplayMode(gpDirectDrawObject);
  IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow, DDSCL_NORMAL);
  ShowCursor(TRUE);
}

BOOLEAN RestoreVideoManager(void) {
  HRESULT ReturnCode;

  //
  // Make sure the video manager is indeed suspended before moving on
  //

  if (guiVideoManagerState == VIDEO_SUSPENDED) {
    //
    // Restore the Primary and Backbuffer
    //

    ReturnCode = IDirectDrawSurface2_Restore(gpPrimarySurface);
    if (ReturnCode != DD_OK) {
      return FALSE;
    }

    ReturnCode = IDirectDrawSurface2_Restore(gpBackBuffer);
    if (ReturnCode != DD_OK) {
      return FALSE;
    }

    //
    // Restore the mouse surfaces and make sure to initialize the gpMouseCursor surface
    //

    ReturnCode = IDirectDrawSurface2_Restore(gMouseCursorBackground[0].pSurface);
    if (ReturnCode != DD_OK) {
      return FALSE;
    }

    ReturnCode = IDirectDrawSurface2_Restore(gpMouseCursor);
    if (ReturnCode != DD_OK) {
      return FALSE;
    } else {
      guiMouseBufferState = BUFFER_DIRTY;
    }

    //
    // Set the video state to VIDEO_ON
    //

    guiFrameBufferState = BUFFER_DIRTY;
    guiMouseBufferState = BUFFER_DIRTY;
    gfForceFullScreenRefresh = TRUE;
    guiVideoManagerState = VIDEO_ON;
    return TRUE;
  } else {
    return FALSE;
  }
}

void ScrollJA2Background(UINT32 uiDirection, INT16 sScrollXIncrement, INT16 sScrollYIncrement,
                         LPDIRECTDRAWSURFACE2 pSource, LPDIRECTDRAWSURFACE2 pDest,
                         BOOLEAN fRenderStrip, UINT32 uiCurrentMouseBackbuffer) {
  UINT16 usWidth, usHeight;
  UINT8 ubBitDepth;
  static RECT Region;
  static UINT16 usMouseXPos, usMouseYPos;
  static RECT StripRegions[2], MouseRegion;
  UINT16 usNumStrips = 0;
  INT32 cnt;
  INT16 sShiftX, sShiftY;
  INT32 uiCountY;

  GetCurrentVideoSettings(&usWidth, &usHeight, &ubBitDepth);
  usHeight = (gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);

  StripRegions[0].left = gsVIEWPORT_START_X;
  StripRegions[0].right = gsVIEWPORT_END_X;
  StripRegions[0].top = gsVIEWPORT_WINDOW_START_Y;
  StripRegions[0].bottom = gsVIEWPORT_WINDOW_END_Y;
  StripRegions[1].left = gsVIEWPORT_START_X;
  StripRegions[1].right = gsVIEWPORT_END_X;
  StripRegions[1].top = gsVIEWPORT_WINDOW_START_Y;
  StripRegions[1].bottom = gsVIEWPORT_WINDOW_END_Y;

  MouseRegion.left = gMouseCursorBackground[uiCurrentMouseBackbuffer].usLeft;
  MouseRegion.top = gMouseCursorBackground[uiCurrentMouseBackbuffer].usTop;
  MouseRegion.right = gMouseCursorBackground[uiCurrentMouseBackbuffer].usRight;
  MouseRegion.bottom = gMouseCursorBackground[uiCurrentMouseBackbuffer].usBottom;

  usMouseXPos = gMouseCursorBackground[uiCurrentMouseBackbuffer].usMouseXPos;
  usMouseYPos = gMouseCursorBackground[uiCurrentMouseBackbuffer].usMouseYPos;

  switch (uiDirection) {
    case SCROLL_LEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;
      DDBltFastSurface(pDest, sScrollXIncrement, gsVIEWPORT_WINDOW_START_Y, pSource,
                       (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }

      StripRegions[0].right = (INT16)(gsVIEWPORT_START_X + sScrollXIncrement);
      usMouseXPos += sScrollXIncrement;

      usNumStrips = 1;
      break;

    case SCROLL_RIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;
      DDBltFastSurface(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource, (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (UINT8 *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }

      StripRegions[0].left = (INT16)(gsVIEWPORT_END_X - sScrollXIncrement);
      usMouseXPos -= sScrollXIncrement;

      usNumStrips = 1;
      break;

    case SCROLL_UP:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;
      DDBltFastSurface(pDest, 0, gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement, pSource,
                       (LPRECT)&Region);

      for (uiCountY = sScrollYIncrement - 1 + gsVIEWPORT_WINDOW_START_Y;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].bottom = (INT16)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      usNumStrips = 1;

      usMouseYPos += sScrollYIncrement;

      break;

    case SCROLL_DOWN:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;
      DDBltFastSurface(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource, (LPRECT)&Region);

      // Zero out z
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].top = (INT16)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      usNumStrips = 1;

      usMouseYPos -= sScrollYIncrement;

      break;

    case SCROLL_UPLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;
      DDBltFastSurface(pDest, sScrollXIncrement, gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement,
                       pSource, (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (INT16)(gsVIEWPORT_START_X + sScrollXIncrement);
      StripRegions[1].bottom = (INT16)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].left = (INT16)(gsVIEWPORT_START_X + sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos += sScrollYIncrement;
      usMouseXPos += sScrollXIncrement;

      break;

    case SCROLL_UPRIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;
      DDBltFastSurface(pDest, 0, gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement, pSource,
                       (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (UINT8 *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (INT16)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].bottom = (INT16)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].right = (INT16)(gsVIEWPORT_END_X - sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos += sScrollYIncrement;
      usMouseXPos -= sScrollXIncrement;

      break;

    case SCROLL_DOWNLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;
      DDBltFastSurface(pDest, sScrollXIncrement, gsVIEWPORT_WINDOW_START_Y, pSource,
                       (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (INT16)(gsVIEWPORT_START_X + sScrollXIncrement);

      StripRegions[1].top = (INT16)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].left = (INT16)(gsVIEWPORT_START_X + sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos -= sScrollYIncrement;
      usMouseXPos += sScrollXIncrement;

      break;

    case SCROLL_DOWNRIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;
      DDBltFastSurface(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource, (LPRECT)&Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (UINT8 *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((UINT8 *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (INT16)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].top = (INT16)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].right = (INT16)(gsVIEWPORT_END_X - sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos -= sScrollYIncrement;
      usMouseXPos -= sScrollXIncrement;

      break;
  }

  if (fRenderStrip) {
    // Memset to 0

    for (cnt = 0; cnt < usNumStrips; cnt++) {
      RenderStaticWorldRect((INT16)StripRegions[cnt].left, (INT16)StripRegions[cnt].top,
                            (INT16)StripRegions[cnt].right, (INT16)StripRegions[cnt].bottom, TRUE);
      DDBltFastSurface(pDest, StripRegions[cnt].left, StripRegions[cnt].top, gpFrameBuffer,
                       (LPRECT) & (StripRegions[cnt]));
    }

    sShiftX = 0;
    sShiftY = 0;

    switch (uiDirection) {
      case SCROLL_LEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = 0;
        break;

      case SCROLL_RIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = 0;
        break;

      case SCROLL_UP:

        sShiftX = 0;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_DOWN:

        sShiftX = 0;
        sShiftY = -sScrollYIncrement;
        break;

      case SCROLL_UPLEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_UPRIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_DOWNLEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = -sScrollYIncrement;
        break;

      case SCROLL_DOWNRIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = -sScrollYIncrement;
        break;
    }

    // RESTORE SHIFTED
    RestoreShiftedVideoOverlays(sShiftX, sShiftY);

    // SAVE NEW
    SaveVideoOverlaysArea(BACKBUFFER);

    // BLIT NEW
    ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);
  }
}

void printFramebuffer() {
  LPDIRECTDRAWSURFACE _pTmpBuffer;
  LPDIRECTDRAWSURFACE2 pTmpBuffer;
  DDSURFACEDESC SurfaceDescription;
  FILE *OutputFile;
  CHAR8 FileName[64];
  struct Str512 ExecDir;
  UINT16 *p16BPPData;

  if (!Plat_GetExecutableDirectory(&ExecDir)) {
    return;
  }
  Plat_SetCurrentDirectory(ExecDir.buf);

  //
  // Create temporary system memory surface. This is used to correct problems with the
  // backbuffer surface which can be interlaced or have a funky pitch
  //

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = gusScreenWidth;
  SurfaceDescription.dwHeight = gusScreenHeight;
  HRESULT ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_pTmpBuffer, NULL);

  IID tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface((IDirectDrawSurface *)_pTmpBuffer, &tmpID,
                                                 (LPVOID *)&pTmpBuffer);

  //
  // Copy the primary surface to the temporary surface
  //

  RECT Region;
  Region.left = 0;
  Region.top = 0;
  Region.right = gusScreenWidth;
  Region.bottom = gusScreenHeight;
  DDBltFastSurface(pTmpBuffer, 0, 0, gpPrimarySurface, &Region);

  //
  // Ok now that temp surface has contents of backbuffer, copy temp surface to disk
  //

  sprintf(FileName, "SCREEN%03d.TGA", guiPrintFrameBufferIndex++);
  if ((OutputFile = fopen(FileName, "wb")) != NULL) {
    fprintf(OutputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0x80, 0x02, 0xe0, 0x01, 0x10, 0);

    //
    // Lock temp surface
    //

    ZEROMEM(SurfaceDescription);
    SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
    ReturnCode = IDirectDrawSurface2_Lock(pTmpBuffer, NULL, &SurfaceDescription, 0, NULL);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
    }

    //
    // Copy 16 bit buffer to file
    //

    // 5/6/5.. create buffer...
    if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
      p16BPPData = (UINT16 *)MemAlloc(640 * 2);
    }

    for (INT32 iIndex = 479; iIndex >= 0; iIndex--) {
      // ATE: OK, fix this such that it converts pixel format to 5/5/5
      // if current settings are 5/6/5....
      if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
        // Read into a buffer...
        memcpy(p16BPPData, (((UINT8 *)SurfaceDescription.lpSurface) + (iIndex * 640 * 2)), 640 * 2);

        // Convert....
        ConvertRGBDistribution565To555(p16BPPData, 640);

        // Write
        fwrite(p16BPPData, 640 * 2, 1, OutputFile);
      } else {
        fwrite((void *)(((UINT8 *)SurfaceDescription.lpSurface) + (iIndex * 640 * 2)), 640 * 2, 1,
               OutputFile);
      }
    }

    // 5/6/5.. Delete buffer...
    if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
      MemFree(p16BPPData);
    }

    fclose(OutputFile);

    //
    // Unlock temp surface
    //

    ZEROMEM(SurfaceDescription);
    SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
    ReturnCode = IDirectDrawSurface2_Unlock(pTmpBuffer, &SurfaceDescription);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
    }
  }

  //
  // Release temp surface
  //

  gfPrintFrameBuffer = FALSE;
  IDirectDrawSurface2_Release(pTmpBuffer);

  strcat(ExecDir.buf, "\\Data");
  Plat_SetCurrentDirectory(ExecDir.buf);
}

void RefreshScreen() {
  static UINT32 uiRefreshThreadState, uiIndex;
  UINT16 usScreenWidth, usScreenHeight;
  static BOOLEAN fShowMouse;
  HRESULT ReturnCode;
  static RECT Region;
  static BOOLEAN fFirstTime = TRUE;

  usScreenWidth = usScreenHeight = 0;

  if (fFirstTime) {
    fShowMouse = FALSE;
  }

  switch (guiVideoManagerState) {
    case VIDEO_ON:
      // Excellent, everything is cosher, we continue on
      uiRefreshThreadState = guiRefreshThreadState = THREAD_ON;
      usScreenWidth = gusScreenWidth;
      usScreenHeight = gusScreenHeight;
      break;
    case VIDEO_OFF:
      // Hot damn, the video manager is suddenly off. We have to bugger out of here. Don't forget to
      // leave the critical section
      guiRefreshThreadState = THREAD_OFF;
      return;
    case VIDEO_SUSPENDED:  //
      // This are suspended. Make sure the refresh function does try to access any of the direct
      // draw surfaces
      uiRefreshThreadState = guiRefreshThreadState = THREAD_SUSPENDED;
      break;
    case VIDEO_SHUTTING_DOWN:
      // Well things are shutting down. So we need to bugger out of there.
      // Don't forget to leave the critical section before returning
      guiRefreshThreadState = THREAD_OFF;
      return;
  }

  //
  // Get the current mouse position
  //

  struct Point MousePos = GetMousePoint();

  // RESTORE OLD POSITION OF MOUSE
  if (gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore == TRUE) {
    Region.left = gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft;
    Region.top = gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop;
    Region.right = gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight;
    Region.bottom = gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom;
    if (!DDBltFastSurface(gpBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
                          gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos,
                          gMouseCursorBackground[CURRENT_MOUSE_DATA].pSurface, (LPRECT)&Region)) {
      goto ENDOFLOOP;
    }

    // Save position into other background region
    memcpy(&(gMouseCursorBackground[PREVIOUS_MOUSE_DATA]),
           &(gMouseCursorBackground[CURRENT_MOUSE_DATA]), sizeof(MouseCursorBackground));
  }

  // Ok we were able to get a hold of the frame buffer stuff. Check to see if it needs updating
  // if not, release the frame buffer stuff right away
  if (guiFrameBufferState == BUFFER_DIRTY) {
    if (PlatformCallback_IsInFade()) {
      PlatformCallback_Fade();
    } else {
      if (gfForceFullScreenRefresh == TRUE) {
        // Method (1) - We will be refreshing the entire screen

        Region.left = 0;
        Region.top = 0;
        Region.right = usScreenWidth;
        Region.bottom = usScreenHeight;
        if (!DDBltFastSurface(gpBackBuffer, 0, 0, gpFrameBuffer, (LPRECT)&Region)) {
          goto ENDOFLOOP;
        }
      } else {
        for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
          Region.left = gListOfDirtyRegions[uiIndex].iLeft;
          Region.top = gListOfDirtyRegions[uiIndex].iTop;
          Region.right = gListOfDirtyRegions[uiIndex].iRight;
          Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;
          if (!DDBltFastSurface(gpBackBuffer, Region.left, Region.top, gpFrameBuffer,
                                (LPRECT)&Region)) {
            goto ENDOFLOOP;
          }
        }

        // Now do new, extended dirty regions
        for (uiIndex = 0; uiIndex < guiDirtyRegionExCount; uiIndex++) {
          Region.left = gDirtyRegionsEx[uiIndex].iLeft;
          Region.top = gDirtyRegionsEx[uiIndex].iTop;
          Region.right = gDirtyRegionsEx[uiIndex].iRight;
          Region.bottom = gDirtyRegionsEx[uiIndex].iBottom;

          // Do some checks if we are in the process of scrolling!
          if (gfRenderScroll) {
            // Check if we are completely out of bounds
            if (Region.top <= gsVIEWPORT_WINDOW_END_Y && Region.bottom <= gsVIEWPORT_WINDOW_END_Y) {
              continue;
            }
          }

          if (!DDBltFastSurface(gpBackBuffer, Region.left, Region.top, gpFrameBuffer,
                                (LPRECT)&Region)) {
            goto ENDOFLOOP;
          }
        }
      }
    }
    if (gfRenderScroll) {
      ScrollJA2Background(guiScrollDirection, gsScrollXIncrement, gsScrollYIncrement,
                          gpPrimarySurface, gpBackBuffer, TRUE, PREVIOUS_MOUSE_DATA);
    }
    gfIgnoreScrollDueToCenterAdjust = FALSE;

    // Update the guiFrameBufferState variable to reflect that the frame buffer can now be handled
    guiFrameBufferState = BUFFER_READY;
  }

  if (gfPrintFrameBuffer == TRUE) {
    printFramebuffer();
  }

  // Ok we were able to get a hold of the frame buffer stuff. Check to see if it needs updating
  // if not, release the frame buffer stuff right away
  if (guiMouseBufferState == BUFFER_DIRTY) {
    // Well the mouse buffer is dirty. Upload the whole thing
    Region.left = 0;
    Region.top = 0;
    Region.right = gusMouseCursorWidth;
    Region.bottom = gusMouseCursorHeight;
    DDBltFastSurface(gpMouseCursor, 0, 0, gpMouseCursorOriginal, (LPRECT)&Region);
    guiMouseBufferState = BUFFER_READY;
  }

  //
  // Check current state of the mouse cursor
  //

  if (fShowMouse == FALSE) {
    if (guiMouseBufferState == BUFFER_READY) {
      fShowMouse = TRUE;
    } else {
      fShowMouse = FALSE;
    }
  } else {
    if (guiMouseBufferState == BUFFER_DISABLED) {
      fShowMouse = FALSE;
    }
  }

  if (fShowMouse == TRUE) {
    //
    // Step (1) - Save mouse background
    //

    Region.left = MousePos.x - gsMouseCursorXOffset;
    Region.top = MousePos.y - gsMouseCursorYOffset;
    Region.right = Region.left + gusMouseCursorWidth;
    Region.bottom = Region.top + gusMouseCursorHeight;

    if (Region.right > usScreenWidth) {
      Region.right = usScreenWidth;
    }

    if (Region.bottom > usScreenHeight) {
      Region.bottom = usScreenHeight;
    }

    if ((Region.right > Region.left) && (Region.bottom > Region.top)) {
      //
      // Make sure the mouse background is marked for restore and coordinates are saved for the
      // future restore
      //

      gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = TRUE;
      gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight =
          (UINT16)Region.right - (UINT16)Region.left;
      gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom =
          (UINT16)Region.bottom - (UINT16)Region.top;
      if (Region.left < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = (UINT16)(0 - Region.left);
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos = 0;
        Region.left = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos =
            (UINT16)MousePos.x - gsMouseCursorXOffset;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = 0;
      }
      if (Region.top < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos = 0;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop = (UINT16)(0 - Region.top);
        Region.top = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos =
            (UINT16)MousePos.y - gsMouseCursorYOffset;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop = 0;
      }

      if ((Region.right > Region.left) && (Region.bottom > Region.top)) {
        // Save clipped region
        gMouseCursorBackground[CURRENT_MOUSE_DATA].Region = Region;

        //
        // Ok, do the actual data save to the mouse background
        //

        if (!DDBltFastSurface(gMouseCursorBackground[CURRENT_MOUSE_DATA].pSurface,
                              gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft,
                              gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop, gpBackBuffer,
                              &Region)) {
          goto ENDOFLOOP;
        }

        //
        // Step (2) - Blit mouse cursor to back buffer
        //

        Region.left = gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft;
        Region.top = gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop;
        Region.right = gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight;
        Region.bottom = gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom;

        if (!DDBltFastSurfaceWithFlags(gpBackBuffer,
                                       gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
                                       gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos,
                                       gpMouseCursor, &Region, DDBLTFAST_SRCCOLORKEY)) {
          goto ENDOFLOOP;
        }
      } else {
        // Hum, the mouse was not blitted this round. Henceforth we will flag fRestore as FALSE
        gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
      }

    } else {
      // Hum, the mouse was not blitted this round. Henceforth we will flag fRestore as FALSE
      gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
    }
  } else {
    // Well since there was no mouse handling this round, we disable the mouse restore
    gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
  }

  //
  // Step (1) - Flip pages
  //
#ifdef WINDOWED_MODE

  do {
    ReturnCode =
        IDirectDrawSurface_Blt(gpPrimarySurface, &rcWindow, gpBackBuffer, NULL, DDBLT_WAIT, NULL);

    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      if (ReturnCode == DDERR_SURFACELOST) {
        goto ENDOFLOOP;
      }
    }

  } while (ReturnCode != DD_OK);

#else

  do {
    ReturnCode = IDirectDrawSurface_Flip(_gpPrimarySurface, NULL, DDFLIP_WAIT);
    //    if ((ReturnCode != DD_OK)&&(ReturnCode != DDERR_WASSTILLDRAWING))
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      if (ReturnCode == DDERR_SURFACELOST) {
        goto ENDOFLOOP;
      }
    }

  } while (ReturnCode != DD_OK);

#endif

  //
  // Step (2) - Copy Primary Surface to the Back Buffer
  //
  if (gfRenderScroll) {
    Region.left = 0;
    Region.top = 0;
    Region.right = 640;
    Region.bottom = 360;

    if (!DDBltFastSurface(gpBackBuffer, 0, 0, gpPrimarySurface, &Region)) {
      goto ENDOFLOOP;
    }

    // Get new background for mouse
    // Ok, do the actual data save to the mouse background
    gfRenderScroll = FALSE;
    gfScrollStart = FALSE;
  }

  // COPY MOUSE AREAS FROM PRIMARY BACK!

  // FIRST OLD ERASED POSITION
  if (gMouseCursorBackground[PREVIOUS_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[PREVIOUS_MOUSE_DATA].Region;

    if (!DDBltFastSurface(gpBackBuffer, gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseXPos,
                          gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseYPos, gpPrimarySurface,
                          (LPRECT)&Region)) {
      goto ENDOFLOOP;
    }
  }

  // NOW NEW MOUSE AREA
  if (gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[CURRENT_MOUSE_DATA].Region;
    if (!DDBltFastSurface(gpBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
                          gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, gpPrimarySurface,
                          (LPRECT)&Region)) {
      goto ENDOFLOOP;
    }
  }

  if (gfForceFullScreenRefresh == TRUE) {
    //
    // Method (1) - We will be refreshing the entire screen
    //
    Region.left = 0;
    Region.top = 0;
    Region.right = SCREEN_WIDTH;
    Region.bottom = SCREEN_HEIGHT;
    if (!DDBltFastSurface(gpBackBuffer, 0, 0, gpPrimarySurface, &Region)) {
      goto ENDOFLOOP;
    }

    guiDirtyRegionCount = 0;
    guiDirtyRegionExCount = 0;
    gfForceFullScreenRefresh = FALSE;
  } else {
    for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
      Region.left = gListOfDirtyRegions[uiIndex].iLeft;
      Region.top = gListOfDirtyRegions[uiIndex].iTop;
      Region.right = gListOfDirtyRegions[uiIndex].iRight;
      Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;
      if (!DDBltFastSurface(gpBackBuffer, Region.left, Region.top, gpPrimarySurface,
                            (LPRECT)&Region)) {
        goto ENDOFLOOP;
      }
    }

    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = FALSE;
  }

  // Do extended dirty regions!
  for (uiIndex = 0; uiIndex < guiDirtyRegionExCount; uiIndex++) {
    Region.left = gDirtyRegionsEx[uiIndex].iLeft;
    Region.top = gDirtyRegionsEx[uiIndex].iTop;
    Region.right = gDirtyRegionsEx[uiIndex].iRight;
    Region.bottom = gDirtyRegionsEx[uiIndex].iBottom;

    if ((Region.top < gsVIEWPORT_WINDOW_END_Y) && gfRenderScroll) {
      continue;
    }
    if (!DDBltFastSurface(gpBackBuffer, Region.left, Region.top, gpPrimarySurface,
                          (LPRECT)&Region)) {
      goto ENDOFLOOP;
    }
  }

  guiDirtyRegionExCount = 0;

ENDOFLOOP:

  fFirstTime = FALSE;
}

static LPDIRECTDRAW2 GetDirectDraw2Object(void) {
  Assert(gpDirectDrawObject != NULL);

  return gpDirectDrawObject;
}

static BOOLEAN GetRGBDistribution(void) {
  DDSURFACEDESC SurfaceDescription;
  UINT16 usBit;
  HRESULT ReturnCode;

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_PIXELFORMAT;
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(gpPrimarySurface, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Ok we now have the surface description, we now can get the information that we need
  //

  gusRedMask = (UINT16)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  gusGreenMask = (UINT16)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  gusBlueMask = (UINT16)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  // RGB 5,5,5
  if ((gusRedMask == 0x7c00) && (gusGreenMask == 0x03e0) && (gusBlueMask == 0x1f))
    guiTranslucentMask = 0x3def;
  // RGB 5,6,5
  else  // if((gusRedMask==0xf800) && (gusGreenMask==0x03e0) && (gusBlueMask==0x1f))
    guiTranslucentMask = 0x7bef;

  usBit = 0x8000;
  gusRedShift = 8;
  while (!(gusRedMask & usBit)) {
    usBit >>= 1;
    gusRedShift--;
  }

  usBit = 0x8000;
  gusGreenShift = 8;
  while (!(gusGreenMask & usBit)) {
    usBit >>= 1;
    gusGreenShift--;
  }

  usBit = 0x8000;
  gusBlueShift = 8;
  while (!(gusBlueMask & usBit)) {
    usBit >>= 1;
    gusBlueShift--;
  }

  return TRUE;
}

BOOLEAN EraseMouseCursor() {
  struct BufferLockInfo lock = VSurfaceLock(vsMouseCursorOriginal);
  memset(lock.dest, 0, MAX_CURSOR_HEIGHT * lock.pitch);
  VSurfaceUnlock(vsMouseCursorOriginal);
  return (TRUE);
}

BOOLEAN SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight,
                                 UINT16 usCursorWidth) {
  gsMouseCursorXOffset = sOffsetX;
  gsMouseCursorYOffset = sOffsetY;
  gusMouseCursorWidth = usCursorWidth;
  gusMouseCursorHeight = usCursorHeight;
  return (TRUE);
}

void DirtyCursor() { guiMouseBufferState = BUFFER_DIRTY; }

BOOLEAN SetCurrentCursor(UINT16 usVideoObjectSubIndex, UINT16 usOffsetX, UINT16 usOffsetY) {
  BOOLEAN ReturnValue;
  ETRLEObject pETRLEPointer;

  //
  // Make sure we have a cursor store
  //

  if (gpCursorStore == NULL) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "ERROR : Cursor store is not loaded");
    return FALSE;
  }

  EraseMouseCursor();

  //
  // Get new cursor data
  //

  ReturnValue = BltVideoObject(MOUSE_BUFFER, gpCursorStore, usVideoObjectSubIndex, 0, 0,
                               VO_BLT_SRCTRANSPARENCY, NULL);
  guiMouseBufferState = BUFFER_DIRTY;

  if (GetVideoObjectETRLEProperties(gpCursorStore, &pETRLEPointer, usVideoObjectSubIndex)) {
    gsMouseCursorXOffset = usOffsetX;
    gsMouseCursorYOffset = usOffsetY;
    gusMouseCursorWidth = pETRLEPointer.usWidth + pETRLEPointer.sOffsetX;
    gusMouseCursorHeight = pETRLEPointer.usHeight + pETRLEPointer.sOffsetY;

    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "=================================================");
    DebugMsg(TOPIC_VIDEO, DBG_ERROR,
             String("Mouse Create with [ %d. %d ] [ %d, %d]", pETRLEPointer.sOffsetX,
                    pETRLEPointer.sOffsetY, pETRLEPointer.usWidth, pETRLEPointer.usHeight));
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "=================================================");
  } else {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to get mouse info");
  }

  return ReturnValue;
}

void StartFrameBufferRender(void) { return; }

void EndFrameBufferRender(void) {
  guiFrameBufferState = BUFFER_DIRTY;

  return;
}

void PrintScreen(void) { gfPrintFrameBuffer = TRUE; }

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) {
  HRESULT ReturnCode;

  // If we are in 256 colors, then we have to initialize the palette system to 0 (faded out)
  memcpy(gSgpPalette, pPalette, sizeof(struct SGPPaletteEntry) * 256);

  ReturnCode =
      IDirectDraw_CreatePalette(gpDirectDrawObject, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                                (LPPALETTEENTRY)(&gSgpPalette[0]), &gpDirectDrawPalette, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, String("Failed to create palette (Rc = %d)", ReturnCode));
    return (FALSE);
  }
  // Apply the palette to the surfaces
  ReturnCode = IDirectDrawSurface_SetPalette(gpPrimarySurface, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, String("Failed to apply 8-bit palette to primary surface"));
    return (FALSE);
  }

  ReturnCode = IDirectDrawSurface_SetPalette(gpBackBuffer, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, String("Failed to apply 8-bit palette to back buffer"));
    return (FALSE);
  }

  ReturnCode = IDirectDrawSurface_SetPalette(gpFrameBuffer, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_ERROR, String("Failed to apply 8-bit palette to frame buffer"));
    return (FALSE);
  }

  return (TRUE);
}

void FatalError(STR8 pError, ...) {
  va_list argptr;

  va_start(argptr, pError);  // Set up variable argument pointer
  vsprintf(gFatalErrorString, pError, argptr);
  va_end(argptr);

  gfFatalError = TRUE;

  // Release DDraw
  IDirectDraw2_RestoreDisplayMode(gpDirectDrawObject);
  IDirectDraw2_Release(gpDirectDrawObject);
  ShowWindow(ghWindow, SW_HIDE);

  // destroy the window
  // DestroyWindow( ghWindow );

  gfProgramIsRunning = FALSE;

  MessageBox(ghWindow, gFatalErrorString, "JA2 Fatal Error", MB_OK | MB_TASKMODAL);
}

#pragma pack(push, 1)

typedef struct {
  UINT8 ubIDLength;
  UINT8 ubColorMapType;
  UINT8 ubTargaType;
  UINT16 usColorMapOrigin;
  UINT16 usColorMapLength;
  UINT8 ubColorMapEntrySize;
  UINT16 usOriginX;
  UINT16 usOriginY;
  UINT16 usImageWidth;
  UINT16 usImageHeight;
  UINT8 ubBitsPerPixel;
  UINT8 ubImageDescriptor;

} TARGA_HEADER;

#pragma pack(pop)

//////////////////////////////////////////////////////////////////
// VSurface
//////////////////////////////////////////////////////////////////

LPDIRECTDRAW2 GetDirectDraw2Object();
LPDIRECTDRAWSURFACE2 GetPrimarySurfaceInterface();
LPDIRECTDRAWSURFACE2 GetBackbufferInterface();

BOOLEAN SetDirectDraw2Object(LPDIRECTDRAW2 pDirectDraw);
BOOLEAN SetPrimarySurfaceInterface(LPDIRECTDRAWSURFACE2 pSurface);
BOOLEAN SetBackbufferInterface(LPDIRECTDRAWSURFACE2 pSurface);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manipulation Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc) {
  LPDIRECTDRAW2 lpDD2Object;
  DDPIXELFORMAT PixelFormat;
  LPDIRECTDRAWSURFACE lpDDS;
  LPDIRECTDRAWSURFACE2 lpDDS2;
  struct VSurface *hVSurface;
  struct Image *hImage;
  SGPRect tempRect;
  UINT16 usHeight;
  UINT16 usWidth;
  UINT8 ubBitDepth;
  UINT32 fMemUsage;

  UINT32 uiRBitMask;
  UINT32 uiGBitMask;
  UINT32 uiBBitMask;

  lpDD2Object = GetDirectDraw2Object();
  fMemUsage = VSurfaceDesc->fCreateFlags;

  if (VSurfaceDesc->fCreateFlags & VSURFACE_CREATE_FROMFILE) {
    hImage = CreateImage(VSurfaceDesc->ImageFile, IMAGE_ALLIMAGEDATA);
    if (hImage == NULL) {
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Invalid Image Filename given");
      return (NULL);
    }
    usHeight = hImage->usHeight;
    usWidth = hImage->usWidth;
    ubBitDepth = hImage->ubBitDepth;
  } else {
    usHeight = VSurfaceDesc->usHeight;
    usWidth = VSurfaceDesc->usWidth;
    ubBitDepth = VSurfaceDesc->ubBitDepth;
  }

  Assert(usHeight > 0);
  Assert(usWidth > 0);

  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

  switch (ubBitDepth) {
    case 8:
      PixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
      PixelFormat.dwRGBBitCount = 8;
      break;

    case 16:
      PixelFormat.dwFlags = DDPF_RGB;
      PixelFormat.dwRGBBitCount = 16;
      if (!(GetPrimaryRGBDistributionMasks(&uiRBitMask, &uiGBitMask, &uiBBitMask))) {
        return FALSE;
      }
      PixelFormat.dwRBitMask = uiRBitMask;
      PixelFormat.dwGBitMask = uiGBitMask;
      PixelFormat.dwBBitMask = uiBBitMask;
      break;

    default:
      DebugMsg(TOPIC_VIDEOSURFACE, DBG_NORMAL, "Invalid BPP value, can only be 8 or 16.");
      return (FALSE);
  }

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

  if (fMemUsage & VSURFACE_SYSTEM_MEM_USAGE) {
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  } else {
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  }

  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = usWidth;
  SurfaceDescription.dwHeight = usHeight;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;

  DDCreateSurface(lpDD2Object, &SurfaceDescription, &lpDDS, &lpDDS2);

  hVSurface = VSurfaceNew();
  if (!hVSurface) {
    return FALSE;
  }

  hVSurface->usHeight = usHeight;
  hVSurface->usWidth = usWidth;
  hVSurface->ubBitDepth = ubBitDepth;
  hVSurface->pSurfaceData1 = (PTR)lpDDS;
  hVSurface->pSurfaceData = (PTR)lpDDS2;
  hVSurface->pPalette = NULL;
  hVSurface->p16BPPPalette = NULL;
  hVSurface->TransparentColor = FROMRGB(0, 0, 0);
  hVSurface->fFlags = 0;
  hVSurface->pClipper = NULL;

  DDGetSurfaceDescription(lpDDS2, &SurfaceDescription);

  if (SurfaceDescription.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) {
    hVSurface->fFlags |= VSURFACE_SYSTEM_MEM_USAGE;
  }

  if (VSurfaceDesc->fCreateFlags & VSURFACE_CREATE_FROMFILE) {
    tempRect.iLeft = 0;
    tempRect.iTop = 0;
    tempRect.iRight = hImage->usWidth - 1;
    tempRect.iBottom = hImage->usHeight - 1;
    SetVideoSurfaceDataFromHImage(hVSurface, hImage, 0, 0, &tempRect);
    if (hImage->ubBitDepth == 8) {
      SetVideoSurfacePalette(hVSurface, hImage->pPalette);
    }
    DestroyImage(hImage);
  }

  //
  // All is well
  //

  hVSurface->usHeight = usHeight;
  hVSurface->usWidth = usWidth;
  hVSurface->ubBitDepth = ubBitDepth;

  giMemUsedInSurfaces += (hVSurface->usHeight * hVSurface->usWidth * (hVSurface->ubBitDepth / 8));

  DebugMsg(TOPIC_VIDEOSURFACE, DBG_INFO, String("Success in Creating Video Surface"));

  return (hVSurface);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Called when surface is lost, for the most part called by utility functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct BufferLockInfo VSurfaceLock(struct VSurface *vs) {
  struct BufferLockInfo res = {.dest = NULL, .pitch = 0};
  if (vs) {
    res = DDLockSurface((LPDIRECTDRAWSURFACE2)vs->pSurfaceData);
  }
  return res;
}

void VSurfaceUnlock(struct VSurface *vs) {
  if (vs) {
    IDirectDrawSurface2_Unlock((LPDIRECTDRAWSURFACE2)vs->pSurfaceData, NULL);
  }
}

void UnLockVideoSurfaceBuffer(struct VSurface *vs) {
  if (vs) {
    VSurfaceUnlock(vs);
  }
}

// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  Assert(hVSurface != NULL);

  // Create palette object if not already done so
  if (hVSurface->pPalette == NULL) {
    DDCreatePalette(GetDirectDraw2Object(), (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                    (LPPALETTEENTRY)(&pSrcPalette[0]), (LPDIRECTDRAWPALETTE *)&hVSurface->pPalette,
                    NULL);
  } else {
    // Just Change entries
    DDSetPaletteEntries((LPDIRECTDRAWPALETTE)hVSurface->pPalette, 0, 0, 256,
                        (PALETTEENTRY *)pSrcPalette);
  }

  // Delete 16BPP Palette if one exists
  if (hVSurface->p16BPPPalette != NULL) {
    MemFree(hVSurface->p16BPPPalette);
    hVSurface->p16BPPPalette = NULL;
  }

  // Create 16BPP Palette
  hVSurface->p16BPPPalette = Create16BPPPalette(pSrcPalette);

  DebugMsg(TOPIC_VIDEOSURFACE, DBG_INFO, String("Video Surface Palette change successfull"));
  return (TRUE);
}

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor) {
  DDCOLORKEY ColorKey;
  DWORD fFlags = CLR_INVALID;
  LPDIRECTDRAWSURFACE2 lpDDSurface;

  // Assertions
  Assert(hVSurface != NULL);

  // Set trans color into Video Surface
  hVSurface->TransparentColor = TransColor;

  // Get surface pointer
  lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;
  if (!(lpDDSurface != NULL)) {
    return FALSE;
  }

  // Get right pixel format, based on bit depth

  switch (hVSurface->ubBitDepth) {
    case 8:

      // Use color directly
      ColorKey.dwColorSpaceLowValue = TransColor;
      ColorKey.dwColorSpaceHighValue = TransColor;
      break;

    case 16:

      fFlags = Get16BPPColor(TransColor);

      // fFlags now contains our closest match
      ColorKey.dwColorSpaceLowValue = fFlags;
      ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;
      break;
  }

  DDSetSurfaceColorKey(lpDDSurface, DDCKEY_SRCBLT, &ColorKey);

  return (TRUE);
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  if (!(hVSurface->pPalette != NULL)) {
    return FALSE;
  }

  DDGetPaletteEntries((LPDIRECTDRAWPALETTE)hVSurface->pPalette, 0, 0, 256,
                      (PALETTEENTRY *)pPalette);

  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) {
  // Assertions
  if (!(hVSurface != NULL)) {
    return FALSE;
  }

  // Release palette
  if (hVSurface->pPalette != NULL) {
    DDReleasePalette((LPDIRECTDRAWPALETTE)hVSurface->pPalette);
    hVSurface->pPalette = NULL;
  }

  // Get surface pointer
  LPDIRECTDRAWSURFACE2 lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;

  // Release surface
  if (hVSurface->pSurfaceData1 != NULL) {
    DDReleaseSurface((LPDIRECTDRAWSURFACE *)&hVSurface->pSurfaceData1, &lpDDSurface);
  }

  // // Release backup surface
  // if (hVSurface->pSavedSurfaceData != NULL) {
  //   DDReleaseSurface((LPDIRECTDRAWSURFACE *)&hVSurface->pSavedSurfaceData1,
  //                    (LPDIRECTDRAWSURFACE2 *)&hVSurface->pSavedSurfaceData);
  // }

  // If there is a 16bpp palette, free it
  if (hVSurface->p16BPPPalette != NULL) {
    MemFree(hVSurface->p16BPPPalette);
    hVSurface->p16BPPPalette = NULL;
  }

  giMemUsedInSurfaces -= (hVSurface->usHeight * hVSurface->usWidth * (hVSurface->ubBitDepth / 8));

  // Release object
  MemFree(hVSurface);

  return (TRUE);
}

// *****************************************************************************
//
// Private DirectDraw manipulation functions
//
// *****************************************************************************

LPDIRECTDRAWSURFACE2 GetVideoSurfaceDDSurface(struct VSurface *hVSurface) {
  Assert(hVSurface != NULL);

  return ((LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData);
}

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface) {
  // Create Video Surface

  // Set values based on DD Surface given
  DDSURFACEDESC DDSurfaceDesc;
  DDGetSurfaceDescription(lpDDSurface, &DDSurfaceDesc);
  DDPIXELFORMAT PixelFormat = DDSurfaceDesc.ddpfPixelFormat;

  struct VSurface *hVSurface = VSurfaceNew();
  hVSurface->usHeight = (UINT16)DDSurfaceDesc.dwHeight;
  hVSurface->usWidth = (UINT16)DDSurfaceDesc.dwWidth;
  hVSurface->ubBitDepth = (UINT8)PixelFormat.dwRGBBitCount;
  hVSurface->pSurfaceData = (PTR)lpDDSurface;

  // Get and Set palette, if attached, allow to fail
  LPDIRECTDRAWPALETTE pDDPalette;
  HRESULT ReturnCode = IDirectDrawSurface2_GetPalette(lpDDSurface, &pDDPalette);

  if (ReturnCode == DD_OK) {
    // Set 8-bit Palette and 16 BPP palette
    hVSurface->pPalette = pDDPalette;

    // Create 16-BPP Palette
    struct SGPPaletteEntry SGPPalette[256];
    DDGetPaletteEntries(pDDPalette, 0, 0, 256, (LPPALETTEENTRY)SGPPalette);
    hVSurface->p16BPPPalette = Create16BPPPalette(SGPPalette);
  } else {
    hVSurface->pPalette = NULL;
    hVSurface->p16BPPPalette = NULL;
  }

  // Set meory flags
  if (DDSurfaceDesc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) {
    hVSurface->fFlags |= VSURFACE_SYSTEM_MEM_USAGE;
  }

  // if (DDSurfaceDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) {
  //   hVSurface->fFlags |= VSURFACE_VIDEO_MEM_USAGE;
  // }

  return (hVSurface);
}

BOOLEAN FillSurface(struct VSurface *hDestVSurface, struct BltOpts *pBltFx) {
  DDBLTFX BlitterFX;

  Assert(hDestVSurface != NULL);
  if (!(pBltFx != NULL)) {
    return FALSE;
  }

  BlitterFX.dwSize = sizeof(DDBLTFX);
  BlitterFX.dwFillColor = pBltFx->ColorFill;

  DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, NULL, NULL, NULL, DDBLT_COLORFILL,
               &BlitterFX);

  return (TRUE);
}

BOOLEAN FillSurfaceRect(struct VSurface *hDestVSurface, struct BltOpts *pBltFx) {
  DDBLTFX BlitterFX;

  Assert(hDestVSurface != NULL);
  if (!(pBltFx != NULL)) {
    return FALSE;
  }

  BlitterFX.dwSize = sizeof(DDBLTFX);
  BlitterFX.dwFillColor = pBltFx->ColorFill;

  DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, (LPRECT) & (pBltFx->FillRect),
               NULL, NULL, DDBLT_COLORFILL, &BlitterFX);

  return (TRUE);
}

static BOOLEAN ClipReleatedSrcAndDestRectangles(struct VSurface *hDestVSurface,
                                                struct VSurface *hSrcVSurface, RECT *DestRect,
                                                RECT *SrcRect) {
  Assert(hDestVSurface != NULL);
  Assert(hSrcVSurface != NULL);

  // Check for invalid start positions and clip by ignoring blit
  if (DestRect->left >= hDestVSurface->usWidth || DestRect->top >= hDestVSurface->usHeight) {
    return (FALSE);
  }

  if (SrcRect->left >= hSrcVSurface->usWidth || SrcRect->top >= hSrcVSurface->usHeight) {
    return (FALSE);
  }

  // For overruns
  // Clip destination rectangles
  if (DestRect->right > hDestVSurface->usWidth) {
    // Both have to be modified or by default streching occurs
    DestRect->right = hDestVSurface->usWidth;
    SrcRect->right = SrcRect->left + (DestRect->right - DestRect->left);
  }
  if (DestRect->bottom > hDestVSurface->usHeight) {
    // Both have to be modified or by default streching occurs
    DestRect->bottom = hDestVSurface->usHeight;
    SrcRect->bottom = SrcRect->top + (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->right > hSrcVSurface->usWidth) {
    // Both have to be modified or by default streching occurs
    SrcRect->right = hSrcVSurface->usWidth;
    DestRect->right = DestRect->left + (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->bottom > hSrcVSurface->usHeight) {
    // Both have to be modified or by default streching occurs
    SrcRect->bottom = hSrcVSurface->usHeight;
    DestRect->bottom = DestRect->top + (SrcRect->bottom - SrcRect->top);
  }

  // For underruns
  // Clip destination rectangles
  if (DestRect->left < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->left = 0;
    SrcRect->left = SrcRect->right - (DestRect->right - DestRect->left);
  }
  if (DestRect->top < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->top = 0;
    SrcRect->top = SrcRect->bottom - (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->left < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->left = 0;
    DestRect->left = DestRect->right - (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->top < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->top = 0;
    DestRect->top = DestRect->bottom - (SrcRect->bottom - SrcRect->top);
  }

  return (TRUE);
}

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect) {
  RECT DestRect;

  RECT srcRect = {SrcRect->left, SrcRect->top, SrcRect->right, SrcRect->bottom};

  // Blit using the correct blitter
  if (fBltFlags & VS_BLT_FAST) {
    // Validations
    if (!(iDestX >= 0)) {
      return FALSE;
    }
    if (!(iDestY >= 0)) {
      return FALSE;
    }

    BltFastSurfaceWithFlags(hDestVSurface, iDestX, iDestY, hSrcVSurface, &srcRect, fBltFlags);
  } else {
    // Normal, specialized blit for clipping, etc

    // Default flags
    u32 uiDDFlags = DDBLT_WAIT;

    // Convert flags into DD flags, ( for transparency use, etc )
    if (fBltFlags & VS_BLT_USECOLORKEY) {
      uiDDFlags |= DDBLT_KEYSRC;
    }

    // Setup dest rectangle
    DestRect.top = (int)iDestY;
    DestRect.left = (int)iDestX;
    DestRect.bottom = (int)iDestY + (SrcRect->bottom - SrcRect->top);
    DestRect.right = (int)iDestX + (SrcRect->right - SrcRect->left);

    // Do Clipping of rectangles
    if (!ClipReleatedSrcAndDestRectangles(hDestVSurface, hSrcVSurface, &DestRect, &srcRect)) {
      // Returns false because dest start is > dest size
      return (TRUE);
    }

    // Check values for 0 size
    if (DestRect.top == DestRect.bottom || DestRect.right == DestRect.left) {
      return (TRUE);
    }

    // Check for -ve values

    DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, &DestRect,
                 (LPDIRECTDRAWSURFACE2)hSrcVSurface->pSurfaceData, &srcRect, uiDDFlags, NULL);
  }

  return (TRUE);
}

BOOLEAN BltVSurfaceUsingDDBlt(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                              UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect,
                              struct Rect *DestRect) {
  UINT32 uiDDFlags;
  RECT srcRect = {SrcRect->left, SrcRect->top, SrcRect->right, SrcRect->bottom};

  // Default flags
  uiDDFlags = DDBLT_WAIT;

  // Convert flags into DD flags, ( for transparency use, etc )
  if (fBltFlags & VS_BLT_USECOLORKEY) {
    uiDDFlags |= DDBLT_KEYSRC;
  }

  {
    RECT destRect = {
        .left = DestRect->left,
        .right = DestRect->right,
        .top = DestRect->top,
        .bottom = DestRect->bottom,
    };
    DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->pSurfaceData, &destRect,
                 (LPDIRECTDRAWSURFACE2)hSrcVSurface->pSurfaceData, &srcRect, uiDDFlags, NULL);
  }

  return (TRUE);
}

//////////////////////////////////////////////////////////////////
// Cinematics
//////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Smack.h"
#include "Utils/Cinematics.h"
#include "Utils/radmalw.i"
#include "platform_win.h"
#include "rust_fileman.h"

#if 1
// must go after other includes
#include <ddraw.h>
#endif

#define SMK_NUM_FLICS 4  // Maximum number of flics open

#define SMK_FLIC_OPEN 0x00000001       // Flic is open
#define SMK_FLIC_PLAYING 0x00000002    // Flic is playing
#define SMK_FLIC_LOOP 0x00000004       // Play flic in a loop
#define SMK_FLIC_AUTOCLOSE 0x00000008  // Close when done

struct SmkFlic {
  CHAR8 *cFilename;
  FileID file_id;
  struct SmackTag *SmackHandle;
  struct SmackBufTag *SmackBuffer;
  UINT32 uiFlags;
  LPDIRECTDRAWSURFACE2 lpDDS;
  HWND hWindow;
  UINT32 uiFrame;
  UINT32 uiLeft, uiTop;
};

struct SmkFlic SmkList[SMK_NUM_FLICS];

HWND hDisplayWindow = 0;
UINT32 uiDisplayHeight, uiDisplayWidth;
BOOLEAN fSuspendFlics = FALSE;
UINT32 uiFlicsPlaying = 0;
UINT32 guiSmackPixelFormat = SMACKBUFFER565;

LPDIRECTDRAWSURFACE lpVideoPlayback = NULL;
LPDIRECTDRAWSURFACE2 lpVideoPlayback2 = NULL;

//-Function-Prototypes-------------------------------------------------------------
void SmkInitialize(UINT32 uiWidth, UINT32 uiHeight);
void SmkShutdown(void);
struct SmkFlic *SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
struct SmkFlic *SmkOpenFlic(CHAR8 *cFilename);
void SmkSetBlitPosition(struct SmkFlic *pSmack, UINT32 uiLeft, UINT32 uiTop);
void SmkCloseFlic(struct SmkFlic *pSmack);
struct SmkFlic *SmkGetFreeFlic(void);
void SmkSetupVideo(void);
void SmkShutdownVideo(void);

BOOLEAN SmkPollFlics(void) {
  UINT32 uiCount;
  BOOLEAN fFlicStatus = FALSE;

  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++) {
    if (SmkList[uiCount].uiFlags & SMK_FLIC_PLAYING) {
      fFlicStatus = TRUE;
      if (!fSuspendFlics) {
        if (!SmackWait(SmkList[uiCount].SmackHandle)) {
          struct BufferLockInfo lock = DDLockSurface(SmkList[uiCount].lpDDS);
          SmackToBuffer(SmkList[uiCount].SmackHandle, SmkList[uiCount].uiLeft,
                        SmkList[uiCount].uiTop, lock.pitch, SmkList[uiCount].SmackHandle->Height,
                        lock.dest, guiSmackPixelFormat);
          SmackDoFrame(SmkList[uiCount].SmackHandle);
          IDirectDrawSurface2_Unlock(SmkList[uiCount].lpDDS, lock.dest);

          // Check to see if the flic is done the last frame
          if (SmkList[uiCount].SmackHandle->FrameNum ==
              (SmkList[uiCount].SmackHandle->Frames - 1)) {
            // If flic is looping, reset frame to 0
            if (SmkList[uiCount].uiFlags & SMK_FLIC_LOOP)
              SmackGoto(SmkList[uiCount].SmackHandle, 0);
            else if (SmkList[uiCount].uiFlags & SMK_FLIC_AUTOCLOSE)
              SmkCloseFlic(&SmkList[uiCount]);
          } else
            SmackNextFrame(SmkList[uiCount].SmackHandle);
        }
      }
    }
  }
  if (!fFlicStatus) SmkShutdownVideo();

  return (fFlicStatus);
}

void SmkInitialize(UINT32 uiWidth, UINT32 uiHeight) {
  void *pSoundDriver = NULL;

  // Wipe the flic list clean
  memset(SmkList, 0, sizeof(struct SmkFlic) * SMK_NUM_FLICS);

  // Set playback window properties
  hDisplayWindow = ghWindow;
  uiDisplayWidth = uiWidth;
  uiDisplayHeight = uiHeight;

  // Use MMX acceleration, if available
  SmackUseMMX(1);

  // Get the sound Driver handle
  pSoundDriver = SoundGetDriverHandle();

  // if we got the sound handle, use sound during the intro
  if (pSoundDriver) SmackSoundUseMSS(pSoundDriver);
}

void SmkShutdown(void) {
  UINT32 uiCount;

  // Close and deallocate any open flics
  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++) {
    if (SmkList[uiCount].uiFlags & SMK_FLIC_OPEN) SmkCloseFlic(&SmkList[uiCount]);
  }
}

struct SmkFlic *SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fClose) {
  struct SmkFlic *pSmack;

  // Open the flic
  if ((pSmack = SmkOpenFlic(cFilename)) == NULL) return (NULL);

  // Set the blitting position on the screen
  SmkSetBlitPosition(pSmack, uiLeft, uiTop);

  // We're now playing, flag the flic for the poller to update
  pSmack->uiFlags |= SMK_FLIC_PLAYING;
  if (fClose) pSmack->uiFlags |= SMK_FLIC_AUTOCLOSE;

  return (pSmack);
}

struct SmkFlic *SmkOpenFlic(CHAR8 *cFilename) {
  struct SmkFlic *pSmack;

  // Get an available flic slot from the list
  if (!(pSmack = SmkGetFreeFlic())) {
    ErrorMsg("SMK ERROR: Out of flic slots, cannot open another");
    return (NULL);
  }

  // Attempt opening the filename
  if (!(pSmack->file_id = File_OpenForReading(cFilename))) {
    ErrorMsg("SMK ERROR: Can't open the SMK file");
    return (NULL);
  }

  {
    // Get the real file handle for the file man handle for the smacker file
    HANDLE hFile = (void *)File_GetWinHandleToReadFile(pSmack->file_id);

    // Allocate a Smacker buffer for video decompression
    if (!(pSmack->SmackBuffer = SmackBufferOpen(hDisplayWindow, SMACKAUTOBLIT, 640, 480, 0, 0))) {
      ErrorMsg("SMK ERROR: Can't allocate a Smacker decompression buffer");
      return (NULL);
    }

    if (!(pSmack->SmackHandle =
              SmackOpen((CHAR8 *)hFile, SMACKFILEHANDLE | SMACKTRACKS, SMACKAUTOEXTRA)))
    //	if(!(pSmack->SmackHandle=SmackOpen(cFilename, SMACKTRACKS, SMACKAUTOEXTRA)))
    {
      ErrorMsg("SMK ERROR: Smacker won't open the SMK file");
      return (NULL);
    }
  }

  // Make sure we have a video surface
  SmkSetupVideo();

  pSmack->cFilename = cFilename;
  pSmack->lpDDS = lpVideoPlayback2;
  pSmack->hWindow = hDisplayWindow;

  // Smack flic is now open and ready to go
  pSmack->uiFlags |= SMK_FLIC_OPEN;

  return (pSmack);
}

void SmkSetBlitPosition(struct SmkFlic *pSmack, UINT32 uiLeft, UINT32 uiTop) {
  pSmack->uiLeft = uiLeft;
  pSmack->uiTop = uiTop;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {
  File_Close(pSmack->file_id);
  SmackBufferClose(pSmack->SmackBuffer);
  SmackClose(pSmack->SmackHandle);
  memset(pSmack, 0, sizeof(struct SmkFlic));
}

struct SmkFlic *SmkGetFreeFlic(void) {
  UINT32 uiCount;

  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++)
    if (!(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN)) return (&SmkList[uiCount]);

  return (NULL);
}

void SmkSetupVideo(void) {
  DDSURFACEDESC SurfaceDescription;
  HRESULT ReturnCode;
  UINT16 usRed, usGreen, usBlue;
  struct VSurface *hVSurface;

  // DEF:
  //	lpVideoPlayback2=CinematicModeOn();

  GetVideoSurface(&hVSurface, FRAME_BUFFER);
  lpVideoPlayback2 = GetVideoSurfaceDDSurface(hVSurface);

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(lpVideoPlayback2, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return;
  }

  usRed = (UINT16)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  usGreen = (UINT16)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  usBlue = (UINT16)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  if ((usRed == 0xf800) && (usGreen == 0x07e0) && (usBlue == 0x001f))
    guiSmackPixelFormat = SMACKBUFFER565;
  else
    guiSmackPixelFormat = SMACKBUFFER555;
}

void SmkShutdownVideo(void) {
  // DEF:
  //	CinematicModeOff();
}

//////////////////////////////////////////////////////////////////
// WinFont
//////////////////////////////////////////////////////////////////

#include "SGP/Font.h"
#include "SGP/WinFont.h"

INT32 FindFreeWinFont(void);
BOOLEAN gfEnumSucceed = FALSE;

#define MAX_WIN_FONTS 10

// Private struct not to be exported
// to other modules
struct HWINFONT {
  HFONT hFont;
  COLORVAL ForeColor;
  COLORVAL BackColor;
};

LOGFONT gLogFont;

struct HWINFONT WinFonts[MAX_WIN_FONTS];

void Convert16BitStringTo8BitChineseBig5String(UINT8 *dst, UINT16 *src) {
  INT32 i, j;
  char *ptr;

  i = j = 0;
  ptr = (char *)src;
  while (ptr[j] || ptr[j + 1]) {
    if (ptr[j]) {
      dst[i] = ptr[j];
      dst[i + 1] = '\0';
      i++;
    }
    j++;
  }
}

void InitWinFonts() { memset(WinFonts, 0, sizeof(WinFonts)); }

void ShutdownWinFonts() {}

INT32 FindFreeWinFont(void) {
  INT32 iCount;

  for (iCount = 0; iCount < MAX_WIN_FONTS; iCount++) {
    if (WinFonts[iCount].hFont == NULL) {
      return (iCount);
    }
  }

  return (-1);
}

struct HWINFONT *GetWinFont(INT32 iFont) {
  if (iFont == -1) {
    return (NULL);
  }

  if (WinFonts[iFont].hFont == NULL) {
    return (NULL);
  } else {
    return (&(WinFonts[iFont]));
  }
}

CHAR16 gzFontName[32];

void SetWinFontForeColor(INT32 iFont, COLORVAL *pColor) {
  struct HWINFONT *pWinFont;

  pWinFont = GetWinFont(iFont);

  if (pWinFont != NULL) {
    pWinFont->ForeColor = (*pColor);
  }
}

INT16 WinFontStringPixLength(STR16 string2, INT32 iFont) {
  struct HWINFONT *pWinFont;
  HDC hdc;
  SIZE RectSize;
  char string[512];

  pWinFont = GetWinFont(iFont);

  if (pWinFont == NULL) {
    return (0);
  }

#ifdef TAIWANESE
  Convert16BitStringTo8BitChineseBig5String(string, string2);
#else
  sprintf(string, "%S", string2);
#endif

  hdc = GetDC(NULL);
  SelectObject(hdc, pWinFont->hFont);
  GetTextExtentPoint32(hdc, string, strlen(string), &RectSize);
  ReleaseDC(NULL, hdc);

  return ((INT16)RectSize.cx);
}

INT16 GetWinFontHeight(STR16 string2, INT32 iFont) {
  struct HWINFONT *pWinFont;
  HDC hdc;
  SIZE RectSize;
  char string[512];

  pWinFont = GetWinFont(iFont);

  if (pWinFont == NULL) {
    return (0);
  }

#ifdef TAIWANESE
  Convert16BitStringTo8BitChineseBig5String(string, string2);
#else
  sprintf(string, "%S", string2);
#endif

  hdc = GetDC(NULL);
  SelectObject(hdc, pWinFont->hFont);
  GetTextExtentPoint32(hdc, string, strlen(string), &RectSize);
  ReleaseDC(NULL, hdc);

  return ((INT16)RectSize.cy);
}

UINT32 WinFont_mprintf(INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string[512];

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  PrintWinFont(FontDestBuffer, iFont, x, y, string);

  return (1);
}

int CALLBACK EnumFontFamProc(CONST LOGFONT *lplf, CONST TEXTMETRIC *lptm, DWORD dwType,
                             LPARAM lpData) {
  gfEnumSucceed = TRUE;

  return (TRUE);
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType,
                               LPARAM lParam) {
  CHAR8 szFontName[32];

  sprintf(szFontName, "%S", gzFontName);
  if (!strcmp(szFontName, (STR8)lpelfe->elfFullName)) {
    gfEnumSucceed = TRUE;
    memcpy(&gLogFont, &(lpelfe->elfLogFont), sizeof(LOGFONT));
  }

  return TRUE;
}

void PrintWinFont(UINT32 uiDestBuf, INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...) {
  va_list argptr;
  wchar_t string2[512];
  char string[512];
  struct VSurface *hVSurface;
  LPDIRECTDRAWSURFACE2 pDDSurface;
  HDC hdc;
  RECT rc;
  struct HWINFONT *pWinFont;
  int len;
  SIZE RectSize;

  pWinFont = GetWinFont(iFont);

  if (pWinFont == NULL) {
    return;
  }

  va_start(argptr, pFontString);  // Set up variable argument pointer
  len = vswprintf(string2, ARR_SIZE(string2), pFontString,
                  argptr);  // process gprintf string (get output str)
  va_end(argptr);

#ifdef TAIWANESE
  Convert16BitStringTo8BitChineseBig5String(string, string2);
#else
  snprintf(string, ARR_SIZE(string), "%S", string2);
#endif

  // Get surface...
  GetVideoSurface(&hVSurface, uiDestBuf);

  pDDSurface = GetVideoSurfaceDDSurface(hVSurface);

  IDirectDrawSurface2_GetDC(pDDSurface, &hdc);

  SelectObject(hdc, pWinFont->hFont);
  SetTextColor(hdc, pWinFont->ForeColor);
  SetBkColor(hdc, pWinFont->BackColor);
  SetBkMode(hdc, TRANSPARENT);

  GetTextExtentPoint32(hdc, string, len, &RectSize);
  SetRect(&rc, x, y, x + RectSize.cx, y + RectSize.cy);
  ExtTextOut(hdc, x, y, ETO_OPAQUE, &rc, string, len, NULL);
  IDirectDrawSurface2_ReleaseDC(pDDSurface, hdc);
}

//////////////////////////////////////////////////////////////////
// DirectDrawCalls
//////////////////////////////////////////////////////////////////

// DirectDrawSurface2 Calls
void DDCreateSurface(LPDIRECTDRAW2 pExistingDirectDraw, DDSURFACEDESC *pNewSurfaceDesc,
                     LPDIRECTDRAWSURFACE *ppNewSurface1, LPDIRECTDRAWSURFACE2 *ppNewSurface2) {
  Assert(pExistingDirectDraw != NULL);
  Assert(pNewSurfaceDesc != NULL);
  Assert(ppNewSurface1 != NULL);
  Assert(ppNewSurface2 != NULL);

  // create the directdraw surface
  IDirectDraw2_CreateSurface(pExistingDirectDraw, pNewSurfaceDesc, ppNewSurface1, NULL);

  // get the direct draw surface 2 interface
  IID tmpID = IID_IDirectDrawSurface2;
  IDirectDrawSurface_QueryInterface(*ppNewSurface1, &tmpID, (LPVOID *)ppNewSurface2);
}

static struct BufferLockInfo DDLockSurface(LPDIRECTDRAWSURFACE2 src) {
  struct BufferLockInfo res = {.dest = NULL, .pitch = 0};
  if (!src) {
    return res;
  }

  DDSURFACEDESC descr;
  ZEROMEM(descr);
  descr.dwSize = sizeof(descr);

  HRESULT ReturnCode;
  do {
    ReturnCode = IDirectDrawSurface2_Lock(src, NULL, &descr, 0, 0);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Failed to lock surface");
      return res;
    }
  } while (ReturnCode != DD_OK);

  res.pitch = descr.lPitch;
  res.dest = (uint8_t *)descr.lpSurface;
  return res;
}

void DDGetSurfaceDescription(LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc) {
  Assert(pSurface != NULL);
  Assert(pSurfaceDesc != NULL);

  ZEROMEM(*pSurfaceDesc);
  pSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);

  IDirectDrawSurface2_GetSurfaceDesc(pSurface, pSurfaceDesc);
}

void DDReleaseSurface(LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2) {
  Assert(ppOldSurface1 != NULL);
  Assert(ppOldSurface2 != NULL);
  Assert(*ppOldSurface1 != NULL);
  Assert(*ppOldSurface2 != NULL);

  IDirectDrawSurface2_Release(*ppOldSurface2);
  IDirectDrawSurface_Release(*ppOldSurface1);

  *ppOldSurface1 = NULL;
  *ppOldSurface2 = NULL;
}

void DDRestoreSurface(LPDIRECTDRAWSURFACE2 pSurface) {
  Assert(pSurface != NULL);

  IDirectDrawSurface2_Restore(pSurface);
}

static bool DDBltFastSurfaceWithFlags(LPDIRECTDRAWSURFACE2 dest, UINT32 uiX, UINT32 uiY,
                                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect, u32 ddFlags) {
  HRESULT ReturnCode;

  Assert(dest != NULL);
  Assert(src != NULL);

  // https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-bltfast

  do {
    ReturnCode = IDirectDrawSurface2_BltFast(dest, uiX, uiY, src, pSrcRect, ddFlags);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      if (ReturnCode == DDERR_SURFACELOST) {
        return false;
      }
    }
  } while (ReturnCode != DD_OK);
  return true;
}

bool BltFastSurfaceWithFlags(struct VSurface *dest, u32 x, u32 y, struct VSurface *src,
                             LPRECT pSrcRect, u32 flags) {
  u32 ddFlags = 0;

  if (flags & VS_BLT_USECOLORKEY) {
    ddFlags |= DDBLTFAST_SRCCOLORKEY;
  }

  return DDBltFastSurfaceWithFlags((LPDIRECTDRAWSURFACE2)dest->pSurfaceData, x, y,
                                   (LPDIRECTDRAWSURFACE2)src->pSurfaceData, pSrcRect, ddFlags);
}

bool DDBltFastSurface(LPDIRECTDRAWSURFACE2 dest, UINT32 uiX, UINT32 uiY, LPDIRECTDRAWSURFACE2 src,
                      LPRECT pSrcRect) {
  return DDBltFastSurfaceWithFlags(dest, uiX, uiY, src, pSrcRect, DDBLTFAST_NOCOLORKEY);
}

void DDBltSurface(LPDIRECTDRAWSURFACE2 dest, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 src,
                  LPRECT pSrcRect, UINT32 uiFlags, LPDDBLTFX pDDBltFx) {
  HRESULT ReturnCode;

  Assert(dest != NULL);

  // https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-blt

  do {
    ReturnCode = IDirectDrawSurface2_Blt(dest, pDestRect, src, pSrcRect, uiFlags, pDDBltFx);

  } while (ReturnCode == DDERR_WASSTILLDRAWING);

  ReturnCode;
}

void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, UINT32 uiFlags, LPPALETTEENTRY pColorTable,
                     LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter) {
  Assert(pDirectDraw != NULL);

  IDirectDraw2_CreatePalette(pDirectDraw, uiFlags, pColorTable, ppDDPalette, pUnkOuter);
}

void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiStartingEntry,
                         UINT32 uiCount, LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  IDirectDrawPalette_SetEntries(pPalette, uiFlags, uiStartingEntry, uiCount, pEntries);
}

void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, UINT32 uiFlags, UINT32 uiBase,
                         UINT32 uiNumEntries, LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  IDirectDrawPalette_GetEntries(pPalette, uiFlags, uiBase, uiNumEntries, pEntries);
}

void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette) {
  Assert(pPalette != NULL);

  IDirectDrawPalette_Release(pPalette);
}

void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, UINT32 uiFlags, LPDDCOLORKEY pDDColorKey) {
  Assert(pSurface != NULL);
  Assert(pDDColorKey != NULL);

  IDirectDrawSurface2_SetColorKey(pSurface, uiFlags, pDDColorKey);
}

// Clipper FUnctions
void DDCreateClipper(LPDIRECTDRAW2 pDirectDraw, UINT32 fFlags, LPDIRECTDRAWCLIPPER *pDDClipper) {
  Assert(pDirectDraw != NULL);
  Assert(pDDClipper != NULL);

  IDirectDraw2_CreateClipper(pDirectDraw, 0, pDDClipper, NULL);
}

void DDSetClipper(LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper) {
  Assert(pSurface != NULL);
  Assert(pDDClipper != NULL);

  IDirectDrawSurface2_SetClipper(pSurface, pDDClipper);
}

void DDReleaseClipper(LPDIRECTDRAWCLIPPER pDDClipper) {
  Assert(pDDClipper != NULL);

  IDirectDrawClipper_Release(pDDClipper);
}

void DDSetClipperList(LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, UINT32 uiFlags) {
  Assert(pDDClipper != NULL);
  Assert(pClipList != NULL);

  IDirectDrawClipper_SetClipList(pDDClipper, pClipList, uiFlags);
}

//////////////////////////////////////////////////////////////////
// DirectXCommon
//////////////////////////////////////////////////////////////////

void DirectXZeroMem(void *pMemory, int nSize) { memset(pMemory, 0, nSize); }
