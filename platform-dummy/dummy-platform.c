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

BOOLEAN Plat_GetExecutableDirectory(char *buf, u16 bufSize) { return FALSE; }

void DebugPrint(const char *message) {}

int strcasecmp(const char *s1, const char *s2) { return 0; }

int strncasecmp(const char *s1, const char *s2, size_t n) { return 0; }

extern u32 Plat_GetTickCount() { return 0; }

uint32_t GetClock(void) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfProgramIsRunning;

BOOLEAN Plat_SetCurrentDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetCurrentDirectory(STRING512 pcDirectory) { return FALSE; }

BOOLEAN Plat_DeleteFile(const char *filename) { return FALSE; }

u32 Plat_GetFileSize(SYS_FILE_HANDLE handle) { return 0; }

BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, u32 bytesToRead, u32 *readBytes) {
  return FALSE;
}

void Plat_CloseFile(SYS_FILE_HANDLE handle) {}

BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle) { return FALSE; }

u32 Plat_SetFilePointer(SYS_FILE_HANDLE handle, i32 distance, int seekType) { return 0; }

uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() { return 0; }

BOOLEAN Plat_CreateDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) { return FALSE; }

HWFILE FileMan_Open(STR strFilename, uint32_t uiOptions, BOOLEAN fDeleteOnClose) { return 0; }

void FileMan_Close(HWFILE hFile) {}

BOOLEAN FileMan_Read(HWFILE hFile, PTR pDest, uint32_t uiBytesToRead, uint32_t *puiBytesRead) {
  return FALSE;
}

BOOLEAN FileMan_Write(HWFILE hFile, PTR pDest, uint32_t uiBytesToWrite, uint32_t *puiBytesWritten) {
  return FALSE;
}

BOOLEAN FileMan_Seek(HWFILE hFile, uint32_t uiDistance, uint8_t uiHow) { return FALSE; }

INT32 FileMan_GetPos(HWFILE hFile) { return 0; }

uint32_t FileMan_GetSize(HWFILE hFile) { return 0; }

BOOLEAN Plat_DirectoryExists(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_RemoveDirectory(const char *pcDirectory, BOOLEAN fRecursive) { return FALSE; }

BOOLEAN Plat_EraseDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct) { return FALSE; }

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) { return FALSE; }

void Plat_GetFileClose(struct GetFile *pGFStruct) {}

BOOLEAN Plat_ClearFileAttributes(STR strFilename) { return FALSE; }

BOOLEAN FileMan_CheckEndOfFile(HWFILE hFile) { return FALSE; }

BOOLEAN FileMan_GetFileWriteTime(HWFILE hFile, uint64_t *pLastWriteTime) { return FALSE; }

uint32_t FileMan_Size(STR strFilename) { return 0; }

void Plat_FileBaseName(const char *path, char *outputBuf, u32 bufSize) { outputBuf[0] = 0; }

BOOLEAN Plat_FileEntityExists(const char *path) { return FALSE; }

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

void RestrictMouseCursor(SGPRect *pRectangle) {}

void FreeMouseCursor(void) {}

void GetRestrictedClipCursor(SGPRect *pRectangle) {}

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

void SetCustomizableTimerCallbackAndDelay(INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace) {}

void CheckCustomizableTimer(void) {}

/////////////////////////////////////////////////////////////////////////////////
// Sound
/////////////////////////////////////////////////////////////////////////////////

uint32_t SoundPlay(STR pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayStreamedFile(STR pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayRandom(STR pFilename, RANDOMPARMS *pParms) { return (SOUND_ERROR); }

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

uint32_t SoundLoadSample(STR pFilename) { return (NO_SAMPLE); }

uint32_t SoundLockSample(STR pFilename) { return (NO_SAMPLE); }

uint32_t SoundUnlockSample(STR pFilename) { return (NO_SAMPLE); }

void SoundRemoveSampleFlags(uint32_t uiSample, uint32_t uiFlags) {}

/////////////////////////////////////////////////////////////////////////////////
// Video
/////////////////////////////////////////////////////////////////////////////////

struct VSurface *ghFrameBuffer = NULL;
INT32 giNumFrames = 0;

void InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom) {}

void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, uint32_t uiFlags) {}

void InvalidateScreen(void) {}

void RefreshScreen(void *DummyVariable) {}

BOOLEAN GetPrimaryRGBDistributionMasks(uint32_t *RedBitMask, uint32_t *GreenBitMask,
                                       uint32_t *BlueBitMask) {
  return FALSE;
}

BOOLEAN EraseMouseCursor() { return FALSE; }

BOOLEAN SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, uint16_t usCursorHeight,
                                 uint16_t usCursorWidth) {
  return FALSE;
}

void DirtyCursor() {}

BOOLEAN SetCurrentCursor(uint16_t usVideoObjectSubIndex, uint16_t usOffsetX, uint16_t usOffsetY) {
  return FALSE;
}

void StartFrameBufferRender(void) {}

void EndFrameBufferRender(void) {}

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) { return FALSE; }

void FatalError(STR8 pError, ...) {}

uint32_t guiVSurfaceSize = 0;

BOOLEAN AddStandardVideoSurface(VSURFACE_DESC *pVSurfaceDesc, uint32_t *puiIndex) { return FALSE; };

BYTE *LockVideoSurface(uint32_t uiVSurface, uint32_t *puiPitch) { return NULL; }

void UnLockVideoSurface(uint32_t uiVSurface) {}

BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor) { return FALSE; }

BOOLEAN GetVideoSurface(struct VSurface **hVSurface, uint32_t uiIndex) { return FALSE; }

BOOLEAN BltVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, uint16_t usRegionIndex,
                        INT32 iDestX, INT32 iDestY, uint32_t fBltFlags, blt_vs_fx *pBltFx) {
  return FALSE;
}

BOOLEAN ColorFillVideoSurfaceArea(uint32_t uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, uint16_t Color16BPP) {
  return FALSE;
}

BOOLEAN ImageFillVideoSurfaceArea(uint32_t uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, INT16 Ox, INT16 Oy) {
  return FALSE;
}

struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc) { return NULL; }

BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  return FALSE;
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  return FALSE;
}

BOOLEAN DeleteVideoSurfaceFromIndex(uint32_t uiIndex) { return FALSE; }

BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) { return FALSE; }

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      uint16_t usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      blt_vs_fx *pBltFx) {
  return FALSE;
}

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           uint32_t fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceRect(uint32_t uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(uint32_t uiDestVSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2) {
  return FALSE;
}

BOOLEAN BltStretchVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, INT32 iDestX,
                               INT32 iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceImage(uint32_t uiDestVSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY) {
  return FALSE;
}

void DumpVSurfaceInfoIntoFile(CHAR8 *filename, BOOLEAN fAppend) {}

BOOLEAN _AddAndRecordVSurface(VSURFACE_DESC *VSurfaceDesc, uint32_t *uiIndex, uint32_t uiLineNum,
                              CHAR8 *pSourceFile) {
  return FALSE;
}

BOOLEAN SmkPollFlics(void) { return FALSE; }

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {}

void SmkShutdown(void) {}

struct SmkFlic *SmkPlayFlic(CHAR8 *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fClose) {
  return NULL;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {}

void SetWinFontForeColor(INT32 iFont, COLORVAL *pColor) {}

INT16 WinFontStringPixLength(STR16 string2, INT32 iFont) { return 0; }

INT16 GetWinFontHeight(STR16 string2, INT32 iFont) { return 0; }

uint32_t WinFont_mprintf(INT32 iFont, INT32 x, INT32 y, STR16 pFontString, ...) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
