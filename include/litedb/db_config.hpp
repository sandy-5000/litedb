#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace litedb::config {


inline std::string DB_FILE_PATH;

inline void initDbPath(const std::string& path) {
    namespace fs = std::filesystem;

    if (fs::path(path).extension() != ".ldb") {
        throw std::invalid_argument("Database file must have a .ldb extension");
    }

    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        throw std::invalid_argument("Database file does not exist or is not a regular file");
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Database file cannot be opened in binary mode");
    }

    DB_FILE_PATH = path;
}


}
