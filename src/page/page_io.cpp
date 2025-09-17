#include "litedb/page/page_io.hpp"
#include "litedb/globals.hpp"

namespace litedb::page {

ssize_t PageIO::readPage(uint32_t page_id, uint8_t* buffer) {
    off_t offset = static_cast<off_t>(page_id) * litedb::constants::PAGE_SIZE;
    ssize_t n = ::pread(
        litedb::g::DB_FILE_DESCRIPTOR,
        buffer,
        litedb::constants::PAGE_SIZE,
        offset
    );
    return n;
}

ssize_t PageIO::writePage(uint32_t page_id, const uint8_t* buffer) {
    off_t offset = static_cast<off_t>(page_id) * litedb::constants::PAGE_SIZE;
    ssize_t n = ::pwrite(
        litedb::g::DB_FILE_DESCRIPTOR,
        buffer,
        litedb::constants::PAGE_SIZE,
        offset
    );
    return n;
}


}
