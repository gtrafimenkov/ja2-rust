#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_

#include "SGP/Types.h"

//
//	Keeps track of the game version
//

extern const wchar_t* zBuildInfo;
extern CHAR8 czVersionNumber[16];
extern CHAR16 zTrackingNumber[16];

// #define RUSSIANGOLD

//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file
//

const extern uint32_t guiSavedGameVersion;

#endif
