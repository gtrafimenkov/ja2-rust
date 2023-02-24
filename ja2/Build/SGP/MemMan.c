#include "SGP/MemMan.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MessageBoxScreen.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

//**************************************************************************
//
//				Variables
//
//**************************************************************************

STR16 gzJA2ScreenNames[] = {
    L"EDIT_SCREEN",        L"SAVING_SCREEN",
    L"LOADING_SCREEN",     L"ERROR_SCREEN",
    L"INIT_SCREEN",        L"GAME_SCREEN",
    L"ANIEDIT_SCREEN",     L"PALEDIT_SCREEN",
    L"DEBUG_SCREEN",       L"MAP_SCREEN",
    L"LAPTOP_SCREEN",      L"LOADSAVE_SCREEN",
    L"MAPUTILITY_SCREEN",  L"FADE_SCREEN",
    L"MSG_BOX_SCREEN",     L"MAINMENU_SCREEN",
    L"AUTORESOLVE_SCREEN", L"SAVE_LOAD_SCREEN",
    L"OPTIONS_SCREEN",     L"SHOPKEEPER_SCREEN",
    L"SEX_SCREEN",         L"GAME_INIT_OPTIONS_SCREEN",
    L"DEMO_EXIT_SCREEN",   L"INTRO_SCREEN",
    L"CREDIT_SCREEN",
#ifdef JA2BETAVERSION
    L"AIVIEWER_SCREEN",    L"QUEST_DEBUG_SCREEN",
#endif
};

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

void DebugPrint(void);

//**************************************************************************
//
//				Functions
//
//**************************************************************************

BOOLEAN InitializeMemoryManager(void) { return (TRUE); }

void ShutdownMemoryManager(void) {}
