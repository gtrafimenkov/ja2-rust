//**************************************************************************
//
// Filename :	MemMan.h
//
//	Purpose :	prototypes for the memory manager
//
// Modification history :
//
//		11sep96:HJH				- Creation
//
//**************************************************************************

#ifndef _MEMMAN_H
#define _MEMMAN_H

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "SGP/Types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN InitializeMemoryManager(void);
extern void ShutdownMemoryManager(void);

#include <malloc.h>
#define MemAlloc(size) malloc((size))
#define MemFree(ptr) free((ptr))
#define MemRealloc(ptr, size) realloc((ptr), (size))

#ifdef __cplusplus
}
#endif

#endif
