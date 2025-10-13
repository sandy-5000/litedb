#pragma once

#include <cstdint>
#include <memory>

#include "litedb/page/page.hpp"

namespace litedb::table {

struct find final {
    static uint16_t in_slot(std::shared_ptr<litedb::page::Page> page, std::string &key);
};


}