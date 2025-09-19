#pragma once
#include <cstdint>
#include <unistd.h>

#include "litedb/constants.hpp"
#include "litedb/globals.hpp"

namespace litedb::page {


class PageIO {
public:
    static ssize_t readPage(uint32_t pageId, uint8_t* buffer);
    static ssize_t writePage(uint32_t pageId, const uint8_t* buffer);

    template<typename T>
    T readAt(uint32_t pageId, uint32_t offset, T value) const;

    template<typename T>
    ssize_t writeAt(uint32_t pageId, uint32_t offset, const T value);
};

template <typename T>
T PageIO::readAt(uint32_t pageId, uint32_t offset, T value) const {
    return ::pread(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
}

template <typename T>
ssize_t PageIO::writeAt(uint32_t pageId, uint32_t offset, const T value) {
    return ::pwrite(
        litedb::g::DB_FILE_DESCRIPTOR,
        reinterpret_cast<const char*>(&value),
        sizeof(T),
        static_cast<off_t>(pageId) * litedb::constants::PAGE_SIZE + offset
    );
}


}