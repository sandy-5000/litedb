#pragma once

#include <string>

#include "litedb/constants.hpp"

namespace litedb::g {

extern int DB_FILE_DESCRIPTOR;
extern uint32_t pages_count;

extern std::string DB_FILE_PATH;

inline const uint32_t PAGE_BODY_SIZE = litedb::constants::DB_PAGE_SIZE - litedb::constants::PAGE_HEADER_SIZE;

}
