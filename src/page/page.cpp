#include "litedb/page/page.hpp"

namespace litedb::page {


Page::Page() : Page(-1) {}

Page::Page(uint32_t id) : id_(id), PageHeader(data_, &mtx_) {
    // std::unique_lock lock(mtx_);
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    writeValue(PAGE_ID_OFFSET, id);
}

ssize_t Page::read(uint32_t page_id) {
    if (page_id == id_) {
        return litedb::constants::PAGE_SIZE;
    }
    return forceRead(page_id);
}

ssize_t Page::forceRead(uint32_t page_id) {
    // std::unique_lock lock(mtx_);
    if (static_cast<uint32_t>(-1)== page_id) {
        throw std::runtime_error("Invalid page id for read()");
    }
    id_ = page_id;
    ssize_t byte_count = PageIO::readPage(page_id, data_);
    dirty_ = false;
    return byte_count;
}

ssize_t Page::write() {
    // std::unique_lock lock(mtx_);
    if (static_cast<uint32_t>(-1) == id_) {
        return -1;
    }
    ssize_t byte_count = PageIO::writePage(id_, data_);
    dirty_ = false;
    return byte_count;
}

void Page::setDirty() {
    dirty_ = true;
}

bool Page::isDirty() {
    return dirty_;
}

bool Page::isEmpty() {
    return 0 == getRecordCount();
}

void Page::clear() {
    id_ = -1;
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
}


}
