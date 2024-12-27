#ifndef __DEBUG_MANAGER_
#define __DEBUG_MANAGER_

#include "SGP/TopicIDs.h"
#include "SGP/TopicOps.h"
#include "SGP/Types.h"

extern void DebugPrint(const char *message);

#define INVALID_TOPIC 0xffff
#define MAX_TOPICS_ALLOTED 1024

extern BOOLEAN gfRecordToFile;
extern BOOLEAN gfRecordToDebugger;
extern uint32_t guiProfileStart, guiExecutions, guiProfileTime;
extern INT32 giProfileCount;

#define PROFILE(x)                       \
  guiProfileStart = Plat_GetTickCount(); \
  guiExecutions = x;                     \
  for (giProfileCount = 0; giProfileCount < x; giProfileCount++)

#define PROFILE_REPORT()                                                                   \
  guiProfileTime = (Plat_GetTickCount() - guiProfileStart);                                \
  _RPT3(_CRT_WARN,                                                                         \
        "*** PROFILE REPORT: %d executions took %dms, average of %.2fms per iteration.\n", \
        guiExecutions, guiProfileTime, (FLOAT)guiProfileTime / guiExecutions);

extern void _Null(void);
extern char *String(const char *String, ...);

#if defined(_DEBUG) || defined(FORCE_ASSERTS_ON)

// If DEBUG_ is defined, we need to initialize all the debug macros. Otherwise all the
// debug macros will be substituted by blank lines at compile time
//*******************************************************************************************
// Debug Mode
//*******************************************************************************************

// Modified the Assertion code.  As of the writing of this code, there are no other functions that
// make use of _FailMessage.  With that assumption made, we can then make two functions, the first
// Assert, taking one argument, and passing a NULL string.  The second one, AssertMsg(), accepts a
// string as the second parameter. This string that has vanished for Assert is now built inside of
// fail message.  This is the case for both Asserts, but the second one also is added.  Ex: Assert(
// pointer ); Assert( pointer, "This pointer is null and you tried to access it in function A ");
// It'll make debugging a little simpler.  In anal cases, you could build the string first, then
// assert with it.
extern void _FailMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile);

#define Assert(a) (a) ? _Null() : _FailMessage((STR8)NULL, __LINE__, (STR8)__FILE__)
#define AssertMsg(a, b) (a) ? _Null() : _FailMessage((STR8)b, __LINE__, (STR8)__FILE__)

extern char gubAssertString[128];

#else

#define Assert(a) _Null()
#define AssertMsg(a, b) _Null()

//*******************************************************************************************
#endif

// Moved these out of the defines - debug mgr always initialized
#define InitializeDebugManager() DbgInitialize()
#define ShutdownDebugManager() DbgShutdown()

extern BOOLEAN DbgInitialize(void);
extern void DbgShutdown(void);

#ifdef SGP_DEBUG
// If DEBUG_ is defined, we need to initialize all the debug macros. Otherwise all the
// debug macros will be substituted by blank lines at compile time
//*******************************************************************************************
// Debug Mode
//*******************************************************************************************

extern BOOLEAN gfDebugTopics[MAX_TOPICS_ALLOTED];
// These are the debug macros (the ones the use will use). The user should never call
// the actual debug functions directly

#define DbgMessage(a, b, c) \
  DbgMessageReal((uint16_t)(a), (UINT8)(TOPIC_MESSAGE), (UINT8)(b), (char *)(c))
#define FastDebugMsg(a) _DebugMessage((STR8)(a), (uint32_t)(__LINE__), (STR8)(__FILE__))

#define UnRegisterDebugTopic(a, b) \
  DbgTopicRegistration((UINT8)TOPIC_UNREGISTER, (uint16_t *)(&(a)), (char *)(b))
#define ClearAllDebugTopics() DbgClearAllTopics()

#define ErrorMsg(a) _DebugMessage((STR8)(a), (uint32_t)(__LINE__), (STR8)(__FILE__))

// Enable the debug topic we want
#define RegisterJA2DebugTopic(a, b) DbgTopicRegistration(TOPIC_REGISTER, &(a), (b))
#define RegisterDebugTopic(a, b)
#define DebugMsg(a, b, c) DbgMessageReal((a), TOPIC_MESSAGE, (b), (c))

// public interface to debug methods:
extern void DbgMessageReal(uint16_t TopicId, UINT8 uiCommand, UINT8 uiDebugLevel, char *Str);
extern BOOLEAN DbgSetDebugLevel(uint16_t TopicId, UINT8 uiDebugLevel);
extern void DbgFailedAssertion(BOOLEAN fExpression, char *szFile, int nLine);
// extern	void		_FailMessage(UINT8 *pString, uint32_t uiLineNum, UINT8 *pSourceFile
// );
extern void DbgTopicRegistration(UINT8 ubCmd, uint16_t *usTopicID, char *zMessage);
extern void DbgClearAllTopics(void);
extern void _DebugMessage(STR8 pString, uint32_t uiLineNum, STR8 pSourceFile);

//*******************************************************************************************

#else

//*******************************************************************************************
// Release Mode
//*******************************************************************************************

#define RegisterDebugTopic(a, b)
#define UnRegisterDebugTopic(a, b)
#define ClearAllDebugTopics()

#define FastDebugMsg(a) _Null()
#define ErrorMsg(a)

#define DbgTopicRegistration(a, b, c)
#define DbgMessage(a, b, c)

#define RegisterJA2DebugTopic(a, b)
#define DebugMsg(a, b, c)

//*******************************************************************************************
#endif

void DbgWriteToDebugFile(const char *message);
void DbgWriteToDebugFileW(const wchar_t *message);

#endif
