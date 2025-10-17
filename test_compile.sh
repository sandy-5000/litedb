#!/bin/bash

set -e

OS=$(uname)

if [[ "$OS" == "Darwin" ]]; then
    BOOST_FLAGS="-I/opt/homebrew/include -L/opt/homebrew/lib"
elif [[ "$OS" == "Linux" ]]; then
    BOOST_FLAGS="-I/usr/include -L/usr/lib"
else
    echo "Unsupported OS: $OS"
    exit 1
fi

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
    src/table/remove.cpp \
    src/table/operations.cpp \
    src/table/utils.cpp \
    \
    u_test.cpp \
    \
    -o litedb.out \
    $(pkg-config --cflags --libs libmongocxx) \
    $BOOST_FLAGS -lboost_thread


if [ $? -eq 0 ]; then
    echo "Test Compilation successful. Executable is 'litedb.out'."
else
    echo "Test Compilation failed."
fi

./litedb.out data.ldb
