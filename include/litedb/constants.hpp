#pragma once

#include <cstddef>

namespace litedb::constants {

inline constexpr std::uint32_t DB_PAGE_SIZE           = 4096;
inline constexpr std::uint32_t PAGE_HEADER_SIZE    =   64;
inline constexpr std::uint32_t BUFFER_MANAGER_SIZE = 1331;
inline constexpr std::uint32_t INVALID_PAGE_ID     = 0xFFFFFFFF;

}
