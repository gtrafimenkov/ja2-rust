#include "GameVersion.h"

#include "BuildInfo.h"
#include "SGP/Types.h"

//
//	Keeps track of the game version
//

const wchar_t* zBuildInfo = L"JA2 Rust (" BUILD_INFO ")";

// RELEASE BUILD VERSION
CHAR8 czVersionNumber[16] = {"Build 04.12.02"};
CHAR16 zTrackingNumber[16] = {L"Z"};

//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define SAVE_GAME_VERSION 99

const UINT32 guiSavedGameVersion = SAVE_GAME_VERSION;
