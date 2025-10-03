#pragma once

#include <iostream>

#include "litedb/engine/task_queue.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/json.hpp"

namespace litedb::engine {

void process_query(const Task &task);
void db_worker();

}
