#ifndef _EXCEPTION_HANDLING__H_
#define _EXCEPTION_HANDLING__H_

#include <windows.h>

#include "SGP/Types.h"

#ifndef _DEBUG
#define ENABLE_EXCEPTION_HANDLING
#endif

#ifdef __cplusplus
extern "C" {
#endif

INT32 RecordExceptionInfo(EXCEPTION_POINTERS *pExceptInfo);

#ifdef __cplusplus
}
#endif

#endif
