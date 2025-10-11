#include <iostream>

#include <bsoncxx/json.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

#include "litedb/config.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/table/data_types.hpp"

void json_test() {
    std::string a = R"({"name":"sandy-blaze"})";

    bsoncxx::document::value doc = bsoncxx::from_json(a);
    auto view = doc.view();

    std::cout << "\n     ----- BSON -----\n";
    std::cout << "The BSON document view (as JSON): " << bsoncxx::to_json(view) << "\n";
    std::cout << "Value of 'name': " << view["name"].get_string().value << "\n";
    std::cout << "     ----- BSON -----\n\n";
}

int32_t main(int argc, char* argv[]) {
    json_test();

    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    /** LiteDB Info for user */
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

    auto buffer = litedb::engine::buffer_manager_->getBuffer("root", 0);
    if (!buffer) {
        std::cerr << "Error: " << "Buffer not found" << "\n";
        return 1;
    }

    // for (int i = 0; i < 100; ++i) {
    //     uint32_t new_page_id = litedb::engine::root_manager_->get_free_page();
    //     std::shared_ptr<litedb::page::Page> page = buffer->getPage(new_page_id);
    //     page->read_empty(new_page_id);
    //     page->header.record_count = 1;
    //     std::cout << new_page_id << std::endl;
    // }

    // for (int i = 1; i < 30; ++i) {
    //     litedb::engine::root_manager_->add_free_page(2 * i);
    // }

    // std::cout << "Fetching pages after freeing" << std::endl;
    // for (int i = 0; i < 10; ++i) {
    //     uint32_t new_page_id = litedb::engine::root_manager_->get_free_page();
    //     std::shared_ptr<litedb::page::Page> page = buffer->getPage(new_page_id);
    //     page->read_empty(new_page_id);
    //     page->header.record_count = 1;
    //     std::cout << new_page_id << std::endl;
    // }

    std::string str_a = "sandy-blaze";
    uint16_t total_size;

    std::vector<uint8_t> k_a(
        (2 + 1) +
        (1 + str_a.size() + 1) +
        (1 + sizeof(uint32_t)) +
        (1 + sizeof(uint64_t)) +
        (1)
    );
    total_size = static_cast<uint16_t>(k_a.size());
    std::memcpy(k_a.data(), &total_size, sizeof(uint16_t));
    k_a[2] = 0x82;

    k_a[3] = litedb::table::TYPE_str;
    std::memcpy(k_a.data() + 4, str_a.data(), str_a.size());
    k_a[4 + str_a.size()] = '\0';

    int32_t val_0 = 20;
    k_a[4 + str_a.size() + 1] = litedb::table::TYPE_i32;
    std::memcpy(k_a.data() + 4 + str_a.size() + 1 + 1, &val_0, sizeof(int32_t));


    std::vector<uint8_t> k_b(
        (2 + 1) +
        (1 + str_a.size() + 1) +
        (1 + sizeof(uint64_t)) +
        (1)
    );

    return 0;
}
