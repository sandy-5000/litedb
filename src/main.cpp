#include <iostream>
#include <thread>

#include "litedb/page/page.hpp"
#include "litedb/buffer_manager.hpp"
#include "litedb/thread_test.hpp"
#include "litedb/config.hpp"
#include "litedb/globals.hpp"
#include "litedb/engine.hpp"


int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    /** LiteDB Info for user */
    std::string file_path = std::string(argv[1]);
    litedb::buffer_manager::BufferManager bufferManager;
    litedb::page_manager::PageManager pageManager;

    try {
        litedb::config::init_db_path(argv[1]);
        litedb::engine::buffer_manager = &bufferManager;
        litedb::engine::page_manager = &pageManager;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }


    std::cout << std::endl;
    std::cout << "********** LiteDB **********" << std::endl;
    std::cout << "  Built std: " << __cplusplus << std::endl;
    std::cout << "  Using Page Size: " << litedb::constants::PAGE_SIZE << std::endl;
    std::cout << "  Using File: " << litedb::g::DB_FILE_PATH << std::endl;

    // threads info
    int os_threads_count = std::thread::hardware_concurrency();
    std::cout << "  Hardware concurrency: "
              << os_threads_count
              << " threads\n";
    std::cout << std::endl
              << std::endl
              << std::endl;

    // litedb::thread_test::launchThreads(1024); // testing threads

    for (uint32_t i = 0; i < 3; ++i) {
        litedb::page::Page* page = litedb::engine::buffer_manager->getPage(i);
        page->printHeader();
    }

    litedb::config::release_db_path();
    return 0;
}
