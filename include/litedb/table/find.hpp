#pragma once

#include <string>

#include "litedb/page/page.hpp"

namespace litedb::table {

struct find {
    static std::string in_slot(uint32_t page_id, std::string &key, bool is_unique);
    static uint16_t position_in_slot(std::shared_ptr<litedb::page::Page> page, std::string &key, bool upper_b);
};


}