#include "litedb/table/utils.hpp"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "litedb/table/data_types.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/page/page.hpp"
#include "litedb/constants.hpp"

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

}