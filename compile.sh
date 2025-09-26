#!/bin/bash

g++ -std=c++20 -Iinclude \
    src/globals.cpp \
    src/thread_test.cpp \
    src/config.cpp \
    src/page/page_header.cpp \
    src/page/page_io.cpp \
    src/page/page.cpp \
    src/buffer_manager.cpp \
    src/engine.cpp \
    src/json.cpp \
    src/tcp_server.cpp \
    src/main.cpp \
    -o litedb.out \
    $(pkg-config --cflags --libs libmongocxx)


if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable is 'litedb.out'."
else
    echo "Compilation failed."
fi
