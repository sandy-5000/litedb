#pragma once

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    std::array<litedb::page::Page, litedb::constants::BUFFER_MANAGER_SIZE> pages_;

public:
    BufferManager();
    ~BufferManager();

    litedb::page::Page* getEmptyPage(size_t page_id);
};


}