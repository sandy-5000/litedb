#include "litedb/table/find.hpp"

#include <cstdint>
#include <memory>

#include "litedb/page/page.hpp"
#include "litedb/table/compare.hpp"

namespace litedb::table {

uint16_t find::in_slot(std::shared_ptr<litedb::page::Page> page, std::string &key) {
    if (!page) {
        throw std::invalid_argument("[FIND::in_slot] PAGE: nullptr");
    }

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t record_count = page->header.record_count;
    uint16_t low = 0, high = record_count;

    while (low < high) {
        uint16_t mid = low + (high - low) / 2;

        uint16_t record_offset = slot_ptr[mid];
        int8_t cmp = compare::keys(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            page->data_ + record_offset, false
        );

        if (cmp == -1) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    return low;
}

}