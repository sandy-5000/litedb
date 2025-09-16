#pragma once

#include "litedb/page/page.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    std::array<litedb::page::Page, litedb::constants::BUFFER_MANAGER_SIZE> pages_;

public:
    BufferManager();

    litedb::page::Page* getPage(size_t page_id);
};


}