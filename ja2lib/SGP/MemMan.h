#ifndef _MEMMAN_H
#define _MEMMAN_H

#include "SGP/Types.h"

extern BOOLEAN InitializeMemoryManager(void);
extern void ShutdownMemoryManager(void);

void* MemAlloc(size_t size);
void MemFree(void* ptr);
void* MemRealloc(void* ptr, size_t size);

// Allocate memory and zero it.
void* zmalloc(size_t size);

#endif
