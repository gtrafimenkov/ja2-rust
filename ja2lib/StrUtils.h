#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <stddef.h>

// Copy a string safely.
// 0 character at the end of dest is always added.
extern void strcopy(char *dest, size_t destSize, const char *src);

#endif
