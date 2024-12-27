// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _GAME_SETTINGS__H_
#define _GAME_SETTINGS__H_

#include "SGP/Types.h"

// If you add any options, MAKE sure you add the corresponding string to the Options Screen string
// array
enum {
  TOPTION_SPEECH,
  TOPTION_MUTE_CONFIRMATIONS,
  TOPTION_SUBTITLES,
  TOPTION_KEY_ADVANCE_SPEECH,
  TOPTION_ANIMATE_SMOKE,
  //	TOPTION_HIDE_BULLETS,
  //	TOPTION_CONFIRM_MOVE,
  TOPTION_BLOOD_N_GORE,
  TOPTION_DONT_MOVE_MOUSE,
  TOPTION_OLD_SELECTION_METHOD,
  TOPTION_ALWAYS_SHOW_MOVEMENT_PATH,

  //	TOPTION_TIME_LIMIT_TURNS,			//moved to the game init screen

  TOPTION_SHOW_MISSES,

  TOPTION_RTCONFIRM,

  //	TOPTION_DISPLAY_ENEMY_INDICATOR,		//Displays the number of enemies seen by the
  // merc, ontop of their portrait
  TOPTION_SLEEPWAKE_NOTIFICATION,

  TOPTION_USE_METRIC_SYSTEM,  // If set, uses the metric system

  TOPTION_MERC_ALWAYS_LIGHT_UP,

  TOPTION_SMART_CURSOR,

  TOPTION_SNAP_CURSOR_TO_DOOR,

  TOPTION_GLOW_ITEMS,
  TOPTION_TOGGLE_TREE_TOPS,
  TOPTION_TOGGLE_WIREFRAME,
  TOPTION_3D_CURSOR,

  NUM_GAME_OPTIONS,  // Toggle up this will be able to be Toggled by the player

  // These options will NOT be toggable by the Player
  TOPTION_MERC_CASTS_LIGHT = NUM_GAME_OPTIONS,
  TOPTION_HIDE_BULLETS,
  TOPTION_TRACKING_MODE,

  NUM_ALL_GAME_OPTIONS,
};

typedef struct {
  int8_t bLastSavedGameSlot;  // The last saved game number goes in here

  uint8_t ubMusicVolumeSetting;
  uint8_t ubSoundEffectsVolume;
  uint8_t ubSpeechVolume;

  // The following are set from the status of the toggle boxes in the Options Screen
  uint8_t fOptions[NUM_ALL_GAME_OPTIONS];

  char zVersionNumber[14];

  uint32_t uiSettingsVersionNumber;
  uint32_t uiMeanwhileScenesSeenFlags;

  BOOLEAN fHideHelpInAllScreens;

  BOOLEAN fUNUSEDPlayerFinishedTheGame;  // JA2Gold: for UB compatibility
  uint8_t ubSizeOfDisplayCover;
  uint8_t ubSizeOfLOS;

  uint8_t ubFiller[17];

} GAME_SETTINGS;

// Enums for the difficulty levels
enum {
  DIF_LEVEL_ZERO,
  DIF_LEVEL_EASY,
  DIF_LEVEL_MEDIUM,
  DIF_LEVEL_HARD,
  DIF_LEVEL_FOUR,
};

typedef struct {
  BOOLEAN fGunNut;
  BOOLEAN fSciFi;
  uint8_t ubDifficultyLevel;
  BOOLEAN fTurnTimeLimit;
  BOOLEAN fIronManMode;

  uint8_t ubFiller[7];

} GAME_OPTIONS;

// This structure will contain general Ja2 settings  NOT individual game settings.
extern GAME_SETTINGS gGameSettings;

// This structure will contain the Game options set at the beginning of the game.
extern GAME_OPTIONS gGameOptions;

BOOLEAN SaveGameSettings();
BOOLEAN LoadGameSettings();

void InitGameOptions();

void DisplayGameSettings();

BOOLEAN MeanwhileSceneSeen(uint8_t ubMeanwhile);

BOOLEAN SetMeanwhileSceneSeen(uint8_t ubMeanwhile);

BOOLEAN CanGameBeSaved();

#endif
