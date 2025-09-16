#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

namespace litedb::config {


void initDbPath(const std::string& path);
void releaseDbPath();


}
