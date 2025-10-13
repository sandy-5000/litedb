#include "litedb/table/insert.hpp"

#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "litedb/engine/buffer_manager.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/table/slot.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/table/compact.hpp"
#include "litedb/table/find.hpp"

namespace litedb::table {

uint8_t page_insert::is_insertable(
    std::shared_ptr<litedb::page::Page> page, uint16_t key_and_slot_size, uint16_t slot_size
) {
    uint16_t free_space_offset = page->header.free_space_offset;
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

std::vector<std::string> page_insert::split_key_page(
    std::shared_ptr<litedb::page::Page> cur_page,
    std::vector<std::string> &new_keys,
    uint16_t new_key_index
) {

    printf("key types -- \n");
    for (auto i : new_keys) {
        uint8_t tp = i[2];
        printf("%02X, ", tp);
    }
    printf("------\n");

    uint16_t record_count = cur_page->header.record_count;
    if (new_key_index > record_count) {
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

    printf("key types --| \n");
    for (auto i : keys) {
        uint8_t tp = i[2];
        printf("%02X, ", tp);
    }
    printf("------\n");

    auto root_manager = engine::root_manager_;
    auto buffer = engine::buffer_manager_->get_main_buffer();
    uint16_t total_free_space = g::PAGE_BODY_SIZE;

    uint8_t page_type = cur_page->header.type;

    std::shared_ptr<litedb::page::Page> page = cur_page;
    page->header.free_space_offset = constants::PAGE_SIZE;
    page->header.record_count = 0;
    page->header.free_space = total_free_space;

    std::vector<std::string> parent_nodes;

    std::cout << "Came to Split 2" << std::endl;

    for (uint16_t i = 0; i < keys.size(); ++i) {
        uint16_t free_space = page->header.free_space;

        if (
            page->header.record_count > 0 &&
            ((free_space - keys[i].size() - sizeof(uint16_t)) * 1.0 / total_free_space) > 0.45
        ) {
            uint32_t new_page_id = root_manager->get_free_page();
            std::shared_ptr<litedb::page::Page> new_page = buffer->get_page(new_page_id);

            new_page->set_dirty();
            new_page->header.id = new_page_id;
            new_page->header.free_space_offset = constants::PAGE_SIZE;
            new_page->header.record_count = 0;
            new_page->header.free_space = total_free_space;
            new_page->header.type = page_type;
            new_page->header.p_parent = cur_page->header.p_parent;

            page->header.next_page = new_page_id;
            page = new_page;
            std::cout << "new_page" << std::endl;

            uint8_t key_type = keys[i][2];
            if (key_type == 0x02) {
                std::cout << "if" << std::endl;
                std::string parent_key_node = keys[i];
                std::memcpy(parent_key_node.data() + 3, &new_page_id, sizeof(uint32_t));

                parent_nodes.emplace_back(parent_key_node);
            } else {
                std::cout << "else" << std::endl;

                uint16_t cur_shift = compare::front_shift(
                    reinterpret_cast<uint8_t *>(keys[i].data()), key_type
                );

                bool has_seq = key_type & 0x80;
                uint16_t back_shift = has_seq ? 9 : 0;
                uint16_t new_key_size = keys[i].size() + 4 - cur_shift - back_shift;
                std::string parent_key_node(new_key_size, 0);

                uint16_t body_len = parent_key_node.size() - 7 - (has_seq ? 10 : 0);
                printf("type: %02X\n", key_type);
                std::cout << "shift: " <<  " - " << cur_shift << " " << back_shift << " " << new_key_size << " " << body_len << std::endl;
                std::cout << "cpy: 1" << std::endl;
                std::memcpy(parent_key_node.data() + 7, keys[i].data() + 3 + cur_shift, body_len);
                std::cout << "cpy: 2" << std::endl;
                parent_key_node[2] = 0x02;
                std::cout << "cpy: 3" << std::endl;
                std::memcpy(parent_key_node.data() + 3, &new_page_id, sizeof(uint32_t));
                std::cout << "cpy: 4" << std::endl;

                std::memcpy(parent_key_node.data(), &new_key_size, sizeof(uint16_t));
                std::cout << "cpy: 5" << std::endl;
            }

        }
        std::cout << "Came to Split 3" << std::endl;

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

    std::cout << "Came to Split 4" << std::endl;

    return parent_nodes;
}

void page_insert::add_keys_to_page(
    uint32_t &root_page_id,
    std::vector<std::string> &new_keys,
    std::vector<key_page_change> &changes,
    std::vector<uint32_t> &parents
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
    page->lock_unique();
    page->set_dirty();

    if (is_new_page) {
        page->read_empty(page_id);
        page->header.id = page_id;
        page->header.free_space_offset = constants::PAGE_SIZE;
        page->header.record_count = 0;
        page->header.free_space = g::PAGE_BODY_SIZE;
        page->header.type = 0xC0;
        page->header.p_parent = 0;
    } else {
        page->read(page_id);
    }

    uint16_t index = find::in_slot(page, new_keys[0]);

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );

    uint16_t total_key_size = 0;
    for (auto key : new_keys) {
        uint16_t key_size;
        std::memcpy(&key_size, key.data(), sizeof(uint16_t));
        total_key_size += key_size + sizeof(uint16_t);
    }
    uint8_t fit_state = page_insert::is_insertable(
        page,
        total_key_size,
        sizeof(uint16_t)
    );
    std::cout << "FitState: " << (int32_t)fit_state << std::endl;

    if (fit_state > 0) {
        if (fit_state == 1) {
            compact_page::key(page);
        }

        uint16_t idx = 0;
        std::vector<uint16_t> start_offsets(new_keys.size());
        for (auto &key : new_keys) {
            uint16_t free_space_offset = page->header.free_space_offset;
            uint16_t start_offset = free_space_offset - key.size();
            std::memcpy(page->data_ + start_offset, key.data(), key.size());

            page->header.free_space_offset = start_offset;
            page->header.free_space -= key.size();

            ++idx;
        }

        uint8_t* old_ptr = reinterpret_cast<uint8_t*>(
            page->data_ + slot_ptr[index]
        );
        uint8_t* new_ptr = old_ptr + start_offsets.size() * sizeof(uint16_t);
        uint16_t shift_size = (page->header.record_count - index) * sizeof(uint16_t);

        std::memmove(new_ptr, old_ptr, shift_size);
        std::memcpy(old_ptr, start_offsets.data(), start_offsets.size() * sizeof(uint16_t));
        page->header.record_count += new_keys.size();
        page->header.free_space -= new_keys.size() * sizeof(uint16_t);

        key_page_change change{
            .old_data = {.slot_index = index},
            .page_id = page_id,
            .change_type = 0
        };
        changes.push_back(change);

    } else {
        uint8_t* copied_page_data = nullptr;

        if (!is_new_page) {
            copied_page_data = new uint8_t[litedb::constants::PAGE_SIZE];
            std::memcpy(copied_page_data, page->data_, litedb::constants::PAGE_SIZE);
        }

        key_page_change change{
            .old_data = {.prev_page = copied_page_data},
            .page_id = page_id,
            .change_type = 1
        };
        changes.push_back(change);

        std::cout << "Came to Split" << std::endl;

        std::vector<std::string> split_keys = page_insert::split_key_page(
            page,
            new_keys,
            index
        );

        add_keys_to_page(
            root_page_id,
            split_keys,
            changes,
            parents
        );

    }

    page->unlock_unique();
}

uint32_t page_insert::find_key_page(
    uint32_t page_id,
    std::string &key,
    bool is_unique,
    std::vector<key_page_change> &changes,
    std::vector<uint32_t> &parents
) {

    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);
    page->lock_shared();
    page->read(page_id);

    uint8_t type = page->header.type & 0xC0;
    bool is_internal = (type == 0xC0);
    if (!is_internal) {
        page->unlock_shared();
        page->lock_unique();
    }

    page->read(page_id);

    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + litedb::constants::PAGE_HEADER_SIZE
    );

    if (is_internal) {

        uint16_t offset = find::in_slot(page, key);
        uint32_t child_page_id;
        --offset;

        if (offset < 0) {
            child_page_id = page->header.leftmost_child;
        } else {
            uint16_t record_offset = slot_ptr[offset];
            uint8_t* key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + record_offset
            );
            if (key_ptr[2] != 0x02) {
                return 0;
            }
            std::memcpy(&child_page_id, key_ptr + 3, sizeof(uint32_t));
        }

        parents.push_back(page_id);
        page->unlock_shared();

        return find_key_page(child_page_id, key, is_unique, changes, parents);

    } else {

        uint16_t index = find::in_slot(page, key);

        if (is_unique && index > 0) {
            uint8_t* prev_key_ptr = reinterpret_cast<uint8_t*>(
                page->data_ + slot_ptr[index - 1]
            );
            uint8_t cmp = compare::keys(
                reinterpret_cast<const uint8_t*>(key.c_str()),
                prev_key_ptr, true
            );
            if (cmp == 0) {
                page->unlock_unique();
                return 0; // return 0 to indicate it is a duplicate key
            }
        }

        std::cout << "PageId: " << page_id << ", Index: " << index << std::endl;
        std::cout << "FreeSpace: " << page->header.free_space << std::endl;

        parents.push_back(page_id);

        uint32_t root_page_id = parents[0];
        std::vector<std::string> split_keys = { key };

        add_keys_to_page(
            root_page_id,
            split_keys,
            changes,
            parents
        );

        return root_page_id;
    }
}


std::vector<key_page_change> insert::key (
    std::string table_name, uint32_t root_page, std::string &key, bool is_unique
) {
    std::vector<key_page_change> changes;
    std::vector<uint32_t> parents;

    auto new_root_page = page_insert::find_key_page(
        root_page,
        key,
        is_unique,
        changes,
        parents
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