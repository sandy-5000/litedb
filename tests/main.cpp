#include <cstdint>
#include <iostream>
#include <string>

#include "litedb/config.hpp"
#include "litedb/engine/store.hpp"
#include "tests/key_compare.hpp"
#include "tests/page_allocation.hpp"
#include "tests/table.hpp"

int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    std::string file_path = std::string(argv[1]);

    try {
        litedb::config::init_db_path(argv[1]);

        static litedb::engine::root_manager::RootManager rootManager;
        static litedb::engine::buffer_manager::BufferManager bufferManager;
        litedb::engine::root_manager_ = &rootManager;
        litedb::engine::buffer_manager_ = &bufferManager;

        litedb::config::print_hardware_config();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    // compare_test();
    // test_page_allocations();
    create_tables();
    check_root_table(false);
    check_root_table(true);
    delete_tables();
    find_tables();
    check_root_table(false);
    check_root_table(true);

    // std::vector<uint32_t> pages = {1, 34022, 4944, 68774, 85065, 85067};
    std::vector<uint32_t> pages = {};
    for (auto i : pages) {
        std::cout << "page: " << i << std::endl;
        auto buffer = litedb::engine::buffer_manager_->get_main_buffer();
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(i);
        page->read(i);
        page->print_header();
        // litedb::table::utils::print_slot_page(page);
    }

    std::cout << std::endl;

    return 0;
}