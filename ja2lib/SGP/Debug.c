#include "SGP/Debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "SGP/Types.h"
#include "platform.h"

BOOLEAN gfRecordToFile = FALSE;
BOOLEAN gfRecordToDebugger = TRUE;

// moved from header file: 24mar98:HJH
uint32_t guiProfileStart, guiExecutions, guiProfileTime;
INT32 giProfileCount;

// Had to move these outside the ifdef SGP_DEBUG below, because
// they are required for the String() function, which is NOT a
// debug-mode only function, it's used in release-mode as well! -- DB

char gubAssertString[128];

#define MAX_MSG_LENGTH2 512
char gbTmpDebugString[8][MAX_MSG_LENGTH2];
UINT8 gubStringIndex = 0;

// #ifdef SGP_DEBUG

// //**************************************************************************
// //
// //				Defines
// //
// //**************************************************************************

// #define BUFSIZE 100
// #define TIMER_TIMEOUT 1000

// //**************************************************************************
// //
// //				Variables
// //
// //**************************************************************************

UINT16 TOPIC_MEMORY_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_FILE_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_DATABASE_MANAGER = INVALID_TOPIC;
UINT16 TOPIC_GAME = INVALID_TOPIC;
UINT16 TOPIC_SGP = INVALID_TOPIC;
UINT16 TOPIC_VIDEO = INVALID_TOPIC;
UINT16 TOPIC_INPUT = INVALID_TOPIC;
UINT16 TOPIC_STACK_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_LIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_QUEUE_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_PRILIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_HIMAGE = INVALID_TOPIC;
UINT16 TOPIC_ORDLIST_CONTAINERS = INVALID_TOPIC;
UINT16 TOPIC_3DENGINE = INVALID_TOPIC;
UINT16 TOPIC_VIDEOOBJECT = INVALID_TOPIC;
UINT16 TOPIC_FONT_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_VIDEOSURFACE = INVALID_TOPIC;
UINT16 TOPIC_MOUSE_SYSTEM = INVALID_TOPIC;
UINT16 TOPIC_BUTTON_HANDLER = INVALID_TOPIC;
UINT16 TOPIC_MUTEX = INVALID_TOPIC;
UINT16 TOPIC_JA2 = INVALID_TOPIC;
UINT16 TOPIC_BLIT_QUEUE = INVALID_TOPIC;
UINT16 TOPIC_JA2OPPLIST = INVALID_TOPIC;
UINT16 TOPIC_JA2AI = INVALID_TOPIC;

// uint32_t guiTimerID = 0;
// UINT8 guiDebugLevels[NUM_TOPIC_IDS];  // don't change this, Luis!!!!

// BOOLEAN gfDebugTopics[MAX_TOPICS_ALLOTED];
// UINT16 *gpDbgTopicPtrs[MAX_TOPICS_ALLOTED];

// // remove debug .txt file
// void RemoveDebugText(void);

// STRING512 gpcDebugLogFileName;

// //**************************************************************************
// //
// //				Functions
// //
// //**************************************************************************

// //**************************************************************************
// //
// // DbgGetLogFileName
// //
// //
// //
// // Parameter List :
// // Return Value :
// // Modification history :
// //
// //		xxjun98:CJC		-> creation
// //
// //**************************************************************************
// BOOLEAN DbgGetLogFileName(STRING512 pcName) {
//   // use the provided buffer to get the directory name, then tack on
//   // "\debug.txt"
// #ifndef _NO_DEBUG_TXT
//   if (!Plat_GetExecutableDirectory(pcName)) {
//     return (FALSE);
//   }

//   if (strlen(pcName) > (512 - strlen("\\debug.txt") - 1)) {
//     // no room!
//     return (FALSE);
//   }

//   strcat(pcName, "\\debug.txt");
// #endif

//   return (TRUE);
// }

BOOLEAN DbgInitialize(void) {
  //   INT32 iX;

  //   for (iX = 0; iX < MAX_TOPICS_ALLOTED; iX++) {
  //     gpDbgTopicPtrs[iX] = NULL;
  //   }

  //   DbgClearAllTopics();

  //   gfRecordToFile = TRUE;
  //   gfRecordToDebugger = TRUE;
  //   gubAssertString[0] = '\0';

  // #ifndef _NO_DEBUG_TXT
  //   if (!DbgGetLogFileName(gpcDebugLogFileName)) {
  //     return (FALSE);
  //   }
  //   // clear debug text file out
  //   RemoveDebugText();
  // #endif

  return (TRUE);
}

void DbgMessageReal(UINT16 uiTopicId, UINT8 uiCommand, UINT8 uiDebugLevel, char *strMessage);

void DbgShutdown(void) { DbgMessageReal((UINT16)(-1), CLIENT_SHUTDOWN, 0, "SGP Going Down"); }

#ifdef SGP_DEBUG

