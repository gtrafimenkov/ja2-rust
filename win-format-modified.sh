#!/bin/bash

git status --porcelain | egrep -e '[.](c|cc|cpp|h)$' | awk '{print $2}' \
    | xargs clang-format.exe -i --style=file
