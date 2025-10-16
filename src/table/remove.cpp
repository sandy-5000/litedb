#include <vector>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "litedb/table/remove.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/page/page.hpp"

namespace litedb::table {

uint16_t find_in_slot(std::shared_ptr<litedb::page::Page> page, std::string &key) {
    if (!page) {
        throw std::invalid_argument("[find_in_slot] PAGE: nullptr");
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

delete_responce find_and_remove_key_page(
    uint32_t page_id, std::string &key, std::vector<uint32_t> &parents
) {

    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);

    boost::upgrade_lock<boost::shared_mutex> read_lock(page->mutex());

    page->read(page_id);

    uint8_t type = page->header.type & 0xC0;
    bool is_internal = (type == 0xC0);

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    if (is_internal) {

        uint16_t offset = find_in_slot(page, key);
        uint32_t child_page_id;
        --offset;

        if (offset == 0xFFFF) {
            child_page_id = page->header.leftmost_child;
        } else {
            uint16_t record_offset = slot_ptr[offset];
            uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + record_offset
            );
            if (key_ptr[2] != 0x02) {
                return delete_responce{.new_root_id = 0};
            }
            std::memcpy(&child_page_id, key_ptr + 3, sizeof(uint32_t));
        }

        parents.push_back(page_id);
        read_lock.unlock();

        return find_and_remove_key_page(child_page_id, key, parents);

    }

    boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);

    uint16_t index = find_in_slot(page, key);

    if (index == 0) {
        return delete_responce{.new_root_id = 0};
    }
    --index;

    uint8_t* prev_key_ptr = reinterpret_cast<uint8_t*>(
        page->data_ + slot_ptr[index - 1]
    );
    uint8_t cmp = compare::keys(
        reinterpret_cast<const uint8_t*>(key.c_str()),
        prev_key_ptr, true
    );
    if (cmp == 1) {
        return delete_responce{.new_root_id = 0};
    }

    parents.push_back(page_id);

    uint32_t root_page_id = parents[0];
    std::vector<std::string> split_keys = { key };

    // return remove_keys_to_page(root_page_id, split_keys, parents, 0, false);
}

delete_responce remove::in_slot(uint32_t root_page, std::string &key) {

}

}