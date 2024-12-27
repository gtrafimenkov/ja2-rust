#ifndef _BUILDDEFINES_H
#define _BUILDDEFINES_H

#include "LanguageDefines.h"

// Beta version
// #define	JA2BETAVERSION

// Normal test version
// #define JA2TESTVERSION

// If we want to include the editor
// #define JA2EDITOR

#ifdef _DEBUG
#ifndef JA2TESTVERSION
#define JA2TESTVERSION
#endif
#endif

// Do combinations
#ifdef JA2TESTVERSION
#define JA2BETAVERSION
#define JA2EDITOR
#endif

#ifdef JA2BETAVERSION
#define FORCE_ASSERTS_ON
#endif

#endif
