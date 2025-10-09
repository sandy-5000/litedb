#!/bin/bash

g++ -std=c++20 -Iinclude \
    src/globals.cpp \
    src/thread_test.cpp \
    src/config.cpp \
    \
    src/page/page_header.cpp \
    src/page/page_io.cpp \
    src/page/page.cpp \
    \
    src/engine/root_manager.cpp \
    src/engine/buffer_manager.cpp \
    src/engine/task_queue.cpp \
    src/engine/store.cpp \
    src/engine/query_processor.cpp \
    \
    src/tables/compare.cpp \
    src/tables/compact.cpp \
    src/tables/find.cpp \
    src/tables/insert.cpp \
    src/tables/operations.cpp \
    \
    src/json.cpp \
    src/tcp_server.cpp \
    src/test.cpp \
    \
    -o test.out \
    $(pkg-config --cflags --libs libmongocxx)


if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable is 'test.out'."
else
    echo "Compilation failed."
fi
