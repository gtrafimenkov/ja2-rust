#include "Utils/EncryptedFile.h"

#include "LanguageDefines.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"

BOOLEAN LoadEncryptedDataFromFile(char* pFileName, wchar_t* pDestString, uint32_t uiSeekFrom,
                                  uint32_t uiSeekAmount) {
  HWFILE hFile;
  uint16_t i;
  uint32_t uiBytesRead;

  hFile = FileMan_Open(pFileName, FILE_ACCESS_READ, FALSE);
  if (!hFile) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed to FileMan_Open");
    return (FALSE);
  }

  if (FileMan_Seek(hFile, uiSeekFrom, FILE_SEEK_FROM_START) == FALSE) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed FileMan_Seek");
    return (FALSE);
  }

  if (!FileMan_Read(hFile, pDestString, uiSeekAmount, &uiBytesRead)) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "LoadEncryptedDataFromFile: Failed FileMan_Read");
    return (FALSE);
  }

  // Decrement, by 1, any value > 32
  for (i = 0; (i < uiSeekAmount) && (pDestString[i] != 0); i++) {
    if (pDestString[i] > 33) pDestString[i] -= 1;
#ifdef POLISH
    switch (pDestString[i]) {
      case 260:
        pDestString[i] = 165;
        break;
      case 262:
        pDestString[i] = 198;
        break;
      case 280:
        pDestString[i] = 202;
        break;
      case 321:
        pDestString[i] = 163;
        break;
      case 323:
        pDestString[i] = 209;
        break;
      case 211:
        pDestString[i] = 211;
        break;

      case 346:
        pDestString[i] = 338;
        break;
      case 379:
        pDestString[i] = 175;
        break;
      case 377:
        pDestString[i] = 143;
        break;
      case 261:
        pDestString[i] = 185;
        break;
      case 263:
        pDestString[i] = 230;
        break;
      case 281:
        pDestString[i] = 234;
        break;

      case 322:
        pDestString[i] = 179;
        break;
      case 324:
        pDestString[i] = 241;
        break;
      case 243:
        pDestString[i] = 243;
        break;
      case 347:
        pDestString[i] = 339;
        break;
      case 380:
        pDestString[i] = 191;
        break;
      case 378:
        pDestString[i] = 376;
        break;
    }
#endif
  }

  FileMan_Close(hFile);
  return (TRUE);
}
