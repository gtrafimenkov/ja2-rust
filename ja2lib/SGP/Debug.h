#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "SGP/Types.h"
#include "rust_debug.h"

enum DebugTopic {
  TOPIC_FILE_MANAGER,
  TOPIC_GAME,
  TOPIC_VIDEO,
  TOPIC_INPUT,
  TOPIC_STACK_CONTAINERS,
  TOPIC_LIST_CONTAINERS,
  TOPIC_QUEUE_CONTAINERS,
  TOPIC_HIMAGE,
  TOPIC_ORDLIST_CONTAINERS,
  TOPIC_VIDEOOBJECT,
  TOPIC_FONT_HANDLER,
  TOPIC_VIDEOSURFACE,
  TOPIC_MOUSE_SYSTEM,
  TOPIC_BUTTON_HANDLER,
  TOPIC_JA2,
  TOPIC_JA2OPPLIST,
};

enum DebugLevel {
  DBG_ERROR,
  DBG_NORMAL,
  DBG_INFO,
};

void DebugMsg(enum DebugTopic topic, enum DebugLevel level, const char *message);

extern void PrintToDebuggerConsole(const char *message);
extern void _DebugMessage(char* pString, uint32_t uiLineNum, char* pSourceFile);
extern void _AssertFailedMessage(char* pString, uint32_t uiLineNum, char* pSourceFile);

extern void _Null(void);
extern char *String(const char *String, ...);

extern BOOLEAN InitializeDebugManager(void);
extern void ShutdownDebugManager(void);

// #if defined(_DEBUG) || defined(FORCE_ASSERTS_ON)
#define Assert(a) (a) ? _Null() : _AssertFailedMessage((char*)NULL, __LINE__, (char*)__FILE__)
#define AssertMsg(a, b) (a) ? _Null() : _AssertFailedMessage((char*)b, __LINE__, (char*)__FILE__)
extern char gubAssertString[128];
// #else
// #define Assert(a) _Null()
// #define AssertMsg(a, b) _Null()
// #endif

// #ifdef SGP_DEBUG
#define FastDebugMsg(a) _DebugMessage((char*)(a), (uint32_t)(__LINE__), (char*)(__FILE__))
#define ErrorMsg(a) _DebugMessage((char*)(a), (uint32_t)(__LINE__), (char*)(__FILE__))
// #else
// #define FastDebugMsg(a) _Null()
// #define ErrorMsg(a)
// #endif

#endif
