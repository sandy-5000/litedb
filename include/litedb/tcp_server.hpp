#pragma once

#include <vector>
#include <sys/select.h>

#include "litedb/engine/task_queue.hpp"
#include "litedb/engine/store.hpp"

namespace litedb::server {


class TCPServer {
private:
    void accept_new_client();
    void read_from_client(int client_fd);
    void handle_message(uint32_t client_fd, const std::string& msg);

    int server_fd_;
    int port_;
    fd_set master_set_;
    int max_fd_;
    std::vector<int> clients_;
    volatile bool running_ = true;
    const int8_t max_connections = 10;

public:
    TCPServer(int port);
    ~TCPServer();

    void run();
    void stop();

};


}
