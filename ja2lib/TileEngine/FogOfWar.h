#ifndef __FOG_OF_WAR_H
#define __FOG_OF_WAR_H

#include "SGP/Types.h"

// Called after a map is loaded.  By keying on the MAPELEMENT_REVEALED value, we can easily
// determine what is hidden by the fog.
void InitializeFogInWorld();

// Removes and smooths the adjacent tiles.
void RemoveFogFromGridNo(uint32_t uiGridNo);

#endif
