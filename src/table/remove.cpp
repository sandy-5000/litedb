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

void remove_record_in_page(std::shared_ptr<litedb::page::Page> page, uint16_t index) {
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

void remove_record(
    uint32_t &root_page_id,
    std::vector<d_node> &parents,
    uint32_t child_page_id,
    std::optional<boost::upgrade_to_unique_lock<boost::shared_mutex>> write_lock_opt = std::nullopt
) {
    uint32_t page_id = parents.back().page_id;
    std::string key = std::move(parents.back().key);
    parents.pop_back();

    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);

    if (!write_lock_opt) {
        page->lock_unique();
    }
    page->set_dirty();

    if (child_page_id > 0) {
        auto root_manager = engine::root_manager_;
        root_manager->add_free_page(child_page_id);
    }

    uint16_t index = 0;
    while (true) {
        index = find::position_in_slot(page, key, false);
        if (index < page->header.record_count) {
            break;
        }
        page_id = page->header.next_page;
        page->unlock_unique();
        page = buffer->get_page(page_id);
        page->lock_unique();
    }

    remove_record_in_page(page, index);

    if (page->header.record_count > 0) {

        if (write_lock_opt) {
            if (write_lock_opt->owns_lock()) {
                write_lock_opt.reset();
            }
        } else {
            page->unlock_unique();
        }

        return;
    }

    bool one_page_level = false;

    uint32_t prev_page_id = page->header.prev_page;
    uint32_t next_page_id = page->header.next_page;

    if (prev_page_id > 0) {
        std::shared_ptr<litedb::page::Page> prev_page = buffer->get_page(prev_page_id);
        prev_page->lock_unique();
        prev_page->read(prev_page_id);
        prev_page->set_dirty();
        prev_page->header.next_page = next_page_id;

        one_page_level = prev_page->header.prev_page == 0 && next_page_id == 0;
        if (one_page_level) {
            prev_page->header.p_parent = 0;
            root_page_id = prev_page_id;
        }

        prev_page->unlock_unique();
    }

    if (next_page_id > 0) {
        std::shared_ptr<litedb::page::Page> next_page = buffer->get_page(next_page_id);
        next_page->lock_unique();
        next_page->read(next_page_id);
        next_page->set_dirty();
        next_page->header.prev_page = prev_page_id;

        one_page_level = next_page->header.next_page == 0 && prev_page_id == 0;
        if (one_page_level) {
            next_page->header.p_parent = 0;
            root_page_id = next_page_id;
        }

        next_page->unlock_unique();
    }

    if (one_page_level) {
        auto root_manager = engine::root_manager_;

        while (parents.size() > 0) {
            root_manager->add_free_page(parents.back().page_id);
            parents.pop_back();
        }
    } else {
        remove_record(root_page_id, parents, page_id, std::nullopt);
    }

    if (write_lock_opt) {
        if (write_lock_opt->owns_lock()) {
            write_lock_opt.reset();
        }
    } else {
        page->unlock_unique();
    }
}

delete_responce find_and_remove_key_page(uint32_t page_id, std::string &key) {
    std::vector<d_node> parents;

    auto buffer = engine::buffer_manager_->get_main_buffer();
    uint32_t p_page_id = 0;

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

            uint16_t key_size;
            std::memcpy(&key_size, key_ptr, sizeof(uint16_t));
            std::string high_key(
                page->data_ + record_offset,
                page->data_ + record_offset + key_size
            );

            parents.push_back(d_node{
                .page_id = page_id,
                .key = std::move(high_key)
            });
            read_lock.unlock();

            page_id = child_page_id;
            continue;
        }

        uint16_t record_offset = slot_ptr[index];
        uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
            page->data_ + record_offset
        );
        uint8_t cmp = key::compare(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            key_ptr, true
        );
        if (cmp != 0) {
            return delete_responce{
                .new_root_id = parents[0].page_id,
                .count = 0
            };
        }

        {
            uint16_t key_size;
            std::memcpy(&key_size, key_ptr, sizeof(uint16_t));
            std::string high_key(
                page->data_ + record_offset,
                page->data_ + record_offset + key_size
            );

            parents.push_back(d_node{
                .page_id = page_id,
                .key = std::move(high_key)
            });
        }

        boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);

        uint32_t root_page_id = parents[0].page_id;
        // std::vector<std::string> remove_keys = { key };

        remove_record(root_page_id, parents, 0, std::move(write_lock));

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