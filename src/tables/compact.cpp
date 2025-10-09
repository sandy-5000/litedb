#include "litedb/tables/compact.hpp"

namespace litedb::table::utils {


void compact_page::adjust(std::shared_ptr<litedb::page::Page> page, std::vector<uint16_t> &positions) {
    auto record_count = page->get_record_count();
    uint16_t space_end = litedb::constants::PAGE_SIZE;
    for (int i = record_count; i-- > 0; ) {
        uint16_t old_offset = positions[i];
        uint16_t record_size;
        std::memcpy(&record_size, page->data_ + old_offset, sizeof(uint16_t));
        space_end -= record_size;
        std::memmove(page->data_ + space_end, page->data_ + old_offset, record_size);
        uint16_t* slot_entry = reinterpret_cast<uint16_t*>(
            page->data_ + litedb::constants::PAGE_HEADER_SIZE + i * sizeof(uint16_t)
        );
        *slot_entry = space_end;
    }
    page->set_free_space_offset(space_end);
    uint16_t free_space = space_end - (litedb::constants::PAGE_HEADER_SIZE + record_count * sizeof(uint16_t));
    std::memset(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE + record_count * sizeof(uint16_t),
        0, free_space
    );
}

void compact_page::key(std::shared_ptr<litedb::page::Page> page) {
    auto record_count = page->get_record_count();
    if (0 == record_count) {
        return;
    }
    std::vector<uint16_t> positions(record_count);
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < record_count; ++i, ++slot_ptr) {
        positions[i] = *slot_ptr;
    }
    sort(positions.begin(), positions.end());
    adjust(page, positions);
    page->set_dirty();
}

void compact_page::data_leaf(std::shared_ptr<litedb::page::Page> page) {
    auto record_count = page->get_record_count();
    if (0 == record_count) {
        return;
    }
    std::vector<uint16_t> positions(record_count);
    slot_data_leaf* slot_ptr = reinterpret_cast<slot_data_leaf*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < record_count; ++i, ++slot_ptr) {
        positions[i] = slot_ptr->offset;
    }
    sort(positions.begin(), positions.end());
    adjust(page, positions);
    page->set_dirty();
}

void compact_page::run(std::shared_ptr<litedb::page::Page> page) {
    if (!page) {
        return;
    }
    uint8_t type = page->get_type() & 0xC0;
    switch (type) {
        case 0xC0:
        case 0x80:
            key(page);
            break;
        case 0x00:
            data_leaf(page);
            break;
        default:
            break;
    }
}


}