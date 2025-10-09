#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>

#include "litedb/page/page.hpp"
#include "litedb/tables/compare.hpp"
#include "litedb/tables/slot.hpp"

namespace litedb::table::utils {


struct find final {
    static uint16_t key_internal(std::shared_ptr<litedb::page::Page> page, std::string &key);
    static uint16_t key_leaf(std::shared_ptr<litedb::page::Page> page, std::string &key);
    static uint16_t data_internal(std::shared_ptr<litedb::page::Page> page, uint64_t key);
    static uint16_t data_leaf(std::shared_ptr<litedb::page::Page> page, uint64_t key);
};


}

