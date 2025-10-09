#include "litedb/tables/operations.hpp"

namespace litedb::table::utils {


bool root_table::create_table(std::string &table_name) {

    std::cout << "create_table: " << table_name << std::endl;

    auto root_manager = litedb::engine::root_manager;
    auto root_page = root_manager->get_root();

    uint32_t root_table_page = 0;
    std::memcpy(
        &root_table_page,
        root_page->data_ + litedb::constants::PAGE_HEADER_SIZE,
        sizeof(uint32_t)
    );

    uint64_t seq_number = 0;
    std::memcpy(
        &seq_number,
        root_page->data_ + sizeof(uint32_t) + litedb::constants::PAGE_HEADER_SIZE,
        sizeof(uint64_t)
    );

    if (root_table_page == 0) {
        uint32_t new_root_table_page = root_manager->get_free_page();
        std::memcpy(
            root_page->data_ + litedb::constants::PAGE_HEADER_SIZE,
            &new_root_table_page,
            sizeof(uint32_t)
        );
        root_table_page = new_root_table_page;
        root_page->set_dirty();

        litedb::page::Page new_page;
        new_page.read_empty(new_root_table_page);
        new_page.set_id(new_root_table_page);
        new_page.set_type(0x80);
        new_page.set_record_count(0);
        new_page.set_free_space_offset(litedb::constants::PAGE_SIZE);
        new_page.set_possible_parent(0);
        new_page.set_free_space(litedb::constants::PAGE_SIZE - litedb::constants::PAGE_HEADER_SIZE);
        new_page.write();
    }

    std::string key(3 + table_name.size() + 10, '\0');
    uint16_t size = key.size();

    key[0] = size & 0xFF;
    key[1] = (size >> 8) & 0xFF;

    key[2] = 0x02;
    std::memcpy(&key[3], table_name.data(), table_name.size());

    key[size - 10] = 0x12;
    std::memcpy(&key[size - 9], &seq_number, sizeof(uint64_t));

    key[size - 1] = 0x00;

    std::cout << "create_table_key: " << key << std::endl;

    auto changes = insert::key(table_name, root_table_page, key, false);

    if (changes.size()) {
        std::cout << "creation success" << std::endl;
        return true;
    } else {
        std::cout << "creation failed" << std::endl;
        return false;
    }

    /**
     * [size_bytes|2bytes][table_name<dynamic_size>][0x01][root_page_id|4bytes][0x00] - for table
     * [size_bytes|2bytes][table_name<dynamic_size>][0x02][rand_seq_no|8bytes][mongoxx_bson_object][0x00] - for index
     *   e.g bson - { "name": 1, "age": -1 }
     *            - { "phone_no": 1, "unique": true }
     *            - { "id_1": 1, "id_2": -1, "unique": true }
     */

};


}