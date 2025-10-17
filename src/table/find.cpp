#include <cstdint>
#include <memory>
#include <string>
#include <cstring>
#include <iostream>

#include "litedb/table/find.hpp"
#include "litedb/page/page.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/table/utils.hpp"

namespace litedb::table {

std::string find_in_slot_page(uint32_t page_id, std::string &key, bool is_unique) {

    auto buffer = engine::buffer_manager_->get_main_buffer();

    while (true) {
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
        int8_t cmp_flag = 0 - is_internal;

        while (low < high) {
            uint16_t mid = low + (high - low) / 2;

            uint16_t record_offset = slot_ptr[mid];
            int8_t cmp = compare::keys(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                page->data_ + record_offset, is_unique
            );

            if (cmp > cmp_flag) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }

        if (is_internal) {
            if (low == 0) {
                page_id = page->header.leftmost_child;
            } else {
                --low;
                uint16_t offset = slot_ptr[low];
                uint32_t child_page_id;
                std::memcpy(&child_page_id, page->data_ + offset + 3, sizeof(uint32_t));
                page_id = child_page_id;
            }
            page->unlock_shared();
        } else {
            if (low == page->header.record_count) {
                return std::string({0x04, 0x00, 0x04, 0x00});
            }
            uint16_t offset = slot_ptr[low], key_size;
            std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
            page->unlock_shared();
            return std::string(page->data_ + offset, page->data_ + offset + key_size);
        }
    }

    return "";
}

std::string find::in_slot(uint32_t page_id, std::string &key, bool is_unique) {
    return find_in_slot_page(page_id, key,is_unique);
}

}
