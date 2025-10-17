#include <vector>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "litedb/table/remove.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/page/page.hpp"

namespace litedb::table {

uint16_t find_in_slot_d(std::shared_ptr<litedb::page::Page> page, std::string &key) {
    if (!page) {
        throw std::invalid_argument("[find_in_slot] PAGE: nullptr");
    }

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint8_t type = page->header.type & 0xC0;
    bool is_internal = (type == 0xC0);
    int8_t cmp_flag = 0 - is_internal;

    uint16_t record_count = page->header.record_count;
    uint16_t low = 0, high = record_count;

    while (low < high) {
        uint16_t mid = low + (high - low) / 2;

        uint16_t record_offset = slot_ptr[mid];
        int8_t cmp = compare::keys(
            reinterpret_cast<const uint8_t*>(key.c_str()),
            page->data_ + record_offset, false
        );

        if (cmp > cmp_flag) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    return low;
}

std::vector<std::string> merge_pages(
    std::shared_ptr<litedb::page::Page> cur_page,
    std::shared_ptr<litedb::page::Page> nxt_page
) {
    uint16_t cur_record_count = cur_page->header.record_count;
    uint16_t nxt_record_count = nxt_page->header.record_count;

    std::vector<std::string> keys(cur_record_count + cur_record_count);
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        cur_page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    uint16_t idx = 0;

    uint16_t* slot_ptr_1 = reinterpret_cast<uint16_t*>(
        cur_page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < cur_record_count; ++i) {
        uint16_t offset = slot_ptr_1[i];
        uint16_t key_size;
        std::memcpy(&key_size, cur_page->data_ + offset, sizeof(uint16_t));

        keys[idx++] = std::move(
            std::string(reinterpret_cast<char*>(cur_page->data_ + offset), key_size)
        );
    }

    uint16_t* slot_ptr_2 = reinterpret_cast<uint16_t*>(
        nxt_page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < nxt_record_count; ++i) {
        uint16_t offset = slot_ptr_2[i];
        uint16_t key_size;
        std::memcpy(&key_size, nxt_page->data_ + offset, sizeof(uint16_t));

        keys[idx++] = std::move(
            std::string(reinterpret_cast<char*>(nxt_page->data_ + offset), key_size)
        );
    }

    auto root_manager = engine::root_manager_;
    root_manager->add_free_page(nxt_page->header.id);

    uint16_t total_free_space = g::PAGE_BODY_SIZE;

    cur_page->header.free_space_offset = constants::DB_PAGE_SIZE;
    cur_page->header.record_count = 0;
    cur_page->header.free_space = total_free_space;
    cur_page->header.next_page = nxt_page->header.next_page;

    std::vector<std::string> merge_keys = { keys[cur_record_count] };

    for (uint16_t i = 0; i < keys.size(); ++i) {
        uint8_t* start_ptr = cur_page->data_ + cur_page->header.free_space_offset - keys[i].size();
        std::memcpy(start_ptr, keys[i].c_str(), keys[i].size());
        cur_page->header.free_space_offset -= keys[i].size();
        cur_page->header.free_space -= keys[i].size();

        uint16_t key_offset = cur_page->header.free_space_offset;
        std::memcpy(slot_ptr_1 + cur_page->header.record_count, &key_offset, sizeof(uint16_t));
        cur_page->header.record_count++;
        cur_page->header.free_space -= sizeof(uint16_t);
    }

    if ((merge_keys[0][2] & 0x80) == 0x80) {
        merge_keys[0].erase(merge_keys[0].size() - 9);
        merge_keys[0].back() = 0x00;
        uint16_t key_size = static_cast<uint16_t>(merge_keys[0].size());
        std::memcpy(merge_keys[0].data(), &key_size, sizeof(uint16_t));
    }

    return merge_keys;
}

void remove_keys_to_page(
    uint32_t &root_page_id,
    std::vector<std::string> &old_keys,
    std::vector<uint32_t> &parents,
    uint32_t child_id,
    bool lock_it
) {
    uint32_t page_id = parents.back();
    parents.pop_back();

    uint16_t total_free_space = g::PAGE_BODY_SIZE;

    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);
    if (lock_it) {
        page->lock_unique();
    }
    page->set_dirty();
    page->read(page_id);

    uint32_t new_free_space = page->header.free_space;
    for (auto &key : old_keys) {
        new_free_space += key.size() + sizeof(uint16_t);
    }

    uint16_t index = find_in_slot_d(page, old_keys[0]);

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );

