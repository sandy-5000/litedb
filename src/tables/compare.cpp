#include "litedb/tables/compare.hpp"

namespace litedb::table::utils {


int8_t compare::key(
    const uint8_t* key_a, const uint16_t shift_a, const uint16_t back_a,
    const uint8_t* key_b, const uint16_t shift_b, const uint16_t back_b
) {
    uint16_t len_a = key_a[0] | (key_a[1] << 8);
    uint16_t len_b = key_b[0] | (key_b[1] << 8);

    const uint8_t* p_a = key_a + 2 + shift_a;
    const uint8_t* p_b = key_b + 2 + shift_b;
    const uint8_t* end_a = key_a + len_a - back_a;
    const uint8_t* end_b = key_b + len_b - back_b;
    uint8_t type;

    while (p_a < end_a && p_b < end_b) {
        if (*p_a != *p_b) {
            return *p_a < *p_b ? -1 : 1;
        }
        type = *p_a;
        ++p_a, ++p_b;
        switch (type) {
            case 0x01: {
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
            case 0x02: {
                while (p_a < end_a && p_b < end_b && (*p_a != 0 || *p_b != 0)) {
                    if (*p_a != *p_b) {
                        return *p_a < *p_b ? -1 : 1;
                    }
                    ++p_a, ++p_b;
                }
                if (p_a < end_a && p_b < end_b) {
                    ++p_a, ++p_b;
                }
                break;
            }
            case 0x08: {
                uint8_t a = *p_a;
                uint8_t b = *p_b;
                if (a != b) {
                    return a < b ? -1 : 1;
                }
                p_a += sizeof(uint8_t);
                p_b += sizeof(uint8_t);
                break;
            }
            case 0x10: {
                int32_t a, b;
                if (p_a + sizeof(int32_t) > end_a || p_b + sizeof(int32_t) > end_b) {
                    return -2;
                }
                std::memcpy(&a, p_a, sizeof(int32_t));
                std::memcpy(&b, p_b, sizeof(int32_t));
                if (a != b) {
                    return a < b ? -1 : 1;
                }
                p_a += sizeof(int32_t);
                p_b += sizeof(int32_t);
                break;
            }
            case 0x12: {
                int64_t a, b;
                if (p_a + sizeof(int64_t) > end_a || p_b + sizeof(int64_t) > end_b) {
                    return -2;
                }
                std::memcpy(&a, p_a, sizeof(int64_t));
                std::memcpy(&b, p_b, sizeof(int64_t));
                if (a != b) {
                    return a < b ? -1 : 1;
                }
                p_a += sizeof(int64_t);
                p_b += sizeof(int64_t);
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