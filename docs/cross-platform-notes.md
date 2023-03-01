# Notes on cross-platform compatibility

## Don't use %S in printf family of functions

Because it is not standard and on Windows behave differently depending on the function:
- for printf `%s` means `char *`, `%S` means `wchar_t *`
- for wprintf `%s` means `wchar_t *`, `%S` means `char *`

On Linux:
- `%s` always means `char *`
- `%ls` means `wchar_t *`
- `%S` means `wchar_t *` and is not recommended to use

If you have to use wprintf family of functions:
- use `%hs` for printing `char *`, that will also work on Linux
- use `%ls` for printing `wchar_t *`, that will also work on Linux
- don't use `%S`

Links:
- https://learn.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions


## wchar_t is of different size on Windows and Linux

Be aware of this.
