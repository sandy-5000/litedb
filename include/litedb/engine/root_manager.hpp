#pragma once

#include <deque>

#include "litedb/page/page.hpp"
#include "litedb/page/io.hpp"

namespace litedb::engine::root_manager {

struct root_data {
    uint32_t top_free_page = 0;
    uint32_t root_table_page = 0;
};


class RootManager {
private:
    litedb::page::Page root;
    std::deque<uint32_t> free_pages_;
    int32_t max_batch;

    mutable std::shared_mutex mtx_;

    void free_files_list_read();
    void free_files_list_write();

    root_data page_data;

public:
    RootManager();
    ~RootManager();
    void save_state();

    litedb::page::Page* get_root();
    uint32_t get_free_page();
    void add_free_page(uint32_t page_id);
};


}