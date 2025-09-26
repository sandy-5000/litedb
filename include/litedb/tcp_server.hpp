#pragma once
#include <vector>
#include <sys/select.h>

namespace litedb::server {


class TCPServer {
private:
    void accept_new_client();
    void read_from_client(int client_fd);
    void handle_message(const std::string& msg);

    int server_fd_;
    int port_;
    fd_set master_set_;
    int max_fd_;
    std::vector<int> clients_;
    volatile bool running_ = true;

public:
    TCPServer(int port);
    ~TCPServer();

    void run();
    void stop();

};


}
