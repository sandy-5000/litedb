#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>

#include "litedb/tcp_server.hpp"
#include "litedb/json.hpp"

namespace litedb::server {


TCPServer::TCPServer(int port) : port_(port) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("socket");
        exit(1);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    if (listen(server_fd_, 5) < 0) {
        perror("listen");
        exit(1);
    }

    FD_ZERO(&master_set_);
    FD_SET(server_fd_, &master_set_);
    max_fd_ = server_fd_;

    std::cout << "Server listening on port " << port_ << "...\n";
}


TCPServer::~TCPServer() {
    for (int fd : clients_) {
        close(fd);
    }
    close(server_fd_);
}

void TCPServer::run() {
    while (running_) {
        fd_set read_set = master_set_;
        if (select(max_fd_ + 1, &read_set, nullptr, nullptr, nullptr) < 0) {
            if (!running_) {
                break;
            }
            perror("select");
            break;
        }
        for (int fd = 0; fd <= max_fd_; ++fd) {
            if (FD_ISSET(fd, &read_set)) {
                if (fd == server_fd_) {
                    accept_new_client();
                } else {
                    read_from_client(fd);
                }
            }
        }
    }
    std::cout << "Server stopped\n";
}

void TCPServer::accept_new_client() {
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd >= 0) {
        if (running_ && clients_.size() >= max_connections) {
            close(client_fd);
            std::cout << "Max clients reached. Rejecting new connection.\n";
            return;
        }
        FD_SET(client_fd, &master_set_);
        if (client_fd > max_fd_) {
            max_fd_ = client_fd;
        }
        clients_.push_back(client_fd);
        std::cout << "New client connected: " << client_fd << "\n\n";
    }
}

void TCPServer::read_from_client(int client_fd) {
    char buffer[10240];
    int n = read(client_fd, buffer, sizeof(buffer));
    if (n <= 0) {
        std::cout << "Client disconnected: " << client_fd << "\n";
        close(client_fd);
        FD_CLR(client_fd, &master_set_);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), client_fd), clients_.end());
    } else {
        std::string msg(buffer, n);
        handle_message(msg);
    }
}

void TCPServer::handle_message(const std::string& msg) {
    litedb::json::print_json(msg);
}

void TCPServer::stop() {
    running_ = false;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(port_);
        connect(sock, (sockaddr*)&addr, sizeof(addr));
        close(sock);
    }
}


}