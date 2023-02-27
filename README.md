# Experimenting with JA2 sources

## Overview

The main branch is based on [gtrafimenkov/ja2-vanilla](https://github.com/gtrafimenkov/ja2-vanilla)
with following changes:
- the code is split into platform-independent (`ja2lib`) and platform-specific parts (`win32`)
  for following reasons:
  - make it easier to port to other platforms
  - make it easier to develop on Linux
- small C++ parts of the code were converted to C so that the whole codebase is C now.  That should
  make it easier to integrate this code with other programming languages

Other branches contain different experiments.

## How to build

- open the solution in Visual Studio 2022
- choose Release configuration
- choose Build Solution

## How to play the game

- install the original version of the game (from the original game CDs, Steam, gog.com, etc.)
- copy the builded exe file to the game directory alongside the original ja2.exe
- (for Windows 10) copy Windows 10 compatibility files from `tools/dxwrapper` to the game directory
- launch the builded exe file

The game is tested on Windows 10.

## License

The original JA2 sources were released under the terms of [SFI-SCLA](SFI-SCLA.txt) license.
