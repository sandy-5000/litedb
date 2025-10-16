#include <stdexcept>

#include "litedb/page/io.hpp"

namespace litedb::page::io {

ssize_t read_page(uint32_t page_id, uint8_t* buffer) {
    off_t offset = static_cast<off_t>(page_id) * litedb::constants::DB_PAGE_SIZE;
    return ::pread(
        g::DB_FILE_DESCRIPTOR,
        buffer,
        constants::DB_PAGE_SIZE,
        offset
    );
}

ssize_t write_page(uint32_t page_id, const uint8_t* buffer) {
    off_t offset = static_cast<off_t>(page_id) * litedb::constants::DB_PAGE_SIZE;
    return ::pwrite(
        g::DB_FILE_DESCRIPTOR,
        buffer,
        constants::DB_PAGE_SIZE,
        offset
    );
}

}
