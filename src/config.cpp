#include "litedb/config.hpp"
#include "litedb/page/page.hpp"
#include "litedb/db_globals.hpp"

namespace litedb::config {

void initDbPath(const std::string& path) {
    namespace fs = std::filesystem;

    if (fs::path(path).extension() != ".ldb") {
        throw std::invalid_argument("Database file must have a .ldb extension");
    }

    int fd;
    if (!fs::exists(path)) {
        fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            throw std::runtime_error("Failed to create database file");
        }
        DB_FILE_DESCRIPTOR = fd;
        DB_FILE_PATH = path;
        for (uint32_t i = 0; i < 3; ++i) {
            litedb::page::Page page(i);
            page.setType(i);
            page.setFreeSpace(litedb::constants::PAGE_SIZE - litedb::page::PAGE_HEADER_SIZE);
            page.write();
        }
    } else {
        if (!fs::is_regular_file(path)) {
            throw std::invalid_argument("Database file is not a regular file");
        }
        fd = ::open(path.c_str(), O_RDWR);
        if (fd == -1) {
            throw std::runtime_error("Failed to open database file");
        }
        std::cout << "File D: " << fd << std::endl;
        DB_FILE_DESCRIPTOR = fd;
        DB_FILE_PATH = path;
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