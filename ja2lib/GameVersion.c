// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameVersion.h"

#include "BuildInfo.h"
#include "SGP/Types.h"

//
//	Keeps track of the game version
//

const wchar_t* zBuildInfo = L"JA2 Rust (" BUILD_INFO ")";

// RELEASE BUILD VERSION
char czVersionNumber[16] = {"Build 04.12.02"};
wchar_t zTrackingNumber[16] = {L"Z"};

//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define SAVE_GAME_VERSION 99

const uint32_t guiSavedGameVersion = SAVE_GAME_VERSION;
