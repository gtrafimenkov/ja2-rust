#include "SGP/FileMan.h"

#include <stdio.h>

#include "SGP/Debug.h"
#include "SGP/LibraryDataBase.h"
#include "platform.h"

// TODO: all FileMan_ functions implementations should go here

BOOLEAN FileMan_Initialize() {
  RegisterDebugTopic(TOPIC_FILE_MANAGER, "File Manager");
  return (TRUE);
}

void FileMan_Shutdown(void) { UnRegisterDebugTopic(TOPIC_FILE_MANAGER, "File Manager"); }

BOOLEAN FileMan_ExistsNoDB(STR strFilename) {
  BOOLEAN fExists = FALSE;
  FILE *file;

  // open up the file to see if it exists on the disk
  file = fopen(strFilename, "r");
  if (file) {
    fExists = TRUE;
    fclose(file);
  }

  return (fExists);
}

BOOLEAN FileMan_Delete(STR strFilename) { return (Plat_DeleteFile(strFilename)); }

BOOLEAN FileMan_Exists(STR strFilename) {
  BOOLEAN fExists = FALSE;
  FILE *file;

  // open up the file to see if it exists on the disk
  file = fopen(strFilename, "r");
  if (file) {
    fExists = TRUE;
    fclose(file);
  }

  // if the file wasnt on disk, check to see if its in a library
  if (fExists == FALSE) {
    // if the database is initialized
    if (gFileDataBase.fInitialized) fExists = CheckIfFileExistInLibrary(strFilename);
  }

  return (fExists);
}
