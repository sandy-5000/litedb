#pragma once

#include "litedb/page/page.hpp"

namespace litedb::root_manager {


class RootManager {
private:
    litedb::page::Page root;
    std::deque<uint32_t> free_pages_;
    int32_t max_batch;

    mutable std::shared_mutex mtx_;

    void free_files_list_read();
    void free_files_list_write();

public:
    RootManager();
    ~RootManager();

    litedb::page::Page* getRoot();
    uint32_t getFreePage();
    void addFreePage(uint32_t page_id);
};


}