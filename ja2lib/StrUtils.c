#include "StrUtils.h"

#include <string.h>

// Copy a string safely.
// 0 character at the end of dest is always added.
void strcopy(char *dest, size_t destSize, const char *src) {
  if (destSize > 0) {
    strncpy(dest, src, destSize);
    dest[destSize - 1] = 0;
  }
}
