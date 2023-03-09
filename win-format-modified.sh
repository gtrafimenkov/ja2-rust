#!/bin/bash

git status --porcelain | grep -v "D  " | egrep -e '[.](c|cc|cpp|h)$' | awk '{print $2}' \
    | grep -v rust_ | xargs clang-format.exe -i --style=file
