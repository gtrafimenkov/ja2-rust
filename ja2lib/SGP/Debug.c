#include "SGP/Debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "SGP/Types.h"
#include "platform.h"
#include "rust_debug.h"

char gubAssertString[128];

#define MAX_MSG_LENGTH2 512
char gbTmpDebugString[8][MAX_MSG_LENGTH2];
uint8_t gubStringIndex = 0;

BOOLEAN InitializeDebugManager(void) { return (TRUE); }

void ShutdownDebugManager(void) { DebugMsg(0, DBG_ERROR, "SGP Going Down"); }

void DebugMsg(enum DebugTopic topic, enum DebugLevel level, const char *message) {
  char buf[300];
  snprintf(buf, ARR_SIZE(buf), "%02d/%02d %s", topic, level, message);
  DebugLogWrite(buf);
}

void _DebugMessage(char* pString, uint32_t uiLineNum, char* pSourceFile) {
  char buf[300];
  snprintf(buf, ARR_SIZE(buf), "%s [Line %d in %s]\n", pString, uiLineNum, pSourceFile);
  DebugLogWrite(buf);
}

void _AssertFailedMessage(char* pString, uint32_t uiLineNum, char* pSourceFile) {
  char buf[300];
  snprintf(buf, ARR_SIZE(buf), "Assertion Failure [Line %d in %s]\n", uiLineNum, pSourceFile);
  DebugLogWrite(buf);
}

// This is NOT a _DEBUG only function! It is also needed in
// release mode builds. -- DB
char *String(const char *string, ...) {
  va_list ArgPtr;
  uint8_t usIndex;

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

void _Null(void) {}
