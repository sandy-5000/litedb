#pragma once

#include <string>

namespace litedb::table {

struct find {
    static std::string in_slot(uint32_t page_id, std::string &key, bool is_unique);
};


}