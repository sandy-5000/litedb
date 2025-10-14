#include "litedb/table/compare.hpp"

#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include "litedb/table/data_types.hpp"

namespace litedb::table {

uint16_t compare::front_shift(const uint8_t* key, const uint8_t type) {
    switch (type) {
        case 0x00:
        case 0x05: {
            uint16_t size;
            std::memcpy(&size, key + 3, sizeof(uint16_t));
            return size;
        }

        case 0x01:
            return 2;

        case 0x02:
        case 0x06:
            return 4;

        case 0x03:
            return 8;

        case 0x04:
        default:
            return 0;
    }
    return 0;
}


int8_t compare::keys(const uint8_t* key_a, const uint8_t* key_b, bool unique) {

    uint16_t len_a;
    std::memcpy(&len_a, key_a, sizeof(uint16_t));
    uint8_t k_type_a = key_a[2];

    uint16_t len_b;
    std::memcpy(&len_b, key_b, sizeof(uint16_t));
    uint8_t k_type_b = key_b[2];

    const uint8_t* p_a = key_a + 3 + front_shift(key_a, k_type_a & 0x7f);
    const uint8_t* p_b = key_b + 3 + front_shift(key_b, k_type_b & 0x7f);
    const uint8_t* end_a = key_a + len_a - (unique && (k_type_a & 0x80) ? 10 : 1);
    const uint8_t* end_b = key_b + len_b - (unique && (k_type_b & 0x80) ? 10 : 1);

    uint8_t type, type_b;

    while (p_a < end_a && p_b < end_b) {

        if (
            (*p_a == TYPE_i32 || *p_a == TYPE_i64) &&
            (*p_b == TYPE_i32 || *p_b == TYPE_i64)
        ) {
            // pass
        } else if (*p_a != *p_b) {
            return *p_a < *p_b ? -1 : 1;
        }
        type = *p_a;
        type_b = *p_b;
        ++p_a, ++p_b;

        switch (type) {
            case TYPE_f64: {
                double a, b;
                if (p_a + sizeof(double) > end_a || p_b + sizeof(double) > end_b) {
                    return -2;
                }
                std::memcpy(&a, p_a, sizeof(double));
                std::memcpy(&b, p_b, sizeof(double));
                if (a != b) {
                    return a < b ? -1 : 1;
                }
                p_a += sizeof(double);
                p_b += sizeof(double);
                break;
            }

            case TYPE_str: {
                while (*p_a != 0 || *p_b != 0) {
                    if (*p_a != *p_b) {
                        return *p_a < *p_b ? -1 : 1;
                    }
                    ++p_a, ++p_b;
                }
                ++p_a, ++p_b;
                break;
            }

            case TYPE_u8: {
                uint8_t a = *p_a;
                uint8_t b = *p_b;
                if (a != b) {
                    return a < b ? -1 : 1;
                }
                p_a += sizeof(uint8_t);
                p_b += sizeof(uint8_t);
                break;
            }

            case TYPE_i32:
            case TYPE_i64: {
                int32_t val;
                int64_t a, b;

                switch (type) {
                    case TYPE_i32:
                        std::memcpy(&val, p_a, sizeof(int32_t));
                        a = static_cast<int64_t>(val);
                        p_a += sizeof(uint32_t);
                        break;

                    case TYPE_i64:
                        std::memcpy(&a, p_a, sizeof(int64_t));
                        p_a += sizeof(uint64_t);
                        break;
                }

                switch (type_b) {
                    case TYPE_i32:
                        std::memcpy(&val, p_b, sizeof(int32_t));
                        b = static_cast<int64_t>(val);
                        p_b += sizeof(uint32_t);
                        break;

                    case TYPE_i64:
                        std::memcpy(&b, p_b, sizeof(int64_t));
                        p_b += sizeof(uint64_t);
                        break;
                }

                if (a != b) {
                    return a < b ? -1 : 1;
                }
                break;
            }

            default: {
                return 2;
            }
        }
    }

    if (p_a >= end_a && p_b >= end_b) {
        return 0;
    }
    return p_a == end_a ? -1 : 1;
}

}