#pragma once

#include <cstdint>

#include "litedb/page/page.hpp"

namespace litedb::table::utils {

void print_key(uint8_t* key_a);

void print_slot_page(std::shared_ptr<litedb::page::Page> page);

}