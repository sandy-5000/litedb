#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "litedb/table/insert.hpp"
#include "litedb/engine/buffer_manager.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/table/slot.hpp"
#include "litedb/table/key.hpp"
#include "litedb/table/compact.hpp"
#include "litedb/table/find.hpp"
#include "litedb/table/utils.hpp"
#include "litedb/page/page.hpp"

namespace litedb::table {

uint8_t is_insertable(
    std::shared_ptr<litedb::page::Page> page, uint16_t key_and_slot_size, uint16_t slot_size
) {
    uint32_t free_space_offset = page->header.free_space_offset;
    uint16_t slot_end = litedb::constants::PAGE_HEADER_SIZE
        + page->header.record_count * slot_size;
    if (free_space_offset - slot_end >= key_and_slot_size) {
        return 2;
    }
    if (page->header.free_space >= key_and_slot_size) {
        return 1;
    }
    return 0;
}

std::vector<std::string> split_key_page(
    std::shared_ptr<litedb::page::Page> cur_page,
    std::vector<std::string> &new_keys,
    uint16_t new_key_index
) {

    uint16_t record_count = cur_page->header.record_count;
    if (new_key_index > record_count) {
        std::cerr << "[SPLIT_PAGE] ERROR (index out of bound)" << std::endl;
        return {};
    }

    std::vector<std::string> keys(record_count + new_keys.size());
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        cur_page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );
    uint16_t idx = 0;

    for (uint16_t i = 0; i < record_count; ++i, ++idx) {
        if (i == new_key_index) {
            idx += new_keys.size();
        }

        uint16_t offset = slot_ptr[i];
        uint16_t key_size;
        std::memcpy(&key_size, cur_page->data_ + offset, sizeof(uint16_t));

        keys[idx] = std::move(
            std::string(reinterpret_cast<char*>(cur_page->data_ + offset), key_size)
        );
    }
    for (uint16_t i = 0; i < new_keys.size(); ++i) {
        keys[new_key_index + i] = new_keys[i];
    }

    auto root_manager = engine::root_manager_;
    auto buffer = engine::buffer_manager_->get_main_buffer();
    uint16_t total_free_space = g::PAGE_BODY_SIZE;

    uint8_t page_type = cur_page->header.type;
    uint32_t next_page = cur_page->header.next_page;

    std::shared_ptr<litedb::page::Page> page = cur_page;
    page->header.free_space_offset = constants::DB_PAGE_SIZE;
    page->header.record_count = 0;
    page->header.free_space = total_free_space;

    std::vector<std::string> parent_nodes;

    uint16_t keys_size = keys.size();
    for (uint16_t i = 0; i < keys_size; ++i) {
        uint32_t free_space = page->header.free_space;
        int32_t free_space_percent = (free_space - keys[i].size() - sizeof(uint16_t)) * 100 / total_free_space;

        if (page->header.record_count > 0 && free_space_percent < 60) {

            uint32_t new_page_id = root_manager->get_free_page();
            std::shared_ptr<litedb::page::Page> new_page = buffer->get_page(new_page_id);
            new_page->read_empty(new_page_id);

            new_page->set_dirty();
            new_page->header.id = new_page_id;
            new_page->header.free_space_offset = constants::DB_PAGE_SIZE;
            new_page->header.record_count = 0;
            new_page->header.free_space = total_free_space;
            new_page->header.type = page_type;
            new_page->header.p_parent = cur_page->header.p_parent;
            new_page->header.prev_page = page->header.id;

            page->header.next_page = new_page_id;
            uint32_t c_page_id = page->header.id;

            uint8_t key_type = keys[i - 1][2];
            if (key_type == 0x06) {
                std::string parent_key_node = keys[i - 1];
                std::memcpy(parent_key_node.data() + 3, &c_page_id, sizeof(uint32_t));

                parent_nodes.emplace_back(parent_key_node);
            } else {

                uint16_t cur_shift = key::front_shift(
                    reinterpret_cast<uint8_t *>(keys[i - 1].data()), key_type
                );

                bool has_seq = key_type & 0x80;
                uint16_t back_shift = has_seq ? 9 : 0;
                uint16_t new_key_size = keys[i - 1].size() + 4 - cur_shift - back_shift;
                std::string parent_key_node(new_key_size, 0);

                uint16_t body_len = parent_key_node.size() - 8;
                std::memcpy(parent_key_node.data() + 7, keys[i - 1].data() + 3 + cur_shift, body_len);
                parent_key_node[2] = 0x06;
                std::memcpy(parent_key_node.data() + 3, &c_page_id, sizeof(uint32_t));

                std::memcpy(parent_key_node.data(), &new_key_size, sizeof(uint16_t));
                parent_nodes.emplace_back(parent_key_node);
            }

            page = new_page;
        }

        uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
            page->data_ + litedb::constants::PAGE_HEADER_SIZE
        );

