#include <cstdint>
#include <memory>
#include <string>
#include <cstring>
#include <iostream>

#include "litedb/table/find.hpp"
#include "litedb/page/page.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/table/key.hpp"
#include "litedb/table/utils.hpp"

namespace litedb::table {

std::string find_in_slot(
    uint32_t page_id, std::string &key, bool is_unique, bool upper_b
) {

    auto buffer = engine::buffer_manager_->get_main_buffer();

    while (true) {
        if (page_id == 0) {
            return std::string({0x04, 0x00, 0x04, 0x00});
        }

        std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);
        page->lock_shared();
        page->read(page_id);

        uint8_t type = page->header.type & 0xC0;

        uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
            page->data_ + litedb::constants::PAGE_HEADER_SIZE
        );

        uint16_t record_count = page->header.record_count;
        uint16_t low = 0, high = record_count;

        bool is_internal = (type == 0xC0);
        int8_t cmp_flag = 0 - (int8_t)upper_b;

        while (low < high) {
            uint16_t mid = low + (high - low) / 2;

            uint16_t record_offset = slot_ptr[mid];
            int8_t cmp = key::compare(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                page->data_ + record_offset, is_unique
            );

            if (cmp_flag < cmp) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }

        if (low == page->header.record_count) {
            page_id = page->header.next_page;
            page->unlock_shared();
            continue;
        }

        if (is_internal) {
            uint16_t offset = slot_ptr[low];
            uint32_t child_page_id;
            std::memcpy(&child_page_id, page->data_ + offset + 3, sizeof(uint32_t));
            page_id = child_page_id;
            page->unlock_shared();
        } else {
            uint16_t offset = slot_ptr[low], key_size;
            std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
            page->unlock_shared();
            return std::string(page->data_ + offset, page->data_ + offset + key_size);
        }
    }

    return "";
}

uint16_t find_position_in_slot(
    std::shared_ptr<litedb::page::Page> page, std::string &key, bool upper_b
) {
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->header.record_count;
    uint16_t low = 0, high = record_count;
    int8_t cmp_flag = 0 - (int8_t)upper_b;

    while (low < high) {
        uint16_t mid = low + (high - low) / 2;

        uint16_t record_offset = slot_ptr[mid];
        int8_t cmp = key::compare(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            page->data_ + record_offset, false
        );

        if (cmp_flag < cmp) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return low;
}

std::string find::in_slot(uint32_t page_id, std::string &key, bool is_unique) {
    return find_in_slot(page_id, key, is_unique, false);
}

uint16_t find::position_in_slot(std::shared_ptr<litedb::page::Page> page, std::string &key, bool upper_b) {
    return find_position_in_slot(page, key, upper_b);
}

}
