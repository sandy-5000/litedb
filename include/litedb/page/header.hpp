#pragma once

#include <cstdint>

#include "litedb/constants.hpp"

namespace litedb::page {

struct page_header {
    // 8 bytes
    uint32_t id = constants::INVALID_PAGE_ID;
    uint8_t type;
    uint8_t flag;
    uint16_t free_space;

    // 8 bytes
    uint64_t check_sum;

    // 8 bytes
    uint16_t record_count;
    uint16_t free_space_offset;
    uint32_t next_page;

    // 8 bytes
    uint64_t lsn;

    // 8 bytes
    uint64_t max_key;

    // 8 bytes
    uint32_t p_parent;
    uint32_t left_most_child;
};

}