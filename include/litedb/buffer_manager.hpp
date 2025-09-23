#pragma once

#include "litedb/page/page.hpp"
#include "litedb/globals.hpp"

namespace litedb::buffer_manager {


class BufferManager {
private:
    litedb::page::Page root;

    std::array<litedb::page::Page, litedb::constants::BUFFER_MANAGER_SIZE> pages_;
    std::deque<uint32_t> free_pages_;
    int32_t max_batch;

    mutable std::shared_mutex mtx_;

    void free_files_list_read();
    void free_files_list_write();

public:
    BufferManager();
    ~BufferManager();

    litedb::page::Page* getRoot();
    litedb::page::Page* getEmptyPage(size_t page_id);

    uint32_t getFreePage();
    void addFreePage(uint32_t page_id);

};


}