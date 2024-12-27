#ifndef _LIBRARY_DATABASE_PUB_H
#define _LIBRARY_DATABASE_PUB_H

// Public interface to the library database.

#include "SGP/Types.h"

BOOLEAN InitializeFileDatabase();
BOOLEAN ShutDownFileDatabase();
BOOLEAN IsLibraryOpened(int16_t sLibraryID);
BOOLEAN IsLibraryRealFile(HWFILE hFile);

#endif
