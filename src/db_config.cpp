#include "litedb/db_config.hpp"

namespace litedb::config {


void initDbPath(const std::string& path) {
    namespace fs = std::filesystem;

    if (fs::path(path).extension() != ".ldb") {
        throw std::invalid_argument("Database file must have a .ldb extension");
    }

    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        throw std::invalid_argument("Database file does not exist or is not a regular file");
    }

    int fd = ::open(path.c_str(), O_RDWR);
    if (fd == -1) {
        throw std::runtime_error("Failed to open database file");
    }

    struct flock fl{};
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        ::close(fd);
        throw std::runtime_error("Database file is already locked by another process");
    }

    DB_FILE_PATH = path;
    DB_FILE_DESCRIPTOR = fd;
}

void releaseDbPath() {
    if (DB_FILE_DESCRIPTOR != -1) {
        struct flock fl{};
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        fcntl(DB_FILE_DESCRIPTOR, F_SETLK, &fl);
        ::close(DB_FILE_DESCRIPTOR);
        DB_FILE_DESCRIPTOR = -1;
    }
}

}