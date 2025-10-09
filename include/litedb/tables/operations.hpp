#pragma once

#include <string>

#include "litedb/engine/store.hpp"
#include "litedb/tables/insert.hpp"

namespace litedb::table::utils {


struct root_table final {
    static bool create_table(std::string &table_name);
};


}