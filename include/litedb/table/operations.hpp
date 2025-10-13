#pragma once

#include <string>

#include "litedb/engine/store.hpp"
#include "litedb/table/insert.hpp"

namespace litedb::table {

struct root_table final {
    static bool create_table(const std::string &table_name);
};

}