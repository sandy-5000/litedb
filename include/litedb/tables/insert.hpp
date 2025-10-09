#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <stack>

#include "litedb/page/page.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/tables/find.hpp"
#include "litedb/tables/compact.hpp"

namespace litedb::table::utils {


struct key_page_change {
    union data {
        uint8_t* prev_page;
        uint16_t slot_index;
        uint32_t root_page_id;
    } old_data;
    uint32_t page_id;
    uint8_t change_type; // 0 - slot_inserted, 1 - page_split, 2 - new_root_page
};

struct page_insert final {
    static uint8_t is_insertable(
        std::shared_ptr<litedb::page::Page> page, uint16_t key_size, uint16_t slot_size
    );
    static std::vector<std::string> split_key_page(
        std::shared_ptr<litedb::page::Page> cur_page,
        std::vector<std::string> &new_keys,
        uint16_t new_key_index,
        std::shared_ptr<litedb::buffer_manager::MiniBuffer> buffer
    );
    static void add_keys_to_page(
        uint32_t &root_page_id,
        std::vector<std::string> &new_keys,
        std::vector<key_page_change> &changes,
        std::vector<uint32_t> &parents,
        std::shared_ptr<litedb::buffer_manager::MiniBuffer> buffer
    );
    static uint32_t find_key_page(
        uint32_t page_id,
        std::string &key,
        bool is_unique,
        std::vector<key_page_change> &changes,
        std::vector<uint32_t> &parents,
        std::shared_ptr<litedb::buffer_manager::MiniBuffer> buffer
    );
};

struct insert final {
    static std::vector<key_page_change> key(
        std::string table_name, uint32_t root_page, std::string &key, bool is_unique
    );
    static std::vector<uint8_t*> data(std::shared_ptr<litedb::page::Page> page, uint64_t key);
};


}
