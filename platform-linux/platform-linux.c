#include <wchar.h>
#include <wctype.h>

wchar_t *_wcsupr(wchar_t *str) {
  while (*str) {
    *str = towupper(*str);
    str++;
  }
  return str;
}
