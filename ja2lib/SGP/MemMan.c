#include "SGP/MemMan.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rust_alloc.h"

BOOLEAN InitializeMemoryManager(void) { return (TRUE); }

void ShutdownMemoryManager(void) {}

void* MemAlloc(size_t size) { return malloc(size); }
void MemFree(void* ptr) { free(ptr); };
void* MemRealloc(void* ptr, size_t size) { return realloc(ptr, size); }

void* zmalloc(size_t size) {
  void* p = malloc(size);
  if (p) {
    memset(p, 0, size);
  }
  return p;
}
