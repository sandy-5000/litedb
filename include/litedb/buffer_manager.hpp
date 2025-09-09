#pragma once

#include "litedb/page.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    std::array<litedb::page::Page, litedb::constants::BUFFER_MANAGER_SIZE> pages_;

public:
    BufferManager(std::size_t num_pages);
};


}