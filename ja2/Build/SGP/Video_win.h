#ifndef __VIDEO_WIN_H
#define __VIDEO_WIN_H

#include <windows.h>

#include "SGP/Types.h"

extern HWND ghWindow;

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN InitializeVideoManager(HINSTANCE hInstance, UINT16 usCommandShow, void *WindowProc);

#ifdef __cplusplus
}
#endif

#endif
