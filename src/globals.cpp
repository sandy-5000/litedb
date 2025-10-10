#include "litedb/globals.hpp"

#include <string>

namespace litedb::g {

int DB_FILE_DESCRIPTOR = -1;
std::string DB_FILE_PATH;
uint32_t pages_count;

}