void DbgTopicRegistration(UINT8 ubCmd, UINT16 *usTopicID, char *zMessage) {
  //   UINT16 usIndex, usUse;
  //   BOOLEAN fFound;

  //   if (usTopicID == NULL) return;

  //   if (ubCmd == TOPIC_REGISTER) {
  //     usUse = INVALID_TOPIC;
  //     fFound = FALSE;
  //     for (usIndex = 0; usIndex < MAX_TOPICS_ALLOTED && !fFound; usIndex++) {
  //       if (!gfDebugTopics[usIndex]) {
  //         fFound = TRUE;
  //         usUse = usIndex;
  //       }
  //     }

  //     gfDebugTopics[usUse] = TRUE;
  //     *usTopicID = usUse;
  //     gpDbgTopicPtrs[usUse] = usTopicID;
  //     DbgMessageReal(usUse, TOPIC_MESSAGE, DBG_LEVEL_0, zMessage);
  //   } else if (ubCmd == TOPIC_UNREGISTER) {
  //     if (*usTopicID >= MAX_TOPICS_ALLOTED) return;

  //     DbgMessageReal(*usTopicID, TOPIC_MESSAGE, DBG_LEVEL_0, zMessage);
  //     gfDebugTopics[*usTopicID] = FALSE;

  //     if (gpDbgTopicPtrs[*usTopicID] != NULL) {
  //       gpDbgTopicPtrs[*usTopicID] = NULL;
  //     }

  //     *usTopicID = INVALID_TOPIC;
  //   }
}

#endif

// // *************************************************************************
// // Clear the debug txt file out to prevent it from getting huge
// //
// //
// // *************************************************************************

// void RemoveDebugText(void) { Plat_DeleteFile(gpcDebugLogFileName); }

// //**************************************************************************
// //
// // DbgClearAllTopics
// //
// //
// // Parameter List :
// // Return Value :
// // Modification history :
// //
// //		June 97: BR		-> creation
// //
// //**************************************************************************

void DbgClearAllTopics(void) {
  //   UINT16 usIndex;

  //   for (usIndex = 0; usIndex < MAX_TOPICS_ALLOTED; usIndex++) {
  //     gfDebugTopics[usIndex] = FALSE;
  //     if (gpDbgTopicPtrs[usIndex] != NULL) {
  //       *gpDbgTopicPtrs[usIndex] = INVALID_TOPIC;
  //       gpDbgTopicPtrs[usIndex] = NULL;
  //     }
  //   }
}

void DbgMessageReal(UINT16 uiTopicId, UINT8 uiCommand, UINT8 uiDebugLevel, char *strMessage) {
  printf("%4d / %4d / %4d / %s\n", uiTopicId, uiCommand, uiDebugLevel, strMessage);
}

// void DbgMessageReal(UINT16 uiTopicId, UINT8 uiCommand, UINT8 uiDebugLevel, char *strMessage) {
// #ifndef _NO_DEBUG_TXT
//   FILE *OutFile;
// #endif

//   // Check for a registered topic ID
//   if (uiTopicId < MAX_TOPICS_ALLOTED && gfDebugTopics[uiTopicId]) {
//     DebugPrint(strMessage);
//     DebugPrint("\n");

// // add _NO_DEBUG_TXT to your SGP preprocessor definitions to avoid this f**king huge file from
// // slowly growing behind the scenes!!!!
// #ifndef _NO_DEBUG_TXT
//     if ((OutFile = fopen(gpcDebugLogFileName, "a+t")) != NULL) {
//       fprintf(OutFile, "%s\n", strMessage);
//       fclose(OutFile);
//     }
// #endif
//   }
// }

// //**************************************************************************
// //
// // DbgSetDebugLevel
// //
// //
// //
// // Parameter List :
// // Return Value :
// // Modification history :
// //
// //		11nov96:HJH		-> creation
// //
// //**************************************************************************

// BOOLEAN DbgSetDebugLevel(UINT16 uiTopicId, UINT8 uiDebugLevel) { return (TRUE); }

// //**************************************************************************
// //
// // DbgFailedAssertion
// //
// //
// //
// // Parameter List :
// // Return Value :
// // Modification history :
// //
// //		xxnov96:HJH		-> creation
// //
// //**************************************************************************

// void DbgFailedAssertion(BOOLEAN fExpression, char *szFile, int nLine) {
// #ifndef _NO_DEBUG_TXT
//   FILE *OutFile;

//   if (fExpression == FALSE) {
//     if ((OutFile = fopen(gpcDebugLogFileName, "a+t")) != NULL) {
//       fprintf(OutFile, "Assertion Failed at:\n    line %i\n    %s\n", nLine, szFile);
//       fclose(OutFile);
//     }
//   }
// #endif
// }

// ///////////////////////////////////////////////////////////////////////////////////////////////////

// void _DebugRecordToFile(BOOLEAN gfState) { gfRecordToFile = gfState; }

// ///////////////////////////////////////////////////////////////////////////////////////////////////

// void _DebugRecordToDebugger(BOOLEAN gfState) { gfRecordToDebugger = gfState; }

// ///////////////////////////////////////////////////////////////////////////////////////////////////
// // Wiz8 compatible debug messaging

void _DebugMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile) {
  printf("%s [Line %d in %s]\n", pString, uiLineNum, pSourceFile);
}

// void _DebugMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile) {
//   char ubOutputString[512];
// #ifndef _NO_DEBUG_TXT
//   FILE *DebugFile;
// #endif

