#include <stdio.h>
#include <wchar.h>

#include "SGP/FileMan.h"
#include "SGP/Ja2Libs.h"
#include "SGP/LibraryDataBase.h"
#include "SGP/Types.h"

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
  InitializeFileDatabase();
  FileMan_Initialize();

  int locale = DetectLocale();
  printf("locale:          %d\n", locale);
  printf("? exists cursors\\THROWB.STI:   %d\n", FileMan_Exists("cursors\\THROWB.STI"));
}
