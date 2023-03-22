// OS-independent interface to the platform layer.

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "Rect.h"
#include "SGP/Types.h"
#include "rust_platform.h"

/////////////////////////////////////////////////////////////////////////////////
// Files
/////////////////////////////////////////////////////////////////////////////////

// Gets the amount of free space on the hard drive that the main executeablt is runnning from
uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

struct GetFile {
  int32_t iFindHandle;
  char zFileName[260];  // changed from uint16_t, Alex Meduna, Mar-20'98
  uint32_t uiFileSize;
  uint32_t uiFileAttribs;
};

BOOLEAN Plat_GetFileFirst(char *pSpec, struct GetFile *pGFStruct);
BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct);
void Plat_GetFileClose(struct GetFile *pGFStruct);
BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs);
BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs);

struct FileDialogList {
  struct GetFile FileInfo;
  struct FileDialogList *pNext;
  struct FileDialogList *pPrev;
};

/////////////////////////////////////////////////////////////////////////////////
// Timers
/////////////////////////////////////////////////////////////////////////////////

extern uint32_t Plat_GetTickCount();

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

extern void Plat_OnSGPExit();

/////////////////////////////////////////////////////////////////////////////////
// Mouse
/////////////////////////////////////////////////////////////////////////////////

void Plat_ClipCursor(const struct Rect *rect);

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

struct VSurface;
struct BltOpts;

#define MAX_DIRTY_REGIONS 128

BOOLEAN BltVSurfaceUsingDDBlt(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                              uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                              struct Rect *SrcRect, struct Rect *DestRect);

extern BOOLEAN FillSurfaceRect(struct VSurface *hDestVSurface, struct BltOpts *pBltFx);
extern BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                  uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                                  struct Rect *SrcRect);

#endif
