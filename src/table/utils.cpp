#include "litedb/table/utils.hpp"

#include <bitset>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "litedb/table/data_types.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/page/page.hpp"
#include "litedb/constants.hpp"
#include "litedb/engine/store.hpp"

namespace litedb::table::utils {

void print_key(uint8_t* key) {
    uint16_t key_size;
    std::memcpy(&key_size, key, sizeof(uint16_t));
    printf("Key: [%d][%02x] - ", key_size, key[2]);
    uint8_t *ptr = key + 3 + compare::front_shift(key, key[2] & 0x7f);
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

struct info {
    uint32_t page_id;
    uint8_t status;
    uint64_t depth;
    uint64_t cnt;
};

void check_bplustree(
    uint32_t page_id,
    const std::string left_key, const std::string right_key,
    uint64_t depth,
    std::vector<info> &sts
) {
    std::cout << page_id << std::endl;
    auto buffer = engine::buffer_manager_->get_main_buffer();
    std::shared_ptr<litedb::page::Page> page = buffer->get_page(page_id);

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

    int8_t lft = left_key.size() == 0
        ? 0
        : (compare::keys(
            reinterpret_cast<const uint8_t*>(left_key.data()),
            reinterpret_cast<const uint8_t*>(keys.front().data()),
            false
          ) != 1);

    int8_t rit = right_key.size() == 0
        ? 0
        : (compare::keys(
            reinterpret_cast<const uint8_t*>(right_key.data()),
            reinterpret_cast<const uint8_t*>(keys.back().data()),
            false
          ) == 1);

    info info_;
    info_.page_id = page_id;
    info_.status = 0;
    info_.depth = depth;
    info_.cnt = page->header.record_count;

    info_.status |= lft << 7;
    info_.status |= rit << 6;

    uint8_t flag = 1;
    for (int i = 1; i < keys.size(); ++i) {
        if (compare::keys(
            reinterpret_cast<const uint8_t*>(keys[i - 1].data()),
            reinterpret_cast<const uint8_t*>(keys[i].data()),
            false
        ) == 1) {
            flag = 0;
            break;
        }
    }
    if (!flag) {
        info_.status |= flag << 5;
    }

    sts.push_back(info_);

    uint8_t page_type = page->header.type & 0xC0;

    for (int i = 0; i < keys.size(); ++i) {
        if (keys[i][2] == 0x02 && page_type == 0xC0) {
            uint32_t child_page;
            std::memcpy(&child_page, keys[i].data() + 3, sizeof(uint32_t));
            if (child_page) {
                check_bplustree(
                    child_page,
                    i == 0 ? left_key : keys[i - 1],
                    i == keys.size() - 1 ? keys[i + 1] : right_key,
                    depth + 1,
                    sts
                );
            }
        }
    }
}

void check_table(uint32_t root_page_id) {
    std::vector<info> sts;
    std::cout << "[CHECK_TABLE] page_id: " << root_page_id << std::endl;
    check_bplustree(root_page_id, "", "", 0, sts);
    std::sort(sts.begin(), sts.end(), [](const info& a, const info& b) {
        return a.depth < b.depth;
    });
    for (const auto& entry : sts) {
        printf(
            "Page: %d | Depth : %llu, Status: [%s], cnt: %llu\n",
            entry.page_id, entry.depth,
            std::bitset<8>(entry.status).to_string().c_str(),
            entry.cnt
        );
    }
}

}