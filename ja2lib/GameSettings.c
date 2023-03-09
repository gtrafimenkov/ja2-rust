#include "GameSettings.h"

#include <string.h>

#include "Cheats.h"
#include "GameVersion.h"
#include "HelpScreen.h"
#include "LanguageDefines.h"
#include "OptionsScreen.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SaveLoadGame.h"
#include "SaveLoadScreen.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Campaign.h"
#include "Tactical/Overhead.h"
#include "TileEngine/ShadeTableUtil.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"

#define GAME_SETTINGS_FILE "..\\Ja2.set"

#define GAME_INI_FILE "..\\Ja2.ini"

#define CD_ROOT_DIR "DATA\\"

GAME_SETTINGS gGameSettings;
GAME_OPTIONS gGameOptions;

extern SGPFILENAME gCheckFilenames[];

void InitGameSettings();

// Change this number when we want any who gets the new build to reset the options
#define GAME_SETTING_CURRENT_VERSION 522

BOOLEAN LoadGameSettings() {
  HWFILE hFile;
  UINT32 uiNumBytesRead;

  // if the game settings file does NOT exist, or if it is smaller then what it should be
  if (!FileMan_Exists(GAME_SETTINGS_FILE) ||
      FileMan_Size(GAME_SETTINGS_FILE) != sizeof(GAME_SETTINGS)) {
    // Initialize the settings
    InitGameSettings();

    // delete the shade tables aswell
    DeleteShadeTableDir();
  } else {
    hFile = FileMan_Open(GAME_SETTINGS_FILE, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);
    if (!hFile) {
      FileMan_Close(hFile);
      InitGameSettings();
      return (FALSE);
    }

    FileMan_Read(hFile, &gGameSettings, sizeof(GAME_SETTINGS), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(GAME_SETTINGS)) {
      FileMan_Close(hFile);
      InitGameSettings();
      return (FALSE);
    }

    FileMan_Close(hFile);
  }

  // if the version in the game setting file is older then the we want, init the game settings
  if (gGameSettings.uiSettingsVersionNumber < GAME_SETTING_CURRENT_VERSION) {
    // Initialize the settings
    InitGameSettings();

    // delete the shade tables aswell
    DeleteShadeTableDir();

    return (TRUE);
  }

  //
  // Do checking to make sure the settings are valid
  //
  if (gGameSettings.bLastSavedGameSlot < 0 || gGameSettings.bLastSavedGameSlot >= NUM_SAVE_GAMES)
    gGameSettings.bLastSavedGameSlot = -1;

  if (gGameSettings.ubMusicVolumeSetting > HIGHVOLUME)
    gGameSettings.ubMusicVolumeSetting = MIDVOLUME;

  if (gGameSettings.ubSoundEffectsVolume > HIGHVOLUME)
    gGameSettings.ubSoundEffectsVolume = MIDVOLUME;

  if (gGameSettings.ubSpeechVolume > HIGHVOLUME) gGameSettings.ubSpeechVolume = MIDVOLUME;

  // make sure that at least subtitles or speech is enabled
  if (!gGameSettings.fOptions[TOPTION_SUBTITLES] && !gGameSettings.fOptions[TOPTION_SPEECH]) {
    gGameSettings.fOptions[TOPTION_SUBTITLES] = TRUE;
    gGameSettings.fOptions[TOPTION_SPEECH] = TRUE;
  }

  //
  //	Set the settings
  //

  SetSoundEffectsVolume(gGameSettings.ubSoundEffectsVolume);
  SetSpeechVolume(gGameSettings.ubSpeechVolume);
  MusicSetVolume(gGameSettings.ubMusicVolumeSetting);

#ifndef BLOOD_N_GORE_ENABLED
  gGameSettings.fOptions[TOPTION_BLOOD_N_GORE] = FALSE;
#endif

  // if the user doesnt want the help screens present
  if (gGameSettings.fHideHelpInAllScreens) {
    gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = 0;
  } else {
    // Set it so that every screens help will come up the first time ( the 'x' will be set )
    gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = 0xffff;
  }

  return (TRUE);
}

BOOLEAN SaveGameSettings() {
  HWFILE hFile;
  UINT32 uiNumBytesWritten;

  // create the file
  hFile = FileMan_Open(GAME_SETTINGS_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);
  if (!hFile) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Record the current settings into the game settins structure

  gGameSettings.ubSoundEffectsVolume = (UINT8)GetSoundEffectsVolume();
  gGameSettings.ubSpeechVolume = (UINT8)GetSpeechVolume();
  gGameSettings.ubMusicVolumeSetting = (UINT8)MusicGetVolume();

  strcpy(gGameSettings.zVersionNumber, czVersionNumber);

  gGameSettings.uiSettingsVersionNumber = GAME_SETTING_CURRENT_VERSION;

  // Write the game settings to disk
  FileMan_Write(hFile, &gGameSettings, sizeof(GAME_SETTINGS), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(GAME_SETTINGS)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  FileMan_Close(hFile);

  return (TRUE);
}

void InitGameSettings() {
  memset(&gGameSettings, 0, sizeof(GAME_SETTINGS));

  // Init the Game Settings
  gGameSettings.bLastSavedGameSlot = -1;
  gGameSettings.ubMusicVolumeSetting = 63;
  gGameSettings.ubSoundEffectsVolume = 63;
  gGameSettings.ubSpeechVolume = 63;

  // Set the settings
  SetSoundEffectsVolume(gGameSettings.ubSoundEffectsVolume);
  SetSpeechVolume(gGameSettings.ubSpeechVolume);
  MusicSetVolume(gGameSettings.ubMusicVolumeSetting);

  gGameSettings.fOptions[TOPTION_SUBTITLES] = TRUE;
  gGameSettings.fOptions[TOPTION_SPEECH] = TRUE;
  gGameSettings.fOptions[TOPTION_KEY_ADVANCE_SPEECH] = FALSE;
  gGameSettings.fOptions[TOPTION_RTCONFIRM] = FALSE;
  gGameSettings.fOptions[TOPTION_HIDE_BULLETS] = FALSE;
  gGameSettings.fOptions[TOPTION_TRACKING_MODE] = TRUE;
  gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS] = FALSE;
  gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE] = TRUE;
  gGameSettings.fOptions[TOPTION_BLOOD_N_GORE] = TRUE;
  gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE] = FALSE;
  gGameSettings.fOptions[TOPTION_OLD_SELECTION_METHOD] = FALSE;
  gGameSettings.fOptions[TOPTION_ALWAYS_SHOW_MOVEMENT_PATH] = FALSE;

  gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION] = TRUE;

  gGameSettings.fOptions[TOPTION_USE_METRIC_SYSTEM] = FALSE;

