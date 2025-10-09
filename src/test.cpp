#include <iostream>
#include <thread>
#include <csignal>

#include "litedb/page/page.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"
#include "litedb/engine/task_queue.hpp"
#include "litedb/engine/query_processor.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/tables/compare.hpp"
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
    litedb::engine::TaskQueue taskQueue;

    try {
        litedb::config::init_db_path(argv[1]);
        litedb::engine::root_manager = &rootManager;
        litedb::engine::buffer_manager = &bufferManager;
        litedb::engine::task_q = &taskQueue;
        litedb::config::print_hardware_config();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    auto buffer = litedb::engine::buffer_manager->get_buffer("root");
    if (buffer) {
        std::shared_ptr<litedb::page::Page> page = (*buffer)->get_page(0);
        page->lock_shared();
        page->read(0);
        page->print_header();
        std::cout << std::endl;
        page->unlock_shared();
    }

    uint8_t bytes_a[] = {0x0f, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x02, 0x73, 0x61, 0x6e, 0x64, 0x79, 0x00, 0x00};
    std::string str_a(reinterpret_cast<const char*>(bytes_a), sizeof(bytes_a));

    uint8_t bytes_b[] = {0x0b, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00, 0x02, 0x7a, 0x00, 0x00};
    std::string str_b(reinterpret_cast<const char*>(bytes_b), sizeof(bytes_b));

    std::cout << "Compare:" << " ";
    std::cout << (int32_t)litedb::table::utils::compare::key(
        reinterpret_cast<const uint8_t*>(str_a.data()), 0, 0,
        reinterpret_cast<const uint8_t*>(str_b.data()), 0, 0
    ) << std::endl << std::endl;


    std::thread worker(litedb::engine::db_worker);

    signal(SIGINT, signal_handler);
    litedb::server::TCPServer server(8008);
    global_server = &server;

    pthread_t server_thread;
    pthread_create(&server_thread, nullptr, server_thread_func, &server);
    std::cout << "Test Server running. Press Ctrl+C to stop...\n\n";
    pthread_join(server_thread, nullptr);



    // litedb::thread_test::launchThreads(1024); // testing threads
    worker.join();

    litedb::config::release_db_path();
    std::cout << "Server exited gracefully.\n\n";
    return 0;
}
