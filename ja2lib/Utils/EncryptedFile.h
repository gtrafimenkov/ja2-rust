#ifndef __ENCRYPTED_H_
#define __ENCRYPTED_H_

#include "SGP/Types.h"

BOOLEAN LoadEncryptedDataFromFile(char* pFileName, wchar_t* pDestString, uint32_t uiSeekFrom,
                                  uint32_t uiSeekAmount);

#endif
