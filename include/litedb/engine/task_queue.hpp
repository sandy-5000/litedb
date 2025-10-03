#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace litedb::engine {


struct Task {
    uint32_t client_fd;
    std::string query;
};

class TaskQueue {
private:
    std::queue<Task> q_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    void push(Task task);
    Task pop();

};


}