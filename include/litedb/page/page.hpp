#pragma once

#include <cstdint>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "litedb/constants.hpp"
#include "litedb/page/header.hpp"

namespace litedb::page {

class Page {
private:
    mutable boost::shared_mutex mtx_;
    bool dirty_ = false;

public:
    page_header header;
    uint8_t data_[constants::DB_PAGE_SIZE];

    Page();
    ~Page();

    // inline void lock_shared()   const { }
    // inline void unlock_shared() const { }
    // inline void lock_unique()   const { }
    // inline void unlock_unique() const { }

    inline void lock_shared()   const { mtx_.lock_shared(); }
    inline void unlock_shared() const { mtx_.unlock_shared(); }
    inline void lock_unique()   const { mtx_.lock(); }
    inline void unlock_unique() const { mtx_.unlock(); }

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