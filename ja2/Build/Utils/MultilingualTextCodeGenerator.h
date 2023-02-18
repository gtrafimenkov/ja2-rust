#ifdef _DEBUG

#include "SGP/Types.h"

// If special command line argument is used, then the utility will kick in
// and activate the special code generator.
BOOLEAN ProcessIfMultilingualCmdLineArgDetected(STR8 str);

#else

// macro function out
#define ProcessIfMultilingualCmdLineArgDetected(a) 0

#endif
