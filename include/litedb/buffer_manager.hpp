#pragma once

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    litedb::page::Page root;
    litedb::page::Page* top;
    std::array<litedb::page::Page, litedb::constants::BUFFER_MANAGER_SIZE> pages_;
    mutable std::shared_mutex mtx_;

public:
    BufferManager();

    litedb::page::Page* getRoot();
    litedb::page::Page* getPage(size_t page_id);
    litedb::page::Page* getEmptyPage(size_t page_id);

    uint32_t getFreePage();
    void addFreePage(uint32_t page_id);
};


}