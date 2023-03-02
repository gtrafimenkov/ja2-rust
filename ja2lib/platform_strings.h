#ifndef __PLATFORM_STRINGS_H
#define __PLATFORM_STRINGS_H

#ifdef WIN32

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

#else

wchar_t *_wcsupr(wchar_t *str);

#endif

#endif
