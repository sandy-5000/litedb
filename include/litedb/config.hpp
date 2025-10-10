#pragma once

#include <cstdint>
#include <string>

namespace litedb::config {

void print_hardware_config();
void init_db_path(const std::string& path);
void release_db_path();
void set_root_page();
void set_empty_page(uint32_t page_id, uint8_t page_type);

}
