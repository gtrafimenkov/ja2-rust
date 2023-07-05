#include <process.h>
#include <stdio.h>
#include <stdlib.h>

#include "Globals.h"
#include "Local.h"
#include "SGP/Debug.h"
#include "SGP/Input.h"
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
#include "rust_geometry.h"
#include "rust_images.h"

#define INITGUID
#include <ddraw.h>
#include <windows.h>

#include "Smack.h"
#include "platform_win.h"

// https://learn.microsoft.com/en-us/windows/win32/api/_directdraw/

bool BltFastSurfaceWithFlags(struct VSurface *dest, uint32_t x, uint32_t y, struct VSurface *src,
                             LPRECT pSrcRect, uint32_t flags);

static bool DDBltFastSurfaceWithFlags(LPDIRECTDRAWSURFACE2 dest, uint32_t uiX, uint32_t uiY,
                                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect, uint32_t ddFlags);

#define VIDEO_NO_CURSOR 0xFFFF

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface);

static BOOLEAN GetRGBDistribution(void);

// Surface Functions

void DDCreateSurface(LPDIRECTDRAW2 pExistingDirectDraw, DDSURFACEDESC *pNewSurfaceDesc,
                     LPDIRECTDRAWSURFACE *ppNewSurface1, LPDIRECTDRAWSURFACE2 *ppNewSurface2);
void DDGetSurfaceDescription(LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc);
void DDReleaseSurface(LPDIRECTDRAWSURFACE *ppOldSurface1, LPDIRECTDRAWSURFACE2 *ppOldSurface2);
static struct BufferLockInfo DDLockSurface(LPDIRECTDRAWSURFACE2 pSurface);
void DDRestoreSurface(LPDIRECTDRAWSURFACE2 pSurface);
bool DDBltFastSurface(LPDIRECTDRAWSURFACE2 dest, uint32_t uiX, uint32_t uiY,
                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect);
void DDBltSurface(LPDIRECTDRAWSURFACE2 dest, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 src,
                  LPRECT pSrcRect, uint32_t uiFlags, LPDDBLTFX pDDBltFx);
void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, uint32_t uiFlags,
                          LPDDCOLORKEY pDDColorKey);

// Palette Functions
void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, uint32_t uiFlags, LPPALETTEENTRY pColorTable,
                     LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter);
void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiStartingEntry,
                         uint32_t uiCount, LPPALETTEENTRY pEntries);
void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette);
void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiBase,
                         uint32_t uiNumEntries, LPPALETTEENTRY pEntries);

// Clipper functions
void DDCreateClipper(LPDIRECTDRAW2 pDirectDraw, uint32_t fFlags, LPDIRECTDRAWCLIPPER *pDDClipper);
void DDSetClipper(LPDIRECTDRAWSURFACE2 pSurface, LPDIRECTDRAWCLIPPER pDDClipper);
void DDReleaseClipper(LPDIRECTDRAWCLIPPER pDDClipper);
void DDSetClipperList(LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, uint32_t uiFlags);

// local functions
void DirectXAssert(BOOLEAN fValue, int32_t nLine, char *szFilename);
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
  uint16_t usMouseXPos, usMouseYPos;
  uint16_t usLeft, usTop, usRight, usBottom;
  RECT Region;
  LPDIRECTDRAWSURFACE _pSurface;
  LPDIRECTDRAWSURFACE2 pSurface;
} MouseCursorBackground;

//
// Video state variables
//

#define MAX_NUM_FRAMES 25

uint32_t guiFramePeriod = (1000 / 15);
uint32_t guiLastFrame;
uint16_t *gpFrameData[MAX_NUM_FRAMES];

//
// Direct Draw objects for both the Primary and Backbuffer surfaces
//

static LPDIRECTDRAW ddObject = NULL;
static LPDIRECTDRAW2 dd2Object = NULL;

static LPDIRECTDRAWSURFACE _gpPrimarySurface = NULL;
static LPDIRECTDRAWSURFACE2 gpPrimarySurface = NULL;

//
// Direct Draw Objects for the frame buffer
//

static LPDIRECTDRAWSURFACE _gpFrameBuffer = NULL;
static LPDIRECTDRAWSURFACE2 gpFrameBuffer = NULL;

#ifdef WINDOWED_MODE
static LPDIRECTDRAWSURFACE _gpBackBuffer = NULL;
extern RECT rcWindow;
#endif
static LPDIRECTDRAWSURFACE2 gpBackBuffer = NULL;

