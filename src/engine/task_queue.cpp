#include "litedb/engine/task_queue.hpp"


namespace litedb::engine {


void TaskQueue::push(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        q_.push(std::move(task));
    }
    cv_.notify_one();
}

Task TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() {
        return !q_.empty();
    });
    Task task = std::move(q_.front());
    q_.pop();
    return task;
}


}