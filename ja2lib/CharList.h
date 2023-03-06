#ifndef __CHAR_LIST_H
#define __CHAR_LIST_H

#include "LeanTypes.h"

struct SOLDIERTYPE;

// Max number of our soldiers
#define MAX_CHARACTER_COUNT 20

// Check if the given entry in the character list is valid
bool IsCharListEntryValid(int index);

// Is the given entry in the character list selected?
bool IsCharSelected(int index);

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index);

#endif
