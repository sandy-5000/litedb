#include <vector>
#include <iostream>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "litedb/table/remove.hpp"
#include "litedb/table/key.hpp"
#include "litedb/table/find.hpp"
#include "litedb/table/utils.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/page/page.hpp"

namespace litedb::table {

void remove_record_in_page(std::shared_ptr<litedb::page::Page> page, std::string &key, uint16_t index) {
    page->set_dirty();

    uint8_t* old_ptr = reinterpret_cast<uint8_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE + sizeof(uint16_t) * (index + 1)
    );
    uint8_t* new_ptr = old_ptr - sizeof(uint16_t);
    uint16_t date_size = (page->header.record_count - index - 1) * sizeof(uint16_t);

    uint16_t key_size;
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    std::memcpy(&key_size, page->data_ + slot_ptr[index], sizeof(uint16_t));

    std::memmove(new_ptr, old_ptr, date_size);
    --page->header.record_count;
    page->header.free_space += sizeof(uint16_t) + key_size;
}

delete_responce find_and_remove_key_page(uint32_t page_id, std::string &key) {
    std::vector<uint32_t> parents;

    auto buffer = engine::buffer_manager_->get_main_buffer();

    while (page_id) {
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);

        boost::upgrade_lock<boost::shared_mutex> read_lock(page->mutex());

        page->read(page_id);

        uint8_t type = page->header.type & 0xC0;
        bool is_internal = (type == 0xC0);

        uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
            page->data_ + litedb::constants::PAGE_HEADER_SIZE
        );

        uint16_t index = find::position_in_slot(page, key, false);

        if (index == page->header.record_count) {
            read_lock.unlock();
            page_id = page->header.next_page;
            continue;
        }

        if (is_internal) {

            uint32_t child_page_id;

            uint16_t record_offset = slot_ptr[index];
            uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + record_offset
            );
            if (key_ptr[2] != 0x06) {
                return delete_responce{
                    .new_root_id = 0,
                    .count = 0,
                };
            }
            std::memcpy(&child_page_id, key_ptr + 3, sizeof(uint32_t));

            parents.push_back(page_id);
            read_lock.unlock();

            page_id = child_page_id;
            continue;
        }

        parents.push_back(page_id);

        uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
            page->data_ + slot_ptr[index]
        );
        uint8_t cmp = key::compare(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            key_ptr, true
        );
        if (cmp != 0) {
            std::cout << "came\n";
            return delete_responce{
                .new_root_id = parents[0],
                .count = 0
            };
        }

        boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);

        uint32_t root_page_id = parents[0];
        // std::vector<std::string> remove_keys = { key };

        remove_record_in_page(page, key, index);

        delete_responce responce;
        responce.new_root_id = root_page_id;
        responce.count = 1;
        responce.data = key;

        return responce;
    }

    return delete_responce{
        .new_root_id = 0,
        .count = 0,
    };
}

delete_responce remove::in_slot(uint32_t root_page, std::string &key) {
    std::vector<uint32_t> parents;
    return find_and_remove_key_page(root_page, key);
}

}