//   //
//   // Build the output string
//   //

//   sprintf(ubOutputString, "{ %ld } %s [Line %d in %s]\n", Plat_GetTickCount(), pString,
//   uiLineNum,
//           pSourceFile);

//   //
//   // Output to debugger
//   //

//   if (gfRecordToDebugger) {
//     DebugPrint(ubOutputString);
//   }

//   //
//   // Record to file if required
//   //

// #ifndef _NO_DEBUG_TXT
//   if (gfRecordToFile) {
//     if ((DebugFile = fopen(gpcDebugLogFileName, "a+t")) != NULL) {
//       fputs(ubOutputString, DebugFile);
//       fclose(DebugFile);
//     }
//   }
// #endif
// }

// extern struct VObject* FontObjs[25];

void _FailMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile) {
  printf("Assertion Failure [Line %d in %s]\n", uiLineNum, pSourceFile);
}

// void _FailMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile) {
//   MSG Message;
//   char ubOutputString[512];
// #ifndef _NO_DEBUG_TXT
//   FILE *DebugFile;
// #endif
//   BOOLEAN fDone = FALSE;
//   // Build the output strings
//   sprintf(ubOutputString, "{ %ld } Assertion Failure [Line %d in %s]\n", Plat_GetTickCount(),
//   uiLineNum,
//           pSourceFile);
//   if (pString)
//     sprintf(gubAssertString, pString);
//   else
//     sprintf(gubAssertString, "");

//   // Output to debugger
//   if (gfRecordToDebugger) DebugPrint(ubOutputString);

//     // Record to file if required
// #ifndef _NO_DEBUG_TXT
//   if (gfRecordToFile) {
//     if ((DebugFile = fopen(gpcDebugLogFileName, "a+t")) != NULL) {
//       fputs(ubOutputString, DebugFile);
//       fclose(DebugFile);
//     }
//   }

// #if 0
// 	if( !FontObjs[0] )
// 	{ //Font manager hasn't yet been initialized so use the windows error system
// 		sprintf( gubErrorText, "Assertion Failure -- Line %d in %s", uiLineNum, pSourceFile
// ); 		MessageBox( NULL, gubErrorText, "Jagged Alliance 2", MB_OK );
// gfProgramIsRunning = FALSE; 		return;
// 	}
// #endif

//   // Kris:
//   // NASTY HACK, THE GAME IS GOING TO DIE ANYWAY, SO WHO CARES WHAT WE DO.
//   // This will actually bring up a screen that prints out the assert message
//   // until the user hits esc or alt-x.
//   sprintf(gubErrorText, "Assertion Failure -- Line %d in %s", uiLineNum, pSourceFile);
//   SetPendingNewScreen(ERROR_SCREEN);
//   SetCurrentScreen(ERROR_SCREEN);
//   while (gfProgramIsRunning) {
//     if (PeekMessage(&Message, NULL, 0, 0,
//                     PM_NOREMOVE)) {  // We have a message on the WIN95 queue, let's get it
//       if (!GetMessage(&Message, NULL, 0, 0)) {  // It's quitting time
//         continue;
//       }
//       // Ok, now that we have the message, let's handle it
//       TranslateMessage(&Message);
//       DispatchMessage(&Message);
//     } else {  // Windows hasn't processed any messages, therefore we handle the rest
//       GameLoop();
//       gfSGPInputReceived = FALSE;
//     }
//   }
// #endif
//   exit(0);
// }

// #endif

// This is NOT a _DEBUG only function! It is also needed in
// release mode builds. -- DB
char *String(const char *string, ...) {
  va_list ArgPtr;
  UINT8 usIndex;

  // Record string index. This index is used since we live in a multitasking environment.
  // It is still not bulletproof, but it's better than a single string
  usIndex = gubStringIndex++;
  if (gubStringIndex == 8) {  // reset string pointer
    gubStringIndex = 0;
  }

  va_start(ArgPtr, string);
  vsprintf(gbTmpDebugString[usIndex], string, ArgPtr);
  va_end(ArgPtr);

  return gbTmpDebugString[usIndex];
}

//////////////////////////////////////////////////////////////////////
// This func is used by Assert()
void _Null(void) {}

static FILE *debug_txt_file = NULL;

static void openDebugTxt() {
  if (debug_txt_file == NULL) {
    char dir[200];
    char path[256];
    if (!Plat_GetExecutableDirectory(dir, ARR_SIZE(dir))) {
      return;
    }
    snprintf(path, ARR_SIZE(path), "%s%c%s", dir, FS_SEPARATOR, "debug.txt");
    debug_txt_file = fopen(path, "wt");
  }
}

void DbgWriteToDebugFile(const char *message) {
  openDebugTxt();
  if (debug_txt_file != NULL) {
    fprintf(debug_txt_file, "%s\n", message);
    fflush(debug_txt_file);
  }
}

void DbgWriteToDebugFileW(const wchar_t *message) {
  openDebugTxt();
  if (debug_txt_file != NULL) {
    fprintf(debug_txt_file, "%ls\n", message);
    fflush(debug_txt_file);
  }
}
