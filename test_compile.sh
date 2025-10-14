#!/bin/bash

g++ -std=c++20 -Iinclude \
    \
    src/globals.cpp \
    src/config.cpp \
    \
    src/page/io.cpp \
    src/page/page.cpp \
    \
    src/engine/root_manager.cpp \
    src/engine/buffer_manager.cpp \
    src/engine/store.cpp \
    \
    src/table/compact.cpp \
    src/table/compare.cpp \
    src/table/find.cpp \
    src/table/insert.cpp \
    src/table/operations.cpp \
    src/table/utils.cpp \
    \
    _test.cpp \
    \
    -o litedb.out \
    $(pkg-config --cflags --libs libmongocxx)


if [ $? -eq 0 ]; then
    echo "Test Compilation successful. Executable is 'litedb.out'."
else
    echo "Test Compilation failed."
fi

./litedb.out data.ldb
