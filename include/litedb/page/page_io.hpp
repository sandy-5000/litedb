#pragma once
#include <cstdint>
#include <unistd.h>

#include "litedb/constants.hpp"

namespace litedb::page {


class PageIO {
public:
    static ssize_t readPage(uint32_t pageId, uint8_t* buffer);
    static ssize_t writePage(uint32_t pageId, const uint8_t* buffer);
};


}