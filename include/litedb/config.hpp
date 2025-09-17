#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace litedb::config {


void init_db_path(const std::string& path);
void release_db_path();
void set_root_page();
void set_empty_page(uint32_t page_id, uint8_t page_type);
void set_page_manager_root();


}
