#pragma once

#include <cstdint>

namespace litedb::table {

inline static constexpr uint8_t TYPE_f64 = 0x01;
inline static constexpr uint8_t TYPE_str = 0x02;
inline static constexpr uint8_t TYPE_u8  = 0x08;
inline static constexpr uint8_t TYPE_i32 = 0x10;
inline static constexpr uint8_t TYPE_i64 = 0x12;

}
