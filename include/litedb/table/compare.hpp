#pragma once

#include <cstdint>

namespace litedb::table {

void print_key(const uint8_t* key_a);

struct compare final {
    static uint16_t front_shift(const uint8_t* key, const uint8_t type);
    static int8_t keys(const uint8_t* key_a, const uint8_t* key_b, bool unique);
};

}
