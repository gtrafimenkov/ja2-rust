#include "CharList.h"

#include "Strategic/MapScreenInterface.h"

// the selected list of mercs
extern BOOLEAN fSelectedListOfMercsForMapScreen[MAX_CHARACTER_COUNT];

bool IsCharListEntryValid(int index) { return gCharactersList[index].fValid; }

// Is the given entry in the character list selected?
bool IsCharSelected(int index) { return fSelectedListOfMercsForMapScreen[index] == TRUE; }

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index) {
  return GetSoldierByID(gCharactersList[index].usSolID);
}
