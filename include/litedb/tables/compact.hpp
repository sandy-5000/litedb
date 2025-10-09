#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstring>

#include "litedb/page/page.hpp"
#include "litedb/tables/slot.hpp"

namespace litedb::table::utils {

struct compact_page final {
    static void adjust(std::shared_ptr<litedb::page::Page> page, std::vector<uint16_t> &positions);
    static void key(std::shared_ptr<litedb::page::Page> page);
    static void data_leaf(std::shared_ptr<litedb::page::Page> page);
    static void run(std::shared_ptr<litedb::page::Page> page);
};


}