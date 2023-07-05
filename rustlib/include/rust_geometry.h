#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Copy of SGPRect from the C codebase.
 */
struct GRect {
  int32_t iLeft;
  int32_t iTop;
  int32_t iRight;
  int32_t iBottom;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Create new SGPRect structure
 */
struct GRect NewRect(int32_t left, int32_t top, int32_t width, int32_t height);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
