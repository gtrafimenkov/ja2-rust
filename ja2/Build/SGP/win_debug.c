#include <windows.h>

#include "SGP/Debug.h"

void DebugPrint(const char* message) { OutputDebugStringA(message); }
