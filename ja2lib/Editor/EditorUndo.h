#ifndef __EDITOR_UNDO_H
#define __EDITOR_UNDO_H

#include "BuildDefines.h"
#include "SGP/Types.h"

BOOLEAN AddToUndoList(INT32 iMapIndex);
void AddLightToUndoList(INT32 iMapIndex, INT32 iLightRadius, uint8_t ubLightID);

BOOLEAN RemoveAllFromUndoList();
BOOLEAN ExecuteUndoList();

void EnableUndo();
void DisableUndo();

void DetermineUndoState();

// Undo command flags
#define MAX_UNDO_COMMAND_LENGTH 10

#endif
