An experiment of creating platform-independent layer of
accessing file input/output functions.

- `platform.h` - OS-independent interface for file I/O
- `platform_win_io.c` provides windows implementation of this interface
- `platform_linux_io.c` - linux implementation
- `Plat_*` functions give access to platform layer IO functions
- `FileMan_*` an abstraction on top of platform IO to read regular files and files from file archives (slf)
