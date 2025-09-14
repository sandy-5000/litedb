#!/bin/bash

g++ -std=c++20 -Iinclude \
    src/thread_test.cpp \
    src/db_config.cpp \
    src/main.cpp \
    src/page/page_header.cpp \
    src/page/page.cpp \
    -o litedb.out


if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable is 'litedb.out'."
else
    echo "Compilation failed."
fi
