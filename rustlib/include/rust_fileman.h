#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Special FileID value meaning an error.
 */
#define FILE_ID_ERR 0

enum FileSeekMode {
  FILE_SEEK_START = 1,
  FILE_SEEK_END = 2,
  FILE_SEEK_CURRENT = 4,
};

/**
 * Identifier of an open file.  The file can be a regular file or
 * a file inside of an slf archive.  This is opaque to the user of the library.
 */
typedef uint32_t FileID;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Register slf libraries from directory `dir_path`.
 * `dir_path` must be utf-8 encoded string.
 * Return false, if there were any errors.
 */
bool File_RegisterSlfLibraries(const char *dir_path);

/**
 * Open file for reading.  `path` must be utf-8 encoded string.
 * The file can be a regular file or a file from a loaded slf archive.
 * If file is not found or any other error happened, return FILE_ID_ERR.
 */
FileID File_OpenForReading(const char *path);

/**
 * Open file for appending.  `path` must be utf-8 encoded string.
 * A recular file will be created in the process.
 * Writing to an slf library file is not supported.
 */
FileID File_OpenForAppending(const char *path);

/**
 * Open file for writing.  `path` must be utf-8 encoded string.
 * A recular file will be created in the process.
 * Writing to an slf library file is not supported.
 */
FileID File_OpenForWriting(const char *path);

/**
 * Read data from earlier opened file to the buffer.
 * Buffer must be no less than bytes_to_read in size.
 *
 * # Safety
 *
 * `bytes_read` can be null.
 */
bool File_Read(FileID file_id, void *buf, uint32_t bytes_to_read, uint32_t *bytes_read);

/**
 * Write data to an earlier opened file.
 *
 * # Safety
 *
 * `bytes_written` can be null.
 */
bool File_Write(FileID file_id, const void *buf, uint32_t bytes_to_write, uint32_t *bytes_written);

/**
 * Check if a file exists (regular or inside of slf library).
 */
bool File_Exists(const char *path);

/**
 * Get size of a file.  In case of an error, return 0.
 */
uint32_t File_GetSize(FileID file_id);

/**
 * Change file read or write position.
 */
bool File_Seek(FileID file_id, uint32_t distance, enum FileSeekMode how);

/**
 * Close earlie opened file.
 */
bool File_Close(FileID file_id);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus