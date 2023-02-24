#ifndef __PLATFORM_STRINGS_H
#define __PLATFORM_STRINGS_H

#ifdef _WINDOWS

int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);

#endif

#endif