static LPDIRECTDRAWSURFACE _gpMouseCursor = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursor = NULL;

static LPDIRECTDRAWSURFACE _gpMouseCursorOriginal = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursorOriginal = NULL;

static MouseCursorBackground gMouseCursorBackground[2];

BOOLEAN gfFatalError = FALSE;
char gFatalErrorString[512];

// 8-bit palette stuff

struct SGPPaletteEntry gSgpPalette[256];
LPDIRECTDRAWPALETTE gpDirectDrawPalette;

//
// Dirty rectangle management variables
//

extern struct GRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
extern uint32_t guiDirtyRegionCount;
extern BOOLEAN gfForceFullScreenRefresh;

extern struct GRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
extern uint32_t gDirtyRegionsFlagsEx[MAX_DIRTY_REGIONS];
extern uint32_t guiDirtyRegionExCount;

//
// Screen output stuff
//

BOOLEAN gfPrintFrameBuffer;
uint32_t guiPrintFrameBufferIndex;

BOOLEAN InitializeVideoManager(struct PlatformInitParams *params) {
  uint32_t uiIndex;
  HRESULT ReturnCode;
  HWND hWindow;
  WNDCLASS WindowClass;
  char ClassName[] = APPLICATION_NAME;
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

  ReturnCode = DirectDrawCreate(NULL, &ddObject, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  IID tmpID = IID_IDirectDraw2;
  ReturnCode = IDirectDraw_QueryInterface(ddObject, &tmpID, (LPVOID *)&dd2Object);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Set the exclusive mode
  //
#ifdef WINDOWED_MODE
  ReturnCode = IDirectDraw2_SetCooperativeLevel(dd2Object, ghWindow, DDSCL_NORMAL);
#else
  ReturnCode =
      IDirectDraw2_SetCooperativeLevel(dd2Object, ghWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
#endif
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Set the display mode
  //
#ifndef WINDOWED_MODE
  ReturnCode = IDirectDraw2_SetDisplayMode(dd2Object, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0, 0);
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

  ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpPrimarySurface, NULL);
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
  ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpBackBuffer, NULL);
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

  ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpPrimarySurface, NULL);
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
  ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpFrameBuffer, NULL);
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
    struct BufferLockInfo lock = VSurfaceLock(vsFB);
    memset(lock.dest, 0, 480 * lock.pitch);
    VSurfaceUnlock(vsFB);
  }

  //
  // Initialize the main mouse surfaces
  //

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
  SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
  ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpMouseCursor, NULL);
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
  ReturnCode =
      IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_gpMouseCursorOriginal, NULL);
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
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
    SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
    ReturnCode = IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription,
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

  if (!GetRGBDistribution()) {
    return FALSE;
  }

  // create video surfaces from DD surfaces
  vsPrimary = CreateVideoSurfaceFromDDSurface(gpPrimarySurface);
  vsBB = CreateVideoSurfaceFromDDSurface(gpBackBuffer);
  vsMouseCursor = CreateVideoSurfaceFromDDSurface(gpMouseCursor);
  vsMouseBuffer = CreateVideoSurfaceFromDDSurface(gpMouseCursorOriginal);
  vsFB = CreateVideoSurfaceFromDDSurface(gpFrameBuffer);
  if (!vsPrimary || !vsBB || !vsMouseCursor || !vsMouseBuffer || !vsFB) {
    DebugMsg(TOPIC_VIDEOSURFACE, DBG_ERROR, String("Could not create primary surfaces"));
    return FALSE;
  }

  return TRUE;
}

void ShutdownVideoManager(void) {
  DebugMsg(TOPIC_VIDEO, DBG_ERROR, "Shutting down the video manager");

  DeleteVideoSurface(vsPrimary);
  DeleteVideoSurface(vsBB);
  DeleteVideoSurface(vsFB);
  DeleteVideoSurface(vsMouseCursor);
  DeleteVideoSurface(vsMouseBuffer);

  IDirectDrawSurface2_Release(gpMouseCursorOriginal);
  IDirectDrawSurface2_Release(gpMouseCursor);
  IDirectDrawSurface2_Release(gMouseCursorBackground[0].pSurface);
  IDirectDrawSurface2_Release(gpBackBuffer);
  IDirectDrawSurface2_Release(gpPrimarySurface);

  IDirectDraw2_RestoreDisplayMode(dd2Object);
  IDirectDraw2_SetCooperativeLevel(dd2Object, ghWindow, DDSCL_NORMAL);
  IDirectDraw2_Release(dd2Object);

  guiVideoManagerState = VIDEO_OFF;

  if (gpCursorStore != NULL) {
    DeleteVideoObject(gpCursorStore);
    gpCursorStore = NULL;
  }

  FreeMouseCursor();
}

