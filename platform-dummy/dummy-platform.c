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

char gzCommandLine[100];

void PrintToDebuggerConsole(const char *message) {}

int strcasecmp(const char *s1, const char *s2) { return 0; }

int strncasecmp(const char *s1, const char *s2, size_t n) { return 0; }

extern uint32_t Plat_GetTickCount() { return 0; }

uint32_t GetClock(void) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfProgramIsRunning;

uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() { return 0; }

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileFirst(char *pSpec, struct GetFile *pGFStruct) { return FALSE; }

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) { return FALSE; }

void Plat_GetFileClose(struct GetFile *pGFStruct) {}

/////////////////////////////////////////////////////////////////////////////////
// Input
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfKeyState[256];
BOOLEAN gfLeftButtonState;
BOOLEAN gfRightButtonState;
uint16_t gusMouseXPos;
uint16_t gusMouseYPos;

BOOLEAN gfSGPInputReceived = FALSE;

BOOLEAN DequeueSpecificEvent(InputAtom *Event, uint32_t uiMaskFlags) { return FALSE; }

BOOLEAN DequeueEvent(InputAtom *Event) { return FALSE; }

void GetMousePos(SGPPoint *Point) {}

void RestrictMouseToXYXY(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2) {}

void RestrictMouseCursor(struct GRect *pRectangle) {}

void FreeMouseCursor(void) {}

void GetRestrictedClipCursor(struct GRect *pRectangle) {}

BOOLEAN IsCursorRestricted(void) { return FALSE; }

void SimulateMouseMovement(uint32_t uiNewXPos, uint32_t uiNewYPos) {}

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

void SetCustomizableTimerCallbackAndDelay(int32_t iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace) {}

void CheckCustomizableTimer(void) {}

/////////////////////////////////////////////////////////////////////////////////
// Sound
/////////////////////////////////////////////////////////////////////////////////

uint32_t SoundPlay(char *pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayStreamedFile(char *pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayRandom(char *pFilename, RANDOMPARMS *pParms) { return (SOUND_ERROR); }

BOOLEAN SoundIsPlaying(uint32_t uiSoundID) { return FALSE; }

BOOLEAN SoundStop(uint32_t uiSoundID) { return FALSE; }

BOOLEAN SoundStopAll(void) { return FALSE; }

BOOLEAN SoundSetVolume(uint32_t uiSoundID, uint32_t uiVolume) { return FALSE; }

BOOLEAN SoundSetPan(uint32_t uiSoundID, uint32_t uiPan) { return FALSE; }

uint32_t SoundGetVolume(uint32_t uiSoundID) { return (SOUND_ERROR); }

BOOLEAN SoundServiceRandom(void) { return FALSE; }

BOOLEAN SoundStopAllRandom(void) { return FALSE; }

BOOLEAN SoundServiceStreams(void) { return FALSE; }

uint32_t SoundGetPosition(uint32_t uiSoundID) { return 0; }

uint32_t SoundLoadSample(char *pFilename) { return (NO_SAMPLE); }

uint32_t SoundLockSample(char *pFilename) { return (NO_SAMPLE); }

uint32_t SoundUnlockSample(char *pFilename) { return (NO_SAMPLE); }

void SoundRemoveSampleFlags(uint32_t uiSample, uint32_t uiFlags) {}

/////////////////////////////////////////////////////////////////////////////////
// Video
/////////////////////////////////////////////////////////////////////////////////

void RefreshScreen() {}

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) { return FALSE; }

void FatalError(char *pError, ...) {}

BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *hVSurface, COLORVAL TransColor) {
  return FALSE;
}

struct VSurface *CreateVideoSurface(uint16_t width, uint16_t height) { return NULL; }

BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) { return FALSE; }

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                           struct Rect *SrcRect) {
  return FALSE;
}

BOOLEAN SmkPollFlics(void) { return FALSE; }

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {}

void SmkShutdown(void) {}

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fClose) {
  return NULL;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {}

BOOLEAN FillSurfaceRect(struct VSurface *hDestVSurface, struct BltOpts *pBltFx) { return FALSE; }

void VSurfaceUnlock(struct VSurface *vs) {}
struct BufferLockInfo VSurfaceLock(struct VSurface *vs) {
  struct BufferLockInfo res = {.dest = NULL, .pitch = 0};
  return res;
}
BOOLEAN BltVSurfaceUsingDDBlt(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                              uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                              struct Rect *SrcRect, struct Rect *DestRect) {
  return FALSE;
}
