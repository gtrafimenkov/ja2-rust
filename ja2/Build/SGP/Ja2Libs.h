#ifndef _JA2_LIBS_H_
#define _JA2_LIBS_H_

enum {
	ENGLISH_LANG,
	RUSSIAN_LANG,
	GERMAN_LANG,
	DUTCH_LANG,
	POLISH_LANG,
	FRENCH_LANG,
	ITALIAN_LANG,

	LANG_NUMBER
};

extern INT8 gbLocale;

INT8 DetectLocale();

extern STR8 LocaleNames[LANG_NUMBER];

#define NUMBER_OF_LIBRARIES ((gbLocale != ENGLISH_LANG) ? FULL_NUMBER_OF_LIBRARIES : (FULL_NUMBER_OF_LIBRARIES-1))

	//enums used for accessing the libraries
	enum
	{
		LIBRARY_DATA,
		LIBRARY_AMBIENT,
		LIBRARY_ANIMS,
		LIBRARY_BATTLESNDS,
		LIBRARY_BIGITEMS,
		LIBRARY_BINARY_DATA,
		LIBRARY_CURSORS,
		LIBRARY_FACES,
		LIBRARY_FONTS,
		LIBRARY_INTERFACE,
		LIBRARY_LAPTOP,
		LIBRARY_MAPS,
		LIBRARY_MERCEDT,
		LIBRARY_MUSIC,
		LIBRARY_NPC_SPEECH,
		LIBRARY_NPC_DATA,
		LIBRARY_RADAR_MAPS,
		LIBRARY_SOUNDS,
		LIBRARY_SPEECH,
//		LIBRARY_TILE_CACHE,
		LIBRARY_TILESETS,
		LIBRARY_LOADSCREENS,
		LIBRARY_INTRO,
#ifdef JA2DEMO
		LIBRARY_DEMO_ADS,
#endif
	LIBRARY_NATIONAL_DATA,

		FULL_NUMBER_OF_LIBRARIES
	};


#endif