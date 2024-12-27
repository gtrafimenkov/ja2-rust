#ifndef RADMALWI
#define RADMALWI

#ifdef __RADWINEXT__

uint32_t GetLimit(uint16_t sel);
#pragma aux GetLimit = "lsl eax,ax" parm[ax];

RADEXPFUNC void PTR4* RADEXPLINK radmalloc(uint32_t numbytes) {
  uint32_t temp;
  HGLOBAL handle;
  if ((numbytes == 0) || (numbytes == 0xffffffff)) return (0);

  handle = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, numbytes + 4);
  if (handle) {
    GlobalFix(handle);
    temp = (uint32_t)GlobalLock(handle);
    temp = ((uint32_t)_16To32(temp));
    ((uint32_t PTR4*)temp)[0] = handle;
    return (((uint32_t PTR4*)temp) + 1);
  }
  return (0);
}

RADEXPFUNC void RADEXPLINK radfree(void PTR4* ptr) {
  HGLOBAL h = ((uint32_t PTR4*)ptr)[-1];
  GlobalUnfix(h);
  GlobalUnlock(h);
  GlobalFree(h);
}

#else

#include "radmal.i"

#endif

#endif
