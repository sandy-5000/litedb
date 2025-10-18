#include <iostream>
#include <string>
#include <cstring>

#include "litedb/table/operations.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/table/data_types.hpp"
#include "litedb/table/insert.hpp"
#include "litedb/table/remove.hpp"
#include "litedb/table/find.hpp"

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
        new_page.header.free_space_offset = constants::DB_PAGE_SIZE;
        new_page.header.p_parent = 0;
        new_page.header.free_space = g::PAGE_BODY_SIZE;
        new_page.header.next_page = 0;

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

    auto changes = insert::key(root_table_page, key, false);

    if (changes.size()) {
        if (changes.back().change_type == 2) {
            root_manager->lock_unique();
            root_manager->page_data.root_table_page = changes.back().page_id;
            // std::cout << "[NEW_ROOT_PAGE] " << root_manager->page_data.root_table_page << std::endl;
            root_manager->unlock_unique();
            // changes.pop_back();
        }
        return true;
    } else {
        std::cout << "[CREATE_TABLE] " << table_name << " failed" << std::endl;
        return false;
    }
}

uint64_t root_table::drop_table(const std::string &table_name) {

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
    root_manager->unlock_unique();

    if (root_table_page == 0) {
        return false;
    }

    uint16_t key_size =
        (2 + 1) + // key_size + type
        (1 + name_size + 1) + // table_name
        (1); // [0x00];
    std::string key(key_size, 0);

    uint16_t idx = 0;
    std::memcpy(key.data(), &key_size, sizeof(uint16_t)); idx += 2;
    key[idx++] = 0x84;
    key[idx++] = TYPE_str;
    std::memcpy(key.data() + idx, table_name.data(), name_size);

    auto responce = remove::in_slot(root_table_page, key);

    if (responce.new_root_id == 0) {
        std::cout << "[DROP_TABLE] " << table_name << " failed" << std::endl;
        return false;
    }

    if (root_table_page != responce.new_root_id) {
        root_manager->lock_unique();
        root_manager->page_data.root_table_page = responce.new_root_id;
        root_manager->unlock_unique();
    }

    return responce.count;
}

std::string root_table::find_table(const std::string &table_name) {

    uint16_t name_size = 0;
    if (table_name.size() > 256) {
        std::cout << "[FIND_TABLE] table_name exceeds 256 chars." << std::endl;
        return "";
    } else {
        name_size = table_name.size();
    }

    auto root_manager = engine::root_manager_;
    auto root_page = root_manager->get_root();

    root_manager->lock_unique();
    uint32_t root_table_page = root_manager->page_data.root_table_page;
    root_manager->unlock_unique();

    if (root_table_page == 0) {
        return "";
    }

    uint16_t key_size =
        (2 + 1) + // key_size + type
        (1 + name_size + 1) + // table_name
        (1); // [0x00];
    std::string key(key_size, 0);

    uint16_t idx = 0;
    std::memcpy(key.data(), &key_size, sizeof(uint16_t)); idx += 2;
    key[idx++] = 0x84;
    key[idx++] = TYPE_str;
    std::memcpy(key.data() + idx, table_name.data(), name_size);

    return find::in_slot(root_table_page, key, true);
}

}