        uint8_t* start_ptr = page->data_ + page->header.free_space_offset - keys[i].size();
        std::memcpy(start_ptr, keys[i].c_str(), keys[i].size());
        page->header.free_space_offset -= keys[i].size();
        page->header.free_space -= keys[i].size();

        uint16_t key_offset = page->header.free_space_offset;
        std::memcpy(slot_ptr + page->header.record_count, &key_offset, sizeof(uint16_t));
        page->header.record_count++;
        page->header.free_space -= sizeof(uint16_t);
    }

    page->header.next_page = next_page;
    std::shared_ptr<litedb::page::Page> n_page = buffer->get_page(next_page);
    n_page->read(next_page);
    n_page->set_dirty();
    n_page->header.prev_page = page->header.id;

    {
        uint8_t key_type = keys.back()[2];
        if (key_type == 0x06) {
            std::string parent_key_node = keys.back();
            std::memcpy(parent_key_node.data() + 3, &(page->header.id), sizeof(uint32_t));

            parent_nodes.emplace_back(parent_key_node);
        } else {

            uint16_t cur_shift = key::front_shift(
                reinterpret_cast<uint8_t *>(keys.back().data()), key_type
            );

            bool has_seq = key_type & 0x80;
            uint16_t back_shift = has_seq ? 9 : 0;
            uint16_t new_key_size = keys.back().size() + 4 - cur_shift - back_shift;
            std::string parent_key_node(new_key_size, 0);

            uint16_t body_len = parent_key_node.size() - 8;
            std::memcpy(parent_key_node.data() + 7, keys.back().data() + 3 + cur_shift, body_len);
            parent_key_node[2] = 0x06;
            std::memcpy(parent_key_node.data() + 3, &(page->header.id), sizeof(uint32_t));

            std::memcpy(parent_key_node.data(), &new_key_size, sizeof(uint16_t));
            parent_nodes.emplace_back(parent_key_node);
        }
    }

    return parent_nodes;
}

