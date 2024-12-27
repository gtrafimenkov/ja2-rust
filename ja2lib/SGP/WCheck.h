// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __WCHECK_
#define __WCHECK_

#define CHECKF(exp) \
  if (!(exp)) {     \
    return (FALSE); \
  }
#define CHECKV(exp) \
  if (!(exp)) {     \
    return;         \
  }
#define CHECKN(exp) \
  if (!(exp)) {     \
    return (NULL);  \
  }
#define CHECKBI(exp) \
  if (!(exp)) {      \
    return (-1);     \
  }

#define CHECKASSERTF(exp) \
  if (!(exp)) {           \
    ASSERT(0);            \
    return (FALSE);       \
  }
#define CHECKASSERTV(exp) \
  if (!(exp)) {           \
    ASSERT(0);            \
    return;               \
  }
#define CHECKASSERTN(exp) \
  if (!(exp)) {           \
    ASSERT(0);            \
    return (NULL);        \
  }
#define CHECKASSERTBI(exp) \
  if (!(exp)) {            \
    ASSERT(0);             \
    return (-1);           \
  }

#endif