#ifndef BLOOD_N_GORE_ENABLED
  gGameSettings.fOptions[TOPTION_BLOOD_N_GORE] = FALSE;
#endif

  gGameSettings.fOptions[TOPTION_MERC_ALWAYS_LIGHT_UP] = FALSE;
  gGameSettings.fOptions[TOPTION_SMART_CURSOR] = FALSE;

  gGameSettings.fOptions[TOPTION_SNAP_CURSOR_TO_DOOR] = TRUE;
  gGameSettings.fOptions[TOPTION_GLOW_ITEMS] = TRUE;
  gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS] = TRUE;
  gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME] = TRUE;
  gGameSettings.fOptions[TOPTION_3D_CURSOR] = FALSE;
  // JA2Gold
  gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT] = TRUE;

  gGameSettings.ubSizeOfDisplayCover = 4;
  gGameSettings.ubSizeOfLOS = 4;

  // Since we just set the settings, save them
  SaveGameSettings();
}

void InitGameOptions() {
  memset(&gGameOptions, 0, sizeof(GAME_OPTIONS));

  // Init the game options
  gGameOptions.fGunNut = FALSE;
  gGameOptions.fSciFi = TRUE;
  gGameOptions.ubDifficultyLevel = DIF_LEVEL_EASY;
  // gGameOptions.fTurnTimeLimit		 = FALSE;
  gGameOptions.fIronManMode = FALSE;
}

void DisplayGameSettings() {
  // Display the version number
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", pMessageStrings[MSG_VERSION],
            zBuildInfo);

  // Display the difficulty level
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[GIO_DIF_LEVEL_TEXT],
            gzGIOScreenText[gGameOptions.ubDifficultyLevel + GIO_EASY_TEXT - 1]);

  // Iron man option
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s",
            gzGIOScreenText[GIO_GAME_SAVE_STYLE_TEXT],
            gzGIOScreenText[GIO_SAVE_ANYWHERE_TEXT + gGameOptions.fIronManMode]);

  // Gun option
  if (gGameOptions.fGunNut)
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[GIO_GUN_OPTIONS_TEXT],
              gzGIOScreenText[GIO_GUN_NUT_TEXT]);
  else
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[GIO_GUN_OPTIONS_TEXT],
              gzGIOScreenText[GIO_REDUCED_GUNS_TEXT]);

  // Sci fi option
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[GIO_GAME_STYLE_TEXT],
            gzGIOScreenText[GIO_REALISTIC_TEXT + gGameOptions.fSciFi]);

  // Timed Turns option
  // JA2Gold: no timed turns
  // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s: %s", gzGIOScreenText[
  // GIO_TIMED_TURN_TITLE_TEXT ], gzGIOScreenText[ GIO_NO_TIMED_TURNS_TEXT +
  // gGameOptions.fTurnTimeLimit ] );

  if (CHEATER_CHEAT_LEVEL()) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[58],
              CurrentPlayerProgressPercentage(), HighestPlayerProgressPercentage());
  }
}

BOOLEAN MeanwhileSceneSeen(UINT8 ubMeanwhile) {
  UINT32 uiCheckFlag;

  if (ubMeanwhile > 32 || ubMeanwhile > NUM_MEANWHILES) {
    return (FALSE);
  }

  uiCheckFlag = 0x1 << ubMeanwhile;

  if (gGameSettings.uiMeanwhileScenesSeenFlags & uiCheckFlag) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN SetMeanwhileSceneSeen(UINT8 ubMeanwhile) {
  UINT32 uiCheckFlag;

  if (ubMeanwhile > 32 || ubMeanwhile > NUM_MEANWHILES) {
    // can't set such a flag!
    return (FALSE);
  }
  uiCheckFlag = 0x1 << ubMeanwhile;
  gGameSettings.uiMeanwhileScenesSeenFlags |= uiCheckFlag;
  return (TRUE);
}

BOOLEAN CanGameBeSaved() {
  // if the iron man mode is on
  if (gGameOptions.fIronManMode) {
    // if we are in turn based combat
    if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) {
      // no save for you
      return (FALSE);
    }

    // if there are enemies in the current sector
    if (gWorldSectorX != -1 && gWorldSectorY != -1 && gWorldSectorX != 0 && gWorldSectorY != 0 &&
        NumEnemiesInAnySector((u8)gWorldSectorX, (u8)gWorldSectorY, gbWorldSectorZ) > 0) {
      // no save for you
      return (FALSE);
    }

    // All checks failed, so we can save
    return (TRUE);
  } else {
    return (TRUE);
  }
}