void add_keys_to_page(
    uint32_t &root_page_id,
    std::vector<std::string> &new_keys,
    std::vector<key_page_change> &changes,
    std::vector<uint32_t> &parents,
    std::optional<boost::upgrade_to_unique_lock<boost::shared_mutex>> write_lock_opt = std::nullopt
) {
    uint32_t page_id;
    bool is_new_page = false;

    if (parents.size()) {

        page_id = parents.back();
        parents.pop_back();

    } else {

        auto root_manager = engine::root_manager_;
        page_id = root_manager->get_free_page();
        root_page_id = page_id;
        is_new_page = true;

    }

    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);

    if (!write_lock_opt) {
        page->lock_unique();
    }
    page->set_dirty();

    if (is_new_page) {
        page->read_empty(page_id);
        page->header.id = page_id;
        page->header.free_space_offset = constants::DB_PAGE_SIZE;
        page->header.record_count = 0;
        page->header.free_space = g::PAGE_BODY_SIZE;
        page->header.type = 0xC0;
        page->header.p_parent = 0;
        page->header.prev_page = 0;
        page->header.next_page = 0;
    } else {
        page->read(page_id);
    }

    uint16_t index = 0;
    while (!is_new_page) {
        index = find::position_in_slot(page, new_keys.back(), false);
        if (index < page->header.record_count) {
            break;
        }
        page_id = page->header.next_page;
        page->unlock_unique();
        page = buffer->get_page(page_id);
        page->lock_unique();
    }

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );

    if (!is_new_page && (page->header.type & 0xC0) == 0xC0) {
        uint16_t offset = slot_ptr[index];
        uint32_t child_page_id;
        std::memcpy(&child_page_id, new_keys.back().data() + 3, sizeof(uint32_t));
        std::memcpy(page->data_ + offset + 3, &child_page_id, sizeof(uint32_t));
        new_keys.pop_back();
    }

    uint16_t total_key_size = 0;
    for (auto key : new_keys) {
        uint16_t key_size;
        std::memcpy(&key_size, key.data(), sizeof(uint16_t));
        total_key_size += key_size + sizeof(uint16_t);
    }
    uint8_t fit_state = is_insertable(
        page,
        total_key_size,
        sizeof(uint16_t)
    );

    if (fit_state > 0) {
        if (fit_state == 1) {
            compact_page::key(page);
        }

        uint16_t idx = 0;
        uint16_t new_keys_size = new_keys.size();
        std::vector<uint16_t> start_offsets(new_keys_size);
        for (auto &key : new_keys) {
            uint16_t free_space_offset = page->header.free_space_offset;
            uint16_t start_offset = free_space_offset - key.size();
            std::memcpy(page->data_ + start_offset, key.data(), key.size());

            page->header.free_space_offset = start_offset;
            page->header.free_space -= key.size();

            start_offsets[idx++] = start_offset;
        }

        uint8_t* old_ptr = reinterpret_cast<uint8_t*>(
            page->data_ + constants::PAGE_HEADER_SIZE + sizeof(uint16_t) * index
        );
        uint16_t shift_size = new_keys_size * sizeof(uint16_t);
        uint8_t* new_ptr = old_ptr + shift_size;

        uint16_t date_size = (page->header.record_count - index) * sizeof(uint16_t);
        std::memmove(new_ptr, old_ptr, date_size);
        std::memcpy(old_ptr, start_offsets.data(), start_offsets.size() * sizeof(uint16_t));
        page->header.record_count += new_keys_size;
        page->header.free_space -= new_keys_size * sizeof(uint16_t);

        key_page_change change{
            .old_data = {
                .slot_info = std::make_pair(index, new_keys_size)
            },
            .page_id = page_id,
            .change_type = 0
        };
        changes.push_back(change);

        if (write_lock_opt) {
            if (write_lock_opt->owns_lock()) {
                write_lock_opt.reset();
            }
        } else {
            page->unlock_unique();
        }

    } else {
        uint8_t* copied_page_data = nullptr;

        if (!is_new_page) {
            copied_page_data = new uint8_t[litedb::constants::DB_PAGE_SIZE];
            std::memcpy(copied_page_data, page->data_, litedb::constants::DB_PAGE_SIZE);
        }

        key_page_change change{
            .old_data = {.prev_page = copied_page_data},
            .page_id = page_id,
            .change_type = 1
        };
        changes.push_back(change);

        std::vector<std::string> split_keys = split_key_page(
            page,
            new_keys,
            index
        );

        if (write_lock_opt) {
            if (write_lock_opt->owns_lock()) {
                write_lock_opt.reset();
            }
        } else {
            page->unlock_unique();
        }

        add_keys_to_page(
            root_page_id,
            split_keys,
            changes,
            parents,
            std::nullopt
        );

    }
}

uint32_t find_and_insert_key_page(
    uint32_t page_id,
    std::string &key,
    bool is_unique,
    std::vector<key_page_change> &changes
) {
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
            return 0;
            continue;
        }

        if (is_internal) {

            uint32_t child_page_id;

            uint16_t record_offset = slot_ptr[index];
            uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + record_offset
            );
            if (key_ptr[2] != 0x06) {
                return 0;
            }
            std::memcpy(&child_page_id, key_ptr + 3, sizeof(uint32_t));

            parents.push_back(page_id);
            read_lock.unlock();

            page_id = child_page_id;
            continue;
        }

        if (is_unique) {
            uint8_t* index_key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + slot_ptr[index]
            );
            uint8_t cmp = key::compare(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                index_key_ptr, true
            );
            if (cmp == 0) {
                return 0;
            }
        }

        boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(read_lock);

        parents.push_back(page_id);

        uint32_t root_page_id = parents[0];
        std::vector<std::string> split_keys = { key };

        add_keys_to_page(
            root_page_id,
            split_keys,
            changes,
            parents,
            std::move(write_lock)
        );

        return root_page_id;
    }

    return 0;
}


std::vector<key_page_change> insert::key (
    uint32_t root_page, std::string &key, bool is_unique
) {
    std::vector<key_page_change> changes;

    auto new_root_page = find_and_insert_key_page(
        root_page,
        key,
        is_unique,
        changes
    );

    if (new_root_page == 0) {
        return {};
    }

    if (new_root_page != root_page) {
        key_page_change change{
            .old_data = {.root_page_id = root_page},
            .page_id = new_root_page,
            .change_type = 2
        };
        changes.push_back(change);
    }

    return changes;
}

}