void SuspendVideoManager(void) { guiVideoManagerState = VIDEO_SUSPENDED; }

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

void ScrollJA2Background(uint32_t uiDirection, int16_t sScrollXIncrement, int16_t sScrollYIncrement,
                         LPDIRECTDRAWSURFACE2 pSource, LPDIRECTDRAWSURFACE2 pDest,
                         BOOLEAN fRenderStrip, uint32_t uiCurrentMouseBackbuffer) {
  uint16_t usWidth, usHeight;
  static RECT Region;
  static uint16_t usMouseXPos, usMouseYPos;
  static RECT StripRegions[2], MouseRegion;
  uint16_t usNumStrips = 0;
  int32_t cnt;
  int16_t sShiftX, sShiftY;
  int32_t uiCountY;

  GetCurrentVideoSettings(&usWidth, &usHeight);
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
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
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
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
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
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
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
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
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
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
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
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
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
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);

      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
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
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos -= sScrollYIncrement;
      usMouseXPos -= sScrollXIncrement;

      break;
  }

  if (fRenderStrip) {
    // Memset to 0

    for (cnt = 0; cnt < usNumStrips; cnt++) {
      RenderStaticWorldRect((int16_t)StripRegions[cnt].left, (int16_t)StripRegions[cnt].top,
                            (int16_t)StripRegions[cnt].right, (int16_t)StripRegions[cnt].bottom,
                            TRUE);
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
    SaveVideoOverlaysArea(vsBB);

    // BLIT NEW
    ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);
  }
}

