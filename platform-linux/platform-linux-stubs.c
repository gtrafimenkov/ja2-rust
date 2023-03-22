// Temporary stubs for missing linux platform functionality.

#include "Point.h"
#include "SGP/Input.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/Cinematics.h"
#include "Utils/TimerControl.h"
#include "platform.h"

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

CHAR8 gzCommandLine[100];

void PrintToDebuggerConsole(const char *message) {}

int strcasecmp(const char *s1, const char *s2) { return 0; }

int strncasecmp(const char *s1, const char *s2, size_t n) { return 0; }

extern u32 Plat_GetTickCount() { return 0; }

UINT32 GetClock(void) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfProgramIsRunning;

UINT32 Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() { return 0; }

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct) { return FALSE; }

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) { return FALSE; }

void Plat_GetFileClose(struct GetFile *pGFStruct) {}

/////////////////////////////////////////////////////////////////////////////////
// Input
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfKeyState[256];
BOOLEAN gfLeftButtonState;
BOOLEAN gfRightButtonState;
UINT16 gusMouseXPos;
UINT16 gusMouseYPos;

BOOLEAN gfSGPInputReceived = FALSE;

BOOLEAN DequeueSpecificEvent(InputAtom *Event, UINT32 uiMaskFlags) { return FALSE; }

BOOLEAN DequeueEvent(InputAtom *Event) { return FALSE; }

void GetMousePos(SGPPoint *Point) {}

void RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2) {}

void RestrictMouseCursor(SGPRect *pRectangle) {}

void FreeMouseCursor(void) {}

void GetRestrictedClipCursor(SGPRect *pRectangle) {}

BOOLEAN IsCursorRestricted(void) { return FALSE; }

void SimulateMouseMovement(UINT32 uiNewXPos, UINT32 uiNewYPos) {}

void DequeueAllKeyBoardEvents() {}

struct Point GetMousePoint() {
  struct Point res = {0, 0};
  return res;
}

/////////////////////////////////////////////////////////////////////////////////
// Time
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN InitializeJA2Clock(void) { return FALSE; }

void ShutdownJA2Clock(void) {}

void PauseTime(BOOLEAN fPaused) {}

void SetCustomizableTimerCallbackAndDelay(INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace) {}

void CheckCustomizableTimer(void) {}

/////////////////////////////////////////////////////////////////////////////////
// Sound
/////////////////////////////////////////////////////////////////////////////////

UINT32 SoundPlay(STR pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

UINT32 SoundPlayStreamedFile(STR pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

UINT32 SoundPlayRandom(STR pFilename, RANDOMPARMS *pParms) { return (SOUND_ERROR); }

BOOLEAN SoundIsPlaying(UINT32 uiSoundID) { return FALSE; }

BOOLEAN SoundStop(UINT32 uiSoundID) { return FALSE; }

BOOLEAN SoundStopAll(void) { return FALSE; }

BOOLEAN SoundSetVolume(UINT32 uiSoundID, UINT32 uiVolume) { return FALSE; }

BOOLEAN SoundSetPan(UINT32 uiSoundID, UINT32 uiPan) { return FALSE; }

UINT32 SoundGetVolume(UINT32 uiSoundID) { return (SOUND_ERROR); }

BOOLEAN SoundServiceRandom(void) { return FALSE; }

BOOLEAN SoundStopAllRandom(void) { return FALSE; }

BOOLEAN SoundServiceStreams(void) { return FALSE; }

UINT32 SoundGetPosition(UINT32 uiSoundID) { return 0; }

UINT32 SoundLoadSample(STR pFilename) { return (NO_SAMPLE); }

UINT32 SoundLockSample(STR pFilename) { return (NO_SAMPLE); }

UINT32 SoundUnlockSample(STR pFilename) { return (NO_SAMPLE); }

void SoundRemoveSampleFlags(UINT32 uiSample, UINT32 uiFlags) {}

/////////////////////////////////////////////////////////////////////////////////
// Video
/////////////////////////////////////////////////////////////////////////////////

INT32 giNumFrames = 0;

void RefreshScreen() {}

BOOLEAN EraseMouseCursor() { return FALSE; }

BOOLEAN SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight,
                                 UINT16 usCursorWidth) {
  return FALSE;
}

void DirtyCursor() {}

BOOLEAN SetCurrentCursor(UINT16 usVideoObjectSubIndex, UINT16 usOffsetX, UINT16 usOffsetY) {
  return FALSE;
}

void StartFrameBufferRender(void) {}

void EndFrameBufferRender(void) {}

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) { return FALSE; }

void FatalError(STR8 pError, ...) {}

BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor) {
  return FALSE;
}

struct VSurface *CreateVideoSurface(u16 width, u16 height, u8 bitDepth) { return NULL; }

BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  return FALSE;
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  return FALSE;
}

BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) { return FALSE; }

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect) {
  return FALSE;
}

BOOLEAN SmkPollFlics(void) { return FALSE; }

void SmkInitialize(UINT32 uiWidth, UINT32 uiHeight) {}

void SmkShutdown(void) {}

struct SmkFlic *SmkPlayFlic(CHAR8 *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fClose) {
  return NULL;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {}

void SetWinFontForeColor(INT32 iFont, COLORVAL *pColor) {}

INT16 WinFontStringPixLength(STR16 string2, INT32 iFont) { return 0; }

INT16 GetWinFontHeight(STR16 string2, INT32 iFont) { return 0; }

UINT32 WinFont_mprintf(INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN FillSurfaceRect(struct VSurface *hDestVSurface, struct BltOpts *pBltFx) { return FALSE; }

void VSurfaceUnlock(struct VSurface *vs) {}
struct BufferLockInfo VSurfaceLock(struct VSurface *vs) {
  struct BufferLockInfo res = {.dest = NULL, .pitch = 0};
  return res;
}
BOOLEAN BltVSurfaceUsingDDBlt(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                              UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect,
                              struct Rect *DestRect) {
  return FALSE;
}
