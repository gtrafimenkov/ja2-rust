#!/bin/bash

find . \( -iname '*.c' -o -iname '*.cc' -o -iname '*.cpp' -o -iname '*.h' \) \
    | grep -v rust_ | xargs clang-format.exe -i --style=file
