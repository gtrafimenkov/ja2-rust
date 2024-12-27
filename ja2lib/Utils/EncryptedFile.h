// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __ENCRYPTED_H_
#define __ENCRYPTED_H_

#include "SGP/Types.h"

BOOLEAN LoadEncryptedDataFromFile(char* pFileName, wchar_t* pDestString, uint32_t uiSeekFrom,
                                  uint32_t uiSeekAmount);

#endif
