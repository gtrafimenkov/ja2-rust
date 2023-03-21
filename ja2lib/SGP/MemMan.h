#ifndef _MEMMAN_H
#define _MEMMAN_H

#include <malloc.h>

#include "SGP/Types.h"

extern BOOLEAN InitializeMemoryManager(void);
extern void ShutdownMemoryManager(void);

#define MemAlloc(size) malloc((size))
#define MemFree(ptr) free((ptr))
#define MemRealloc(ptr, size) realloc((ptr), (size))

// Allocate memory and zero it.
void* zmalloc(size_t size);

#endif
