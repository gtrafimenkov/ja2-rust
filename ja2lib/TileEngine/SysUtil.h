#ifndef __SYSTEM_UTILS
#define __SYSTEM_UTILS

#include "SGP/Font.h"
#include "Tactical/Interface.h"
#include "TileEngine/IsometricUtils.h"

// Global variables for video objects
extern uint32_t guiBOTTOMPANEL;
extern uint32_t guiRIGHTPANEL;
extern uint32_t guiRENDERBUFFER;
extern uint32_t guiSAVEBUFFER;
extern uint32_t guiEXTRABUFFER;

extern BOOLEAN gfExtraBuffer;

BOOLEAN InitializeSystemVideoObjects();
BOOLEAN InitializeGameVideoObjects();

#endif
