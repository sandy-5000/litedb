#include <iostream>
#include <thread>
#include <csignal>

#include "litedb/page/page.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/thread_test.hpp"
#include "litedb/config.hpp"
#include "litedb/globals.hpp"
#include "litedb/json.hpp"
#include "litedb/tcp_server.hpp"


litedb::server::TCPServer* global_server = nullptr;

void signal_handler(int signal) {
    if (global_server) {
        global_server->stop();
    }
}

void* server_thread_func(void* arg) {
    litedb::server::TCPServer* server = (litedb::server::TCPServer*)arg;
    server->run();
    return nullptr;
}

int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    /** LiteDB Info for user */
    std::string file_path = std::string(argv[1]);
    litedb::root_manager::RootManager rootManager;
    litedb::buffer_manager::BufferManager bufferManager;

    try {
        litedb::config::init_db_path(argv[1]);
        litedb::engine::root_manager = &rootManager;
        litedb::engine::buffer_manager = &bufferManager;
        litedb::config::print_hardware_config();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    signal(SIGINT, signal_handler);
    litedb::server::TCPServer server(8008);
    global_server = &server;

    pthread_t server_thread;
    pthread_create(&server_thread, nullptr, server_thread_func, &server);
    std::cout << "Server running. Press Ctrl+C to stop...\n\n";
    pthread_join(server_thread, nullptr);

    // litedb::thread_test::launchThreads(1024); // testing threads

    // for (uint32_t i = 0; i < 2; ++i) {
    //     litedb::page::Page* page = litedb::engine::buffer_manager->getEmptyPage(i);
    //     page->lockShared();
    //     page->read(i);
    //     page->printHeader();
    //     page->unlockShared();
    // }

    litedb::config::release_db_path();
    std::cout << "Server exited gracefully.\n\n";
    return 0;
}
