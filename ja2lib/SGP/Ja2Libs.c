// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Ja2Libs.h"

#include <stdio.h>
#include <string.h>

#include "platform.h"

int8_t gbLocale = ENGLISH_LANG;

char* LocaleNames[LANG_NUMBER] = {"default", "russian", "german", "dutch",
                                 "polish",  "french",  "italian"};

#define _MAX_PATH 260

int8_t DetectLocale() {
  gbLocale = ENGLISH_LANG;
  return gbLocale;

  // int8_t bLoc;
  // struct Str512 zPath;
  // char zLocalePath[_MAX_PATH];

  // if (!Plat_GetExecutableDirectory(&zPath)) {
  //   return ENGLISH_LANG;
  // }

  // strcpy(zPath.buf, "\\Data\\");

  // for (bLoc = RUSSIAN_LANG; bLoc < LANG_NUMBER; bLoc++) {
  //   strcpy(zLocalePath, zPath.buf);
  //   strcat(zLocalePath, LocaleNames[bLoc]);
  //   strcat(zLocalePath, ".slf");

  //   if (Plat_FileEntityExists(zLocalePath)) {
  //     gbLocale = bLoc;
  //     snprintf(gGameLibaries[LIBRARY_NATIONAL_DATA].sLibraryName,
  //              sizeof(gGameLibaries[LIBRARY_NATIONAL_DATA].sLibraryName), "%s.slf",
  //              LocaleNames[gbLocale]);
  //     break;
  //   }
  // }
  // return gbLocale;
}

// LibraryInitHeader gGameLibaries[] = {
//     // Library Name					Can be	Init at start
//     //
//     on
//     // cd
//     {"Data.slf", FALSE, TRUE},
//     {"Ambient.slf", FALSE, TRUE},
//     {"Anims.slf", FALSE, TRUE},
//     {"BattleSnds.slf", FALSE, TRUE},
//     {"BigItems.slf", FALSE, TRUE},
//     {"BinaryData.slf", FALSE, TRUE},
//     {"Cursors.slf", FALSE, TRUE},
//     {"Faces.slf", FALSE, TRUE},
//     {"Fonts.slf", FALSE, TRUE},
//     {"Interface.slf", FALSE, TRUE},
//     {"Laptop.slf", FALSE, TRUE},
//     {"Maps.slf", TRUE, TRUE},
//     {"MercEdt.slf", FALSE, TRUE},
//     {"Music.slf", TRUE, TRUE},
//     {"Npc_Speech.slf", TRUE, TRUE},
//     {"NpcData.slf", FALSE, TRUE},
//     {"RadarMaps.slf", FALSE, TRUE},
//     {"Sounds.slf", FALSE, TRUE},
//     {"Speech.slf", TRUE, TRUE},
//     //	{ "TileCache.slf",				FALSE, TRUE },
//     {"TileSets.slf", TRUE, TRUE},
//     {"LoadScreens.slf", TRUE, TRUE},
//     {"Intro.slf", TRUE, TRUE},

// #ifdef GERMAN
//     {"German.slf", FALSE, TRUE},
// #endif

// #ifdef POLISH
//     {"Polish.slf", FALSE, TRUE},
// #endif

// #ifdef DUTCH
//     {"Dutch.slf", FALSE, TRUE},
// #endif

// #ifdef ITALIAN
//     {"Italian.slf", FALSE, TRUE},
// #endif

// #ifdef RUSSIAN
//     {"Russian.slf", FALSE, TRUE},
// #endif

// };
