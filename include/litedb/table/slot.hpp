#pragma once

#include <cstdint>

namespace litedb::table {

#pragma pack(push, 1)
struct slot_data_internal{
    uint64_t key;
    uint32_t page_id;
};
struct slot_data_leaf{
    uint64_t key;
    uint16_t offset;
};
#pragma pack(pop)

}