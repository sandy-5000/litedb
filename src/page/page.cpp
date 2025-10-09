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

void Page::read_empty(uint32_t page_id) {
    id_ = page_id;
    if (get_id() == id_) {
        dirty_ = false;
    } else {
        set_id(id_);
        dirty_ = true;
    }
}

ssize_t Page::read(uint32_t page_id) {
    if (page_id != INVALID_PAGE_ID && page_id == id_) {
        return litedb::constants::PAGE_SIZE;
    }
    return force_read(page_id);
}

ssize_t Page::force_read(uint32_t page_id) {
    if (INVALID_PAGE_ID == page_id) {
        return 0;
    }
    if (dirty_) {
        write();
    }
    id_ = page_id;
    ssize_t byte_count = io::read_page(page_id, data_);
    if (get_id() == id_) {
        dirty_ = false;
    } else {
        set_id(id_);
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

void Page::set_dirty() {
    dirty_ = true;
}

bool Page::is_dirty() {
    return dirty_;
}

bool Page::is_empty() {
    return 0 == get_record_count();
}

void Page::clear() {
    id_ = INVALID_PAGE_ID;
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    set_id(INVALID_PAGE_ID);
    dirty_ = false;
}


}
