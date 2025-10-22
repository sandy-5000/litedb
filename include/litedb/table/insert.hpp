#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace litedb::table {

struct key_page_change {
    union data {
        uint8_t* prev_page;
        std::pair<uint16_t, uint16_t> slot_info;
        uint32_t root_page_id;
    } old_data;
    uint32_t page_id;
    uint8_t change_type; // 0 - slot_inserted, 1 - page_split, 2 - new_root_page
};

struct insert final {
    static std::vector<key_page_change> key(
        uint32_t root_page, std::string &key, bool is_unique
    );
};

}