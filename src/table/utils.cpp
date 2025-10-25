#include <bitset>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "litedb/table/utils.hpp"
#include "litedb/table/data_types.hpp"
#include "litedb/table/key.hpp"
#include "litedb/page/page.hpp"
#include "litedb/constants.hpp"
#include "litedb/engine/store.hpp"

namespace litedb::table::utils {

void print_key(uint8_t* key) {
    uint16_t key_size;
    std::memcpy(&key_size, key, sizeof(uint16_t));
    printf("Key: [%d][%02x] - ", key_size, key[2]);
    uint16_t shift_size = key::front_shift(key, key[2] & 0x7f);
    uint8_t *ptr = key + 3 + shift_size;
    if (key[2] == 0x06) {
        uint32_t val;
        std::memcpy(&val, key + 3, shift_size);
        std::cout << "[page]";
        std::cout << "[" << val << "] - ";
    }
    uint8_t *ed = key + key_size - 1;
    while (ptr < ed) {
        uint8_t type = *ptr;
        ++ptr;
        switch (type) {
            case TYPE_f64: {
                double a;
                std::memcpy(&a, ptr, sizeof(double));
                ptr += sizeof(double);
                std::cout << "[f64]";
                std::cout << "[" << a << "]";
                break;
            }
            case TYPE_str: {
                std::cout << "[str]";
                printf("[");
                while (*ptr != '\0') {
                    printf("%c", *ptr);
                    ++ptr;
                }
                printf("]");
                ++ptr;
                break;
            }
            case TYPE_u8: {
                std::cout << "[u8]";
                if (*ptr) {
                    std::cout << "[true]";
                } else {
                    std::cout << "[false]";
                }
                ++ptr;
                break;
            }
            case TYPE_i32: {
                int32_t a;
                std::memcpy(&a, ptr, sizeof(int32_t));
                ptr += sizeof(int32_t);
                std::cout << "[i32]";
                std::cout << "[" << a << "]";
                break;
            }
            case TYPE_i64: {
                int64_t a;
                std::memcpy(&a, ptr, sizeof(int64_t));
                ptr += sizeof(int64_t);
                std::cout << "[i64]";
                std::cout << "[" << a << "]";
                break;
            }
            case TYPE_max: {
                std::cout << "[inf]";
                break;
            }
            default:
                break;
        }
    }
    std::cout << std::endl;
}

void print_slot_page(std::shared_ptr<litedb::page::Page> page) {
    uint16_t record_count = page->header.record_count;
    std::vector<std::string> keys(record_count);
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < record_count; ++i) {
        uint16_t offset = slot_ptr[i];
        uint16_t key_size;
        std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
        keys[i] = std::move(
            std::string(reinterpret_cast<char*>(page->data_ + offset), key_size)
        );
    }
    for (auto k : keys) {
        print_key(reinterpret_cast<uint8_t *>(k.data()));
    }
}

bool check_slot_page(std::shared_ptr<litedb::page::Page> page) {
    uint16_t record_count = page->header.record_count;
    std::vector<std::string> keys(record_count);
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );
    bool flag = false;
    for (uint16_t i = 0; i < record_count; ++i) {
        uint16_t offset = slot_ptr[i];
        uint16_t key_size;
        std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
        if (key_size > 500) {
            std::cout << "Page Corrupted: " << page->header.id << " Index: " << i << std::endl;
            flag = true;
        }
    }
    return flag;
}

void print_slot_sizes(std::shared_ptr<litedb::page::Page> page) {
    uint16_t record_count = page->header.record_count;
    std::vector<std::string> keys(record_count);
    uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
        page->data_ + constants::PAGE_HEADER_SIZE
    );
    for (uint16_t i = 0; i < record_count; ++i) {
        uint16_t offset = slot_ptr[i];
        uint16_t key_size;
        std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
        std::cout << "size: [" << key_size << "], ";
    }
}

std::pair<uint64_t, uint64_t> count_link(uint32_t page_id, uint32_t page_count, bool rev) {
    auto buffer = litedb::engine::buffer_manager_->get_main_buffer();
    uint64_t count = 0, c_count = 0, size = 0;
    uint32_t first_child = 0, last_child = 0;
    while (page_id > 0) {
        if (page_id > page_count) {
            std::cout << "error can't access page: " << page_id << "\n";
            break;
        }
        std::cout << "\r[page_id]: " << page_id << " " << std::flush;
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);
        page->read(page_id);
        uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
            page->data_ + constants::PAGE_HEADER_SIZE
        );
        uint8_t type = page->header.type & 0xC0;
        if (type == 0xC0) {
            std::memcpy(&last_child, page->data_ + slot_ptr[page->header.record_count - 1] + 3, sizeof(uint32_t));
            std::memcpy(&first_child, page->data_ + slot_ptr[0] + 3, sizeof(uint32_t));
        }

        {
            bool flag = false;
            uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
                page->data_ + constants::PAGE_HEADER_SIZE
            );
            for (uint16_t i = 0; i < page->header.record_count; ++i) {
                uint16_t offset = slot_ptr[i];
                uint16_t key_size;
                std::memcpy(&key_size, page->data_ + offset, sizeof(uint16_t));
                if (key_size > 500) {
                    flag = true;
                    break;
                }
            }
            flag && ++c_count;
        }
        ++size;
        count += page->header.record_count;
        if (rev) {
            page_id = page->header.prev_page;
            std::cout << " | [first_child]: " << first_child << ", [corrupted]: " << c_count;
        } else {
            page_id = page->header.next_page;
            std::cout << " | [last_child]: " << last_child << ", [corrupted]: " << c_count;
        }
    }
    return {count, size};
}

void check_tree_links(uint32_t root_page_id, uint32_t page_count, bool rev) {
    uint32_t page_id = root_page_id;
    int depth = 0;
    while (page_id) {
        auto buffer = litedb::engine::buffer_manager_->get_main_buffer();
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);
        page->read(page_id);
        auto [count, size] = count_link(page_id, page_count, rev);
        std::cout << " | Depth: " << depth << " count: " << count << " size: " << size << std::endl;
        ++depth;
        if ((page->header.type & 0xC0) == 0x80) {
            page_id = 0;
        } else {
            uint16_t* slot_ptr = reinterpret_cast<uint16_t*>(
                page->data_ + constants::PAGE_HEADER_SIZE
            );
            uint16_t offset = rev ? slot_ptr[page->header.record_count - 1] : slot_ptr[0];
            std::memcpy(&page_id, page->data_ + offset + 3, sizeof(uint32_t));
        }
    }
}

}
