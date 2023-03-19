#include <stdio.h>
#include <wchar.h>

#include "SGP/Ja2Libs.h"
#include "SGP/Types.h"
#include "rust_debug.h"
#include "rust_fileman.h"
#include "rust_platform.h"

int main() {
  struct Str512 CurrentDir;
  char DataDir[600];

  if (!Plat_GetExecutableDirectory(&CurrentDir)) {
    printf("error: failed to get the executable directory\n");
    return 1;
  };

  snprintf(DataDir, sizeof(DataDir), "%s/Data", CurrentDir.buf);
  printf("data dir: %s\n", DataDir);
  if (!Plat_SetCurrentDirectory(DataDir)) {
    printf("error: failed to switch to data dir\n");
    return 1;
  }

  int locale = DetectLocale();
  printf("locale:          %d\n", locale);
  printf("? exists cursors\\THROWB.STI:   %d\n", File_Exists("cursors\\THROWB.STI"));

  DebugLogWrite("hello there");
  DebugLogWrite("hello there again");
}
