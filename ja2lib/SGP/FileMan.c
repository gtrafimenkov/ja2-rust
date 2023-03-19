#include "SGP/FileMan.h"

#include <stdio.h>

#include "SGP/Debug.h"
#include "SGP/LibraryDataBase.h"
#include "platform.h"
#include "rust_debug.h"
#include "rust_fileman.h"

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

BOOLEAN FileMan_Exists(STR strFilename) { return File_Exists(strFilename); }

HWFILE FileMan_OpenForWriting(const char *path) {
  return FileMan_Open(path, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);
}

HWFILE FileMan_OpenForAppending(const char *path) {
  return FileMan_Open(path, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);
}

HWFILE FileMan_OpenForReading(const char *path) {
  return FileMan_Open(path, FILE_ACCESS_READ | FILE_ACCESS_READ, FALSE);
}
