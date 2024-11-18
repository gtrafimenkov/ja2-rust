#ifndef __DEBUG_CONTROL_
#define __DEBUG_CONTROL_

#include "SGP/Types.h"

//#define		_PHYSICSSUBSYSTEM_DEBUG
//#define		_AISUBSYSTEM_DEBUG

#ifdef JA2BETAVERSION
//  #define			_ANIMSUBSYSTEM_DEBUG
#endif

void LiveMessage(char *strMessage);

#ifdef _ANIMSUBSYSTEM_DEBUG

#define AnimDebugMsg(c) AnimDbgMessage((c))

extern void AnimDbgMessage(char *Str);

#else

#define AnimDebugMsg(c)

#endif

#ifdef _PHYSICSSUBSYSTEM_DEBUG

#define PhysicsDebugMsg(c) PhysicsDbgMessage((c))

extern void PhysicsDbgMessage(char *Str);

#else

#define PhysicsDebugMsg(c)

#endif

#ifdef _AISUBSYSTEM_DEBUG

#define AiDebugMsg(c) AiDbgMessage((c))

extern void AiDbgMessage(char *Str);

#else

#define AiDebugMsg(c)

#endif

#endif
