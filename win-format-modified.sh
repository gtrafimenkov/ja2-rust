#!/bin/bash

git status --porcelain | grep -v "D  " | egrep -e '[.](c|cc|cpp|h)$' | awk '{print $2}' \
    | grep -v rustlib.h | xargs clang-format.exe -i --style=file
