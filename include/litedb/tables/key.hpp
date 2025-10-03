#include <cstdint>
#include <cassert>
#include <bsoncxx/types.hpp>

namespace litedb::table_key {


int8_t compare_keys(
    const uint8_t* page_a, size_t off_a,
    const uint8_t* page_b, size_t off_b
);


}