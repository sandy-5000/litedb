#pragma once
#include <cstdint>
#include <unistd.h>

#include "litedb/constants.hpp"
#include "litedb/globals.hpp"

namespace litedb::page::io {

ssize_t read_page(uint32_t page_id, uint8_t* buffer);
ssize_t write_page(uint32_t page_id, const uint8_t* buffer);

template <typename T>
T read_align(uint32_t pageId, uint32_t offset, T value) {
    return ::pread(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
}

template <typename T>
ssize_t write_align(uint32_t pageId, uint32_t offset, const T value) {
    return ::pwrite(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<const char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
}

}
