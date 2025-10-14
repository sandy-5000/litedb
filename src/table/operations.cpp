#include "litedb/table/operations.hpp"

#include <iostream>
#include <string>
#include <cstring>

#include "litedb/engine/root_manager.hpp"
#include "litedb/table/data_types.hpp"

namespace litedb::table {

bool root_table::create_table(const std::string &table_name) {

    uint16_t name_size = 0;
    if (table_name.size() > 256) {
        std::cout << "[CREATE_TABLE] table_name exceeds 256 chars." << std::endl;
        return false;
    } else {
        name_size = table_name.size();
    }

    auto root_manager = engine::root_manager_;
    auto root_page = root_manager->get_root();

    root_manager->lock_unique();
    uint32_t root_table_page = root_manager->page_data.root_table_page;
    // std::cout << "Root table page: " << root_table_page << std::endl;
    uint64_t seq_number = root_manager->page_data.seq_number;
    root_manager->page_data.seq_number++;
    root_manager->unlock_unique();

    if (root_table_page == 0) {
        uint32_t new_root_table_page = root_manager->get_free_page();
        std::memcpy(
            root_page->data_ + constants::PAGE_HEADER_SIZE,
            &new_root_table_page,
            sizeof(uint32_t)
        );
        root_table_page = new_root_table_page;
        root_manager->page_data.root_table_page = new_root_table_page;
        root_page->set_dirty();

        litedb::page::Page new_page;
        new_page.read_empty(new_root_table_page);
        new_page.header.id = new_root_table_page;
        new_page.header.type = 0x80;
        new_page.header.record_count = 0;
        new_page.header.free_space_offset = constants::PAGE_SIZE;
        new_page.header.p_parent = 0;
        new_page.header.free_space = g::PAGE_BODY_SIZE;

        new_page.write();
    }

    uint16_t key_size =
        (2 + 1) + // key_size + type
        (1 + name_size + 1) + // table_name
        (1 + 8) + // seq_number
        (1); // [0x00];
    std::string key(key_size, 0);

    uint16_t idx = 0;
    std::memcpy(key.data(), &key_size, sizeof(uint16_t)); idx += 2;
    key[idx++] = 0x84;
    key[idx++] = TYPE_str;
    std::memcpy(key.data() + idx, table_name.data(), name_size); idx += name_size + 1;
    key[idx++] = TYPE_i64;
    std::memcpy(key.data() + idx, &seq_number, sizeof(uint64_t));

    // std::cout << "[CREATE_TABLE] table_name " << table_name << " " << seq_number << std::endl;

    auto changes = insert::key(table_name, root_table_page, key, false);

    if (changes.size()) {
        if (changes.back().change_type == 2) {
            root_manager->lock_unique();
            root_manager->page_data.root_table_page = changes.back().page_id;
            std::cout << "[NEW_ROOT_PAGE] " << root_manager->page_data.root_table_page << std::endl;
            root_manager->unlock_unique();
            changes.pop_back();
        }
        // std::cout << "[CREATE_TABLE] success" << std::endl;
        return true;
    } else {
        std::cout << "[CREATE_TABLE] " << table_name << " failed" << std::endl;
        return false;
    }
}

}