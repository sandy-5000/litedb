#pragma once

#include <cstdint>
#include <unistd.h>
#include <stdexcept>

#include "litedb/constants.hpp"
#include "litedb/globals.hpp"

namespace litedb::page::io {

ssize_t read_page(uint32_t page_id, uint8_t* buffer);
ssize_t write_page(uint32_t page_id, const uint8_t* buffer);

template <typename T>
T read_align(uint32_t pageId, uint32_t offset) {
    T value;
    ssize_t n = ::pread(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
    if (n != sizeof(T)) {
        throw std::runtime_error("Failed to read value");
    }
    return value;
}

template <typename T>
void write_align(uint32_t pageId, uint32_t offset, const T value) {
    ssize_t n = ::pwrite(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<const char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
    if (n != sizeof(T)) {
        throw std::runtime_error("Failed to write value");
    }
}

}
