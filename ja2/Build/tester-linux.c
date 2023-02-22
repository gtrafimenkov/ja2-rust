#include <stdio.h>

#include "SGP/Ja2Libs.h"
#include "SGP/LibraryDataBase.h"
#include "SGP/Types.h"
#include "fileman.h"

int main() {
  char CurrentDir[100];
  char DataDir[200];

  Plat_GetExecutableDirectory(CurrentDir, sizeof(CurrentDir));

  snprintf(DataDir, sizeof(DataDir), "%s/Data", CurrentDir);
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

// build:linux
