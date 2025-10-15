#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "litedb/page/page.hpp"
#include "litedb/engine/buffer_manager.hpp"

namespace litedb::table {

struct key_page_change {
    union data {
        uint8_t* prev_page;
        uint16_t slot_index;
        uint32_t root_page_id;
    } old_data;
    uint32_t page_id;
    uint8_t change_type; // 0 - slot_inserted, 1 - page_split, 2 - new_root_page
};

struct insert final {
    static std::vector<key_page_change> key(
        std::string table_name, uint32_t root_page, std::string &key, bool is_unique
    );
};

}