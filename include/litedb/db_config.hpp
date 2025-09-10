#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

namespace litedb::config {


inline std::string DB_FILE_PATH;
inline int DB_FILE_DESCRIPTOR = -1;

void initDbPath(const std::string& path);

void releaseDbPath();

}
