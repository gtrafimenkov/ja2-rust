#ifndef __ENCRYPTED_H_
#define __ENCRYPTED_H_

#include "SGP/Types.h"

BOOLEAN LoadEncryptedDataFromFile(STR pFileName, STR16 pDestString, uint32_t uiSeekFrom,
                                  uint32_t uiSeekAmount);

#endif
