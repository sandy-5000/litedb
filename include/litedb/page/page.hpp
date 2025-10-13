#pragma once

#include <cstdint>
#include <shared_mutex>

#include "litedb/constants.hpp"
#include "litedb/page/header.hpp"

namespace litedb::page {

class Page {
private:
    mutable std::shared_mutex mtx_;
    bool dirty_ = false;

public:
    page_header header;
    uint8_t data_[litedb::constants::PAGE_SIZE];

    Page();
    ~Page();

    inline void lock_shared()   const { /*mtx_.lock_shared();*/ }
    inline void unlock_shared() const { /*mtx_.unlock_shared();*/ }
    inline void lock_unique()   const { /*mtx_.lock();*/ }
    inline void unlock_unique() const { /*mtx_.unlock();*/ }

    void read_empty(uint32_t page_id);
    ssize_t read(uint32_t page_id);
    ssize_t force_read(uint32_t page_id);
    ssize_t write();

    void set_dirty();
    bool is_dirty() const;
    void clear();

    void print_header() const;
};

}