#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Change the current working directory.
 * The path string must be utf8 encoded.
 */
bool Plat_SetCurrentDirectory(const char *path_utf8);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
