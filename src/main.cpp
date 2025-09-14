#include <iostream>
#include <thread>

#include "litedb/page/page.hpp"
#include "litedb/thread_test.hpp"
#include "litedb/db_config.hpp"


int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    /** LiteDB Info for user */
    std::string file_path = std::string(argv[1]);

    try {
        litedb::config::initDbPath(argv[1]);
        std::cout << "DB Path set to: " << litedb::config::DB_FILE_PATH << "\n";
        litedb::config::releaseDbPath();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }


    std::cout << "***** LiteDB *****" << std::endl;
    std::cout << "Built std: " << __cplusplus << std::endl;
    std::cout << "Using Page Size: " << litedb::constants::PAGE_SIZE << std::endl;
    std::cout << "Using File: " << file_path << std::endl;

    // threads info
    int os_threads_count = std::thread::hardware_concurrency();
    std::cout << "Hardware concurrency: "
              << os_threads_count
              << " threads\n";

    // litedb::thread_test::launchThreads(1024); // testing threads
    litedb::page::Page page(1);
    page.lockUnique();
    page.setFreeSpace(1234);
    page.setChecksum(5);
    page.printHeader();
    std::cout << page.getFreeSpace() << std::endl;
    page.unlockUnique();

    return 0;
}
