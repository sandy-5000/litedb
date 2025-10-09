#pragma once

#include <cstdint>
#include <cassert>
#include <bsoncxx/types.hpp>

namespace litedb::table::utils {


struct compare final {
    static int8_t key(
        const uint8_t* key_a, const uint16_t shift_a, const uint16_t back_a,
        const uint8_t* key_b, const uint16_t shift_b, const uint16_t back_b
    );
};


}