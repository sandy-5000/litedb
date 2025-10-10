#!/bin/bash

g++ -std=c++20 -Iinclude \
    src/globals.cpp \
    src/config.cpp \
    src/main.cpp \
    \
    -o litedb.out \
    $(pkg-config --cflags --libs libmongocxx)


if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable is 'litedb.out'."
else
    echo "Compilation failed."
fi
