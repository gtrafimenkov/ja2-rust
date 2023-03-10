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

UINT32 GetClock(void) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfProgramIsRunning;

BOOLEAN Plat_GetCurrentDirectory(STRING512 pcDirectory) { return FALSE; }

BOOLEAN Plat_DeleteFile(const char *filename) { return FALSE; }

u32 Plat_GetFileSize(SYS_FILE_HANDLE handle) { return 0; }

BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, u32 bytesToRead, u32 *readBytes) {
  return FALSE;
}

void Plat_CloseFile(SYS_FILE_HANDLE handle) {}

BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle) { return FALSE; }

u32 Plat_SetFilePointer(SYS_FILE_HANDLE handle, i32 distance, int seekType) { return 0; }

UINT32 Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() { return 0; }

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) { return FALSE; }

HWFILE FileMan_Open(STR strFilename, UINT32 uiOptions, BOOLEAN fDeleteOnClose) { return 0; }

void FileMan_Close(HWFILE hFile) {}

BOOLEAN FileMan_Read(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead) {
  return FALSE;
}

BOOLEAN FileMan_Write(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite, UINT32 *puiBytesWritten) {
  return FALSE;
}

BOOLEAN FileMan_Seek(HWFILE hFile, UINT32 uiDistance, UINT8 uiHow) { return FALSE; }

INT32 FileMan_GetPos(HWFILE hFile) { return 0; }

UINT32 FileMan_GetSize(HWFILE hFile) { return 0; }

BOOLEAN Plat_RemoveDirectory(const char *pcDirectory, BOOLEAN fRecursive) { return FALSE; }

BOOLEAN Plat_EraseDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct) { return FALSE; }

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) { return FALSE; }

void Plat_GetFileClose(struct GetFile *pGFStruct) {}

BOOLEAN Plat_ClearFileAttributes(STR strFilename) { return FALSE; }

BOOLEAN FileMan_CheckEndOfFile(HWFILE hFile) { return FALSE; }

BOOLEAN FileMan_GetFileWriteTime(HWFILE hFile, uint64_t *pLastWriteTime) { return FALSE; }

UINT32 FileMan_Size(STR strFilename) { return 0; }

void Plat_FileBaseName(const char *path, char *outputBuf, u32 bufSize) { outputBuf[0] = 0; }

BOOLEAN Plat_FileEntityExists(const char *path) { return FALSE; }

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

struct VSurface *ghFrameBuffer = NULL;
INT32 giNumFrames = 0;

void InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom) {}

void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, UINT32 uiFlags) {}

void InvalidateScreen(void) {}

void RefreshScreen(void *DummyVariable) {}

BOOLEAN GetPrimaryRGBDistributionMasks(UINT32 *RedBitMask, UINT32 *GreenBitMask,
                                       UINT32 *BlueBitMask) {
  return FALSE;
}

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

UINT32 guiVSurfaceSize = 0;

BOOLEAN AddStandardVideoSurface(VSURFACE_DESC *pVSurfaceDesc, UINT32 *puiIndex) { return FALSE; };

BYTE *LockVideoSurface(UINT32 uiVSurface, UINT32 *puiPitch) { return NULL; }

void UnLockVideoSurface(UINT32 uiVSurface) {}

BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor) { return FALSE; }

BOOLEAN GetVideoSurface(struct VSurface **hVSurface, UINT32 uiIndex) { return FALSE; }

BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, UINT16 usRegionIndex,
                        INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_vs_fx *pBltFx) {
  return FALSE;
}

BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP) {
  return FALSE;
}

BOOLEAN ImageFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1,
                                  INT32 iDestX2, INT32 iDestY2, struct VObject *BkgrndImg,
                                  UINT16 Index, INT16 Ox, INT16 Oy) {
  return FALSE;
}

struct VSurface *CreateVideoSurface(VSURFACE_DESC *VSurfaceDesc) { return NULL; }

BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  return FALSE;
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  return FALSE;
}

BOOLEAN DeleteVideoSurfaceFromIndex(UINT32 uiIndex) { return FALSE; }

BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) { return FALSE; }

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags,
                                      blt_vs_fx *pBltFx) {
  return FALSE;
}

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceRect(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(UINT32 uiDestVSurface, INT32 X1, INT32 Y1,
                                                   INT32 X2, INT32 Y2) {
  return FALSE;
}

BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX,
                               INT32 iDestY, UINT32 fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect) {
  return FALSE;
}

BOOLEAN ShadowVideoSurfaceImage(UINT32 uiDestVSurface, struct VObject *hImageHandle, INT32 iPosX,
                                INT32 iPosY) {
  return FALSE;
}

void DumpVSurfaceInfoIntoFile(CHAR8 *filename, BOOLEAN fAppend) {}

BOOLEAN _AddAndRecordVSurface(VSURFACE_DESC *VSurfaceDesc, UINT32 *uiIndex, UINT32 uiLineNum,
                              CHAR8 *pSourceFile) {
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
