// OS-independent interface to the platform layer.

#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "rust_geometry.h"
#include "SGP/Types.h"
#include "rust_platform.h"

/////////////////////////////////////////////////////////////////////////////////
// Files
/////////////////////////////////////////////////////////////////////////////////

// Gets the amount of free space on the hard drive that the main executeablt is runnning from
UINT32 Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

struct GetFile {
  INT32 iFindHandle;
  CHAR8 zFileName[260];  // changed from UINT16, Alex Meduna, Mar-20'98
  UINT32 uiFileSize;
  UINT32 uiFileAttribs;
};

BOOLEAN Plat_GetFileFirst(CHAR8 *pSpec, struct GetFile *pGFStruct);
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

extern u32 Plat_GetTickCount();

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
                              UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, struct Rect *SrcRect,
                              struct Rect *DestRect);

extern BOOLEAN FillSurfaceRect(struct VSurface *hDestVSurface, struct BltOpts *pBltFx);
extern BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                  UINT32 fBltFlags, INT32 iDestX, INT32 iDestY,
                                  struct Rect *SrcRect);

#endif
