# Jagged Alliance 2 in Rust

Project goals:
- get experience using Rust programming language
- understand strong and week points of the language

Non-goals:
- support various game mods
- full rewrite in Rust

Optional goals:
- add Linux support by using [SDL](https://www.libsdl.org) library

This project is based on [ja2-experiments](https://github.com/gtrafimenkov/ja2-experiments),
which in turn based on [ja2-vanilla](https://github.com/gtrafimenkov/ja2-vanilla).

## Project structure

```
ja2lib             - platform-independent part of the game code
platfrom-dummy     - dummy implementation of the platfrom code (used in unit tests)
platform-linux     - platform code for Linux
platform-win32     - platform code for win32
bin-win32          - project to build the game binary for Windows
bin-linux          - project to build Linux binary of the game (not implemented)
rustlib            - Rust code compiled to a dynamic shared library
unittester         - an application to run unit tests
```

## Build requirements

On Windows:
- Visual Studio Community 2022
- Rust v1.67.0 or later

On Linux:
- GCC
- Rust v1.67.0 or later

## How to build

On Windows:

```
build.bat
```

On Linux:

```
make build
```

## How to play the game

- install the original version of the game (from the original game CDs, Steam, gog.com, etc.)
- open `JA2.sln` in Visual Studio 2022, build `Release` or `ReleaseWithDebug` configuration of the solution
- copy the builded `ja2-win32.exe` and `rustlib.dll` to the game directory alongside the original ja2.exe
- (for Windows 10) copy Windows 10 compatibility files from `tools/dxwrapper` to the game directory
- launch the builded exe file

The game is tested on Windows 10.

## License

This is not open source as defined by [The Open Source Definition](https://opensource.org/osd/).
The original JA2 sources were released under the terms of [SFI-SCLA](SFI-SCLA.txt) license.
Please read it carefully and make your own mind regarding this.
