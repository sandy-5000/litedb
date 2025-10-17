#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace litedb::table {

struct delete_responce {
    uint32_t new_root_id;
    uint64_t count;
    std::string data;
};

struct remove final {
    static delete_responce in_slot(uint32_t page_id, std::string &key);
};

}
