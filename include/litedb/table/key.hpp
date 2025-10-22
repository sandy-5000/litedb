#pragma once

#include <cstdint>

namespace litedb::table {

struct key final {
    static uint16_t front_shift(const uint8_t* key, const uint8_t type);
    static int8_t compare(const uint8_t* key_a, const uint8_t* key_b, bool unique);
};

}