    uint16_t size = old_keys.size();
    std::vector<std::string> pre_merge_keys(size);

    for (uint16_t i = 0; i < size; ++i) {
        uint8_t* offset = page->data_ + slot_ptr[index + i];
        uint16_t key_size;
        std::memcpy(&key_size, offset, sizeof(uint16_t));
        std::string key(offset, offset + key_size);
        if ((key[2] & 0x80) == 0x80) {
            key.erase(key.size() - 9);
            key.back() = 0x00;
            key_size = static_cast<uint16_t>(key.size());
            std::memcpy(key.data(), &key_size, sizeof(uint16_t));
        }
        pre_merge_keys[i] = std::move(key);
    }

    uint16_t shift_size = size * sizeof(uint16_t);
    uint16_t move_size = (page->header.record_count - index - size) * sizeof(uint16_t);
    uint8_t *move_offset = page->data_ + constants::PAGE_HEADER_SIZE + sizeof(uint16_t) * index;
    std::memmove(
        move_offset,
        move_offset + shift_size,
        move_size
    );

    page->header.free_space = new_free_space;
    page->header.record_count -= size;

    int32_t free_space_percent = new_free_space * 100 / total_free_space;

    if (free_space_percent < 65) {
        if (lock_it) {
            page->unlock_unique();
        }
        return;
    }

    uint32_t next_page_id = page->header.next_page;
    if (next_page_id == 0) {
        if (page->header.record_count == 0) {
            if (page->header.p_parent == 0) {
                root_page_id = child_id;
                auto root_manager = engine::root_manager_;
                root_manager->add_free_page(page_id);
            } else {
                remove_keys_to_page(root_page_id, pre_merge_keys, parents, page_id, true);
            }
        }
        if (lock_it) {
            page->unlock_unique();
        }
        return;
    }

    std::shared_ptr<litedb::page::Page> next_page = buffer->get_page(next_page_id);
    next_page->lock_unique();

    free_space_percent = (new_free_space + next_page->header.free_space) * 100 / total_free_space;

    if (free_space_percent < 45) {
        next_page->unlock_unique();
        if (lock_it) {
            page->unlock_unique();
        }
        return;
    }

    std::vector<std::string> merge_keys = merge_pages(page, next_page);
    remove_keys_to_page(root_page_id, merge_keys, parents, page_id, true);

    next_page->unlock_unique();
    if (lock_it) {
        page->unlock_unique();
    }
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

        uint16_t offset = find_in_slot_d(page, key);
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

    uint16_t index = find_in_slot_d(page, key);

    uint8_t* prev_key_ptr = reinterpret_cast<uint8_t*>(
        page->data_ + slot_ptr[index - 1]
    );
    uint8_t cmp = compare::keys(
        reinterpret_cast<const uint8_t*>(key.c_str()),
        prev_key_ptr, true
    );
    if (cmp != 0) {
        return delete_responce{
            .new_root_id = parents[0],
            .count = 0
        };
    }

    parents.push_back(page_id);

    uint32_t root_page_id = parents[0];
    std::vector<std::string> delete_keys = { key };

    remove_keys_to_page(root_page_id, delete_keys, parents, 0, false);

    delete_responce responce;
    responce.new_root_id = root_page_id;
    responce.count = 1;
    responce.data = key;

    return responce;
}

delete_responce remove::in_slot(uint32_t root_page, std::string &key) {
    std::vector<uint32_t> parents;
    return find_and_remove_key_page(root_page, key, parents);
}

}