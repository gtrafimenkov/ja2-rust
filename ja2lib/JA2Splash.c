#include <stdio.h>

#include "MainMenuScreen.h"
#include "SGP/Debug.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/MultiLanguageGraphicUtils.h"
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

#ifdef ENGLISH
  ClearMainMenu();
#else
  {
    SGPFILENAME ImageFile;
    GetMLGFilename(ImageFile, MLG_SPLASH);
    struct Image *image = CreateImage(ImageFile, false);
    if (!image) {
      AssertMsg(0, String("Failed to load %s", ImageFile));
      return;
    }

    BlitImageToSurface(image, vsFB, 0, 0);
    DestroyImage(image);
  }
#endif

  InvalidateScreen();
  RefreshScreen();

  guiSplashStartTime = GetJA2Clock();
}
