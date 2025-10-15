#include <iostream>
#include <cstring>
#include <vector>

#include "litedb/page/page.hpp"
#include "litedb/page/io.hpp"

namespace litedb::page {

Page::Page() {
    clear();
}

Page::~Page() {
    if (dirty_)  {
        write();
    }
}

void Page::read_empty(uint32_t page_id) {
    if (header.id == page_id) {
        dirty_ = false;
    } else {
        header.id = page_id;
        dirty_ = true;
    }
}

ssize_t Page::read(uint32_t page_id) {
    if (page_id != constants::INVALID_PAGE_ID && page_id == header.id) {
        return litedb::constants::PAGE_SIZE;
    }
    return force_read(page_id);
}

ssize_t Page::force_read(uint32_t page_id) {
    if (constants::INVALID_PAGE_ID == page_id) {
        return 0;
    }
    if (dirty_) {
        write();
    }
    ssize_t byte_count = io::read_page(page_id, data_);
    std::memcpy(&header, data_, sizeof(page_header));
    dirty_ = false;
    return byte_count;
}

ssize_t Page::write() {
    if (constants::INVALID_PAGE_ID == header.id) {
        return 0;
    }
    std::memcpy(data_, &header, sizeof(page_header));
    ssize_t byte_count = io::write_page(header.id, data_);
    if (byte_count == litedb::constants::PAGE_SIZE) {
        dirty_ = false;
    }
    return byte_count;
}

void Page::set_dirty() {
    dirty_ = true;
}

bool Page::is_dirty() const {
    return dirty_;
}

void Page::clear() {
    std::memset(&header, 0, sizeof(page_header));
    header.id = constants::INVALID_PAGE_ID;
    std::memset(data_, 0, litedb::constants::PAGE_SIZE);
    dirty_ = false;
}

void Page::print_header() const {
    std::vector<uint8_t> page_header(constants::PAGE_HEADER_SIZE, 0);
    std::memcpy(page_header.data(), &header, sizeof(header));
    std::cout << "------ Page header ---"
              << " (" << header.id << ")"
              << std::endl;
    for (int i = 0; i < constants::PAGE_HEADER_SIZE; ++i) {
        std::printf("%02X ", page_header[i]);
        if ((i + 1) % 8 == 0) std::cout << std::endl;
    }
}

}