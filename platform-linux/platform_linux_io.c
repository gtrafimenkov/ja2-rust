// Implementation of the platform layer file operations on Linux.

#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "SGP/Types.h"
#include "StrUtils.h"
#include "platform.h"

BOOLEAN Plat_DeleteFile(const char *filename) { return unlink(filename) == 0; }

// Given a path, fill outputBuf with the file name.
void Plat_FileBaseName(const char *path, char *outputBuf, u32 bufSize) {
  char *copy = strdup(path);
  if (copy == NULL) {
    outputBuf[0] = 0;
    return;
  }
  const char *fileName = basename(copy);

  strcopy(outputBuf, bufSize, fileName);

  free(copy);
}

void Plat_CloseFile(SYS_FILE_HANDLE handle) {
  fclose(handle);
  // TODO: check for errors
}

BOOLEAN Plat_OpenForReading(const char *path, SYS_FILE_HANDLE *handle) {
  *handle = fopen(path, "rb");
  return *handle != NULL;
}

BOOLEAN Plat_ReadFile(SYS_FILE_HANDLE handle, void *buffer, u32 bytesToRead, u32 *readBytes) {
  *readBytes = fread(buffer, 1, bytesToRead, handle);
  int error = ferror(handle);
  // TODO: log error
  return error == 0;
}

u32 Plat_GetFileSize(SYS_FILE_HANDLE handle) {
  struct stat st;
  int fd = ((FILE *)handle)->_fileno;
  int ret = fstat(fd, &st);
  // TODO: proper check of errors
  if (ret != 0) {
    return 0;
  }
  return st.st_size;
}

// Check if file (or directory) exists.
BOOLEAN Plat_FileEntityExists(const char *path) {
  struct stat st;
  int ret = stat(path, &st);
  return ret == 0;
}

// Change file pointer.
// In case of an error returns 0xFFFFFFFF
u32 Plat_SetFilePointer(SYS_FILE_HANDLE handle, i32 distance, int seekType) {
  int whence;
  switch (seekType) {
    case FILE_SEEK_FROM_START:
      whence = SEEK_CUR;
      break;
    case FILE_SEEK_FROM_END:
      whence = SEEK_END;
      break;
    case FILE_SEEK_FROM_CURRENT:
      whence = SEEK_SET;
      break;
    default:
      return 0xFFFFFFFF;
  }
  int res = fseek(handle, distance, whence);
  return res;
}

BOOLEAN Plat_GetExecutableDirectory(char *buf, u16 bufSize) {
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

  if (count == -1) {
    buf[0] = 0;
    return FALSE;
  }

  const char *path = dirname(result);
  strcopy(buf, bufSize, path);
  return TRUE;
}

BOOLEAN Plat_SetCurrentDirectory(STR pcDirectory) { return chdir(pcDirectory) == 0; }
