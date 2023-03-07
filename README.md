# Experiment: rust library

Reimplementing some code in Rust and linking it as a dynamic library.

Goals:
- understand if advantages of using Rust overweight additional complexity
- estimate speed of development in Rust in comparison to C

---

# Experimenting with JA2 sources

## Overview

The main branch is based on [gtrafimenkov/ja2-vanilla](https://github.com/gtrafimenkov/ja2-vanilla)
with following changes:
- the code is split into platform-independent (`ja2lib`) and platform-specific parts (`platfrom-*`)
  for following reasons:
  - make it easier to port to other platforms
  - make it easier to develop on Linux
- small C++ parts of the code were converted to C so that the whole codebase is C now.  That should
  make it easier to integrate this code with other programming languages

Other branches contain different experiments.

## Project structure

```
ja2lib             - platform-independent part of the game code
platfrom-dummy     - dummy implementation of the platfrom code (used in unit tests)
platform-linux     - platform code for Linux
platform-win32     - platform code for win32
bin-win32          - project to build the game binary for Windows
bin-linux          - project to build Linux binary of the game (not implemented)
unittester         - an application to run unit tests
```

## How to build

- open the solution in Visual Studio 2022
- choose Release configuration
- choose Build Solution

## How to play the game

- install the original version of the game (from the original game CDs, Steam, gog.com, etc.)
- copy the builded exe file (`Release\ja2-win32.exe`) to the game directory alongside the original ja2.exe
- (for Windows 10) copy Windows 10 compatibility files from `tools/dxwrapper` to the game directory
- launch the builded exe file

The game is tested on Windows 10.

## License

This is not open source as defined by [The Open Source Definition](https://opensource.org/osd/).
The original JA2 sources were released under the terms of [SFI-SCLA](SFI-SCLA.txt) license.
Please read it carefully and make your own mind regarding this.
