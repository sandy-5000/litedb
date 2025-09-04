#include <iostream>
#include <thread>

#include "mongolite/db_page.hpp"
#include "mongolite/thread_test.hpp"


int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.mon" << std::endl;
        return 0;
    }

    /** Mongolite Info for user */
    std::string file_path = std::string(argv[1]);
    std::cout << "***** MongoLite *****" << std::endl;
    std::cout << "Built std: " << __cplusplus << std::endl;
    std::cout << "Using Page Size: " << mongolite::PAGE_SIZE << std::endl;
    std::cout << "Using File: " << file_path << std::endl;

    // threads info
    int os_threads_count = std::thread::hardware_concurrency();
    std::cout << "Hardware concurrency: "
              << os_threads_count
              << " threads\n";

    mongolite::thread_test::launchThreads(1024);

    return 0;
}
