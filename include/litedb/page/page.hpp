#pragma once
#include <cstdint>
#include <shared_mutex>
#include <vector>
#include <mutex>
#include <cstring>

#include "litedb/constants.hpp"
#include "litedb/page/page_header.hpp"
#include "litedb/page/page_io.hpp"

namespace litedb::page {


class Page : public PageHeader {
private:
    uint32_t id_;
    mutable std::shared_mutex mtx_;
    bool dirty_ = false;

public:
    uint8_t data_[litedb::constants::PAGE_SIZE];
    Page();
    ~Page();

    inline void lock_shared()   const { mtx_.lock_shared(); }
    inline void unlock_shared() const { mtx_.unlock_shared(); }
    inline void lock_unique()   const { mtx_.lock(); }
    inline void unlock_unique() const { mtx_.unlock(); }

    uint32_t id() const;
    void read_empty(uint32_t page_id);
    ssize_t read(uint32_t page_id);
    ssize_t force_read(uint32_t page_id);
    ssize_t write();

    void set_dirty();
    bool is_dirty();
    bool is_empty();
    void clear();
};


}