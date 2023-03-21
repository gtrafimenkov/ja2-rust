#include <stdio.h>

#include "MainMenuScreen.h"
#include "SGP/Debug.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/TimerControl.h"
#include "platform.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_platform.h"

UINT32 guiSplashFrameFade = 10;
UINT32 guiSplashStartTime = 0;

// Simply create videosurface, load image, and draw it to the screen.
void InitJA2SplashScreen() {
  struct Str512 CurrentDir;
  char DataDir[600];

  InitializeJA2Clock();

  // Get Executable Directory
  if (!Plat_GetExecutableDirectory(&CurrentDir)) {
    DebugMsg(TOPIC_JA2, DBG_INFO, "Could not find the executable directory, shutting down");
    return;
  }

  // Adjust Current Dir
  snprintf(DataDir, ARR_SIZE(DataDir), "%s\\Data", CurrentDir.buf);
  if (!Plat_SetCurrentDirectory(DataDir)) {
    DebugMsg(TOPIC_JA2, DBG_INFO, "Could not find data directory, shutting down");
    return;
  }

  File_RegisterSlfLibraries(".");

#if !defined(ENGLISH) && defined(JA2TESTVERSION)
  UINT32 uiLogoID = 0;
  VSURFACE_DESC VSurfaceDesc;
  struct VSurface* hVSurface;
  memset(&VSurfaceDesc, 0, sizeof(VSURFACE_DESC));
  VSurfaceDesc.fCreateFlags = VSURFACE_CREATE_FROMFILE;
  sprintf(VSurfaceDesc.ImageFile, "LOADSCREENS\\Notification.sti");
  if (!AddVideoSurface(&VSurfaceDesc, &uiLogoID)) {
    AssertMsg(0, String("Failed to load %s", VSurfaceDesc.ImageFile));
    return;
  }
  GetVideoSurface(&hVSurface, uiLogoID);
  BltVideoSurfaceToVideoSurface(vsFrameBuffer, hVSurface, 0, 0, 0, NULL);
  DeleteVideoSurfaceFromIndex(uiLogoID);

  InvalidateScreen();
  RefreshScreen();

  guiSplashStartTime = GetJA2Clock();
  while (i < 60 * 15)  // guiSplashStartTime + 15000 > GetJA2Clock() )
  {
    // Allow the user to pick his bum.
    InvalidateScreen();
    RefreshScreen();
    i++;
  }
#endif

#ifdef ENGLISH
  ClearMainMenu();
#else
  {
    memset(&VSurfaceDesc, 0, sizeof(VSURFACE_DESC));
    VSurfaceDesc.fCreateFlags = VSURFACE_CREATE_FROMFILE;
    GetMLGFilename(VSurfaceDesc.ImageFile, MLG_SPLASH);
    if (!AddVideoSurface(&VSurfaceDesc, &uiLogoID)) {
      AssertMsg(0, String("Failed to load %s", VSurfaceDesc.ImageFile));
      return;
    }

    GetVideoSurface(&hVSurface, uiLogoID);
    BltVideoSurfaceToVideoSurface(vsFrameBuffer, hVSurface, 0, 0, 0, NULL);
    DeleteVideoSurfaceFromIndex(uiLogoID);
  }
#endif

  InvalidateScreen();
  RefreshScreen();

  guiSplashStartTime = GetJA2Clock();
}
