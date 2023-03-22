#include "SGP/MemMan.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BOOLEAN InitializeMemoryManager(void) { return (TRUE); }

void ShutdownMemoryManager(void) {}

void* zmalloc(size_t size) {
  void* p = malloc(size);
  if (p) {
    memset(p, 0, size);
  }
  return p;
}
