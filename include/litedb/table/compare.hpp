#pragma once

#include <cstdint>

namespace litedb::table {

struct compare final {
    static uint16_t front_shift(uint8_t* key, uint8_t type);
    static int8_t keys(uint8_t* key_a, uint8_t* key_b, bool unique);
};

}