void printFramebuffer() {
  LPDIRECTDRAWSURFACE _pTmpBuffer;
  LPDIRECTDRAWSURFACE2 pTmpBuffer;
  DDSURFACEDESC SurfaceDescription;
  FILE *OutputFile;
  char FileName[64];
  struct Str512 ExecDir;
  uint16_t *p16BPPData;

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
      IDirectDraw2_CreateSurface(dd2Object, &SurfaceDescription, &_pTmpBuffer, NULL);

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
    p16BPPData = (uint16_t *)MemAlloc(640 * 2);

    for (int32_t iIndex = 479; iIndex >= 0; iIndex--) {
      // ATE: OK, fix this such that it converts pixel format to 5/5/5
      // if current settings are 5/6/5....
      // Read into a buffer...
      memcpy(p16BPPData, (((uint8_t *)SurfaceDescription.lpSurface) + (iIndex * 640 * 2)), 640 * 2);

      // Convert....
      ConvertRGBDistribution565To555(p16BPPData, 640);

      // Write
      fwrite(p16BPPData, 640 * 2, 1, OutputFile);
    }

    MemFree(p16BPPData);

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
  static uint32_t uiRefreshThreadState, uiIndex;
  uint16_t usScreenWidth, usScreenHeight;
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
          (uint16_t)Region.right - (uint16_t)Region.left;
      gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom =
          (uint16_t)Region.bottom - (uint16_t)Region.top;
      if (Region.left < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = (uint16_t)(0 - Region.left);
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos = 0;
        Region.left = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos =
            (uint16_t)MousePos.x - gsMouseCursorXOffset;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = 0;
      }
      if (Region.top < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos = 0;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop = (uint16_t)(0 - Region.top);
        Region.top = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos =
            (uint16_t)MousePos.y - gsMouseCursorYOffset;
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

static BOOLEAN GetRGBDistribution(void) {
  DDSURFACEDESC SurfaceDescription;

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_PIXELFORMAT;
  HRESULT ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(gpPrimarySurface, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Ok we now have the surface description, we now can get the information that we need
  //

  uint16_t usRedMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  uint16_t usGreenMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  uint16_t usBlueMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  if ((usRedMask != 0xf800) || (usGreenMask != 0x07e0) || (usBlueMask != 0x001f)) {
    char buf[200];
    snprintf(buf, ARR_SIZE(buf), "XXX RGB distribution: (0x%04x, 0x%04x, 0x%04x)", usRedMask,
             usGreenMask, usBlueMask);
    DebugLogWrite(buf);
    DebugLogWrite("XXX RGB distribution other than 565 is not supported");
    // It may not work some hardware, but 16 bit mode is outdated anyway.
    // We should switch to 32bit mode.
    //
    // Maybe useful:
    //   - https://www.gamedev.net/forums/topic/54104-555-or-565/
    //   - https://learn.microsoft.com/en-us/windows/win32/directshow/working-with-16-bit-rgb
    return FALSE;
  }

  guiTranslucentMask = 0x7bef;

  return TRUE;
}

void PrintScreen(void) { gfPrintFrameBuffer = TRUE; }

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) {
  // If we are in 256 colors, then we have to initialize the palette system to 0 (faded out)
  memcpy(gSgpPalette, pPalette, sizeof(struct SGPPaletteEntry) * 256);

  HRESULT ReturnCode =
      IDirectDraw_CreatePalette(dd2Object, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
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

void FatalError(char *pError, ...) {
  va_list argptr;

  va_start(argptr, pError);  // Set up variable argument pointer
  vsprintf(gFatalErrorString, pError, argptr);
  va_end(argptr);

  gfFatalError = TRUE;

  // Release DDraw
  IDirectDraw2_RestoreDisplayMode(dd2Object);
  IDirectDraw2_Release(dd2Object);
  ShowWindow(ghWindow, SW_HIDE);

  gfProgramIsRunning = FALSE;

  MessageBox(ghWindow, gFatalErrorString, "JA2 Fatal Error", MB_OK | MB_TASKMODAL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manipulation Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct VSurface *CreateVideoSurface(uint16_t width, uint16_t height) {
  Assert(height > 0);
  Assert(width > 0);

  DDPIXELFORMAT PixelFormat;
  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
  PixelFormat.dwFlags = DDPF_RGB;
  PixelFormat.dwRGBBitCount = 16;
  PixelFormat.dwRBitMask = 0xf800;
  PixelFormat.dwGBitMask = 0x07e0;
  PixelFormat.dwBBitMask = 0x001f;

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = width;
  SurfaceDescription.dwHeight = height;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;
  LPDIRECTDRAWSURFACE lpDDS;
  LPDIRECTDRAWSURFACE2 lpDDS2;
  DDCreateSurface(dd2Object, &SurfaceDescription, &lpDDS, &lpDDS2);

  struct VSurface *vs = VSurfaceNew();
  if (!vs) {
    return FALSE;
  }

  vs->usHeight = height;
  vs->usWidth = width;
  vs->pSurfaceData1 = (void *)lpDDS;
  vs->pSurfaceData = (void *)lpDDS2;
  vs->TransparentColor = FROMRGB(0, 0, 0);

  return (vs);
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

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor) {
  DDCOLORKEY ColorKey;
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

  ColorKey.dwColorSpaceLowValue = Get16BPPColor(TransColor);
  ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;

  DDSetSurfaceColorKey(lpDDSurface, DDCKEY_SRCBLT, &ColorKey);

  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) {
  // Assertions
  if (!(hVSurface != NULL)) {
    return FALSE;
  }

  // Get surface pointer
  LPDIRECTDRAWSURFACE2 lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;

  // Release surface
  if (hVSurface->pSurfaceData1 != NULL) {
    DDReleaseSurface((LPDIRECTDRAWSURFACE *)&hVSurface->pSurfaceData1, &lpDDSurface);
  }

  MemFree(hVSurface);

  return (TRUE);
}

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface) {
  DDSURFACEDESC DDSurfaceDesc;
  DDGetSurfaceDescription(lpDDSurface, &DDSurfaceDesc);

  if (DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount != 16) {
    DebugLogWrite("Error: CreateVideoSurfaceFromDDSurface: unsupported bit depth");
    return NULL;
  }

  struct VSurface *hVSurface = VSurfaceNew();
  hVSurface->usHeight = (uint16_t)DDSurfaceDesc.dwHeight;
  hVSurface->usWidth = (uint16_t)DDSurfaceDesc.dwWidth;
  hVSurface->pSurfaceData = (void *)lpDDSurface;

  // Get and Set palette, if attached, allow to fail
  LPDIRECTDRAWPALETTE pDDPalette;
  HRESULT ReturnCode = IDirectDrawSurface2_GetPalette(lpDDSurface, &pDDPalette);

  return (hVSurface);
}

BOOLEAN FillSurfaceRect(struct VSurface *dest, struct BltOpts *pBltFx) {
  Assert(dest != NULL);
  if (!pBltFx) {
    return FALSE;
  }

  DDBLTFX BlitterFX;
  BlitterFX.dwSize = sizeof(DDBLTFX);
  BlitterFX.dwFillColor = pBltFx->ColorFill;

  DDBltSurface((LPDIRECTDRAWSURFACE2)dest->pSurfaceData, (LPRECT) & (pBltFx->FillRect), NULL, NULL,
               DDBLT_COLORFILL, &BlitterFX);

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
                           uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                           struct Rect *SrcRect) {
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
    uint32_t uiDDFlags = DDBLT_WAIT;

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

BOOLEAN BltVSurfaceUsingDDBlt(struct VSurface *dest, struct VSurface *src, uint32_t fBltFlags,
                              int32_t iDestX, int32_t iDestY, struct Rect *SrcRect,
                              struct Rect *DestRect) {
  uint32_t uiDDFlags;
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
    DDBltSurface((LPDIRECTDRAWSURFACE2)dest->pSurfaceData, &destRect,
                 (LPDIRECTDRAWSURFACE2)src->pSurfaceData, &srcRect, uiDDFlags, NULL);
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
  char *cFilename;
  FileID file_id;
  struct SmackTag *SmackHandle;
  struct SmackBufTag *SmackBuffer;
  uint32_t uiFlags;
  LPDIRECTDRAWSURFACE2 lpDDS;
  HWND hWindow;
  uint32_t uiFrame;
  uint32_t uiLeft, uiTop;
};

struct SmkFlic SmkList[SMK_NUM_FLICS];

HWND hDisplayWindow = 0;
uint32_t uiDisplayHeight, uiDisplayWidth;
BOOLEAN fSuspendFlics = FALSE;
uint32_t uiFlicsPlaying = 0;
uint32_t guiSmackPixelFormat = SMACKBUFFER565;

LPDIRECTDRAWSURFACE lpVideoPlayback = NULL;
LPDIRECTDRAWSURFACE2 lpVideoPlayback2 = NULL;

//-Function-Prototypes-------------------------------------------------------------
void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight);
void SmkShutdown(void);
struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
struct SmkFlic *SmkOpenFlic(char *cFilename);
void SmkSetBlitPosition(struct SmkFlic *pSmack, uint32_t uiLeft, uint32_t uiTop);
void SmkCloseFlic(struct SmkFlic *pSmack);
struct SmkFlic *SmkGetFreeFlic(void);
void SmkSetupVideo(void);
void SmkShutdownVideo(void);

BOOLEAN SmkPollFlics(void) {
  uint32_t uiCount;
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

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {
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
  uint32_t uiCount;

  // Close and deallocate any open flics
  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++) {
    if (SmkList[uiCount].uiFlags & SMK_FLIC_OPEN) SmkCloseFlic(&SmkList[uiCount]);
  }
}

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fClose) {
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

struct SmkFlic *SmkOpenFlic(char *cFilename) {
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
              SmackOpen((char *)hFile, SMACKFILEHANDLE | SMACKTRACKS, SMACKAUTOEXTRA)))
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

void SmkSetBlitPosition(struct SmkFlic *pSmack, uint32_t uiLeft, uint32_t uiTop) {
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
  uint32_t uiCount;

  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++)
    if (!(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN)) return (&SmkList[uiCount]);

  return (NULL);
}

void SmkSetupVideo(void) {
  DDSURFACEDESC SurfaceDescription;
  HRESULT ReturnCode;
  uint16_t usRed, usGreen, usBlue;
  struct VSurface *hVSurface;

  GetVideoSurface(&hVSurface, FRAME_BUFFER);
  lpVideoPlayback2 = (LPDIRECTDRAWSURFACE2)hVSurface->pSurfaceData;

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(lpVideoPlayback2, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return;
  }

  usRed = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  usGreen = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  usBlue = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

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

int32_t FindFreeWinFont(void);
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

void Convert16BitStringTo8BitChineseBig5String(uint8_t *dst, uint16_t *src) {
  int32_t i, j;
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

int32_t FindFreeWinFont(void) {
  int32_t iCount;

  for (iCount = 0; iCount < MAX_WIN_FONTS; iCount++) {
    if (WinFonts[iCount].hFont == NULL) {
      return (iCount);
    }
  }

  return (-1);
}

struct HWINFONT *GetWinFont(int32_t iFont) {
  if (iFont == -1) {
    return (NULL);
  }

  if (WinFonts[iFont].hFont == NULL) {
    return (NULL);
  } else {
    return (&(WinFonts[iFont]));
  }
}

wchar_t gzFontName[32];

int CALLBACK EnumFontFamProc(CONST LOGFONT *lplf, CONST TEXTMETRIC *lptm, DWORD dwType,
                             LPARAM lpData) {
  gfEnumSucceed = TRUE;

  return (TRUE);
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType,
                               LPARAM lParam) {
  char szFontName[32];

  sprintf(szFontName, "%S", gzFontName);
  if (!strcmp(szFontName, (char *)lpelfe->elfFullName)) {
    gfEnumSucceed = TRUE;
    memcpy(&gLogFont, &(lpelfe->elfLogFont), sizeof(LOGFONT));
  }

  return TRUE;
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

static bool DDBltFastSurfaceWithFlags(LPDIRECTDRAWSURFACE2 dest, uint32_t uiX, uint32_t uiY,
                                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect, uint32_t ddFlags) {
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

bool BltFastSurfaceWithFlags(struct VSurface *dest, uint32_t x, uint32_t y, struct VSurface *src,
                             LPRECT pSrcRect, uint32_t flags) {
  uint32_t ddFlags = 0;

  if (flags & VS_BLT_USECOLORKEY) {
    ddFlags |= DDBLTFAST_SRCCOLORKEY;
  }

  return DDBltFastSurfaceWithFlags((LPDIRECTDRAWSURFACE2)dest->pSurfaceData, x, y,
                                   (LPDIRECTDRAWSURFACE2)src->pSurfaceData, pSrcRect, ddFlags);
}

bool DDBltFastSurface(LPDIRECTDRAWSURFACE2 dest, uint32_t uiX, uint32_t uiY,
                      LPDIRECTDRAWSURFACE2 src, LPRECT pSrcRect) {
  return DDBltFastSurfaceWithFlags(dest, uiX, uiY, src, pSrcRect, DDBLTFAST_NOCOLORKEY);
}

void DDBltSurface(LPDIRECTDRAWSURFACE2 dest, LPRECT pDestRect, LPDIRECTDRAWSURFACE2 src,
                  LPRECT pSrcRect, uint32_t uiFlags, LPDDBLTFX pDDBltFx) {
  HRESULT ReturnCode;

  Assert(dest != NULL);

  // https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-blt

  do {
    ReturnCode = IDirectDrawSurface2_Blt(dest, pDestRect, src, pSrcRect, uiFlags, pDDBltFx);

  } while (ReturnCode == DDERR_WASSTILLDRAWING);

  ReturnCode;
}

void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, uint32_t uiFlags, LPPALETTEENTRY pColorTable,
                     LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter) {
  Assert(pDirectDraw != NULL);

  IDirectDraw2_CreatePalette(pDirectDraw, uiFlags, pColorTable, ppDDPalette, pUnkOuter);
}

void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiStartingEntry,
                         uint32_t uiCount, LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  IDirectDrawPalette_SetEntries(pPalette, uiFlags, uiStartingEntry, uiCount, pEntries);
}

void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiBase,
                         uint32_t uiNumEntries, LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  IDirectDrawPalette_GetEntries(pPalette, uiFlags, uiBase, uiNumEntries, pEntries);
}

void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette) {
  Assert(pPalette != NULL);

  IDirectDrawPalette_Release(pPalette);
}

void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, uint32_t uiFlags,
                          LPDDCOLORKEY pDDColorKey) {
  Assert(pSurface != NULL);
  Assert(pDDColorKey != NULL);

  IDirectDrawSurface2_SetColorKey(pSurface, uiFlags, pDDColorKey);
}

// Clipper FUnctions
void DDCreateClipper(LPDIRECTDRAW2 pDirectDraw, uint32_t fFlags, LPDIRECTDRAWCLIPPER *pDDClipper) {
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

void DDSetClipperList(LPDIRECTDRAWCLIPPER pDDClipper, LPRGNDATA pClipList, uint32_t uiFlags) {
  Assert(pDDClipper != NULL);
  Assert(pClipList != NULL);

  IDirectDrawClipper_SetClipList(pDDClipper, pClipList, uiFlags);
}

//////////////////////////////////////////////////////////////////
// DirectXCommon
//////////////////////////////////////////////////////////////////

void DirectXZeroMem(void *pMemory, int nSize) { memset(pMemory, 0, nSize); }
