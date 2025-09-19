#include "litedb/config.hpp"
#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"

namespace litedb::config {

void init_db_path(const std::string& path) {
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
        litedb::g::DB_FILE_DESCRIPTOR = fd;
        litedb::g::DB_FILE_PATH = path;
        litedb::g::pages_count = 2;
        set_root_page();
        set_empty_page(1, 2);
    } else {
        if (!fs::is_regular_file(path)) {
            throw std::invalid_argument("Database file is not a regular file");
        }
        fd = ::open(path.c_str(), O_RDWR);
        if (fd == -1) {
            throw std::runtime_error("Failed to open database file");
        }
        struct stat st;
        if (::fstat(fd, &st) == -1) {
            ::close(fd);
            throw std::runtime_error("Failed to stat database file");
        }
        if (
            st.st_size % litedb::constants::PAGE_SIZE != 0 ||
            st.st_size < static_cast<off_t>(litedb::constants::PAGE_SIZE * 2)
        ) {
            ::close(fd);
            throw std::runtime_error("Database file is not valid");
        }
        litedb::g::DB_FILE_DESCRIPTOR = fd;
        litedb::g::DB_FILE_PATH = path;
        litedb::g::pages_count = st.st_size / litedb::constants::PAGE_SIZE;
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

void release_db_path() {
    if (litedb::g::DB_FILE_DESCRIPTOR != -1) {
        struct flock fl{};
        fl.l_type = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;

        fcntl(litedb::g::DB_FILE_DESCRIPTOR, F_SETLK, &fl);
        ::close(litedb::g::DB_FILE_DESCRIPTOR);
        litedb::g::DB_FILE_DESCRIPTOR = -1;
    }
}

void set_root_page() {
    litedb::page::Page page(0);
    page.setType(0);
    page.setFreeSpace(litedb::constants::PAGE_SIZE - litedb::constants::PAGE_HEADER_SIZE);
    page.setNextPage(0);
    page.write();
}

void set_empty_page(uint32_t page_id, uint8_t page_type) {
    litedb::page::Page page(page_id);
    page.setType(page_type);
    page.setFreeSpace(litedb::constants::PAGE_SIZE - litedb::constants::PAGE_HEADER_SIZE);
    page.setNextPage(0);
    page.write();
}

}