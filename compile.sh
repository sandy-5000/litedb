#!/bin/bash

g++ -std=c++20 -Iinclude src/main.cpp src/thread_test.cpp -o mongolite.out


if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable is 'mongolite.out'."
else
    echo "Compilation failed."
fi
