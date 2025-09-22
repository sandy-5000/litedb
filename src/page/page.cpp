#include "litedb/page/page.hpp"

namespace litedb::page {


Page::Page() : PageHeader(data_) {
    clear();
}

Page::~Page() {
    if (dirty_)  {
        write();
    }
}

uint32_t Page::id() const {
    return id_;
}

void Page::readEmpty(uint32_t page_id) {
    id_ = page_id;
    if (getId() == id_) {
        dirty_ = false;
    } else {
        setId(id_);
        dirty_ = true;
    }
}

ssize_t Page::read(uint32_t page_id) {
    if (page_id != INVALID_PAGE_ID && page_id == id_) {
        return litedb::constants::PAGE_SIZE;
    }
    return forceRead(page_id);
}

ssize_t Page::forceRead(uint32_t page_id) {
    if (INVALID_PAGE_ID == page_id) {
        return 0;
    }
    if (dirty_) {
        write();
    }
    id_ = page_id;
    ssize_t byte_count = io::read_page(page_id, data_);
    if (getId() == id_) {
        dirty_ = false;
    } else {
        setId(id_);
        dirty_ = true;
    }
    return byte_count;
}

ssize_t Page::write() {
    if (INVALID_PAGE_ID == id_) {
        return 0;
    }
    ssize_t byte_count = io::write_page(id_, data_);
    if (byte_count == litedb::constants::PAGE_SIZE) {
        dirty_ = false;
    }
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
    id_ = INVALID_PAGE_ID;
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    setId(INVALID_PAGE_ID);
    dirty_ = false;
}


}
