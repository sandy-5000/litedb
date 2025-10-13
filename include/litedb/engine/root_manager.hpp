#pragma once

#include <deque>

#include "litedb/page/page.hpp"
#include "litedb/page/io.hpp"

namespace litedb::engine::root_manager {

struct root_data {
    uint32_t top_free_page = 0;
    uint32_t root_table_page = 0;
    uint64_t seq_number = 0;
};


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

    root_data page_data;

    inline void lock_shared()   const { mtx_.lock_shared(); }
    inline void unlock_shared() const { mtx_.unlock_shared(); }
    inline void lock_unique()   const { mtx_.lock(); }
    inline void unlock_unique() const { mtx_.unlock(); }

    void save_state();

    litedb::page::Page* get_root();
    uint32_t get_free_page();
    void add_free_page(uint32_t page_id);
};


}