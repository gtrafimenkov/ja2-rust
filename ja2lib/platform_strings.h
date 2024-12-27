// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __PLATFORM_STRINGS_H
#define __PLATFORM_STRINGS_H

#ifdef WIN32

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

#else

wchar_t *_wcsupr(wchar_t *str);

#endif

#endif
