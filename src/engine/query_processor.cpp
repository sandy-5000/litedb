#include "litedb/engine/query_processor.hpp"

namespace litedb::engine {


void process_query(const Task &task) {
    std::cout << "Client FD: " << task.client_fd << std::endl;
    std::cout << "Query: ";
    litedb::json::print_json(task.query);
    write(task.client_fd, task.query.c_str(), task.query.size());
}

void db_worker() {
    while (litedb::engine::task_q) {
        auto task = litedb::engine::task_q->pop();
        if (task.query == "\\STOP") {
            break;
        }
        process_query(task);
    }
}


}