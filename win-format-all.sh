#!/bin/bash

find . \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
    | xargs clang-format.exe -i --style=file
