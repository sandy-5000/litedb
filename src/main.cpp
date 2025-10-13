#include <iostream>

#include <bsoncxx/json.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

#include "litedb/config.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"

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

    return 0;
}
